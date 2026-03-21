### [Unreleased]

### [3.3.0] - 2024-09-16

- Change: Rename ControlChannel to Controller, SleepOnlyControl to SleepOnlyController.
- New: Control::Quit message, Notification::Quitting to cleanly exit the updater
- New: Derive Clone for Notification
- Change: Rust 1.81.0
- Change: Update dependencies
- Change: Only provide "daemonize" for unix targets
- Change: improve help

### [3.2.0] - 2024-07-10

- Fix: properly parse hostnames with round-robin @label
- Change: refactor to library, add observer and control traits for future UI work
- Change: Rust 1.79.0
- Change: Update dependencies

### [3.1.1] - 2024-04-10

- Change: Update dependencies, handle adviory https://rustsec.org/advisories/RUSTSEC-2024-0019 in mio
- New: gnu arm64/aarch64 deb package
- Change: Rust 1.77.2

### [3.1.0] - 2024-02-09

- New: Set CURRENT_IP and LAST_IP environment variables when running exec_on_change command
- New: Replace {{CURRENT_IP}} and {{LAST_IP}} in command string when running exec_on_change command
- Change: Update dependencies including env_logger
- Change: Rust 1.76.0

### [3.0.0] - 2023-09-20

- Change: Rust 1.72.1
- Change: Update dependencies including trust-dns-resolver

### [3.0.0-beta.7] - 2023-07-12

- Change: update Rust edition 2018 to 2021
- Change: DNS method no longer causes exit on creation when nameserver doesn't resolve
- Change: Improve "nohost" error message
- Change: Update dependencies

### [3.0.0-beta.6] - 2022-12-21

- New: Encode colon in username to further support group auth
- Change: Rust 1.59 to 1.66
- Change: Dependency updates
- Change: Improve error message when dns ip method can't be created

### [3.0.0-beta.5] - 2022-03-29

- New: DNS public IP method
- New: Handle a list of public IP methods
- Change: Rust 1.57 to 1.59
- Change: Backoff retry seconds reduced for the first and second retry.

### [3.0.0-beta.4] - 2022-01-12

- New: cargo deny
- Change: IP method `aws` renamed to `aws-metadata`
- Change: Rust 1.56 to 1.57
- Change: Clap 3 out of beta

### [3.0.0-beta.3] - 2021-12-14

- New: Import config from noip2
- Change: static builds

### [3.0.0-beta.2] - 2021-11-02

- Change: Rust 1.53 to 1.56

### [3.0.0-beta.1] - 2021-07-07

- Initial write, everything working.
