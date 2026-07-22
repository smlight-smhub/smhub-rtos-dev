# SMHUB RTOS Developer Workspace

Welcome to the `smhub-rtos-dev` meta-repository!

This repository acts as an all-in-one, containerized VS Code Workspace tailored specifically for core C++ and Python development of the **SMHUB ESPHome ecosystem**. It integrates all the essential upstream repositories via Git submodules and automatically configures a unified development environment using DevContainers.

## Included Submodules

The workspace mounts the following repositories side-by-side in `src/`:

1. **`esphome`**: The SMHUB fork of ESPHome (Core Python/C++ generation logic).
2. **`platform-sg2000`**: The PlatformIO integration for the SG2000 SoC.
3. **`framework-sg2000-rtos`**: The underlying FreeRTOS framework for the SG2000.
4. **`device-builder`**: The Python wrapper/API for configuring devices.
5. **`device-builder-frontend`**: The web UI for the device builder.
6. **`rtos-config`**: Configuration definitions for the RTOS build.

## Getting Started

### 1. Clone this Repository
Because this is a meta-repository, be sure to clone it with its submodules:
```bash
git clone --recursive https://github.com/smlight-smhub/smhub-rtos-dev.git
cd smhub-rtos-dev
```

*(If you forgot `--recursive`, you can initialize them later by running `git submodule update --init --recursive`)*

### 2. Open in DevContainer
1. Open the cloned `smhub-rtos-dev` **folder** in VS Code (Do not open the workspace file just yet).
2. When prompted, click **"Reopen in Container"** (or press `F1` and search for *Dev Containers: Reopen in Container*).

### 3. Automatic Bootstrapping
When the container finishes building, the included `.devcontainer/bootstrap.sh` script automatically runs in the background. It will:
- Install the latest versions of your local submodules.
- Install the `esphome` core as an editable Python package.
- Pre-stage the `platformio_override.ini` symlink so your ESPHome builds compile directly against your local C++ platform and framework submodules instead of pulling them from GitHub.

### 4. Open the Multi-Root Workspace
Once the container finishes booting, VS Code will just be looking at the root directory. To enable the multi-root setup (where all the submodules appear side-by-side):
1. In the file explorer, click on `smhub.code-workspace`.
2. A button will appear in the bottom right corner of VS Code that says **"Open Workspace"**. Click it!
*(Alternatively, you can go to `File` > `Open Workspace from File...` and select `smhub.code-workspace`)*

## Development Workflow

### C++ / PlatformIO IntelliSense
The C++ server in VS Code (powered by the Microsoft C/C++ extension) is pre-configured to understand the SMHUB architecture, it resolves headers directly from your local SG2000 framework and platform checkouts. 

### Python Development
The Python environment uses `uv` for lightning-fast dependency management and provides a virtual environment located at `/home/esphome/.local/esphome-venv`. Pre-commit, Ruff, and Pylance are all natively supported inside the DevContainer.

### Web Frontend
The container is pre-provisioned with Node.js (v24), allowing you to jump straight into `src/device-builder-frontend` to run `npm install` and `npm run dev`.
