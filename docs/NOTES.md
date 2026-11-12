# Development notes

## 2026-11-12 — First public package

294 CC0 AKWF tables + INIT, separate from the earlier private bank. Real-engine state/render checks and project-specific wrapper checks pass on PC under sanitizers where applicable. The custom wrapper retains bounded parameter notifications, queued graph updates and native programs. Exact payload installation on Force Gen1 completed with a verified complete PC backup and hashes. MPC restarted normally. Listed requested; no new Verified status or CPU measurement claimed.


## 2026-11-12 — 0.1.1 bank replacement
100 original multi-frame patches + INIT. Edited Velvet Field recovered from the Force and placed first. Seven preceding audition patches removed. Names/settings of retained sounds preserved. No new DSP/UI processing. Exact-package hardware test pending.


## 2026-11-12 — 0.1.1 label image padding
Black label rectangles were observed on Force. The 50 source caption images contain their text correctly. Increase only their backing canvas to at least 6400 pixels; retain the original clipping bounds and all visible pixels. This is a compatibility candidate, not yet hardware-confirmed. Engine and factory states are unchanged.

LFO/MOD follow-up: knob labels returned on the device, while small headers/toggles/indicators remained black. Extend the same clipping-preserving padding to all regular Image components, covering 24 additional assets across pages and subpages. Native Button images and filmstrips retain their existing rendering definitions.


## 2026-11-12 — IMPORT candidate
Append-only disk import/browser controls. User-approved PCM32/float32, stereo-to-mono average and sample-rate-independent wavetable playback. External paths persist in optional PMORPH1 external field; original factory states remain unchanged. Invalid input retains the active table; source WAVs are never rewritten. New filesystem and conversion work stays on the worker. 4027 native parameters, same first 4012 indices. Hardware validation pending.


## 2026-11-12 — IMPORT candidate.2
Respect valid cycle metadata before inferring a 256-frame export. RandomFFT2 header corresponds to 64 x 2048 float32 mono and is valid. Two-tap cached validation/load workflow, red ERROR : and green VALID/LOADING. Import button 36px with 16px gap and current WAV name. Append import_phase and table_name without shifting existing controls. No hardware validation claimed.


## 2026-11-12 — 0.1.2 Custom Wavetable release
Candidate.2 confirmed on Force Gen1. Promote without changing runtime bytes. FR/EN manuals and README describe encodings, mono/stereo adaptation, cycle metadata, validation/loading colours and persistent external paths. Public package and sources updated; no restart required for this documentation promotion.
