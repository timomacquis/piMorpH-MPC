# πMorpH 0.1.2-candidate.1 — IMPORT

Local Force Gen1 candidate; not installed or published.

- IMPORT button in SPACE with a compact, dimmed-background disk browser and 36 px panel buttons.
- PCM16, PCM32 and float32 WAV; mono or stereo; 256 waveforms of 512, 1024 or 2048 samples each.
- Missing cycle metadata is inferred from PCM frame count. Conflicting metadata and incomplete tables are rejected.
- Stereo is averaged to mono, decoded to the internal representation and peak-normalized in memory. The WAV sample-rate field does not alter oscillator pitch; no audio resampling is applied.
- Import preserves sound parameters and the source file. Saved presets/projects retain the absolute external path. Keep that file at the same location.
- Loading and browser scans run on the existing worker. A failed import retains the current table. A successful change retains the existing voice-reset behavior.
- Existing 4012 parameter indices are unchanged; 15 import/browser parameters are appended (4027 total).
- The native bank stays at 100 sounds + INIT. Existing regular-image padding and graphics notification limits are retained.

PC validation covers import formats, stereo cancellation, state persistence, failure fallback, browser navigation, latest-request wins, original factory sounds and native engine/wrapper sanitizer checks. Device UI and audio validation remain pending.

---

# πMorpH 0.1.1

## Skin compatibility update

The 0.1.1 package was refreshed to pad all undersized regular images, including 50 static knob labels and 24 additional images used by LFO titles, on/off controls, cycle indicators and MOD row numbers. Their visible pixels, text positions, presets and engine binary are unchanged. This addresses undersized-image rendering compatibility; offline image and package checks pass, while confirmation on the Force is pending.

## New native preset bank

- 100 original multi-frame wavetable sound patches, plus INIT.
- Replaces the Adventure Kid bank assembled from individual single-cycle waveforms with dedicated multi-frame tables for scanning and morphing.
- Mono WAV, PCM 16-bit, 44.1 kHz; 1024 samples per waveform, 256 waveforms per table.
- Velvet Field is preset 1, with its edited settings recovered from the Force.
- The seven presets preceding Velvet Field in the private audition bank are omitted. Retained patches keep their names and sound settings.
- No synthesis or graphical-engine changes; Force Gen1 / ARMv7 only.

## Existing projects

This is a replacement factory bank, not a transparent migration. Projects referencing Adventure Kid table filenames require the previous 0.1.0 package and tables. Keep that package before updating.

## Validation

The previous 107-patch audition variant was installed on the author's Force. This exact 100-patch package is checked offline; installation and musical validation of this exact package remain pending. No new Verified status or CPU measurement is claimed.

---

# πMorpH 0.1.0

First public Force Gen1 / ARMv7 release.

- Eight-voice XY wavetable synthesis with a per-voice path and orbit.
- Independent amplitude/filter envelopes, MS-style HP/LP filters, eleven LFO shapes and eight modulation routes.
- Precise cached graphical previews, compact panels over a dimmed current screen, yellow opening selectors and 36 px panel choices.
- 294 Adventure Kid wavetable sounds plus INIT in the native PRESET menu.
- French and English manuals with native Force screenshots.

The waveforms are sourced from Adventure Kid's CC0 collection and its documented Animoog conversion. Credits and original notices are included. No private-bank or commercial instrument factory-table extractions are shipped.

PC engine/wrapper sanitizer checks, state roundtrips and all 295 factory renders pass. Earlier engine/skin versions were tried on the author's Force. Exact-release installation status is recorded separately in VALIDATION.md. No Verified catalog status, exhaustive device qualification, or Gen2 support is claimed.

Earlier private presets reference different tables. Keep their original private instrument and bank for those projects; this public bank is not a transparent replacement for those filenames.
