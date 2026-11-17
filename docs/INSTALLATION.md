# Installation and removal

πMorpH installs the way Akai's plugin SDK defines it: **one folder inside `VST2 Plugins`**. There is no installer script, nothing goes into `Synths`, and nothing is written to MPC.settings. The MPC finds the plugin by scanning `VST2 Plugins` when its app starts.

## Requirements

- MPC OS **3.10.1 or later**.
- **Preferences → Enhancements (Beta) → Enable Enhancements** and **External Plugin Support** switched on.
- Gen1 / Force (ARM32): the `-vst2-armv7.zip` package. Gen2 (ARM64): the `-vst2-aarch64.zip` package, see [Gen2 guide](INSTALLATION-GEN2.md) — not tested on Gen2 hardware.
- A drive that allows executable plugins. The internal User Memory always does; removable drives may be mounted `noexec`.

## Install or update

1. Save your project.
2. Download the ZIP for your device and check it against its `.sha256` file.
3. Unzip it. It contains the folder **`Timo - VST - πMorpH`** and `INSTALL.txt`.
4. Copy that folder into **`VST2 Plugins`** at the root of a drive. On the internal User Memory this is `/sdcard/VST2 Plugins/`. When updating, replace the previous folder entirely.
5. Restart the MPC app (or the device). πMorpH appears in the plugin browser under **External Plugins**.

Only the direct children of `VST2 Plugins` are examined; do not nest the folder deeper and do not rename it. The folder name, `plugin.json` and the binary `pimorph.so` form one plugin identity.

## Remove

Save your project, delete the folder `VST2 Plugins/Timo - VST - πMorpH`, then restart MPC. Projects, samples and your own presets (`Plugin Presets`) are separate and are not touched.

## Moving from an older catalogue package

Earlier releases were also distributed as catalogue packages whose `install.sh` copied the plugin into `Synths` and registered it in MPC.settings. If you used one of them, remove that copy with its own `uninstall.sh` (same `-t` destination) before installing this folder, otherwise MPC lists the plugin twice. The UID and parameters are unchanged, so existing projects reopen with this version.

## Presets

The 100 factory sounds + INIT are native programs built into the binary: choose them from the PRESET arrow. Your own saved presets live in the MPC user `Plugin Presets` folder. Custom wavetables keep their absolute path: keep the WAV file where it was imported from.
