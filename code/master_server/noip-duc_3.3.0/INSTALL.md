How to Install No-IP Linux DUC 3
================================

With a Package
--------------

Download the package and install as your OS recommends. For instance, with Ubuntu,

```
dpkg -i noip-duc_3*.deb
```

Then create the config. See Configuration section below.

Statically-linked Binary
----------------------

No-IP provides a statically-linked binary of `noip-duc`. It should run on any Linux as long as the architecture is correct. For instance, if you use a current Intel or AMD processor, choose the package with the architecture `x86_64`.

```
tar xzvf noip-duc_3*-musl.gz
sudo mv noip-duc_3*-musl /usr/local/bin/noip-duc
```

Create an init script to run `noip-duc` on startup. Here is a simple Systemd service unit,

```
[Unit]
Description=No-IP Dynamic Update Client
After=network.target auditd.service

[Service]
EnvironmentFile=/etc/default/noip-duc
ExecStart=/usr/local/bin/noip-duc
Restart=on-failure
Type=simple

[Install]
WantedBy=multi-user.target
```

From Source
-----------

[Rust](https://www.rust-lang.org/) is required to build from source. Follow the instructions at https://rustup.rs/

```
curl -OL https://www.noip.com/download/linux\?package=source > noip-duc3.tgz
tar xzvf noip-duc3.tgz
cd noip-duc-*
cargo build --release
sudo cp target/release/noip-duc /usr/local/bin

# For systemd.
# - On non-Debian OSes, edit the EnvironmentFile entry.
sed '/^ExecStart=/ s#usr/bin#usr/local/bin#' debian/service | sudo tee /etc/systemd/system/noip-duc.service
sudo systemctl daemon-reload
```

Configuration
=============

Configuration may be done with command line options or environment variables. Environment variables make it easy to create a configuration file that integrates with Systemd, sysvinit, or other init system.

Here is an example configuration file. It contains a password, so set permissions appropriately, ideally `0600`. See `noip-duc --help` for a full explanation of each option.

```
## /etc/defaults/noip-duc (Debian) or /etc/sysconfig/noip-duc (RedHat, Suse)
## or anywhere you like.
NOIP_USERNAME=
NOIP_PASSWORD=

## Comma separated list of hostnames and group names
NOIP_HOSTNAMES=

## Less common options
#NOIP_CHECK_INTERVAL=5m
#NOIP_EXEC_ON_CHANGE=
#NOIP_HTTP_TIMEOUT=10s
## ip methods: aws, http, http-port-8245, static:<IP>
#NOIP_IP_METHOD=dns,http,http-port-8245
#NOIP_LOG_LEVEL=info

## Daemon options should not be set if using systemd. They only apply when `--daemon` is used.
#NOIP_DAEMON_GROUP=
#NOIP_DAEMON_PID_FILE=
#NOIP_DAEMON_USER=
```

Migrating From noip2
--------------------

`noip-duc` includes a method to generate an environment variables file from the noip2 config.

```
noip-duc --import /usr/local/etc/no-ip2.conf | sudo tee /etc/default/noip-duc
```
