# torizon-qt6-diagnostics

Qt6/QML diagnostics dashboard for **Apalis i.MX8** running **Torizon OS**.

Displays a real-time GUI on Wayland (via Weston) showing:
- **SPI loopback status** — opens `/dev/apalis-spi1-cs0`, sends 8 bytes and verifies the echo (MOSI ↔ MISO bridge required).
- **I2C temperature** — reads the TMP75C sensor at address `0x4F` on `i2c-4` every 5 seconds.

All hardware access runs inside a Docker container. The Qt6 application communicates with QML via a `QObject`-based model (`DiagnosticsModel`) exposed through `QQmlContext`.

![Torizon Qt6 Diagnostics Dashboard](docs/screenshot.png)

---

## Table of contents

1. [Prerequisites](#prerequisites)
2. [Hardware setup](#hardware-setup)
3. [Project structure](#project-structure)
4. [Architecture](#architecture)
5. [Step-by-step setup](#step-by-step-setup)
6. [Build](#build)
7. [Container setup](#container-setup)
8. [Running the application](#running-the-application)
9. [Expected output](#expected-output)
10. [Troubleshooting](#troubleshooting)
11. [License](#license)

---

## Prerequisites

| Tool | Version | Purpose |
|---|---|---|
| [VS Code](https://code.visualstudio.com/) | Latest | IDE |
| [Torizon IDE Extension](https://developer.toradex.com/torizon/application-development/ide-extension/) | Latest | Cross-compile & deploy to board |
| [Docker Desktop](https://www.docker.com/products/docker-desktop/) | Latest | Build container images |
| [WSL 2](https://learn.microsoft.com/en-us/windows/wsl/install) | Ubuntu recommended | Linux environment on Windows |
| Git | Any | Version control |
| Toradex board flashed with **Torizon OS** | 6.x or later | Target hardware |

You also need a **Toradex account** to push images to the container registry (`${DOCKER_LOGIN}`).

> **Qt6 is not required on the host machine.** The Torizon SDK container includes Qt6 for ARM64 cross-compilation. Building locally without the SDK will fail with `Could not find Qt6Config.cmake` — this is expected. Always build via the Torizon extension (**Build Debug**).

---

## Hardware setup

| Field | Value |
|---|---|
| SoM | Toradex Apalis i.MX8 |
| Carrier | Apalis Evaluation Board |
| OS | Torizon OS |
| SPI device | `/dev/apalis-spi1-cs0` → `spidev0.0` |
| I2C device | `/dev/i2c-4` |
| Temperature sensor | TMP75C at address `0x4F` |
| Display | Wayland compositor (Weston) via HDMI |

**SPI loopback wiring:**  
Connect **MOSI** to **MISO** on the Apalis Evaluation Board expansion connector with a short jumper wire.

> Refer to the [Apalis Evaluation Board datasheet](https://docs.toradex.com/101028-apalis-evaluation-board-datasheet.pdf) for exact pin numbers on the X3 expansion connector.

---

## Project structure

```
torizon-qt6-diagnostics/
├── include/
│   ├── spi_device.hpp          # SpiDevice class declaration
│   ├── i2c_bus.hpp             # I2cBus class declaration
│   └── diagnostics_model.hpp  # QObject model for QML binding
├── src/
│   ├── spi_device.cpp          # spidev ioctl() implementation
│   ├── i2c_bus.cpp             # i2c-dev ioctl() implementation
│   └── diagnostics_model.cpp  # Refresh logic + Qt signals
├── QML/
│   ├── main.qml               # Main application window
│   └── assets/
│       └── torizon-logo.png
├── main.cpp                    # QGuiApplication + QML engine setup
├── CMakeLists.txt              # Qt6 CMake build configuration
├── docker-compose.yml          # Container definition (debug + release)
├── Dockerfile                  # Release image
├── Dockerfile.debug            # Debug image (SSH + GDB server)
├── Dockerfile.sdk              # SDK image for cross-compilation
├── torizonPackages.json        # Container runtime dependencies
└── .gitignore
```

---

## Architecture

The application follows a clean separation between C++ hardware logic and QML presentation:

```
┌──────────────────────────────────────────┐
│              QML (main.qml)              │
│   Reads: spiOk, temperature, errorMsg   │
│   Calls: diagnostics.refresh()          │
└──────────────────┬───────────────────────┘
                   │ QQmlContext property
┌──────────────────▼───────────────────────┐
│         DiagnosticsModel (C++)           │
│  Q_PROPERTY + signals + QTimer (5s)     │
└──────────────┬────────────┬──────────────┘
               │            │
┌──────────────▼───┐   ┌────▼─────────────────┐
│   SpiDevice      │   │      I2cBus           │
│  spidev0.0       │   │  i2c-4 @ 0x4F         │
└──────────────────┘   └──────────────────────┘
```

`DiagnosticsModel` runs a `QTimer` every 5 seconds, reads both peripherals and emits Qt signals. QML binds to the properties reactively — no polling from the UI side.

---

## Step-by-step setup

### 1. Clone the repository

```bash
git clone https://github.com/DaniCampusIoT/torizon-qt6-diagnostics.git
cd torizon-qt6-diagnostics
```

### 2. Open in VS Code

```bash
code .
```

The Torizon extension detects the project automatically. Allow it to initialize the workspace if prompted.

### 3. Configure the `.env` file

The Torizon extension generates a `.env` file with your credentials and board IP. Verify it contains:

```env
DOCKER_LOGIN=your_toradex_or_dockerhub_username
IMAGE_ARCH=arm64
DEBUG_SSH_PORT=2231
DEBUG_PORT1=2345
DEBUG_PORT2=3456
TAG=latest
```

> `.env` is listed in `.gitignore` — never commit it.

### 4. Verify devices on the board

SSH into the board and confirm both device nodes exist:

```bash
ssh torizon@<board-ip>
ls /dev/apalis-spi* /dev/spidev*
# Expected: /dev/apalis-spi1-cs0  /dev/apalis-spi2-cs0  /dev/spidev0.0  /dev/spidev1.0

ls /dev/i2c-*
# Expected: /dev/i2c-0  /dev/i2c-1  ...  /dev/i2c-4

i2cdetect -y 4
# TMP75C should appear at address 0x4F
```

### 5. Connect the SPI loopback wire

Bridge MOSI and MISO on the expansion connector before deploying.

### 6. Build and deploy

Use the Torizon extension tasks in VS Code:

- **Build Debug** → cross-compiles for ARM64 inside the SDK container and builds the debug Docker image.
- **Deploy Debug** → pushes the image to the board and starts the container with Weston.
- The application window appears automatically on the HDMI display.

---

## Build

Building is done **inside the cross-compilation SDK container** — Qt6 for ARM64 is provided by the container, not the host.

```bash
# Triggered automatically by VS Code, or manually:
docker run --rm -v $(pwd):/home/torizon/app cross-toolchain-arm64-diagnostics \
  /usr/local/bin/qt-cmake -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -Bbuild-arm64

docker run --rm -v $(pwd):/home/torizon/app cross-toolchain-arm64-diagnostics \
  cmake --build build-arm64
```

> **Do not run `cmake` directly on the host** — it will fail with `Could not find Qt6Config.cmake`. This is expected; Qt6 is only available inside the SDK container.

---

## Container setup

Both the SPI and I2C device nodes must be explicitly mapped into the container. The key sections in `docker-compose.yml`:

```yaml
devices:
  - /dev/spidev0.0:/dev/apalis-spi1-cs0   # real node mapped to the symlink name
  - /dev/i2c-4:/dev/i2c-4
device_cgroup_rules:
  - "c 153:* rmw"   # spidev major
  - "c 89:* rmw"    # i2c major
```

> **Important:** Docker cannot map symlinks as `devices` source — always use the real node (`spidev0.0`), not the symlink (`apalis-spi1-cs0`). The mapping renames the node inside the container so the application code uses the expected path.

Both `diagnostics-debug` (used by **Deploy Debug**) and `diagnostics` (release profile) require these entries. Missing them from either service causes `Operation not permitted` at runtime.

---

## Running the application

<img width="1405" height="786" alt="WhatsApp Image 2026-07-08 at 14 32 57" src="https://github.com/user-attachments/assets/ab354676-2bb5-4099-8dc6-b6a500bfdb8c" />

Once deployed, the Qt6 window appears on the HDMI display automatically. The UI refreshes every 5 seconds.

To trigger a manual refresh, press the **Refresh** button in the application window.

---

## Expected output

With the SPI loopback wire connected and the TMP75C sensor present:

- **SPI card** → green background, `✔ LOOPBACK OK`
- **I2C card** → temperature reading, e.g. `23.5625 °C`
- **Error line** → hidden
- **Last update** → current time, updating every 5 seconds

Without the loopback wire:

- **SPI card** → red background, `✘ FAILED`

If the I2C sensor is unreachable:

- **Error line** → `⚠ Cannot open I2C device /dev/i2c-4: Operation not permitted`

---

## Troubleshooting

### `Could not find Qt6Config.cmake` during CMake configure

You ran `cmake` directly on the host instead of through the SDK container.

- **Fix:** Always use VS Code **Build Debug** or the `docker run` command shown in [Build](#build).
- To build locally on WSL: `sudo apt install -y qt6-base-dev qt6-declarative-dev libqt6qml6 libqt6quick6`.

---

### `Cannot find source file: src/spi_device.cpp`

CMake cannot locate one of the source files listed in `CMakeLists.txt`.

- Verify the files exist:
  ```bash
  ls src/*.cpp include/*.hpp
  ```
- Remove Windows metadata files if present:
  ```bash
  rm src/*.Identifier include/*.Identifier 2>/dev/null
  ```
- Make sure `diagnostics_model.hpp` is listed in `qt_add_executable` so AUTOMOC processes it:
  ```cmake
  qt_add_executable(torizonqt6diagnostics WIN32
      main.cpp
      src/spi_device.cpp
      src/i2c_bus.cpp
      src/diagnostics_model.cpp
      include/diagnostics_model.hpp
  )
  ```

---

### `undefined reference to vtable for DiagnosticsModel`

AUTOMOC did not process `diagnostics_model.hpp` — the MOC-generated file is missing.

- Add `include/diagnostics_model.hpp` to `qt_add_executable` (see above).
- Delete the build folder and rebuild:
  ```bash
  rm -rf build-arm64
  # Then Build Debug from VS Code
  ```

---

### SPI: `Operation not permitted` inside the container

- Check that `docker-compose.yml` has **both** `devices` and `device_cgroup_rules` under the **active service** (`diagnostics-debug` for VS Code, `diagnostics` for release).
- Confirm the SPI major number is `153` on your board:
  ```bash
  ls -la /dev/spidev0.0
  # crw-rw-r-- 1 root spidev 153, 0 ...
  ```
- Re-deploy after editing `docker-compose.yml` — a running container does **not** pick up changes until recreated.

---

### SPI: `✘ FAILED` (loopback mismatch)

- **No wire:** Check MOSI and MISO are physically bridged.
- **Wrong pins:** Verify the pinout in the [Apalis Evaluation Board datasheet](https://docs.toradex.com/101028-apalis-evaluation-board-datasheet.pdf).
- **Speed too high:** Lower `cfg.speedHz` in `diagnostics_model.cpp` from `500000` to `100000`.

---

### I2C: `Operation not permitted`

- Add `c 89:* rmw` to `device_cgroup_rules` and `/dev/i2c-4:/dev/i2c-4` to `devices` under the active service.
- Redeploy the container after changes.

---

### I2C temperature reads `0.0000 °C`

- Verify the sensor is detected: `i2cdetect -y 4` — should show `4f`.
- If on a different address, update `cfg.address` in `diagnostics_model.cpp`.
- If on a different bus, update `cfg.device` and the `devices` mapping in `docker-compose.yml`.

---

### `detected dubious ownership` (Git from PowerShell)

Run all git commands from inside WSL, not from PowerShell:

```bash
cd /home/<your-user>/torizonQt6Diagnostics
git ...
```

---

### Debug SSH port conflict

```bash
docker ps --format '{{.Names}}\t{{.Ports}}'
docker stop <conflicting-container>
```

Or change `DEBUG_SSH_PORT` in `.env` to a free port and redeploy.

---

## License

MIT — see [LICENSE](LICENSE).
