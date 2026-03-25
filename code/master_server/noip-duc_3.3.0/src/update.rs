use std::fmt;
use std::net::IpAddr;
use std::time::Duration;

use log::debug;
use percent_encoding::{utf8_percent_encode, AsciiSet, CONTROLS};

const UPDATE_URL: &str = "https://dynupdate.no-ip.com/nic/update";

// https://url.spec.whatwg.org/#query-percent-encode-set
const QUERY_SET: &AsciiSet = &CONTROLS.add(b' ').add(b'"').add(b'#').add(b'<').add(b'>');

type Changed = bool;
type UpdateResult = std::result::Result<Changed, UpdateError>;

#[derive(Clone, Debug)]
pub enum UpdateError {
    NoHost,
    BadAuth,
    BadAgent,
    NotDonator,
    Abuse,
    NineOneOne,
    Unknown(String),
    StatusCode(i32, String),
    Connection(String),
}

impl fmt::Display for UpdateError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use UpdateError::*;
        match self {
            NoHost => f.write_str("No host or group was specified. Please create a group with a password to update at https://my.noip.com/dynamic-dns/groups"),
            BadAuth => f.write_str("Incorrect credentials"),
            BadAgent => f.write_str("Client disabled, client must not perform further updates"),
            NotDonator => f.write_str("This feature is not available for you account"),
            Abuse => f.write_str("Client rejected due to abuse"),
            NineOneOne => f.write_str("System outage, please wait longer than usual to try again"),
            Unknown(msg) => write!(f, "unknown error, received '{}'", msg),
            StatusCode(code, reason) => write!(f, "HTTP error {} {}", code, reason),
            Connection(msg) => write!(f, "Connection failed, {}", msg),
        }
    }
}

impl std::error::Error for UpdateError {}

// Can't use u64::MAX here, it'll panic :). Let's give the user an occasional reminder.
const FOREVER: u64 = 14 * 24 * 60 * 60;

impl UpdateError {
    // Cause the retry interval to jump to the max when we receive a "disable" type response from
    // dynupdate. This will avoid a process manager restarting the daemon if we exit. The user may
    // still restart the service if/when they fix the problem.
    pub fn retry_backoff(&self, retry: u8, base_interval: Duration) -> Duration {
        use UpdateError::*;

        match self {
            NoHost | BadAuth | BadAgent | NotDonator | Abuse => Duration::from_secs(FOREVER),
            _ => {
                base_interval
                    + Duration::from_secs(match retry {
                        0 | 1 => 0,
                        2 => 300,
                        3 => 600,
                        4 => 3600,
                        _ => 24 * 60 * 60,
                    })
            }
        }
    }
}

pub fn update(
    username: &str,
    password: &str,
    hostnames: Option<&Vec<String>>,
    ip: IpAddr,
    timeout: Duration,
) -> UpdateResult {
    let url = match hostnames {
        Some(h) => format!(
            "{}?myip={}&hostname={}",
            UPDATE_URL,
            utf8_percent_encode(&ip.to_string(), QUERY_SET),
            utf8_percent_encode(h.join(",").as_str(), QUERY_SET)
        ),
        None => format!(
            "{}?myip={}",
            UPDATE_URL,
            utf8_percent_encode(&ip.to_string(), QUERY_SET)
        ),
    };

    debug!("Updating with url {}", url);

    let r = minreq::get(url)
        .with_header("user-agent", crate::USER_AGENT)
        .with_header(
            "Authorization",
            format!(
                "Basic {}",
                base64_encode(format!("{}:{}", encode_username(username), password))
            ),
        )
        .with_timeout(timeout.as_secs())
        .send()
        .map_err(|e| UpdateError::Connection(format!("{}", e)))?;

    debug!("{:?}", r);

    let body = r
        .as_str()
        .map_err(|e| UpdateError::Unknown(format!("{}", e)))?;

    match r.status_code {
        200 => {}
        401 => return Err(UpdateError::BadAuth),
        _ => {
            return Err(UpdateError::StatusCode(
                r.status_code,
                r.reason_phrase.clone(),
            ))
        }
    }

    match body.trim_end() {
        s if s.starts_with("good ") => Ok(true),
        s if s.starts_with("nochg ") => Ok(false),
        "nohost" => Err(UpdateError::NoHost),
        "badauth" => Err(UpdateError::BadAuth),
        "badagent" => Err(UpdateError::BadAgent),
        "!donator" => Err(UpdateError::NotDonator),
        "abuse" => Err(UpdateError::Abuse),
        "911" => Err(UpdateError::NineOneOne),
        s => Err(UpdateError::Unknown(s.to_owned())),
    }
}

fn encode_username(username: &str) -> String {
    // The No-IP knowledgebase page says to use `:`. Unfortunately that doesn't work with Basic
    // auth. But the dynupdate code is aware of this and handles percent encoded colons and hashes
    // as well.
    //
    // - https://www.noip.com/support/knowledgebase/limit-hostnames-updated-dynamic-dns-client/
    // - https://www.rfc-editor.org/rfc/rfc7617#section-2
    //
    username.replace(':', "%3A")
}

fn base64_encode<T: AsRef<[u8]>>(bytes: T) -> String {
    use base64::Engine as _;
    base64::engine::general_purpose::STANDARD.encode(bytes)
}
