#include "../src/import_wavetable.h"
#include "../third_party/PolyWerk/dsp/wavetable.h"
#include <cassert>
#include <filesystem>
#include <cmath>
#include <iostream>
static void u16(std::ofstream&f,unsigned v){f.put(v&255);f.put((v>>8)&255);}static void u32(std::ofstream&f,unsigned v){u16(f,v);u16(f,v>>16);}
int main(){auto path=std::filesystem::temp_directory_path()/"pimorph-import-format-sanitized.wav";
 {std::ofstream f(path,std::ios::binary);unsigned n=512*256*4;f.write("RIFF",4);u32(f,n+36);f.write("WAVEfmt ",8);u32(f,16);u16(f,1);u16(f,2);u32(f,48000);u32(f,48000*4);u16(f,4);u16(f,16);f.write("data",4);u32(f,n);for(unsigned i=0;i<512*256;i++){int v=std::lround(12000*sin(i*6.283185307179586/512));u16(f,v);u16(f,-v);}}
 std::string error;int frame=0;assert(pimorph::inspectImport(path.string(),frame,error)&&frame==512);pf::Wavetable table;assert(!pf::loadWavetable(path.string(),table,&error,nullptr,frame)&&error=="silent");
 // Reproduce the real Vital-style RandomFFT2 header: float32, 64 x 2048.
 {std::ofstream f(path,std::ios::binary);unsigned n=2048*64*4;f.write("RIFF",4);u32(f,n+52);f.write("WAVEfmt ",8);u32(f,16);u16(f,3);u16(f,1);u32(f,44100);u32(f,44100*4);u16(f,4);u16(f,32);f.write("clm ",4);u32(f,7);f.write("<!>2048",7);f.put(0);f.write("data",4);u32(f,n);for(unsigned i=0;i<2048*64;i++){float v=.4f*sin(i*6.283185307179586/2048);unsigned raw;memcpy(&raw,&v,4);u32(f,raw);}}
 assert(pimorph::inspectImport(path.string(),frame,error)&&frame==2048);assert(pf::loadWavetable(path.string(),table,&error,nullptr,frame)&&table.frames==64);
 {std::ofstream f(path,std::ios::binary);f.write("RIFF",4);u32(f,100000);f.write("WAVEdata",8);u32(f,100000);}
 assert(!pimorph::inspectImport(path.string(),frame,error));
 {std::ofstream f(path,std::ios::binary);f<<"not a wave";}
 assert(!pimorph::inspectImport(path.string(),frame,error));std::filesystem::remove(path);
 std::cout<<"PASS: sanitized import inspection, stereo cancellation, valid 64 x 2048 Vital metadata, truncated and non-WAV rejection\n";
}
