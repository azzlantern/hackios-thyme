# HackiOS on Xiaomi Mi 10S (`thyme`)

A device package that adds the Xiaomi Mi 10S (M2102J2SC, SM8250-AC / SD870, Adreno 650) to
the HackiOS XNU tree — the project that boots Apple's open-source XNU kernel
(`xnu-8796.121.2`, the kernel of iOS 16.5) on Android phone hardware and runs the unmodified
iOS 16.5 user space on top of it.

## Status

Experimental research software, verified on **one** phone in RAM sessions. It is not a ROM
and not a daily driver.

| Milestone | Evidence |
|---|---|
| UEFI + loader | `xnuloader v0.5 / display 1080x2340 at 9c000000` |
| XNU | Darwin 22.5.0, 7 cores via PSCI, `bsd_init` complete |
| iOS 16.5 SpringBoard | `finished startup after 12.30s`, 137 processes |
| Touch | `UMI FTS: poll=24375 events=2211 errors=0 irq=1` |
| GPU | `THE GPU STAYS ON`, GMU 587 MHz, `Mesa 25.0.7; FD650` |
| Wi-Fi | PCIe → amss20 → QMI → radio registered, networks visible in Settings |

Not working, in order of how much they matter:

- **Charging** under the ported kernel: the phone only discharges, so a session ends with the
  battery. This is the blocker between this port and anything daily.
- **Android recovery** is not established, and the loader can only write its diagnostic log to
  `logdump`. There is no path back from a wrong write to the phone's storage.
- **Bluetooth**: QCA6390's BT half is on SLIMbus, where upstream has nothing to build on. The
  reconnaissance (hardware, DT nodes, references) is in `docs/thyme-port-plan.md`.
- **The GPU is not handed back**: rendering works (row above), but after a run the GPU and
  TrustZone keep their state and the loader has no reset for it, so the *next* run hangs in
  the GPU steps and needs a cold boot (power key 10–15 s) to clear it. The Mi 10 Ultra port
  met the same wall.

## Acknowledgements

- **OOSEX GANG** — the HackiOS project this is built on: the XNU patches, the EFI loader, the
  UEFI port and the Wi-Fi compatibility layer ([upstream](https://github.com/oosexgang/hackios-xnu-kernel)).
- **JopangJovan** — `hackios-xnu-kernel-umi` / `hackios-xnu-userland-umi`, the Mi 10 device
  package the driver overlay here is derived from (ST FTS touch, Adreno 650, rpmh, display).
- **DBC1349** — the Mi 10 Ultra (`cas`) port, the first kona phone where touch, GPU and Wi-Fi
  worked on this code base.
- **chenbiwei520-alt** — the Mi 10S bring-up of 2026-10-25 (M1–M6 in one day), whose archive
  supplied the device tree, the zap-window correction and the verified command line;
  `docs/thyme-port-*.md` are that archive's notes.
- **Project Silicium** — Mu-Silicium, the EDK2 UEFI port the boot image is built from.

## Layout

| Path | What |
|---|---|
| `devices/thyme/` | the port: device tree, driver overlay, loader source, UEFI package, build scripts. Start at [devices/thyme/THYME.md](devices/thyme/THYME.md) |
| `docs/thyme-port-plan.md` | the porting plan (Chinese) |
| `docs/thyme-port-summary.md` | the result on hardware, milestone by milestone (Chinese) |
| `legacy/` | the first attempt (2026-10-24): an overlay on the older HackiOS tree with its own ST FTS driver. Superseded by `devices/thyme/`; kept for its measurements and its SM8250 UEFI artifacts |
| `AGENTS.md` | conventions for contributors and coding agents: the facts not to "fix", the commands, and what cannot be checked without a phone |

This repository is a **device package, not a kernel tree**: `devices/thyme` is applied onto a
checkout of the HackiOS tree (see Build). It does not build on its own.

## Build

Point `THYME_ROOT` at the HackiOS checkout:

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

## Safety

The loader keeps everything in RAM and the only write it makes is its diagnostic log; the
system image is uploaded again after every reboot. Keep it that way unless you have a way to
restore the partitions you are about to write — `fastboot fetch` does not work on this phone
and the recovery path out of a wiped partition is unknown. The full list of traps that cost
time on hardware is in `devices/thyme/THYME.md`.

## Provenance

The port follows the Mi 10 (`umi`) port: same SoC family, same ST FTS touch controller and
pins, same Adreno 650, same panel geometry. The device tree and the verified command line
come from the 2026-10-25 bring-up (the `thyme-port` archive, summarised in
`docs/thyme-port-summary.md`); the driver code is the Mi 10 port's.

## Licenses

- No iOS files are in this repository. `ios165/fetch.sh` in the tree this package is applied
  to downloads the iOS 16.5 IPSW from Apple's CDN.
- The XNU changes are a derivative work of XNU and are distributed under APSL 2.0, like XNU;
  `LICENSE` is that license. Everything this repository adds — scripts, documents, `AGENTS.md`
  — is under the same license unless a file says otherwise.
- `legacy/prebuilt/uefi_sm8250/` holds the SM8250 UEFI and BootShim used by the first
  attempt, with the upstream notice and local hashes; see its `README.md` before
  redistributing.
- Neither `legacy/` nor `devices/thyme/` contains Apple or vendor firmware: the phone's own
  firmware is taken from the phone it runs on.
- HackiOS is an independent research project; it is not affiliated with, authorised or
  endorsed by Apple Inc. iOS, iPhone and Apple are trademarks of Apple Inc.
