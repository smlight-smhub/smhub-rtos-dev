# Framework for SMLIGHT SMHUB RTOS (SG2000)

This repository serves as the core, generic bare-metal Software Development Kit (SDK) for the **SMLIGHT SMHUB** hardware architecture, which is based on the SG2000 SoC. It provides the low-level operating system fundamentals required to execute code on the heterogeneous RISC-V C906L core.

It was created to support the native integration of ESPHome onto the SMHUB hardware rtos core.

## Components Included

This framework bundles the following critical SDK components:
- **FreeRTOS Kernel:** Pre-configured for the RISC-V RV64 architecture and tuned for the C906L core.
- **SMHUB HAL (`smhub-hal`):** The hardware abstraction layer for the SG2000, built entirely from the ground up. It maps perfectly to mainline Linux device topologies for clocks, soft resets, and pin multiplexing, ensuring 100% architectural compatibility with the U-Boot hardware gatekeeper.
- **OpenAMP & libmetal:** The IPC (Inter-Processor Communication) stack. This provides the virtio/RPMsg layer allowing the RTOS to seamlessly exchange Protobuf payloads with the Linux `smhub-broker` daemon.
- **Boot Entry & Memory Maps:** Contains `smhub_lscript.ld` and `smhub_board_memmap.ld`, strictly defining the isolated 1.875MB DRAM region allocated exclusively to the RTOS.

## Usage (PlatformIO)

This framework is designed to be consumed by the [platform-sg2000](https://github.com/smlight-smhub/platform-sg2000.git) registry package. When building EPSHome via PlatformIO, the build manager will automatically fetch this framework, merge it with your local application code (e.g., overriding the weak `app_setup` and `app_loop` symbols), and statically link the payload.

### Developer Workflow (Working locally without releases)

When doing active development on the framework, you can override the platform configuration locally to use the raw git repository instead of the bundled release.

In your `platformio.ini`, you can force PlatformIO to use your local development version of the framework using a symlink, or a specific git branch:

**Using a local directory (symlink):**
```ini
platform_packages =
    framework-sg2000-rtos @ symlink:///path/to/local/framework-sg2000-rtos
```

**Using a specific git branch (e.g., `dev`):**
```ini
platform_packages =
    framework-sg2000-rtos @ git+https://github.com/smlight-smhub/framework-sg2000-rtos.git#dev
```
*Note: If using the `git+https` method during development, ensure you have git installed and run `git submodule update --init --recursive` manually inside the PIO cache directory if you run into missing submodule errors.*

---

*(C) 2026 SMLIGHT All rights reserved.*
