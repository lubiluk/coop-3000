# coop-3000

Automated chicken coop controller for Raspberry Pi.

## Build and install on the Pi

Upload the source tree (including `debian/`) to the Pi over SSH. Build from the
source root. The Pi needs WiringPi headers and the library already installed,
and 1-Wire enabled for the temperature sensor.

```sh
sudo apt install build-essential debhelper cmake libboost-program-options-dev
dpkg-buildpackage -b -us -uc
```

Install the `coop-3000` `.deb` produced in the parent directory (not the optional
debug-symbol package). Substitute the actual filename printed by the build:

```sh
sudo apt install ./../coop-3000_1.0_arm64.deb
```

The package does not start or enable the service on first install. Copy the
example once, then edit it to match your pins, location, and sensor:

```sh
sudo install -d /etc/coop-3000
sudo cp -n /usr/share/doc/coop-3000/config_example.cfg /etc/coop-3000/config.cfg
sudoedit /etc/coop-3000/config.cfg
sudo systemctl enable --now coop-3000
```

The unit runs as root on the test Pi. Starting or restarting the controller
moves the door. An upgrade restarts a running service and leaves the config and
CSV untouched. Bump the version in `debian/changelog` for a new release.

If you previously installed a manual unit at
`/etc/systemd/system/coop-3000.service`, stop the old service and remove or
migrate that unit before enabling the packaged one. The local unit otherwise
takes precedence.

### Installed paths

| Path | Purpose |
| --- | --- |
| `/usr/bin/coop-3000` | Controller executable |
| `/usr/lib/systemd/system/coop-3000.service` (or the distribution's vendor unit directory) | Packaged systemd unit |
| `/usr/share/doc/coop-3000/config_example.cfg` | Configuration template |
| `/etc/coop-3000/config.cfg` | Local config, passed explicitly by the unit |
| `/var/log/coop-3000/monitor.csv` | CSV output with the example `monitor-file` setting; systemd creates the parent directory |
| `/sys/bus/w1/devices/<temp-sensor-id>/w1_slave` | Temperature input |

View service output with `journalctl -u coop-3000`.

## Local development on macOS

The CMake build defaults to real GPIO (`USE_MOCK=OFF`). On macOS, install a
C++20 compiler, CMake, and Boost (`program_options`), then opt into mock GPIO:

```sh
cmake -S . -B build-mock -DUSE_MOCK=ON
cmake --build build-mock
cp -n config/config_example.cfg config/config_mock.cfg
```

Set `monitor-file` in `config/config_mock.cfg` to a writable path, such as
`monitor.csv`, then run:

```sh
./build-mock/coop-3000 --config config/config_mock.cfg
```

The mock build still attempts the 1-Wire read and uses a fallback temperature
if the sensor is unavailable. All direct runs require `--config` (or `-c`);
`--help` works without a config.
