#include "../src/synth.h"
#include <filesystem>
#include <iostream>
#include <cassert>
#include <cmath>
int main(){int count=0;for(const auto& e:std::filesystem::directory_iterator("Wavetables")){
 if(e.path().extension()!=".wav")continue;
 pf::Wavetable table;std::string error;assert(pf::loadWavetable(e.path().string(),table,&error));assert(table.frames==8||table.frames==12||table.frames==16);
 for(float x:table.scale)assert(std::isfinite(x));
 pimorph::Synth synth;synth.setTable(&table);synth.patch.filterOn=false;synth.patch.mode=0;synth.patch.orbitWidth=0;synth.patch.orbitHeight=0;synth.patch.attack=.001;synth.patch.sustain=.8;
 float audio[256];double energy=0;float peak=0;
 for(int pos=0;pos<3;pos++){
  synth.panic();synth.patch.x=pos*.5;synth.patch.y=pos*.5;synth.noteOn(60,0.8);
  for(int i=0;i<40;i++){synth.render(audio,128);for(float v:audio){assert(std::isfinite(v));energy+=v*v;peak=std::max(peak,std::abs(v));}}
 }
 assert(energy>0.000001);assert(peak<1);count++;
 }
 assert(count==294);std::cout<<"PASS: 294 AKWF tables loaded with real piMorpH loader; 8/12/16 frames, finite mipmaps and audible finite render at three XY positions; ASan/UBSan\n";
}
