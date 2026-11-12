# πMorpH

A polyphonic XY wavetable synthesizer for Akai Force Gen1.

Pick a sound. Play a note. Let its timbre travel.

![πMorpH — SPACE](Cover.png)

## Play

Each voice moves through a wavetable space using a path and an elliptical orbit. Shape it with independent amplitude/filter envelopes, a high-pass → low-pass filter pair, two LFOs and eight modulation routes.

- Up to eight voices, POLY / MONO / LEGATO and glide.
- Editable paths with 2–12 points; fixed, one-shot, loop and ping-pong playback.
- Tempo-synced path/orbit timing and dual LFOs with eleven shapes.
- Native touch controls, contextual Q-Links and compact selection panels.
- Cached graphical previews, with background updates held during editing.
- **100 original multi-frame wavetable sounds + INIT**, in the native PRESET menu.
- Native preset/project state, including all musical parameters and the selected table.

## Download

[πMorpH 0.1.1 — Force Gen1 / ARMv7](https://github.com/timomacquis/piMorpH-MPC/releases/tag/v0.1.1)

Package: **piMorpH-0.1.1-mpc-armv7.zip**. Use the included standard installer after saving your project. No Gen2 package is provided.

[Français](docs/MODE-EMPLOI.md) · [English](docs/USER-MANUAL.md) · [Build](docs/BUILD.md) · [Validation](docs/VALIDATION.md).

This update replaces the previous Adventure Kid bank, assembled from single-cycle waveforms, with 100 original multi-frame wavetable patches by Greg plus INIT. Velvet Field, edited on the Force, is preset 1. Keep the 0.1.0 package for projects using its old table filenames.

## Status

Offline engine, wrapper, state, wavetable-loading and rendering tests pass. Earlier versions of the engine and skin were exercised on the author's Force; see version-specific validation notes. No new exhaustive device or Gen2 qualification is implied. Screenshots show the native interface before the bank change, with earlier preset names.

## Sources and licensing

GPL-3.0-or-later for the instrument. Original factory wavetables by Greg, included with the author's permission. Full credits and component licenses: [THIRD-PARTY.md](THIRD-PARTY.md). No ROM or commercial instrument factory-bank extractions are included.
