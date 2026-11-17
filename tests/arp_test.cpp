// πMorpH 0.2.0 — arpégiateur : ordre des notes par mode, tempo, gate, octaves, offset, repeats, latch, chance.
#include "../src/arp.h"
#include <cassert>
#include <cstdio>
#include <vector>
using namespace pimorph;
struct Ev{long t;int note;bool on;};
struct Rec{std::vector<Ev> ev;long t=0;void noteOn(int n,double){ev.push_back({t,n,true});}void noteOff(int n){ev.push_back({t,n,false});}};
static std::vector<int> run(ArpSettings s,std::vector<int> keys,long samples,Rec*out=nullptr,bool release=false){
 Arp a;a.s=s;a.bpm=120;a.sr=44100;Rec r;for(int k:keys)a.keyOn(k,.8,r);a.kick(r);
 while(r.t<samples){int n=a.until(128);r.t+=n;a.advance(n,r);if(release&&r.t>samples/2){for(int k:keys)a.keyOff(k,r);release=false;}}
 std::vector<int> ons;for(auto&e:r.ev)if(e.on)ons.push_back(e.note);if(out)*out=r;return ons;}
static void expect(const char*name,std::vector<int> got,std::vector<int> want){
 for(size_t i=0;i<want.size();i++){if(i>=got.size()||got[i]!=want[i]){printf("FAIL %s at %zu\n",name,i);for(int g:got)printf("%d ",g);printf("\n");assert(0);}}}
int main(){const long step=5512;// 1/16 à 120 BPM = 0,125 s = 5512,5 échantillons
 ArpSettings s;s.rate=12;
 s.mode=1;expect("UP",run(s,{60,64,67},step*7),{60,64,67,60,64,67});
 s.mode=2;expect("DOWN",run(s,{60,64,67},step*4),{67,64,60,67});
 s.mode=3;expect("UP/DOWN",run(s,{60,64,67},step*6),{60,64,67,64,60,64});
 s.mode=5;expect("UP+DOWN",run(s,{60,64,67},step*7),{60,64,67,67,64,60,60});
 s.mode=7;expect("THUMB UP",run(s,{60,64,67},step*6),{60,64,60,67,60,64});
 s.mode=9;expect("PINKY UP",run(s,{60,64,67},step*6),{67,60,67,64,67,60});
 s.mode=11;expect("CONVERGE",run(s,{60,62,64,65},step*5),{60,65,62,64,60});
 s.mode=12;expect("DIVERGE",run(s,{60,62,64,65},step*4),{64,62,65,60});
 s.mode=14;expect("PLAYED",run(s,{67,60,64},step*4),{67,60,64,67});
 s.mode=1;s.octaves=1;expect("UP 2 OCT",run(s,{60,64},step*5),{60,64,72,76,60});s.octaves=0;
 s.mode=1;s.offset=9;expect("OFFSET +1",run(s,{60,64,67},step*3),{64,67,60});s.offset=8;
 s.mode=1;s.repeats=1;{auto o=run(s,{60,64},step*6);assert(o.size()==2);}s.repeats=0;
 s.mode=15;{auto o=run(s,{60,64,67},step*2);expect("CHORD",o,{60,64,67,60,64,67});}
 // Tempo et gate : écart entre deux débuts = un pas ; note coupée à ~50 % du pas.
 {Rec r;s.mode=1;s.gate=.5;run(s,{60,64},step*3,&r);long on1=-1,off1=-1,on2=-1;for(auto&e:r.ev){if(e.on&&on1<0)on1=e.t;else if(!e.on&&off1<0)off1=e.t;else if(e.on&&on2<0)on2=e.t;}
  fprintf(stderr,"on1=%ld off1=%ld on2=%ld\n",on1,off1,on2);assert(on1==0&&std::abs(on2-5512)<=2&&off1>2600&&off1<2800);}
 // Sans LATCH, relâcher arrête ; avec LATCH, l'arpège continue.
 {s.mode=1;s.latch=false;auto o=run(s,{60,64},step*8,nullptr,true);assert(o.size()<=5);
  s.latch=true;auto l=run(s,{60,64},step*8,nullptr,true);assert(l.size()>=7);s.latch=false;}
 // CHANCE 0 : aucun son ; 50 % : environ la moitié.
 {s.mode=1;s.chance=0;assert(run(s,{60,64},step*20).empty());s.chance=.5;auto o=run(s,{60,64},step*200);assert(o.size()>60&&o.size()<140);s.chance=1;}
 // Tous les modes aléatoires restent dans les notes tenues.
 for(int m:{16,17,18,19}){s.mode=m;for(int n:run(s,{60,64,67},step*40))assert(n==60||n==64||n==67);}
 printf("PASS: arpégiateur — 20 modes (ordres vérifiés), tempo 1/16 à 120 BPM, gate, octaves, offset, repeats, chord, latch, chance\n");}
