// πMorpH native DSP. GPL-3.0-or-later; sources and licences in THIRD-PARTY.md.
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include "../third_party/PolyWerk/dsp/wavetable.h"
#include "../third_party/MS20XXL/src/ms20_dsp.h"
#include "lfo.h"
namespace pimorph {
constexpr double SR=44100.0,TAU=6.283185307179586;
inline double bound(double x,double a,double b){return std::clamp(x,a,b);}
struct Point{double x,y;};
// Independent per-voice ADSR adapted from Overcast; values in seconds.
struct Env{enum Stage{Idle,Attack,Decay,Sustain,Release};Stage stage=Idle;double value=0;bool gate=false;
 double tick(bool g,bool retrigger,double a,double d,double s,double r,double dt){
  if(g&&(!gate||retrigger))stage=Attack;if(!g&&gate&&stage!=Idle)stage=Release;gate=g;
  switch(stage){case Attack:value+=dt/std::max(.001,a);if(value>=1){value=1;stage=Decay;}break;
  case Decay:value-=dt/std::max(.001,d)*(1-s);if(value<=s){value=s;stage=Sustain;}break;
  case Sustain:value=s;break;case Release:value-=dt/std::max(.001,r);if(value<=0){value=0;stage=Idle;}break;
  default:value=0;}return value;}
};
struct Patch{
 double x=.5,y=.5,pathBars=2,pathDepth=.65,orbitBars=4,orbitWidth=.06,orbitHeight=.04,orbitPhase=0,pathOffset=0;
 int mode=2;
 std::array<Point,12> points{{{.12,.54},{.54,.84},{.82,.36},{.27,.22},{.36,.79},{.86,.58},{.61,.17}}};int pointCount=7;
 double attack=.035,decay=2.4,sustain=.65,release=.5,filterAttack=.08,filterDecay=1.2,filterSustain=.2,filterRelease=.4;
 double hp=80,lp=6000,hpPeak=.1,lpPeak=.15,drive=1.0,hpEnv=0,lpEnv=1.0,keytrack=.3,bpm=120;
 int filterModel=1;bool filterOn=true;
 double lfoHz[2]={.25,.17},lfoDepth[2]={.5,.5},lfoOffset[2]={},lfoPhase[2]={};bool lfoOn[2]={false,false},lfoReset[2]={};
 int source[8]={},dest[8]={};double amount[8]={};
 double level=.65,pan=0,transpose=0,bend=0,glide=0,modwheel=0,pressure=0;int voiceLimit=8,playMode=0;
};
struct Voice{int note=-1;bool gate=false,retrigger=false;double phase=0,seconds=0,velocity=1,pan=.5,hz=440,targetHz=440,x=.5,y=.5,dx=0,dy=0,gain=0,last=0,tail=0;int fade=0;Env amp,filter;ms20::Ms20Hpf hpf;ms20::Ms20Lpf lpf;};
class Synth{
 public:
 Patch patch;std::array<Voice,8> voices;std::array<Lfo,2> lfo;const pf::Wavetable* table=nullptr;
 uint64_t samples=0;unsigned steals=0,nonFiniteResets=0;Point lastPosition{};double latestLfo[2]={};
 Synth(){lfo[1].seed^=0x6f317u;}
 std::array<bool,128> held{};std::array<unsigned,128> order{};std::array<double,128> velocities{};unsigned sequence=0;bool pedal=false;
 void setTable(const pf::Wavetable* t){panic();table=t;}
 void panic(){for(auto&v:voices)v=Voice{};held.fill(false);pedal=false;}
 void resetLfos(){for(int j=0;j<2;j++)if(patch.lfoReset[j]){lfo[j].phase=patch.lfoPhase[j];lfo[j].walk=ModWalk{};}}
 void noteOn(int note,double velocity){if(note<0||note>127||!std::isfinite(velocity))return;if(velocity<=0){noteOff(note);return;}
  held[note]=true;velocities[note]=bound(velocity,0,1);order[note]=++sequence;
  if(patch.playMode==2&&voices[0].note>=0&&voices[0].gate){auto&v=voices[0];v.note=note;v.velocity=velocities[note];return;}
  Voice* target=nullptr;int limit=std::clamp(patch.voiceLimit,1,8);
  if(patch.playMode)target=&voices[0];else{for(int i=0;i<limit;i++)if(voices[i].note<0){target=&voices[i];break;}}
  if(!target){target=&*std::max_element(voices.begin(),voices.begin()+limit,[](const Voice&a,const Voice&b){return a.seconds<b.seconds;});steals++;}
  const double previous=target->last,previousHz=target->hz;bool used=target->note>=0;
  *target=Voice{};target->note=note;target->hz=used&&patch.glide>0?previousHz:440*std::exp2((note-69+patch.transpose+patch.bend)/12.);target->gate=true;target->retrigger=true;target->velocity=velocities[note];target->pan=.5;target->x=patch.x;target->y=patch.y;
  if(used){target->tail=previous;target->fade=32;}resetLfos();
 }
 void noteOff(int note){if(note<0||note>127)return;held[note]=false;
  if(patch.playMode&&voices[0].note==note){int next=-1;for(int i=0;i<128;i++)if(held[i]&&(next<0||order[i]>order[next]))next=i;
   if(next>=0){if(patch.playMode==2){voices[0].note=next;voices[0].velocity=velocities[next];}else noteOn(next,velocities[next]);return;}}
  if(!pedal)for(auto&v:voices)if(v.note==note)v.gate=false;
 }
 void sustain(bool value){pedal=value;if(!value)for(auto&v:voices)if(v.note>=0&&!held[v.note])v.gate=false;}
 Point position(double seconds,double pathScale=1,double orbitScale=1,double widthMod=0,double heightMod=0)const{
  double x=patch.x,y=patch.y;
  if(patch.mode!=0&&patch.pointCount>1){double t=seconds*patch.bpm/(240*std::max(.03125,patch.pathBars))*pathScale+patch.pathOffset;
   if(patch.mode==1)t=std::min(t,1.);else if(patch.mode==2)t-=std::floor(t);else{t=std::fmod(t,2.);if(t>1)t=2-t;}
   int n=std::clamp(patch.pointCount,2,12);double z=t*(n-1);int a=std::min(n-2,(int)z);double f=z-a;
   x+=(patch.points[a].x*(1-f)+patch.points[a+1].x*f-.5)*patch.pathDepth;
   y+=(patch.points[a].y*(1-f)+patch.points[a+1].y*f-.5)*patch.pathDepth;
  }
  const double p=seconds*patch.bpm/(240*std::max(.03125,patch.orbitBars))*orbitScale+patch.orbitPhase;
  x+=std::cos(TAU*p)*bound(patch.orbitWidth+widthMod,0,1);y+=std::sin(TAU*p)*bound(patch.orbitHeight+heightMod,0,1);
  return {bound(x,0,1),bound(y,0,1)};
 }
 double oscillator(Voice&v){if(!table||table->frames<1)return 0;
  int cols=(int)std::ceil(std::sqrt(table->frames)),rows=(table->frames+cols-1)/cols;
  double x=bound(v.x,0,1)*(cols-1),y=bound(v.y,0,1)*(rows-1);int a=(int)x,b=(int)y;double fx=x-a,fy=y-b;
  int mip=pf::mipFor((float)(v.hz/SR)),len=pf::mipLength(mip);double z=v.phase*len;int i=(int)z;double frac=z-i;
  auto read=[&](int c,int r){int frame=std::min(table->frames-1,r*cols+c);const int16_t* q=table->get(frame,mip);return (q[i]*(1-frac)+q[i+1]*frac)*table->scale[frame];};
  int aa=std::min(cols-1,a+1),bb=std::min(rows-1,b+1);
  double out=(read(a,b)*(1-fx)+read(aa,b)*fx)*(1-fy)+(read(a,bb)*(1-fx)+read(aa,bb)*fx)*fy;
  v.phase+=v.hz/SR;v.phase-=std::floor(v.phase);return out;
 }
 void render(float* out,int frames){const double glide=patch.glide>0?1-std::exp(-1/(patch.glide*SR)):1;for(int k=0;k<frames;k++,samples++){
   if(samples%32==0)for(int j=0;j<2;j++){lfo[j].phase=std::max(0.,lfo[j].phase);latestLfo[j]=patch.lfoOn[j]?bound(lfo[j].sample(patch.lfoHz[j]*32/SR)*patch.lfoDepth[j]+patch.lfoOffset[j],-1,1):0;}
   double left=0,right=0;
   for(auto&v:voices){if(v.note<0)continue;bool update=samples%32==0||v.retrigger;
    double amp=v.amp.tick(v.gate,v.retrigger,patch.attack,patch.decay,patch.sustain,patch.release,1/SR);
    double env=v.filter.tick(v.gate,v.retrigger,patch.filterAttack,patch.filterDecay,patch.filterSustain,patch.filterRelease,1/SR);v.retrigger=false;
    if(v.amp.stage==Env::Idle){v.note=-1;continue;}
    if(update){double mod[14]={};double source[]={0,latestLfo[0],latestLfo[1],amp,env,v.velocity,(v.note-60)/60.,patch.modwheel,patch.pressure};
     for(int n=0;n<8;n++)mod[std::clamp(patch.dest[n],0,13)]+=source[std::clamp(patch.source[n],0,8)]*patch.amount[n];
     Point pos=position(v.seconds,std::exp2(bound(mod[3],-3,3)),std::exp2(bound(mod[4],-3,3)),mod[5],mod[6]);
     pos.x=bound(pos.x+mod[1],0,1);pos.y=bound(pos.y+mod[2],0,1);
     v.dx=(pos.x-v.x)/32;v.dy=(pos.y-v.y)/32;lastPosition=pos;
     double key=(v.note-60)/12.*patch.keytrack;
     v.hpf.set((float)bound(patch.hp*std::exp2(key+env*patch.hpEnv+mod[8]*4),10,15000),(float)bound(patch.hpPeak+mod[10],0,1),patch.filterModel);
     v.lpf.set((float)bound(patch.lp*std::exp2(key+env*patch.lpEnv+mod[9]*4),10,15000),(float)bound(patch.lpPeak+mod[11],0,1),patch.filterModel);
     v.targetHz=440*std::exp2(bound((v.note-69+patch.transpose+patch.bend+mod[7]*12)/12.,-10,5));
     v.gain=bound(1+mod[12],0,2);v.pan=bound(.5+patch.pan*.5+mod[13]*.5,0,1);
    }
    v.x+=v.dx;v.y+=v.dy;
    v.hz+=(v.targetHz-v.hz)*glide;v.hz=bound(v.hz,5,18000);
    double s=oscillator(v);
    if(patch.filterOn)s=v.lpf.tick(v.hpf.tick((float)(s*2.5*patch.drive)))/2.5;
    s*=amp*v.velocity*.18*v.gain*patch.level;
    if(v.fade>0){double f=v.fade/32.;s=s*(1-f)+v.tail*f;v.fade--;}
    if(!std::isfinite(s)){nonFiniteResets++;v=Voice{};continue;}v.last=s;
    left+=s*std::sqrt(1-v.pan);right+=s*std::sqrt(v.pan);v.seconds+=1/SR;
   }
   out[k*2]=ms20::fast_tanh((float)left);out[k*2+1]=ms20::fast_tanh((float)right);
  }}
};
}
