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
UEFI_FD=$PWD/legacy/prebuilt/uefi_sm8250/SILICIUM_UEFI.fd ./run.sh
```

## Sessions: the resident image

The system image is uploaded once per loader session, not once per run. `xl ramdisk
system.apfs` asks the loader for a window (`oem ramdisk`) and sends the image (~2.4 GB); it
stays in RAM, and every later `xl run` sends only the payload (~35 MB: kernel plus command
line) and takes seconds. `xl ramdisk none` drops the resident image.

Because it lives in RAM, a new upload is needed after a cold boot, `xl reboot`, or a fresh
`fastboot boot` of the loader. `xl` records who uploaded which file and how big it was, and
warns when a run would start against an image another working copy uploaded: a foreign image
of the right kind is harmless, a foreign one of another kind kills the run without saying
why.

Booting from a flashed image (the iOS root volume on the phone's flash, `ufs_write=1`) would
survive reboots, but it writes the phone's storage and has not been tried on this port.

## Loader

`loader/loader.c` is the loader source kept for this port: it carries `platform_thyme`
(framebuffer base 0x9C000000, GIC 0x17A00000/0x17A60000, 8 cores, small mask 0x0f). It speaks
the same resident-loader protocol the tree's `xl` drives (`oem ramdisk`, `stage`, `oem run`,
`get_staged`, `oem screenshot`), so it can replace the Mi 10 loader unchanged.
`loader/loader.umi.c` is the Mi 10 port's loader, kept for reference only — `build-loader.sh`
compiles `loader.c`.

The kernel reads no board name (`hackios-board` appears nowhere in it); each driver is
enabled by its boot argument, so the loader only has to describe the platform and pass the
command line.

Before the kernel starts, the loader checks that command line (`thyme_bootargs_ok()`): an
argument it does not know stops the run with one line naming it, instead of being passed on
silently and leaving a driver switched off. Values are checked too — `cpus=1..8`,
`bootcpu=0..7`, `gpu=`/`metal=`/`wifi=`/`fb_scan=` take `1`, `wifi_mac=` needs six hex
pairs — and `wifi=1` without `wifi_mac=` prints a warning. To try a new kernel argument
before adding it to the list, put `bootargs_loose=1` on the command line: unknown arguments
are then printed and passed through.

Known gap, and the one thing worth adding: the loader does not hand the GPU back to
TrustZone, so a run that follows a GPU run hangs in the GPU steps and needs a cold boot
(power key 10–15 s) to clear the secure state. The Mi 10 Ultra port hit the same thing and
solved it in its loader.

## RPMh rails (Wi-Fi supplies)

`umi_rpmh.c` refuses to continue when a rail it asks for is missing from the command
database (`UMI RPMH: <rail> lookup matches=0; refusing`) — that is what stopped the Mi 10
Ultra's Wi-Fi. For the Mi 10S the rails in that table match the device tree exactly:

| Supply | Rail | Regulator |
|---|---|---|
| WLAN IO | `smpa4` | pm8150_s4 |
| WLAN AON | `smpa6` | pm8150_s6 |
| WLAN digital | `smpf2` | pm8009_s2 |
| WLAN RFA1 | `smpa5` | pm8150_s5 |
| WLAN RFA2 | `smpc8` | pm8150a_s8 |
| PCIe 1.8 V | `ldoa9` | pm8150_l9 |
| PCIe 0.9 V | `ldoa5` | pm8150_l5 |

Seven of seven, `smpf2` included. If a firmware update ever changes the database, the
command to run is [tools/rpmh-rails.c](tools/rpmh-rails.c): it prints the database's VRM
entries so the table can be corrected against evidence instead of guesswork.

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
