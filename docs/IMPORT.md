# Wavetable IMPORT — 0.1.2 candidate

In SPACE, press **IMPORT** beneath the display.

**DISKS** opens `/media`. **USER** opens `/sdcard`. Use **UP**, **PREV** and **NEXT** to navigate; tap a folder to enter it, or a WAV to import it. The panel preserves the current screen behind a dimmed veil; its opener is yellow and its buttons are 36 px high.

Supported: **WAV PCM 16-bit, PCM 32-bit or float32**, mono or stereo, with **256 waveforms of 512, 1024 or 2048 samples each**. Without cycle metadata, the loader derives the cycle size from the PCM sample count; conflicting metadata is rejected. Stereo is averaged to mono. Values are converted to the internal wavetable representation and the complete table is peak-normalized, preserving relative levels between frames.

The WAV sample-rate field does not determine wavetable oscillator pitch. Valid sample rates, including 44.1 and 48 kHz, are accepted without audio resampling: cycle geometry is unchanged.

Import replaces only the table. Envelope, filter, motion, LFO and modulation settings remain. Reading, conversion and graphical preparation run on the worker. The current table stays available while loading, or if the new file is missing, unsupported or silent. A successful table change uses the existing voice-reset behavior.

Save using the native PRESET menu or save the project. The original absolute WAV path is retained, not the WAV contents. Keep the file at that location when reopening or moving a project; samples are not automatically collected beside a preset. The source WAV is never modified and importing does not add an entry to the built-in factory list.

The browser shows up to 512 matching files/folders per directory and reports this limit when reached. Dot-files are hidden.

Hardware validation is pending. This candidate is not published or installed automatically.
