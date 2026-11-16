// PERF-CANDIDATE.2: engine publishes effective positions of path points (pdot*), outside saved musical state.
extern "C" {
#include "../third_party/mpc-wrapper/wrapper/engine.h"
}
#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <thread>
#include <chrono>
static std::string get(const mpc_engine_t*a,void*p,const char*k){char b[32768];int n=a->get_param(p,k,b,sizeof b);assert(n>0);return b;}
static double num(const mpc_engine_t*a,void*p,const char*k){return std::stod(get(a,p,k));}
static void settle(const mpc_engine_t*a,void*p){int16_t out[256];for(int i=0;i<400;i++){a->render(p,out,128);std::this_thread::sleep_for(std::chrono::milliseconds(1));}}
static void check(const mpc_engine_t*a,void*p,double X,double Y,double D,int count,const double px[],const double py[]){
 for(int i=0;i<12;i++){char k[32];
  snprintf(k,sizeof k,"pdot%d_on",i);double on=num(a,p,k);assert(on==(i<count?1:0));
  if(i>=count)continue;
  double ex=std::fmin(1,std::fmax(0,X/100+(px[i]/100-.5)*D/100))*100,ey=std::fmin(1,std::fmax(0,Y/100+(py[i]/100-.5)*D/100))*100;
  snprintf(k,sizeof k,"pdot%d_x",i);assert(std::fabs(num(a,p,k)-ex)<1e-6);
  snprintf(k,sizeof k,"pdot%d_y",i);assert(std::fabs(num(a,p,k)-ey)<1e-6);}
}
int main(){const auto*a=mpc_engine();void*p=a->create("build");assert(p);settle(a,p);
 double px[12],py[12];int count=4;double X=30,Y=70,D=80;
 a->set_param(p,"x","30");a->set_param(p,"y","70");a->set_param(p,"path_depth","80");a->set_param(p,"point_count","4");
 for(int i=0;i<12;i++){px[i]=5+i*8;py[i]=95-i*7;char k[32],v[32];snprintf(v,sizeof v,"%g",px[i]);snprintf(k,sizeof k,"point%d_x",i);a->set_param(p,k,v);snprintf(v,sizeof v,"%g",py[i]);snprintf(k,sizeof k,"point%d_y",i);a->set_param(p,k,v);}
 settle(a,p);count=(int)num(a,p,"point_count");check(a,p,X,Y,D,count,px,py);
 a->set_param(p,"x","90");a->set_param(p,"path_depth","100");settle(a,p);check(a,p,90,Y,100,count,px,py);   // clamping at the edge
 std::string state=get(a,p,"state");assert(state.find("pdot")==std::string::npos);                       // never saved
 a->destroy(p);puts("PASS: path point markers follow X/Y/depth/count, clamp to the view and stay out of saved state");}
