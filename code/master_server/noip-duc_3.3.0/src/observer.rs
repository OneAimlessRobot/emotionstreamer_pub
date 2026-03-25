use std::net::IpAddr;
use std::sync::mpsc::Sender;
use std::time::Duration;

use crate::UpdateError;

#[derive(Clone, Debug)]
pub enum Notification {
    Error(String),

    CheckIp,
    ExecCommand(String),
    ExecCommandSuccess {
        command: String,
        stdout: String,
        stderr: String,
    },
    ExecCommandError {
        command: String,
        exit: i32,
        stdout: String,
        stderr: String,
    },
    UpdateFailed(UpdateError),
    IpChanged {
        previous: IpAddr,
        current: IpAddr,
    },
    NextCheck(Duration),
    NoUpdateNeeded(IpAddr),
    Updated {
        previous: IpAddr,
        current: IpAddr,
    },

    GetIpFailedWillRetry(String, u8, Duration),

    Quitting,
}

pub trait Observer {
    fn notify(&self, notification: Notification);
}

impl Observer for Sender<Notification> {
    fn notify(&self, notification: Notification) {
        if let Err(e) = self.send(notification) {
            log::error!("failed to write to mpsc channel; {e}");
        }
    }
}

#[derive(Clone)]
pub struct NotificationLogger;

impl Observer for NotificationLogger {
    fn notify(&self, notification: Notification) {
        use log::{debug, error, info, warn};

        match notification {
            Notification::Error(e) => error!("{e}"),

            Notification::CheckIp => debug!("checking for new ip"),

            Notification::ExecCommand(cmd) => debug!("running command; exec_on_change={cmd}"),
            Notification::ExecCommandSuccess {
                command,
                stdout,
                stderr,
            } => info!("execute success for '{command}'; stdout='{stdout}', stderr='{stderr}'"),
            Notification::ExecCommandError {
                command,
                exit,
                stdout,
                stderr,
            } => {
                error!("execute failure for '{command}'; exit={exit}, stdout={stdout}, stderr={stderr}")
            }
            Notification::UpdateFailed(e) => error!("update failed; {e}"),
            Notification::IpChanged { previous, current } => {
                info!("got new ip; current={current}, previous={previous}")
            }
            Notification::NextCheck(d) => {
                info!("checking ip again in {}", humantime::format_duration(d))
            }
            Notification::NoUpdateNeeded(ip) => debug!("no update needed; ip={ip}"),
            Notification::Updated { previous, current } => {
                info!("update successful; current={current}, previous={previous}")
            }
            Notification::GetIpFailedWillRetry(error, retries, next_try) => {
                warn!(
                    "Failed to get ip (retry={}), retrying after {}; {}",
                    retries,
                    humantime::format_duration(next_try),
                    error
                )
            }
            Notification::Quitting => {
                info!("stopping updates and exiting")
            }
        }
    }
}
