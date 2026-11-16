# Validation — πMorpH 0.1.3

The 0.1.2-perf-candidate.4 build (display and wrapper changes only) was installed on Force Gen1 and judged by the author to deserve release. The 0.1.3 release is rebuilt from the same sources with only the version number changed; a short device check of the release package is recommended before publication.

PC checks (x86_64, ASan/UBSan) pass: native engine and actual wrapper tests, 101 factory programs, state round-trip, notification budgets (ordinary values ≤ 2 per block; FILTER curve ≤ 8 per block; other heavy graphics spaced by 256 frames), selected-point ring and point-dot formulas, skin references and assets. An A/B render (3 notes, path modes, orbit, XY sweep, point edit, filter changes; 3,000 blocks) is byte-identical before and after the display changes.

Import checks from 0.1.2 are retained unchanged (formats, metadata, two-tap loading, fallbacks, external-path restoration).

This records an author-reported device use, not exhaustive device qualification, an install/uninstall certification or Gen2 compatibility.

---

# Validation — πMorpH 0.1.2

The installed 0.1.2-candidate.2 was confirmed on Force Gen1. The public release is a packaging/documentation promotion with identical engine, native skin, factory tables and program states. The current device capture shows a custom WAV loaded in SPACE.

PC checks pass for the 18 combinations of PCM16/PCM32/float32, mono/stereo and 512/1024/2048 samples per waveform; valid 64 × 2048 cycle metadata; two-tap validation/loading; unchanged sound during selection; missing, invalid and silent-file fallback; browser navigation and cancellation; and external-path restoration through real VST chunks. All 101 factory programs pass state/render checks. Native and wrapper checks retain bounded graphics notifications.

This records an author-reported device smoke test, not exhaustive device qualification, an install/uninstall certification or Gen2 compatibility. Existing catalog verification status is unchanged.
