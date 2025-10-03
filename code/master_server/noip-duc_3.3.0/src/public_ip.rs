use std::cell::Cell;
use std::net::{AddrParseError, IpAddr};
use std::{thread, time::Duration};

use log::{debug, info, warn};
use url::Url;

use crate::dns_method::{self, DnsMethod};
use crate::{Notification, Observer};

const IP_URL: &str = "http://ip1.dynupdate.no-ip.com";
const IP_URL_8245: &str = "http://ip1.dynupdate.no-ip.com:8245";
const IP_URL_AWS: &str = "http://169.254.169.254/latest/meta-data/public-ipv4";

#[derive(Debug, thiserror::Error)]
pub enum Error {
    #[error("DNS method failed; {0}")]
    DnsMethod(#[from] dns_method::Error),

    #[error(transparent)]
    ParseError(#[from] ParseError),

    #[error(transparent)]
    HttpError(#[from] minreq::Error),

    #[error("Failed to get IP request body from {0}; {1}")]
    ResponseBodyNotStr(String, String),

    #[error("Failed to parse IP from {0}; err={1}, body={2}")]
    ResponseParseIp(String, AddrParseError, String),

    #[cfg(test)]
    #[error("Fail expected")]
    ExpectedFail,
}

#[derive(Debug, thiserror::Error)]
pub enum ParseError {
    #[error("Failed to parse IP URL; {0}")]
    UrlParse(#[from] url::ParseError),

    #[error("Failed to parse IP; {0}")]
    AddrParse(#[from] AddrParseError),

    #[error("Failed to parse IP; {0}")]
    DnsMethodError(#[from] dns_method::Error),

    #[error("Unknown IP method '{0}'")]
    UnknownMethod(String),
}

// TODO: Consider Box<DnsMethod> when making a list of IpMethod and getting rid of this clippy
// suppression
#[allow(clippy::large_enum_variant)]
#[derive(Debug)]
pub enum IpMethod {
    Dns(DnsMethod),
    Http(String),
    Static(IpAddr),

    #[cfg(test)]
    Fail(Cell<bool>),
}

#[derive(Debug)]
pub struct IpMethods {
    methods: Vec<(IpMethod, Cell<bool>)>,
}

impl Default for IpMethods {
    fn default() -> Self {
        [
            IpMethod::Http(IP_URL.to_owned()),
            IpMethod::Http(IP_URL_8245.to_owned()),
        ]
        .into_iter()
        .collect()
    }
}

impl std::str::FromStr for IpMethod {
    type Err = ParseError;

    fn from_str(method: &str) -> Result<Self, Self::Err> {
        match method {
            "aws-metadata" => Ok(Self::Http(IP_URL_AWS.to_owned())),
            "dns" => Ok(Self::Dns(DnsMethod::ipcast()?)),
            "http" => Ok(Self::Http(IP_URL.to_owned())),
            "http-port-8245" => Ok(Self::Http(IP_URL_8245.to_owned())),

            #[cfg(test)]
            "fail" => Ok(Self::Fail(Cell::new(false))),

            m if m.starts_with("dns:") => Ok(Self::Dns(m[4..].parse()?)),
            m if m.starts_with("http://") => Ok(Self::Http(Url::parse(m)?.to_string())),
            m if m.starts_with("https://") => Ok(Self::Http(Url::parse(m)?.to_string())),
            m if m.starts_with("static:") => Ok(Self::Static(m[7..].parse()?)),

            m => Err(ParseError::UnknownMethod(m.into())),
        }
    }
}

fn get_ip_http(url: &str, timeout: Duration) -> Result<IpAddr, Error> {
    let resp = minreq::get(url)
        .with_header("user-agent", crate::USER_AGENT)
        .with_timeout(timeout.as_secs())
        .send()?;

    let body = resp
        .as_str()
        .map_err(|e| Error::ResponseBodyNotStr(url.into(), e.to_string()))?;

    body.parse()
        .map_err(|e| Error::ResponseParseIp(url.into(), e, body.into()))
}

const fn retry_backoff(retry: u8) -> Duration {
    Duration::from_secs(match retry {
        0 => 3,
        1 => 6,
        2 => 30,
        3 => 300,
        4 => 600,
        _ => 1800,
    })
}

impl IpMethod {
    fn try_get(&self, http_timeout: Duration) -> Result<IpAddr, Error> {
        match self {
            Self::Http(url) => get_ip_http(url, http_timeout),
            Self::Dns(m) => m.get_ip().map_err(Into::into),
            Self::Static(ip) => Ok(*ip),

            #[cfg(test)]
            Self::Fail(b) => {
                if b.get() {
                    panic!("failed ip method should not be called!");
                } else {
                    b.set(true);
                    Err(Error::ExpectedFail)
                }
            }
        }
    }

    pub fn get(&self, http_timeout: Duration, observer: impl Observer) -> IpAddr {
        let mut retries = 0u8;

        loop {
            let error = match self.try_get(http_timeout) {
                Ok(ip) => return ip,
                Err(e) => e,
            };

            let next_try = retry_backoff(retries);

            observer.notify(Notification::GetIpFailedWillRetry(
                error.to_string(),
                retries,
                next_try,
            ));

            retries += 1;
            thread::sleep(next_try);
        }
    }
}

impl std::str::FromStr for IpMethods {
    type Err = ParseError;

    fn from_str(methods: &str) -> Result<Self, Self::Err> {
        methods.split(',').map(IpMethod::from_str).collect()
    }
}

impl std::iter::FromIterator<IpMethod> for IpMethods {
    fn from_iter<T>(iter: T) -> Self
    where
        T: IntoIterator<Item = IpMethod>,
    {
        Self {
            methods: iter.into_iter().map(|m| (m, Cell::new(false))).collect(),
        }
    }
}

impl IpMethods {
    pub fn empty() -> Self {
        Self {
            methods: Vec::new(),
        }
    }

    fn len(&self) -> usize {
        self.methods.len()
    }

    fn reset_failed(&self) {
        for i in 0..self.methods.len() {
            self.methods[i].1.set(false);
        }
    }

    pub fn get(&self, http_timeout: Duration, observer: impl Observer) -> IpAddr {
        if self.len() == 1 {
            return self.methods[0].0.get(http_timeout, observer);
        }

        let mut retries = 0u8;

        loop {
            for (m, had_error) in &self.methods {
                if had_error.get() {
                    debug!("Skipping failed IP method {:?}", m);
                    continue;
                }

                info!("Attempting to get IP with method {:?}", m);

                let error = match m.try_get(http_timeout) {
                    Ok(ip) => return ip,
                    Err(e) => e,
                };

                warn!("Failed to get IP with method {:?}; {}", m, error);
                had_error.set(true);
            }

            info!("Setting all failed IP methods to try again");
            self.reset_failed();

            let d = retry_backoff(retries);

            warn!(
                "Failed to get IP (retry={}), retrying after {}",
                retries,
                humantime::format_duration(d)
            );

            retries += 1;
            thread::sleep(d);
        }
    }
}

#[cfg(test)]
mod test {
    use super::IpMethods;
    use crate::NotificationLogger;

    #[test]
    fn ipmethods_fromstr_for_one() {
        let x = "http".parse::<IpMethods>();
        assert!(x.is_ok());
        assert_eq!(1, x.unwrap().len());
    }

    #[test]
    fn ipmethods_fromstr_for_two() {
        let x = "dns,http".parse::<IpMethods>();
        assert!(x.is_ok());
        assert_eq!(2, x.unwrap().len());
    }

    #[test]
    fn ipmethods_fromstr_for_repeats() {
        let x = "dns,http,dns,http,dns,http,dns,http".parse::<IpMethods>();
        assert!(x.is_ok());
        assert_eq!(8, x.unwrap().len());
    }

    #[test]
    fn ipmethods_fromstr_for_all_formats() {
        let x = "aws-metadata,dns,http,http-port-8245,dns:localhost:1:h:A,http://h,https://h,static:169.254.1.1".parse::<IpMethods>();
        dbg!(&x);
        assert!(x.is_ok());
        assert_eq!(8, x.unwrap().len());
    }

    #[test]
    fn ipmethods_fromstr_fails_trailing_comma() {
        let x = "dns,http,".parse::<IpMethods>();
        assert!(x.is_err());
    }

    #[test]
    fn ipmethods_fromstr_fails_leading_comma() {
        let x = ",dns,http".parse::<IpMethods>();
        assert!(x.is_err());
    }

    #[test]
    fn ipmethods_fromstr_fails_first() {
        let x = "dns,x".parse::<IpMethods>();
        assert!(x.is_err());
    }

    #[test]
    fn ipmethods_fromstr_fails_second() {
        let x = "dns,x".parse::<IpMethods>();
        assert!(x.is_err());
    }

    #[test]
    fn ipmethods_failed_methods_are_skipped() {
        // IpMethod::Fail panics if try_get is called twice
        let x = "fail,static:169.254.1.1"
            .parse::<IpMethods>()
            .expect("ip methods");
        let _ = x.get(std::time::Duration::from_secs(1), NotificationLogger);
        assert!(x.methods[0].1.get());
        //dbg!(x);
        //assert!(false);
    }

    #[test]
    #[should_panic]
    fn ipmethods_failed_methods_reset_on_all_failed() {
        let x = "fail,fail".parse::<IpMethods>().expect("ip methods");
        // We expect this to panic because failed methods should be retried if all fail and Fail
        // always panic's on a second call.
        x.get(std::time::Duration::from_secs(1), NotificationLogger);
    }
}
