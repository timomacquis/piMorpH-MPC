// πMorpH adapter around unchanged MIT πClideaN wave functions.
#pragma once
#include <cmath>
#include <cstdint>
namespace pimorph {
using std::sin;using std::floor;using std::fabs;using std::fmax;using std::fmin;using std::pow;using std::sqrt;
static int clampi(int v,int a,int b){return v<a?a:v>b?b:v;}
static unsigned mix32(unsigned x){x^=x>>16;x*=0x7feb352du;x^=x>>15;x*=0x846ca68bu;return x^(x>>16);}
struct ModOsc{int f[18]={};int extra[4]={};};
struct ModWalk{double previous=0,next=0;int64_t cycle=0;unsigned seed=0;int ready=0;};
/* Extended control generators. Own MIT implementation, inspired by documented
   Lunar behaviours; no audio engine or hardware dependency. All loops bounded. */
static double mod_clip(double v){return fmax(-1.,fmin(1.,v));}
static double mod_draw(unsigned seed,uint64_t cycle){return mix32(seed^(unsigned)cycle^mix32((unsigned)(cycle>>32)))/4294967295.*2.-1.;}
static double mod_walk_value(ModWalk *w,const ModOsc *o,double phase,unsigned seed){
 int64_t c=(int64_t)floor(fmax(0.,phase));double p=phase-floor(phase);
 if(!w->ready||w->seed!=seed||c<w->cycle||c-w->cycle>1024){
  w->cycle=c;w->seed=seed;w->ready=1;
  w->previous=mod_draw(seed^0x715a39u,(uint64_t)c);
  w->next=w->previous;
 }
 double step=.01+.99*(o->extra[0]+100)/200.;
 /* Next target is a reflected random step, not a new full-range draw. */
 if(w->next==w->previous){double x=w->previous+step*mod_draw(seed^0x35a791u,(uint64_t)w->cycle);w->next=x>1?2-x:x< -1?-2-x:x;}
 while(w->cycle<c){w->previous=w->next;w->cycle++;double x=w->previous+step*mod_draw(seed^0x35a791u,(uint64_t)w->cycle);w->next=x>1?2-x:x< -1?-2-x:x;}
 return mod_clip(w->previous+(w->next-w->previous)*p*p*(3-2*p));
}
static double mod_loop_value(const ModOsc *o,double phase,unsigned seed){
 int length=clampi(1+(int)floor(31.*(o->extra[0]+100)/200.),1,32);
 uint64_t c=(uint64_t)floor(fmax(0.,phase));unsigned pos=(unsigned)(c%(unsigned)length);
 double evolution=(o->extra[1]+100)/200.;
 /* Each position evolves on its own deterministic schedule. At zero, the
    exact sequence repeats forever. No frame-rate-dependent random draws. */
 uint64_t generation=c/(unsigned)length;
 unsigned salt=mix32(seed^pos^0x5de891u);
 uint64_t epoch=(uint64_t)floor(generation*evolution+salt/4294967296.);
 return mod_draw(seed^mix32(pos+0x79a31u),epoch);
}
static double mod_bounce_value(const ModOsc *o,double p){
 double restitution=.25+.65*(o->extra[1]+100)/200.;
 double gravity=.25+3.75*(o->extra[0]+100)/200.;
 p*=sqrt(gravity);if(p>=1.)return -1.;
 double first=(1.-restitution)/(1.-pow(restitution,8.));
 double start=0.,width=first,height=1.;
 for(int k=0;k<8;k++){
  if(p<start+width||k==7){double x=fmax(0.,fmin(1.,(p-start)/width));return 2*height*fmax(0.,4*x*(1-x))-1.;}
  start+=width;width*=restitution;height*=restitution*restitution;
 }
 return -1.;
}
static double mod_shape(const ModOsc *o,double phase,unsigned seed){double cycle=floor(phase),p=phase-cycle;
 switch(o->f[1]){case 0:return sin(6.283185307179586*p);case 1:return 1-4*fabs(p-.5);case 2:return 2*p-1;case 3:return 1-2*p;case 4:return p<(o->extra[0]+100)/200.?1:-1;case 5:return (mix32(seed^(unsigned)(int64_t)cycle)/4294967295.)*2-1;case 6:{double a=(mix32(seed^(unsigned)(int64_t)cycle)/4294967295.)*2-1,b=(mix32(seed^(unsigned)((int64_t)cycle+1))/4294967295.)*2-1;return a+(b-a)*p*p*(3-2*p);}case 8:{ModWalk w={0};return mod_walk_value(&w,o,phase,seed);}
 case 9:return mod_loop_value(o,phase,seed);
 case 10:return mod_bounce_value(o,p);
 default:{double z=p*8;int k=(int)z;return (o->f[10+k%8]+(o->f[10+(k+1)%8]-o->f[10+k%8])*(z-k))/100.;}}
}

struct Lfo{ModOsc spec;ModWalk walk;double phase=0;unsigned seed=0x1933u;Lfo(){spec.extra[1]=-100;for(int k=0;k<8;k++)spec.f[10+k]=(int)std::lround(100*sin(k*6.283185307179586/8));}
 double sample(double step){double value=spec.f[1]==8?mod_walk_value(&walk,&spec,phase,seed):mod_shape(&spec,phase,seed);phase+=step;return value;}
};
}
