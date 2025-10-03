use std::path::PathBuf;
use std::time::Duration;

use anyhow::Result;
use clap::Parser;
use log::debug;

use noip_duc::{noip2, public_ip::IpMethods, updater, NotificationLogger, SleepOnlyController};

// This is used to handle --import since the `exclusive` and `conflicts_with` options don't seem to
// work in clap 3.0.0-beta.5. Perhaps they will work in the future or when it goes stable. This
// should be revisited.
#[derive(Debug, Parser)]
struct PreConfig {
    /// Import config from noip2 and display it as environment variables.
    #[clap(long, parse(from_os_str), default_missing_value = "/etc/no-ip2.conf")]
    import: PathBuf,
}

#[derive(Debug, Parser)]
#[clap(about = "No-IP Dynamic Update Client", version = clap::crate_version!())]
struct Config {
    /// Your www.noip.com username. For better security, use Update Group credentials. https://www.noip.com/members/dns/dyn-groups.php
    #[clap(short, long, env = "NOIP_USERNAME")]
    username: String,

    /// Your www.noip.com password. For better security, use Update Group credentials. https://www.noip.com/members/dns/dyn-groups.php
    #[clap(short, long, env = "NOIP_PASSWORD")]
    password: String,

    /// Comma separated list of groups and hostnames to update.
    // use std::vec::Vec to avoid Clap magic
    #[clap(short = 'g', long, env = "NOIP_HOSTNAMES", parse(try_from_str = parse_hostnames))]
    hostnames: Option<std::vec::Vec<String>>,

    /// How often to check for a new IP address. Minimum: every 2 minutes.
    #[clap(long, env = "NOIP_CHECK_INTERVAL", default_value = "5m", parse(try_from_str = humantime::parse_duration))]
    check_interval: Duration,

    /// Timeout when making HTTP requests.
    #[clap(long, env = "NOIP_HTTP_TIMEOUT", default_value = "10s", parse(try_from_str = humantime::parse_duration))]
    http_timeout: Duration,

    #[cfg(target_family = "unix")]
    /// Fork into the background
    #[clap(long)]
    daemonize: bool,

    #[cfg(target_family = "unix")]
    /// When daemonizing, become this user.
    #[clap(long, env = "NOIP_DAEMON_USER")]
    daemon_user: Option<String>,

    #[cfg(target_family = "unix")]
    /// When daemonizing, become this group.
    #[clap(long, env = "NOIP_DAEMON_GROUP")]
    daemon_group: Option<String>,

    #[cfg(target_family = "unix")]
    /// When daemonizing, write process id to this file.
    #[clap(long, env = "NOIP_DAEMON_PID_FILE", parse(from_os_str))]
    daemon_pid_file: Option<PathBuf>,

    /// Increase logging verbosity. May be used multiple times.
    #[clap(short, long, parse(from_occurrences))]
    verbose: i32,

    /// Set the log level. Possible values: trace, debug, info, warn, error, critical. Overrides --verbose.
    #[clap(short, long, env = "NOIP_LOG_LEVEL")]
    log_level: Option<LogLevel>,

    /// Command to run when the IP address changes. It is run with the environment variables
    /// CURRENT_IP and LAST_IP set. Also, {{CURRENT_IP}} and {{LAST_IP}} are replaced with the
    /// respective values. This allows you to provide the variables as arguments to your command or
    /// read them from the environment. The command is always executed in a shell, sh or cmd on
    /// Windows.
    ///
    /// Example
    ///
    ///   noip_duc -e 'mail -s "IP changed to {{CURRENT_IP}} from {{LAST_IP}}" user@example.com'
    #[clap(short = 'e', long, env = "NOIP_EXEC_ON_CHANGE")]
    exec_on_change: Option<String>,

    /// Methods used to discover the public IP, as a comma separated list. They are tried in order
    /// until a public IP is found. Failed methods are not retried unless all methods fail.
    ///
    /// Possible values are
    /// - 'aws-metadata': uses the AWS metadata URL to get the Elastic IP
    ///                   associated with your instance.
    /// - 'dns': Use No-IP's DNS public IP lookup system.
    /// - 'dns:<nameserver>:<port>:<qname>:<record type>': custom DNS lookup.
    /// - 'http': No-IP's HTTP method on port 80.
    /// - 'http-port-8245': No-IP's HTTP method on port 8245.
    /// - 'static:<ip address>': always use this IP address. Helpful with --once.
    /// - HTTP URL: An HTTP URL that returns only an IP address.
    #[clap(
        long,
        env = "NOIP_IP_METHOD",
        default_value = "dns,http,http-port-8245",
        verbatim_doc_comment
    )]
    ip_method: IpMethods,

    /// Find the public IP and send an update, then exit. This is a good method to verify correct
    /// credentials.
    #[clap(long)]
    once: bool,

    /// Import config from noip2 and display it as environment variables.
    #[clap(long, default_value = "/etc/no-ip2.conf")]
    import: Option<Option<PathBuf>>,
}

impl<'a> From<&'a Config> for noip_duc::Config<'a> {
    fn from(config: &'a Config) -> Self {
        Self {
            username: config.username.as_str(),
            password: config.password.as_str(),
            hostnames: config.hostnames.as_ref(),
            check_interval: config.check_interval,
            http_timeout: config.http_timeout,
            exec_on_change: config.exec_on_change.as_deref(),
            ip_method: &config.ip_method,
            once: config.once,
        }
    }
}

#[derive(Debug)]
enum LogLevel {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Critical,
}

