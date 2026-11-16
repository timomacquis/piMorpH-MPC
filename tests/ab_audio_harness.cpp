extern "C" {
#include "engine.h"
}
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
static std::string get(const mpc_engine_t*a,void*p,const char*k){char b[4096];a->get_param(p,k,b,sizeof b);return b;}
int main(int argc,char**argv){const auto*a=mpc_engine();void*p=a->create(argv[1]);int16_t out[256];FILE*f=fopen(argv[2],"wb");
 for(int w=0;w<5000&&get(a,p,"load_status_display").find(".wav")==std::string::npos;w++){a->render(p,out,128);std::this_thread::sleep_for(std::chrono::milliseconds(1));}
 // Scenario: path on, depth, orbit, filter moves, point edits, several notes.
 const char* setup[][2]={{"path_mode","2"},{"path_depth","70"},{"orbit_width","30"},{"orbit_height","20"},{"point_count","6"},{"filter_model","1"},{"hp_cutoff","120"},{"lp_cutoff","3000"},{"lp_res","40"}};
 for(auto&s:setup)a->set_param(p,s[0],s[1]);
 uint8_t n1[]={0x90,48,100},n2[]={0x90,55,90},n3[]={0x90,62,110};a->midi(p,n1,3);
 for(int b=0;b<3000;b++){
  if(b==200)a->midi(p,n2,3);if(b==400)a->midi(p,n3,3);
  if(b%50==0){char v[16];snprintf(v,sizeof v,"%d",(b/50)%100);a->set_param(p,"x",v);snprintf(v,sizeof v,"%d",(b/37)%100);a->set_param(p,"y",v);}
  if(b==800){a->set_param(p,"point_select","2");a->set_param(p,"point_x","80");a->set_param(p,"point_y","15");}
  if(b==1200){a->set_param(p,"lp_cutoff","800");a->set_param(p,"hp_res","60");a->set_param(p,"path_rate","1");}
  if(b==1800){uint8_t o[]={0x80,48,0};a->midi(p,o,3);a->set_param(p,"path_mode","3");a->set_param(p,"path_depth","100");}
  a->render(p,out,128);fwrite(out,2,256,f);
  if(b%10==0)std::this_thread::sleep_for(std::chrono::milliseconds(1));}
 fclose(f);a->destroy(p);}
