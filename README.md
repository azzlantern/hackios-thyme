# HackiOS on Xiaomi Mi 10S (`thyme`)

A device package that adds the Xiaomi Mi 10S (M2102J2SC, SM8250-AC / SD870, Adreno 650) to
the HackiOS XNU tree, which boots Apple's open-source XNU kernel (`xnu-8796.121.2`, the
kernel of iOS 16.5) on Android phone hardware and runs the unmodified iOS 16.5 user space on
top of it.

## Layout

| Path | What |
|---|---|
| `devices/thyme/` | the port: device tree, driver overlay, loader source, UEFI package, build scripts. Start at [devices/thyme/THYME.md](devices/thyme/THYME.md) |
| `docs/thyme-port-plan.md` | the porting plan (Chinese) |
| `docs/thyme-port-summary.md` | the result on hardware, milestone by milestone (Chinese) |
| `legacy/` | the first attempt (2026-10-24): an overlay on the older HackiOS tree with its own ST FTS driver. Superseded by `devices/thyme/`; kept for its measurements and its SM8250 UEFI artifacts |

## What is verified

On one Mi 10S, 2026-10-25, in RAM sessions — nothing written except the loader's diagnostic
log:

| Milestone | Evidence |
|---|---|
| UEFI + loader | `xnuloader v0.5 / display 1080x2340 at 9c000000` |
| XNU | Darwin 22.5.0, 7 cores via PSCI, `bsd_init` complete |
| iOS 16.5 SpringBoard | `finished startup after 12.30s`, 137 processes |
| Touch | `UMI FTS: poll=24375 events=2211 errors=0 irq=1` |
| GPU | `THE GPU STAYS ON`, GMU 587 MHz, `Mesa 25.0.7; FD650` |
| Wi-Fi | PCIe → amss20 → QMI → radio registered, networks visible in Settings |

Not working: Bluetooth (QCA6390 over SLIMbus; upstream has no base — see the plan in
`docs/thyme-port-plan.md`), and the loader cannot hand the GPU back, so a run that follows a
GPU run needs a cold boot.

## Build

`devices/thyme` builds against a checkout of the HackiOS tree it is applied to; point
`THYME_ROOT` at it:

```sh
cd devices/thyme
THYME_ROOT=/path/to/hackios-xnu-kernel-umi ./build-loader.sh
THYME_ROOT=/path/to/hackios-xnu-kernel-umi ./build-kernel.sh
THYME_ROOT=/path/to/hackios-xnu-kernel-umi DEVICE=umi ./build-userland.sh
THYME_ROOT=/path/to/hackios-xnu-kernel-umi ./build-wifi.sh
THYME_ROOT=/path/to/hackios-xnu-kernel-umi ./run.sh
```

The prebuilt kernel in the tree has no touch driver, so the kernel has to be built. The Wi-Fi
firmware is gated on `DEVICE=umi` in the userland build. Add `wifi_mac=<address>` to
`devices/thyme/boot.args` (from HyperOS: `/sys/class/net/wlan0/address`), or the scan comes
back empty.

## Licenses

- No iOS files are in this repository. `ios165/fetch.sh` in the tree this package is applied
  to downloads the iOS 16.5 IPSW from Apple's CDN.
- The XNU changes are a derivative work of XNU and are distributed under APSL 2.0, like XNU;
  `LICENSE` is that license.
- `legacy/prebuilt/uefi_sm8250/` holds the SM8250 UEFI and BootShim used by the first
  attempt, with the upstream notice and local hashes; see its `README.md` before
  redistributing.
- Neither `legacy/` nor `devices/thyme/` contains Apple or vendor firmware: the phone's own
  firmware is taken from the phone it runs on.
- HackiOS is an independent research project; it is not affiliated with, authorised or
  endorsed by Apple Inc. iOS, iPhone and Apple are trademarks of Apple Inc.
