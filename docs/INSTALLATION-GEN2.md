# πMorpH 0.1.2 — Gen2 ARM64

**Compiled and checked on PC; not tested on Gen2 hardware.** The Force Gen1 ARM32 package remains unchanged.

Same synthesis/import sources, parameter indices, 100 factory tables and 100 sounds + INIT. The small Image padding previously used for πClideaN is applied for Gen2 rendering. No device compatibility, CPU figure or installation/uninstallation validation is claimed.

Download `piMorpH-0.1.2-mpc-aarch64.zip` from release v0.1.2; follow its included Gen2 installation guide. Keep external custom WAV files at their saved paths. Please report device model/OS, audio, native presets, Q-Links, Custom Wavetable import and the SPACE/MOTION, filter, envelope and LFO graphics.

PC tests passed: sanitized native engine and actual wrapper; 101 factory programs, MIDI/state/instance isolation, bounded graphics notifications; PCM16/PCM32/float32 import, browser, state restore and malformed/missing-file fallback. ELF64 AArch64 and imported GLIBC versions are checked (maximum 2.34). The portable catalogue package and payload hashes pass. Six pre-existing tall-filmstrip warnings remain; Gen2 rendering is not verified.

## Rebuild

Prepare the normal Gen1 build and receipt first, retaining the public ARMv7 ZIP in `dist/`. Use GCC/G++ 12 AArch64 in `PIMORPH_AARCH64_SYSROOT`, Python with Pillow, then run `scripts/prepare_gen2_compat.py` and `scripts/build_aarch64.py`. `PIMORPH_AARCH64_COMPAT` chooses the extracted GCC10/glibc2.31 compatibility runtime; pinned downloads and hashes are in `reference/GEN2-TOOLCHAIN.json`. C++ and libgcc are linked statically. No system package is changed and no device is contacted.
