# AGENTS.md

Instructions for an AI agent (or a new contributor) working in this repository.

## What this repository is

A **device package**, not a kernel tree. It adds Xiaomi Mi 10S (`thyme`, M2102J2SC,
SM8250-AC, Adreno 650) support to HackiOS, the project that boots Apple's open-source XNU
kernel and the unmodified iOS 16.5 user space on Android phones.

| Path | Role |
|---|---|
| `devices/thyme/` | the port itself: device tree, driver overlay, loader source, UEFI package, build scripts. Read `devices/thyme/THYME.md` first |
| `docs/thyme-port-plan.md`, `docs/thyme-port-summary.md` | the porting plan and the verified result (Chinese, from the archive of the 2026-10-25 bring-up) |
| `legacy/` | the superseded 2026-10-24 attempt: an overlay on the older HackiOS tree, its own ST FTS driver, the SM8250 UEFI artifacts. Kept for its measurements and those binaries |

`devices/thyme` is applied onto a checkout of the HackiOS tree named by `THYME_ROOT`
(default: this repository, which does not contain that tree). Nothing here builds on its own.

## Ground truth — do not "improve" these

- `devices/thyme/prebuilt/device-tree.dtb` is a **real Mi 10S device tree**, 933 590 bytes,
  SHA-256 `300724c507857f55968543edbcddb767ab32ed8d3dec0b49915277469056b7d1`. It is binary:
  never edit it, never regenerate it from a guess. It parses to `/model = Qualcomm
  Technologies, Inc. xiaomi thyme`, `compatible = qcom,kona-mtp`, touch at
  `/soc/i2c@a94000/fts@49` (`st,fts`, `st_fts_j2s_default.ftb`).
- **GPU zap window is `0x8c41a000`** for this phone (its `pil_gpu_region@8c41a000`). The
  Mi 10 port uses `0x8681a000`; that value is correct there and wrong here. A wrong address
  makes TrustZone refuse the zap image and the GPU steps hang.
- **Boot-arg names stay `umi_*`** (`umi_touch=1`, `umi_keys=1`), and the userland build stays
  `DEVICE=umi`. The touch driver, the A650 GPU path and the Wi-Fi stack are shared with the
  Mi 10 port; the kernel reads no board name. Renaming them is not a cleanup — it is a
  port-wide change with no benefit.
- `devices/thyme/loader/loader.c` is the loader this port builds. `loader.umi.c` beside it is
  a **reference only** — do not compile it and do not keep it in sync.
- No Apple files and no vendor firmware belong in this repository. The phone's own firmware is
  pulled from the phone; iOS comes from Apple's CDN on the user's machine.

## Commands

```sh
cd devices/thyme
THYME_ROOT=/path/to/hackios-xnu-kernel-umi ./build-loader.sh
THYME_ROOT=... ./build-kernel.sh                 # prebuilt kernel has no touch driver
THYME_ROOT=... DEVICE=umi ./build-userland.sh    # Wi-Fi firmware is gated on DEVICE=umi
THYME_ROOT=... ./build-wifi.sh
THYME_ROOT=... ./run.sh                          # resident loader + xl
THYME_ROOT=... python3 prepare.py verify         # upstream template hashes
```

## Rules for changes

- **`boot.args` is one line of arguments.** `run.sh` strips `#` comments and collapses
  whitespace, but a multi-line argument list has bitten before: the file is passed to the
  loader, which now checks every token.
- **A new boot argument needs two edits**: `boot.args` (or wherever it is passed) and the
  `thyme_boot_args[]` list in `loader/loader.c`. The loader refuses unknown arguments before
  the kernel starts; a driver that stays off because its argument was rejected looks exactly
  like a broken driver. `bootargs_loose=1` exists for experiments, not for shipping.
- `devices/thyme/overlays/` is a **full-file overlay**: `prepare.py` copies files over the
  kernel tree, it does not apply patches. Edit the whole file, and expect its content to be
  what the build uses.
- `prepare.py verify` compares the upstream tree against `upstream-templates.sha256`. If it
  fails, the upstream tree changed or was modified — investigate; do not edit the manifest.
- Keep `THYME_ROOT` plumbing intact in the `build-*.sh` scripts and `prepare.py`; it is what
  lets this package live in its own repository.

## What can be verified here — and what cannot

This checkout has **no C compiler** (`clang`, `lld-link`, `gcc`, `tcc` are all absent) and
**no phone**. Therefore:

- do: `python3 -m py_compile` on every `.py`, `bash -n` on every `.sh`, `prepare.py verify`,
  parse the DTB instead of trusting a comment, cross-check `boot.args` against
  `thyme_boot_args[]`, count braces/parens when editing C;
- do not claim a build, a boot or a hardware result happened. Say what was checked and what
  was not — the loader change in `6dbd7a9` is a standing example: structure checked, never
  compiled.

## Do not

- `dd`, `fastboot flash`, GPT edits or any write to the phone's storage: they are destructive
  and there is no recovery path on this port.
- Delete or rewrite `legacy/` without being asked: it holds the only copy of the measured
  findings and the SM8250 UEFI/BootShim artifacts.
- Add a platform entry per board to the loader for this repository's sake: the kernel does not
  read the board name, so an extra platform only risks matching the wrong phone by its
  framebuffer address.

## Commit style

Imperative subject, no prefix tags, body says why (one paragraph is enough). Do not mention
tools or assistants in the message.
