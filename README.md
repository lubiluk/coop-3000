# coop-3000

Automated chicken coop controller.

## Dependencies

- CMake 3.10+
- C++20 compiler
- Boost (`program_options`)
- [wiringPi](http://wiringpi.com/) (required on target hardware; not needed when building with mock)

## Build

Clone the repository and create a build directory:

```bash
git clone <repo-url> coop-3000
cd coop-3000
mkdir build && cd build
```

### Mock build (no hardware required)

```bash
cmake .. -DUSE_MOCK=ON
make
```

### Hardware build (Raspberry Pi with wiringPi)

```bash
cmake .. -DUSE_MOCK=OFF
make
```

## Install

```bash
sudo make install
```

This installs:
- `coop-3000` binary to `/usr/local/bin`
- `config.cfg` to `/usr/local/etc/coop-3000/`

To install to a custom prefix (e.g. `/opt/coop`):

```bash
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/coop
make
sudo make install
```

## Configuration

Edit the installed config file at `/usr/local/etc/coop-3000/config.cfg` (or `config/config_example.cfg` as a reference) before running the application.

## Run

It's best to make a service out of it, but you can also run it directly:

```bash
coop-3000 --config /path/to/config.cfg
```

### Systemd Service

1. Create a service file `coop-3000.service`:

```ini
[Unit]
Description=Coop 3000 Automation Service
After=network.target

[Service]
ExecStart=/usr/local/bin/coop-3000
WorkingDirectory=/usr/local/bin
Restart=always
User=cooper
Group=cooper
Environment=HOME=/home/cooper

[Install]
WantedBy=multi-user.target
```

2. Copy it to the systemd directory and enable it:

```bash
sudo cp coop-3000.service /etc/systemd/system/
sudo systemctl daemon-reload
sudo systemctl enable coop-3000
sudo systemctl start coop-3000
```