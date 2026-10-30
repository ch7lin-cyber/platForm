# CKLIB archive compatibility

## Compatibility warning

The current `libcklib.a` is a valid archive, but its object files are **ELF64 AArch64**. It is **incompatible with the LPC55S16 Cortex-M33 target** and must not be used to link LPC55S16 firmware.

Replace it with a CKLIB archive built for the intended 32-bit ARM Cortex-M33 target before performing a target firmware build. Check the archive members and object architecture with the target toolchain, then confirm by linking the firmware.

Host unit tests use CKLIB test doubles and do not link this archive.
