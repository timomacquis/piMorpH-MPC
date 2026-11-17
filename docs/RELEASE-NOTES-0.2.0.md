# πMorpH 0.2.0 — Arpeggiator and FX

Force Gen1 / ARM32 release, with an untested Gen2 ARM64 variant. **0.1.x projects and presets reopen with the same sound**: the new features start switched off.

- **ARP page**: native arpeggiator with 20 modes, tempo-locked RATE (bar to 1/64, triplets and dotted), REPEATS, RETRIG, LATCH (and sustain pedal), OCTAVES, GATE, CHANCE and OFFSET. Each arpeggiated note is a full πMorpH note.
- **FX page**: tape **DELAY** from *Dub Station* (SYNC/FREE, feedback to 120 %, DJ filter in the loop) followed by **REVERB** *Tidal Verb* (10 types including HALO+, ABYSS and CASCADE), in series. LEVEL 0 = off (default).
- **Live LFO dot**: a gold / violet dot follows each LFO along its curve on the LFO and SHAPE pages.
- **Colour charter**: amber when a selector leaves its neutral value (MOD SOURCE/DESTINATION, FILTER MODEL, ARP, RETRIG), blue outline for selectors without one; larger selector text; FILTER help text removed.
- 31 parameters appended (4,096 total, the host limit); existing indices are unchanged.
- **Installation follows Akai's SDK rule**: copy the plugin folder into `VST2 Plugins`. No installer script, no `Synths` folder, no MPC.settings entry. Remove an older catalogue installation with its own `uninstall.sh` first.

Third-party code (MIT): Dub Station (nachtaktiv303), CHOMPI TAPE 2.0, DaisySP, Tidal Verb (Fullban); see THIRD-PARTY.md. PC validation: sanitized engine, arpeggiator and effects tests, 101 factory sounds, skin references. The build was installed and used on Force Gen1.
