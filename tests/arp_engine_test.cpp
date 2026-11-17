// πMorpH 0.2.0 — arpégiateur dans le moteur : notes rythmées, état sauvegardé, anciens états, ARP OFF inchangé.
extern "C" {
#include "../third_party/mpc-wrapper/wrapper/engine.h"
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
static std::string get(const mpc_engine_t*a,void*p,const char*k){static char b[32768];int n=a->get_param(p,k,b,sizeof b);assert(n>0);return b;}
int main(){const auto*a=mpc_engine();void*p=a->create("build");assert(p);int16_t out[256];
 for(int i=0;i<400&&get(a,p,"load_status_display").find("Loading")!=std::string::npos;i++){a->render(p,out,128);std::this_thread::sleep_for(std::chrono::milliseconds(5));}
 std::string plain=get(a,p,"state");assert(plain.find("arp_mode 0")!=std::string::npos);
 // Un ancien état sans les lignes arp_* reste valide et charge ARP OFF.
 std::string old;{size_t s=0;while(s<plain.size()){size_t e=plain.find('\n',s);std::string line=plain.substr(s,e-s);if(line.rfind("arp_",0)!=0)old+=line+"\n";s=e+1;}}
 a->set_param(p,"arp_mode","1");a->set_param(p,"state",old.c_str());assert(get(a,p,"state_valid")=="1");assert(get(a,p,"arp_mode")=="0");
 // ARP UP 1/16, gate 50 %, amp attack/release courts : l'enveloppe du son montre un battement régulier.
 a->set_param(p,"amp_attack","1");a->set_param(p,"amp_release","5");a->set_param(p,"amp_sustain","100");
 a->set_param(p,"arp_mode","1");a->set_param(p,"arp_rate","12");assert(get(a,p,"arp_rate_display")=="1/16");
 a->set_param(p,"state",get(a,p,"state").c_str());assert(get(a,p,"arp_mode")=="1");
 uint8_t on1[3]={0x90,60,100},on2[3]={0x90,67,100};a->midi(p,on1,3);a->midi(p,on2,3);
 std::vector<double> rms;for(int i=0;i<344;i++){a->render(p,out,128);double s=0;for(int k=0;k<256;k++)s+=out[k]*(double)out[k];rms.push_back(std::sqrt(s/256));}
 int edges=0;bool loud=false;double peak=0;for(double r:rms)peak=std::max(peak,r);for(double r:rms){bool l=r>peak*.3;if(l&&!loud)edges++;loud=l;}
 // 344 × 128 échantillons ≈ 1 s = 8 pas de 1/16 à 120 BPM.
 std::fprintf(stderr,"peak=%.0f edges=%d\n",peak,edges);assert(peak>500&&edges>=6&&edges<=10);
 uint8_t off1[3]={0x80,60,0},off2[3]={0x80,67,0};a->midi(p,off1,3);a->midi(p,off2,3);
 for(int i=0;i<100;i++)a->render(p,out,128);double tail=0;for(int k=0;k<256;k++)tail=std::max(tail,(double)std::abs(out[k]));assert(tail<50);
 a->destroy(p);std::printf("PASS: arpégiateur moteur — battement de 1/16 à 120 BPM, arrêt au relâchement, arp_* sauvegardés, anciens états = ARP OFF\n");}
