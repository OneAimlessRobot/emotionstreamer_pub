use std::net::{IpAddr, Ipv4Addr};
use std::process::Command;
use std::time::Duration;

mod control;
mod observer;

pub mod dns_method;
pub mod noip2;
pub mod public_ip;
pub mod update;

pub use control::*;
pub use observer::*;

use public_ip::IpMethods;
use update::{update, UpdateError};

const USER_AGENT: &str = concat!(
    clap::crate_name!(),
    "/",
    clap::crate_version!(),
    " <support@noip.com>",
);

pub struct Config<'a> {
    pub username: &'a str,
    pub password: &'a str,
    pub hostnames: Option<&'a std::vec::Vec<String>>,
    pub check_interval: Duration,
    pub http_timeout: Duration,
    pub exec_on_change: Option<&'a str>,
    pub ip_method: &'a IpMethods,
    pub once: bool,
}

pub fn updater(
    c: Config,
    observer: impl Observer + Clone,
    control: impl Controller,
) -> Result<(), UpdateError> {
    let mut last_ip: IpAddr = IpAddr::V4(Ipv4Addr::new(0, 0, 0, 0));
    let mut retries = 0u8;
    let mut last_error: Option<UpdateError> = None;

    loop {
        observer.notify(Notification::CheckIp);

        let ip = c.ip_method.get(c.http_timeout, observer.clone());

        if last_ip != ip {
            observer.notify(Notification::IpChanged {
                current: ip,
                previous: last_ip,
            });

            match update(c.username, c.password, c.hostnames, ip, c.http_timeout) {
                Ok(changed) => {
                    observer.notify(Notification::Updated {
                        current: ip,
                        previous: last_ip,
                    });

                    if changed {
                        if let Some(cmd_tmpl) = c.exec_on_change {
                            exec_command(cmd_tmpl, ip.to_string(), last_ip.to_string(), &observer);
                        }
                    }

                    last_ip = ip;
                    retries = 0;
                    last_error = None;
                }
                Err(e) => {
                    observer.notify(Notification::UpdateFailed(e.clone()));
                    last_error = Some(e);
                    retries += 1;
                }
            }
        } else {
            observer.notify(Notification::NoUpdateNeeded(ip));
        }

        if c.once {
            return match last_error {
                Some(e) => Err(e),
                None => Ok(()),
            };
        }

        let dur = match last_error {
            Some(ref e) => e.retry_backoff(retries, c.check_interval),
            None => c.check_interval,
        };

        observer.notify(Notification::NextCheck(dur));

        let start = std::time::Instant::now();

        loop {
            let remaining = dur - start.elapsed();

            if remaining.is_zero() {
                break;
            }

            match control.recv_timeout(remaining) {
                Some(Control::UpdateNow) => break,
                Some(Control::NotifyNextCheck) => {
                    observer.notify(Notification::NextCheck(dur - start.elapsed()));
                }
                Some(Control::Quit) | None => {
                    observer.notify(Notification::Quitting);
                    return Ok(());
                }
            }
        }
    }
}

fn exec_command<T: Observer>(
    cmd_tmpl: &str,
    ip: impl AsRef<str>,
    last_ip: impl AsRef<str>,
    observer: &T,
) {
    use std::str::from_utf8;

    fn shell() -> Command {
        if cfg!(target_os = "windows") {
            let mut cmd = Command::new("cmd");
            cmd.arg("/C");
            cmd
        } else {
            let mut cmd = Command::new("sh");
            cmd.arg("-c");
            cmd
        }
    }

    let cmd = cmd_tmpl
        .replace("{{CURRENT_IP}}", ip.as_ref())
        .replace("{{LAST_IP}}", last_ip.as_ref());

    observer.notify(Notification::ExecCommand(cmd.clone()));

    match shell()
        .arg(&cmd)
        .env("CURRENT_IP", ip.as_ref())
        .env("LAST_IP", last_ip.as_ref())
        .output()
    {
        Ok(output) => {
            if output.status.success() {
                observer.notify(Notification::ExecCommandSuccess {
                    command: cmd.clone(),
                    stdout: from_utf8(&output.stdout).unwrap_or("").to_string(),
                    stderr: from_utf8(&output.stderr).unwrap_or("").to_string(),
                });
            } else {
                observer.notify(Notification::ExecCommandError {
                    command: cmd.clone(),
                    exit: output.status.code().unwrap_or(-1),
                    stdout: from_utf8(&output.stdout).unwrap_or("").to_string(),
                    stderr: from_utf8(&output.stderr).unwrap_or("").to_string(),
                });
            }
        }
        Err(e) => {
            observer.notify(Notification::ExecCommandError {
                command: cmd.clone(),
                exit: -1,
                stdout: String::new(),
                stderr: format!("{e}"),
            });
        }
    }
}
