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

All 294 factory tables come from Adventure Kid / Kristoffer Ekstrand's AKWF collection, converted for Animoog and hosted on Animoog.org. The source AKWF collection is dedicated under CC0 1.0. The license text, pinned author-repository URL and archive hashes are under `licenses/AdventureKid`.

Animoog conversion credit: Chrisco. Hosting and concatenated-table documentation: Satri / Animoog.org. Source pages:

- https://www.adventurekid.se/akrt/waveforms/adventure-kid-waveforms/
- https://github.com/KristofferKarlAxelEkstrand/AKWF-FREE
- https://forum.moogmusic.com/t/diy-timbres-for-animoog/11924
- https://www.animoog.org/database/timbres/

Prepared WAV files retain the concatenated PCM data and add a frame-size marker specifying 1024 samples. There are 292 tables with 16 frames, one with 8 and one with 12. The factory bank includes one parameter recipe per table plus INIT. Original source notices are preserved. No Animoog factory/expansion-bank extractions or private ROMs are included. These credits do not imply endorsement by the named authors or companies.

GCC 10 runtime libraries are linked statically under the GCC Runtime Library Exception. The runtime binary does not require a separate device libstdc++ installation.
