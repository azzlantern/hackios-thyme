# Xiaomi Mi 10S (`thyme`) status

The `thyme` port is experimental. It has reached iOS SpringBoard on one Xiaomi Mi 10S (M2102J2SC), but the ST FTS touchscreen driver remains unvalidated and touch did not work in the latest test. Do not describe this as a usable ROM or promise Android recovery.

## Latest device test

Tested on 2026-10-24 using the RAM boot path and the checked-in XNU kernel and loader artifacts:

- The phone reached SpringBoard.
- The driver enabled PM8150A `ldoc1` at 1.8 V, the controller ACKed I²C address `0x49`, and the controller reported firmware `0x0124`.
- The controller emitted startup events, but no touch reports followed. The driver stayed alive for the 240-second session with `reports=0` and `INT=1`; the user confirmed taps still had no effect.
- `xl stop` returned the phone to the resident loader. This test did not reboot Android. Earlier experiments had unreliable Android return behavior, so Android recovery remains unverified.

The log from this test is local under `logs/` and is not part of the source release.

## What a source release needs

The port is a set of changes to this repository, not a standalone image. Preserve these parts together:

- Device profile and boot integration: `common/devices/thyme.conf`, `common/build.sh`, `common/lib.sh`, `common/run.sh`, `mkxnuimg.py`, `xl`, and the `platform_thyme` entry in `xnuloader/loader.c`.
- XNU implementation and build support: the changes under `xnu/` (including the unfinished `redwood_goodix.c` driver), `xnu-deps/hostinc/`, `xnu-deps/setup.sh`, and `xnu-env.sh`.
- Loader and UEFI source changes: `xnuloader/loader.c`, `uefi/apply.sh`, `uefi/mu-silicium.patch`, and the related `uefi/files/` content.
- Boot artifacts: `prebuilt/mach.development.bcm2837`, `prebuilt/loader.efi`, and `prebuilt/uefi_sm8250/{BootShim.bin,SILICIUM_UEFI.fd}`. The loader binary must include `platform_thyme`; the older loader did not, so the checked-in prebuilt loader was refreshed from the tested build.
- `prebuilt/uefi_sm8250/README.md` and `LICENSE.Mu-Silicium` record the UEFI binary provenance claim and the upstream license text.

`common/devices/thyme.platform.c` is a reference fragment for the loader platform entry. The build uses the actual entry in `xnuloader/loader.c`.

## Inputs kept on the builder's machine

The following are intentionally not bundled:

- A device tree blob for the phone. Set `THYME_DTB=/path/to/thyme.dtb`; this repository does not include one.
- The iOS IPSW and extracted files. `ios165/fetch.sh` obtains the IPSW from Apple's CDN on the builder's machine.
- The userland checkout and generated `userland/root.apfs` and `userland/system.apfs` images. `common/setup.sh` prepares the userland checkout and `userland/build.sh` builds the images locally.
- Phone logs, extracted vendor firmware, and local UEFI analysis reports.

Do not add the APFS images, the device DTB, phone-specific firmware, `logs/`, `ios165/`, or analysis reports to the repository.

## Build and run

On a clean checkout, prepare the userland and iOS inputs, obtain a thyme DTB, then run:

```sh
common/setup.sh
ios165/fetch.sh
THYME_DTB=/path/to/thyme.dtb common/build.sh thyme
THYME_DTB=/path/to/thyme.dtb common/run.sh thyme 240
```

The repository currently contains a prebuilt kernel and loader for the tested port. Rebuilding the kernel requires the XNU build dependencies documented in `xnu/README.md`; rebuilding the EFI loader requires `clang` and `lld-link`. The full workflow has not yet been reproduced from a clean checkout, so the commands above are documented entry points, not a clean-build guarantee.

The run path needs both APFS images: `system.apfs` is uploaded as the loader's resident RAM image, and `root.apfs` is included in the XNU payload as the second image. When using `xl` manually, leave `XL_RAMDISK` at its default so `root.apfs` is included. Omitting it prevents the system from reaching SpringBoard.

## Storage behavior

The UEFI/loader and XNU kernel are started with `fastboot boot`, and the APFS images are transferred to RAM. The loader writes its diagnostic log to the dedicated `logdump` partition. The recorded XNU command line did not include `ufs_write=1`, which gates XNU UFS writes. The known persistent write in this workflow is the loader log; other storage write paths have not been verified.

## UEFI binary provenance

The SM8250 UEFI image and BootShim are described in [`prebuilt/uefi_sm8250/README.md`](../prebuilt/uefi_sm8250/README.md). The upstream v4.0 release and upstream BSD-2-Clause license are verifiable, but the specific local firmware extraction has not been reproduced from the release asset. Keep that distinction visible until the original asset is extracted and its result matches the local SHA-256.
