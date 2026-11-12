# Custom Wavetable — IMPORT · 0.1.2

In SPACE, press the **36 px IMPORT button** below the display, beside the current WAV filename. The old ONE PASS caption is removed.

**DISKS** opens `/media`. **USER** opens `/sdcard`. Use **UP**, **PREV** and **NEXT** to navigate; tap a folder to enter it. Tap a WAV once to validate it without changing the current sound. **VALID** appears in green. Tap the same WAV again: **LOADING** stays green until loading completes and the panel closes. Errors appear in red as **ERROR : ...**, leaving the current table active. The panel preserves the current screen behind a dimmed veil; its opener is yellow and its buttons are 36 px high.

Supported: **WAV PCM 16-bit, PCM 32-bit or float32**, mono or stereo, with **1–256 waveforms of 512, 1024 or 2048 samples each** when valid cycle metadata describes the file. Metadata is honored first: a 64 × 2048 table is not reinterpreted as 256 × 512. Without metadata, the established 256-waveform export convention is used when possible; other unambiguous partitions are accepted. Conflicting metadata, partial cycles and ambiguous lengths are rejected instead of trimming audio or guessing a different layout. Stereo is averaged to mono. Values are converted to the internal wavetable representation and the complete table is peak-normalized, preserving relative levels between frames.

The WAV sample-rate field does not determine wavetable oscillator pitch. Valid sample rates, including 44.1 and 48 kHz, are accepted without audio resampling: cycle geometry is unchanged.

Import replaces only the table. Envelope, filter, motion, LFO and modulation settings remain. Reading, validation, conversion and graphical preparation run on the worker. A validated table is cached for the second tap and reused if its file size and modification time remain unchanged. Changing folders/pages or closing the panel clears the selection. The current table stays available while loading, or if the new file is missing, unsupported or silent. A successful table change uses the existing voice-reset behavior.

Save using the native PRESET menu or save the project. The original absolute WAV path is retained, not the WAV contents. Keep the file at that location when reopening or moving a project; samples are not automatically collected beside a preset. The source WAV is never modified and importing does not add an entry to the built-in factory list.

The browser shows up to 512 matching files/folders per directory and reports this limit when reached. Dot-files are hidden.

The installed candidate was validated on Force Gen1. The public 0.1.2 package retains the same engine and skin. Broader device qualification and Gen2 support are not claimed.
