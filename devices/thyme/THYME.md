# `devices/thyme` — Xiaomi Mi 10S

A device directory for the HackiOS XNU tree: the Mi 10S (`thyme`, M2102J2SC, SM8250-AC /
SD870, Adreno 650) port. It follows the layout of `devices/xiaomi-mi-10` (the Mi 10 `umi`
port) and keeps that port's code, because the two phones share the blocks that matter:

| Block | Why the Mi 10 code works on the Mi 10S |
|---|---|
| Touch | ST FTS at I²C 0x49, SDA 36 / SCL 37 / RESET 38 / IRQ 39 — the values `umi_touch.c` hardcodes |
| GPU | Adreno 650; `hakios.adreno.c` picks the A650 path from the `a650_*` firmware names |
| Display | 1080×2340, framebuffer at 0x9C000000, command-mode panel |
| SoC | kona: GIC 0x17A00000/0x17A60000, 8 cores, `io_base` 0x00400000 |

## What is different from the Mi 10 port

1. **Device tree** — `prebuilt/device-tree.dtb` is a real Mi 10S DTB (933 590 bytes):
   `/model` = `Qualcomm Technologies, Inc. xiaomi thyme`, `compatible` `qcom,kona-mtp`,
   touch at `/soc/i2c@a94000/fts@49` (`compatible = st,fts`, `st_fts_j2s_default.ftb`),
   `fts,x-max` 1080 / `fts,y-max` 2340.
2. **GPU zap window** — the Mi 10S DTB puts `pil_gpu_region` at **0x8c41a000**, the Mi 10 at
   0x8681a000. `overlays/xnu/files/bsd/dev/hakios.adreno.c` here carries
   `ZAP_MEMORY_THYME 0x8c41a000`. A wrong address makes TrustZone refuse the zap image and
   the run hangs in the GPU steps (recover with a cold boot, power key 10–15 s).
3. **Command line** — `boot.args`: `bootcpu=7 cpus=7` leaves one core for the loader, so its
   USB link stays up during the session (logs and screenshots). Add `wifi_mac=<address>`
   from HyperOS (`/sys/class/net/wlan0/address`) or the Wi-Fi scan comes back empty.
4. **UEFI package** — the Mi 10 port used `umiPkg`. For the Mi 10S build the package from its
   own `xbl` (`uefi/extract.py`); `extract.py` exits early on a `TzDxeLA` hash mismatch, in
   which case the `RawFiles` have to be copied by hand.
5. **TE / display calibration** — `hakios.display.c` in this port calibrates the command-mode
   TE from the panel's line rate (the Mi 10S counters do not wrap per frame, so a wrap-based
   measurement does not work).

Everything else — driver names, boot args (`umi_touch=1`, `umi_keys=1`), `DEVICE=umi` for the
userland build, the `XNUP` payload container — stays as in the Mi 10 port on purpose. The
kernel reads no board name; each driver is enabled by its boot argument.

## Build and run

```sh
./build-loader.sh                  # EFI loader
./build-kernel.sh                  # kernel — the prebuilt one has no touch driver
DEVICE=umi ./build-userland.sh     # WiFi firmware is gated on DEVICE=umi
./build-wifi.sh                    # ath11k + firmware
./run.sh                           # DTB/UEFI/loader/kernel/payload via the loader's own USB link
```

`run.sh` expects the UEFI FD at `Mu-Silicium/Build/umiPkg/...`; to reuse the SM8250 image
already in this repository instead:

```sh
UEFI_FD=$PWD/devices/thyme/prebuilt/SILICIUM_UEFI.fd ./run.sh
```

## Traps (all reproduced on hardware)

- The command line must not carry `rd=` — the loader adds `rd=md0` itself.
- `fw-gpu/` goes next to the tree root, not under `userland/`.
- The cctools `ld64` has no TAPI support: link with the `lld` wrapper (`lld-clang-wrapper`,
  and strip CRLF if it came from a Windows checkout).
- The USB cable must be in a USB 3.0 port; a 2.0 port makes the loader fail to enumerate.

## Provenance

The device tree and the verified command line come from the `thyme-port` archive
(`dtb/thyme.dtb`, `docs/thyme-移植总结.md`, 2026-10-25, M1–M6 verified on hardware). The
driver code comes from the Mi 10 port. `docs/thyme-port-plan.md` and
`docs/thyme-port-summary.md` keep that archive's notes.
