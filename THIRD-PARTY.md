# Third-party sources and licenses

πMorpH is distributed under GPL-3.0-or-later. Component licenses and notices remain applicable.

- PolyWerk by Devko: wavetable loader and mipmap construction, MIT, under `third_party/PolyWerk`.
- MS20XXL by L’Cronx: HP/LP filter DSP, GPL-3.0-or-later, under `third_party/MS20XXL`. No hardware-emulation fidelity claim is made.
- Overcast by Padbangers: envelope implementation adapted for independent per-voice envelopes, MIT, under `third_party/overcast`.
- πClideaN by Timo: LFO shapes and bounded random generators, adapted in `src/lfo.h`, MIT; license in `licenses/piClideaN-MIT.txt`.
- MPC VST wrapper: MIT, locally customized for native programs, saved state, bounded notifications, Q-Link mapping and cached graphics. Source and inherited provenance under `third_party/mpc-wrapper`; no claim of byte identity with upstream.
- Titillium Web Regular: SIL OFL 1.1, font and license under `reference/fonts`.
- Open MPC release tools: MIT, pinned under `third_party/open-mpc-release`.

## Factory wavetables

The 0.1.1 bank contains 100 original multi-frame wavetable files supplied for inclusion in the factory bank with redistribution permission. Format: mono WAV, PCM 16-bit, 44.1 kHz, 1024 samples per waveform, 256 waveforms per table. Original PCM is preserved; frame-size metadata is added for the loader. The bank includes 100 sound patches plus INIT. Velvet Field incorporates the edited Force preset.

The Adventure Kid bank belongs to release 0.1.0 and is not included in the 0.1.1 factory bank. Its historical license and provenance notices are retained for traceability; they do not assign CC0 to the new factory wavetables.

GCC 10 runtime libraries are linked statically under the GCC Runtime Library Exception. The runtime binary does not require a separate device libstdc++ installation.
