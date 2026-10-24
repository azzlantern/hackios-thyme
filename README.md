# HackiOS `thyme` port

This repository is a small overlay for the public [HackiOS XNU kernel project](https://github.com/oosexgang/hackios-xnu-kernel). It collects the device-specific source changes and the prebuilt boot files used for the Xiaomi Mi 10S (`thyme`, Snapdragon 870).

## Status

The port booted to iOS SpringBoard on one Xiaomi Mi 10S in a 240-second RAM session. Touch still does not work: the ST FTS controller ACKed I²C and reported firmware `0x0124`, but the driver observed `reports=0` throughout the session. Android recovery has not been established. This is experimental research software, not a daily-use ROM.

The tested boot files are included in `overlay/prebuilt/`: the XNU kernel, loader, SM8250 UEFI image, and BootShim. A device tree blob and the iOS APFS images are not included; provide/build those locally as described below.

## Apply to HackiOS

The overlay was prepared against HackiOS base commit:

```text
d82c64a3a31cd27a0a0afbf6e213e467e44b1b4c
```

Use a clean checkout of that base, then copy the overlay into its root:

```sh
git clone https://github.com/oosexgang/hackios-xnu-kernel.git hackios
cd hackios
git checkout d82c64a3a31cd27a0a0afbf6e213e467e44b1b4c
cd ..
cp -a thyme-open-source/overlay/. hackios/
```

Do not apply it on top of a modified checkout. It replaces the tracked files listed in `overlay/` with the versions tested for this port.

## Build and run

Install the HackiOS build dependencies, prepare its userland checkout and iOS inputs, and obtain a thyme DTB from your own device or a source you are entitled to use:

```sh
cd hackios
common/setup.sh
ios165/fetch.sh
THYME_DTB=/path/to/thyme.dtb common/build.sh thyme
THYME_DTB=/path/to/thyme.dtb common/run.sh thyme 240
```

The run path uses both APFS images: `system.apfs` is uploaded to loader RAM and `root.apfs` is sent with the XNU payload. `common/run.sh` handles both. The full workflow has not yet been reproduced from a clean checkout.

## Device storage

The boot images are started through `fastboot boot` and the APFS images are transferred to RAM. The loader writes diagnostics to the phone's `logdump` partition. The tested XNU command line did not enable `ufs_write=1`; other storage paths are not claimed to be write-free. Read the detailed [thyme status](overlay/docs/THYME.md) before testing.

## Provenance and licenses

The overlay includes the HackiOS license and the upstream Mu-Silicium license notice. The local UEFI image has a recorded SHA-256, but its extraction from the named upstream release asset has not been reproduced; see `overlay/prebuilt/uefi_sm8250/README.md`. Do not treat the local artifact's exact provenance as verified.

The bundle deliberately excludes `thyme.dtb`, `xbl.img`, extracted phone firmware, iOS/APFS images, logs, and build output. The `SHA256SUMS` file covers the files in this bundle.
