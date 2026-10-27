# Xiaomi Mi 10S (`thyme`)

This directory is the Mi 10S device package for the HackiOS XNU tree.

- Start with [THYME.md](THYME.md): what differs from the Mi 10 port, how to build and run,
  and the traps.
- [UMI-PORT.md](UMI-PORT.md) is the original Mi 10 (`umi`) port readme this package is
  derived from; its source-isolation rules still apply.
- `overlays/` holds the XNU, userland and Wi-Fi changes applied by `prepare.py`.
- `prebuilt/device-tree.dtb` is the real Mi 10S device tree.
