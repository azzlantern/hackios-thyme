# SM8250 UEFI and BootShim

These are the boot artifacts used by the experimental Xiaomi Mi 10S (`thyme`) port. The firmware image targets
the closely related `alioth` Snapdragon 870 platform; it is not a thyme-specific upstream build.

## `SILICIUM_UEFI.fd`

- Size: 3,145,728 bytes.
- SHA-256: `f51bd28a2c7a345fdec16c0e62e777d936812365b7b6edd82a1adadf1c4c01ca`.
- Measured CRC-32: `A41F8E04`.
- The previous handover reported CRC-32 `35BA206A`; that value does not match this checked-in file.
- Candidate source recorded by the handover: `Mu-alioth-2.img` in the `UEFI-Images.zip` asset of
  [Project-Silicium/Mu-Silicium v4.0](https://github.com/Project-Silicium/Mu-Silicium/releases/tag/v4.0).
  The release page confirms v4.0 was published on 2026-10-01 and lists four assets, but the asset filename,
  archive contents, and extraction have not been independently verified against this local binary.

The following is a proposed reproduction procedure, not a verified build recipe. It is retained so a maintainer
can compare the source asset with the local file before making stronger provenance or redistribution claims:

```sh
curl -L -o /tmp/UEFI-Images.zip \
  https://github.com/Project-Silicium/Mu-Silicium/releases/download/v4.0/UEFI-Images.zip
mkdir -p /tmp/alioth
7z e -y /tmp/UEFI-Images.zip Mu-alioth-2.img -o/tmp/alioth

python3 - <<'PY'
import struct
import zlib

with open('/tmp/alioth/Mu-alioth-2.img', 'rb') as f:
    header = f.read(2048)
    magic, kernel_size = struct.unpack('<8sI', header[:12])
    assert magic == b'ANDROID!'
    f.seek(2048)
    kernel = f.read(kernel_size)

image = zlib.decompressobj(31).decompress(kernel)
firmware = image[0x70:0x70 + 0x300000]
assert len(firmware) == 0x300000
with open('/tmp/SILICIUM_UEFI.fd', 'wb') as f:
    f.write(firmware)
PY

sha256sum /tmp/SILICIUM_UEFI.fd
```

Only if that SHA-256 matches the local value above should the claimed derivation be treated as reproduced.

## `BootShim.bin`

- Size: 176 bytes.
- SHA-256: `310db2f5a44c9726b44e7560f7e0b644c030fcf47a66004b7a3de6ca1f2d78f9`.
- The corresponding source is [`shim/BootShim.S`](../../shim/BootShim.S). It is based on Mu-Silicium's BootShim
  and includes HackiOS's XNUP payload relocation change from [`uefi/mu-silicium.patch`](../../uefi/mu-silicium.patch).

The recorded local build command is:

```sh
aarch64-linux-gnu-gcc -nostdlib -Wl,-Ttext=0x0 \
  -DFD_BASE=0x9FC00000 -DFD_SIZE=0x300000 -DPAYLOAD_BASE=0xF0000000 \
  -DREQUIRES_KERNEL_HEADER=1 shim/BootShim.S -o /tmp/BootShim.elf
llvm-objcopy -O binary /tmp/BootShim.elf prebuilt/uefi_sm8250/BootShim.bin
```

## License and redistribution notes

The upstream Mu-Silicium repository identifies its code as BSD-2-Clause. A copy of its root `LICENSE` is kept in
[`LICENSE.Mu-Silicium`](LICENSE.Mu-Silicium), including the notices required for binary redistribution. This
documents the upstream project's license; it does not establish that the specific local firmware image has been
reproduced from that release asset or that every component inside the binary has the same terms. Verify the asset
and its contents before claiming the binary's exact provenance or redistribution status.

The local `.guids.csv` and `.report.txt` files are analysis output and are excluded from Git.