impl std::str::FromStr for LogLevel {
    type Err = anyhow::Error;

    fn from_str(s: &str) -> Result<Self, Self::Err> {
        use LogLevel::*;
        Ok(match s.to_lowercase().as_str() {
            "trace" => Trace,
            "debug" => Debug,
            "info" => Info,
            "warn" | "warning" => Warning,
            "error" => Error,
            "critical" => Critical,
            _ => anyhow::bail!("unknown log level"),
        })
    }
}

use std::fmt;
impl fmt::Display for LogLevel {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use LogLevel::*;
        match self {
            Trace => f.write_str("trace"),
            Debug => f.write_str("debug"),
            Info => f.write_str("info"),
            Warning => f.write_str("warning"),
            Error => f.write_str("error"),
            Critical => f.write_str("critical"),
        }
    }
}

// May be hostnames or group names
fn parse_hostnames(s: &str) -> Result<Vec<String>> {
    if s.len() >= 4000 {
        anyhow::bail!("hostnames too long");
    }

    let hostnames: Vec<String> = s.split(',').map(|s| s.trim().to_owned()).collect();

    for h in &hostnames {
        // Group names are alphanumeric only
        if h.chars().all(|c| char::is_ascii_alphanumeric(&c)) {
            continue;
        }
        if !is_hostname(h) {
            anyhow::bail!(
                "invalid hostname {}. Hostnames must be a comma separated list of hostnames and group names.",
                h
            );
        }
    }

    Ok(hostnames)
}

fn is_hostname(h: &str) -> bool {
    // May contain a round-robin label
    let h = match h.split_once('@') {
        Some((h, rr)) => {
            if !is_rr_label(rr) {
                return false;
            }
            h
        }
        None => h,
    };

    if h.split('.').count() > 63 {
        return false;
    }

    h.split('.').all(is_label)
}

// Must be all alphanumeric or hyphen. Since these will always be A or AAAA they cannot
// start with `_` like TXT or SRV can.
fn is_label(s: &str) -> bool {
    s.chars().all(|c| char::is_ascii_alphanumeric(&c) || c == '-')
        // Cannot start with hyphen or be empty
        && s.chars().next().map_or(false, |c| c != '-')
        // Cannot end with hyphen or be empty
        && s.chars().last().map_or(false, |c| c != '-')
}

// Check round-robin label. It is the part after an @ in the hostname field.
fn is_rr_label(s: &str) -> bool {
    s.chars().all(|c| char::is_ascii_alphanumeric(&c) || matches!(c, '-' | '_'))
        // Cannot start with hyphen or be empty
        && s.chars().next().map_or(false, |c| c != '-')
}

fn main() -> anyhow::Result<()> {
    // Handle --import first to avoid required --username and --password
    if let Ok(c) = PreConfig::try_parse() {
        let imported = noip2::import(&c.import)?;
        print!("{}", imported);
        return Ok(());
    };

    let config = Config::parse();

    if config.check_interval < Duration::from_secs(120) {
        anyhow::bail!("--check_interval must be no less than 2 minutes");
    }

    let log_level = config.log_level.as_ref().unwrap_or(match config.verbose {
        0 => &LogLevel::Info,
        1 => &LogLevel::Debug,
        _ => &LogLevel::Trace,
    });

    env_logger::Builder::from_env(
        env_logger::Env::default().default_filter_or(log_level.to_string()),
    )
    .init();

    #[cfg(target_family = "unix")]
    if config.daemonize {
        daemonize(&config)?;
    }

    debug!("{:?}", config);

    updater(
        (&config).into(),
        NotificationLogger {},
        SleepOnlyController {},
    )
    .map_err(Into::into)
}

#[cfg(target_family = "unix")]
fn daemonize(c: &Config) -> Result<()> {
    use daemonize::Daemonize;

    let mut daemonize = Daemonize::new().working_directory("/");

    if let Some(user) = &c.daemon_user {
        daemonize = match user.parse::<u32>() {
            Err(_) => daemonize.user(user.as_str()),
            Ok(uid) => daemonize.user(uid),
        }
    }

    if let Some(group) = &c.daemon_group {
        daemonize = match group.parse::<u32>() {
            Err(_) => daemonize.group(group.as_str()),
            Ok(gid) => daemonize.group(gid),
        }
    }

    if let Some(pid_file) = &c.daemon_pid_file {
        daemonize = daemonize.pid_file(pid_file).chown_pid_file(true);
    }

    daemonize.start()?;

    log::info!("running in background");

    Ok(())
}

#[cfg(test)]
mod test {
    use super::*;

    #[test]
    fn is_rr_label_good() {
        for s in ["SERVER-1", "SERVER_1", "_TEST", "_test", "test-"] {
            assert!(is_rr_label(s), r#"input="{s}""#);
        }
    }

    #[test]
    fn is_rr_label_bad() {
        for s in ["SERVER 1", "-test", "^TEST", "te&st", "te|t"] {
            assert!(!is_rr_label(s), r#"input="{s}""#);
        }
    }

    #[test]
    fn is_hostname_good() {
        for s in ["h", "h.test", "h.example.com", "h.example.com@test"] {
            assert!(is_hostname(s), r#"input="{s}""#);
        }
    }

    #[test]
    fn is_hostname_bad() {
        for s in [
            " ",
            "h test",
            "h.example com",
            "h.example.com@-test",
            "h.example.com^test",
        ] {
            assert!(!is_hostname(s), r#"input="{s}""#);
        }
    }
}
