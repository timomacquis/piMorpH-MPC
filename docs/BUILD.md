# Building πMorpH 0.1.2

Target: Force Gen1, ARMv7 hard-float. The x86_64 build is for offline tests.

Requirements: Python 3, Pillow, GCC/G++ C++17, and an ARMv7 cross-toolchain. `scripts/prepare_toolchain.py` prepares the GCC 10 C++ sysroot under `build/gen1-sysroot`; set `PIMORPH_SYSROOT` to the root containing ARMv7 GCC/G++ 12 and its host libraries. No compiler/sysroot is shipped in the source archive.

```sh
python3 tools/prepare_tables.py
python3 scripts/prepare_toolchain.py
python3 scripts/build.py
python3 scripts/build_skin.py
python3 scripts/test_native.py
python3 tests/factory_sounds_test.py
python3 tests/import_test.py
python3 scripts/package_release.py
```

The 100 original multi-frame wavetables are included under `Wavetables`. `presets/factory-bank.json` is the authoritative bank order and complete sound-state definition, including the edited Velvet Field preset. Generation preserves these states rather than rerandomizing them.

`params.lock` preserves parameter indices. Do not force regeneration or reorder existing controls. Factory INIT is appended after the 100 table-based programs.

Packaging uses the pinned Open MPC release tools and validates the portable catalog format. It does not access or install on a device.

Custom Wavetable uses the existing background worker. Import/browser parameters are append-only (4029 total); `import_phase` controls the native status colours. `src/import_wavetable.h` validates cycle layout and the optional loader hint enables stereo averaging without changing factory-table decoding. Run `tests/import_format_test.cpp` with ASan/UBSan for malformed-header/stereo-cancellation cases; `tests/import_wrapper_test.c` covers real native VST chunk persistence.
