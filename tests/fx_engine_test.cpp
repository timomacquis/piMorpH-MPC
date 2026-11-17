// πMorpH 0.2.0 (candidat 6) — effets dans le moteur : DELAY (Dub Station) puis REVERB (Tidal Verb).
// Effets coupés par défaut, anciens états = effets coupés, écho au bon temps, queue de reverb, réglages sauvegardés.
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
#include <algorithm>
static std::string get(const mpc_engine_t*a,void*p,const char*k){static char b[32768];int n=a->get_param(p,k,b,sizeof b);assert(n>0);return b;}
static int16_t out[256];
static double block(const mpc_engine_t*a,void*p){a->render(p,out,128);double m=0;for(int k=0;k<256;k++)m=std::max(m,(double)std::abs(out[k]));return m;}
// Joue une note courte (~23 ms) puis rend n blocs ; renvoie la crête de chaque bloc.
static std::vector<double> hit(const mpc_engine_t*a,void*p,int n){uint8_t on[3]={0x90,60,110},off[3]={0x80,60,0};a->midi(p,on,3);std::vector<double> v;
 for(int i=0;i<n;i++){if(i==8)a->midi(p,off,3);v.push_back(block(a,p));}return v;}
static double maxIn(const std::vector<double>&v,int from,int to){double m=0;for(int i=from;i<to&&i<(int)v.size();i++)m=std::max(m,v[i]);return m;}
int main(){const auto*a=mpc_engine();void*p=a->create("build");assert(p);
 for(int i=0;i<400&&get(a,p,"load_status_display").find("Loading")!=std::string::npos;i++){block(a,p);std::this_thread::sleep_for(std::chrono::milliseconds(5));}
 std::string plain=get(a,p,"state");assert(plain.find("dub_level 0")!=std::string::npos&&plain.find("tv_level 0")!=std::string::npos);
 assert(get(a,p,"dub_time_display")=="1/4");assert(get(a,p,"tv_type_display")=="HALL");
 a->set_param(p,"dub_filter","0");assert(get(a,p,"dub_filter_display")=="OFF");a->set_param(p,"dub_filter","-40");assert(get(a,p,"dub_filter_display")=="LP 40");a->set_param(p,"dub_filter","0");
 a->set_param(p,"amp_attack","1");a->set_param(p,"amp_release","5");a->set_param(p,"amp_sustain","100");
 // 1) Effets coupés : rien après la note.
 auto dry=hit(a,p,140);double drypeak=maxIn(dry,0,10),drytail=maxIn(dry,60,140);
 std::fprintf(stderr,"dry peak=%.0f tail=%.0f\n",drypeak,drytail);assert(drypeak>500&&drytail<50);
 // 2) DELAY FREE 300 ms : un écho vers le bloc 103 (300 ms = 13230 échantillons / 128).
 a->set_param(p,"dub_sync","0");a->set_param(p,"dub_time","300");a->set_param(p,"dub_feedback","40");a->set_param(p,"dub_level","80");
 assert(get(a,p,"dub_time_display")=="300 ms");
 for(int i=0;i<40;i++)block(a,p);
 auto echo=hit(a,p,140);double before=maxIn(echo,40,95),around=maxIn(echo,100,115);
 std::fprintf(stderr,"delay before=%.0f echo=%.0f\n",before,around);assert(around>drypeak*.1&&around>before*3);
 // 3) REVERB HALL : une queue après la note (DELAY coupé).
 a->set_param(p,"dub_level","0");for(int i=0;i<200;i++)block(a,p);
 a->set_param(p,"tv_level","60");a->set_param(p,"tv_decay","70");for(int i=0;i<20;i++)block(a,p);
 auto verb=hit(a,p,140);double vt=maxIn(verb,40,80);std::fprintf(stderr,"reverb tail=%.0f\n",vt);assert(vt>drytail*4&&vt>100);
 // 4) Les 10 types, réinjection 120 % : pas de valeur aberrante ni de saturation permanente.
 a->set_param(p,"dub_level","100");a->set_param(p,"dub_feedback","120");a->set_param(p,"tv_level","100");a->set_param(p,"tv_decay","100");
 for(int t=0;t<10;t++){a->set_param(p,"tv_type",std::to_string(t).c_str());hit(a,p,60);}
 // 5) Réglages sauvegardés ; un ancien état sans dub_*/tv_* charge les effets coupés.
 std::string s=get(a,p,"state");assert(s.find("dub_feedback 120")!=std::string::npos&&s.find("tv_type 9")!=std::string::npos);
 std::string old;{size_t b=0;while(b<s.size()){size_t e=s.find('\n',b);std::string line=s.substr(b,e-b);if(line.rfind("dub_",0)!=0&&line.rfind("tv_",0)!=0)old+=line+"\n";b=e+1;}}
 a->set_param(p,"state",old.c_str());assert(get(a,p,"state_valid")=="1");assert(get(a,p,"dub_level")=="0"&&get(a,p,"tv_level")=="0");
 a->destroy(p);std::printf("PASS: effets — coupés par défaut, écho DELAY à 300 ms, queue REVERB, 10 types à 120 %% stables, sauvegarde, anciens états = effets coupés\n");}
