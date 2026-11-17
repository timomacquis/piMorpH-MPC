// πMorpH arpeggiator (0.2.0). GPL-3.0-or-later; original implementation by Timo.
// Controls mirror the πSyrinG ARP page (20 modes, rate, octaves, gate, chance, offset, repeats, retrig, latch);
// behaviour is re-implemented here, not copied: πSyrinG's DSP is only available as translated WebAssembly.
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace pimorph {
struct ArpSettings{int mode=0,rate=12,octaves=0,offset=8,repeats=0,retrig=0;bool latch=false;double gate=.5,chance=1;};
// Fractions of a whole note (4 beats), same order as the ARP RATE list.
inline double arpRateBeats(int i){static const double w[]={1,.75,2./3,.5,.375,1./3,.25,.1875,1./6,.125,.09375,1./12,.0625,.046875,1./24,.03125,.0234375,1./48,.015625};return 4*w[std::clamp(i,0,18)];}
inline double arpRetrigBeats(int i){static const double b[]={0,0,1,2,4,8,16};return b[std::clamp(i,0,6)];}
class Arp{
 public:
 enum Mode{Off,Up,Down,UpDown,DownUp,UpPlusDown,DownPlusUp,ThumbUp,ThumbUD,PinkyUp,PinkyUD,Converge,Diverge,ConDiverge,Played,Chord,Random,RandNoDup,RandDrift,RandOnce};
 ArpSettings s;double sr=44100,bpm=120;
 bool enabled()const{return s.mode>0;}
 // Keyboard input. Returns true when the note was taken by the arpeggiator.
 template<class Out> void keyOn(int note,double v,Out&out){if(note<0||note>127)return;
  if(s.latch&&!anyKeyDown())clearLatched(out);
  down[note]=true;vel[note]=v;when[note]=++clock;active[note]=true;changed=true;
  if(!running){start();}else if(s.retrig==1){step=0;cycles=0;}
 }
 template<class Out> void keyOff(int note,Out&out){if(note<0||note>127)return;down[note]=false;if(!s.latch&&!pedal){active[note]=false;changed=true;}if(!anyActive())stop(out);}
 template<class Out> void sustain(bool on,Out&out){pedal=on;if(!on&&!s.latch){for(int i=0;i<128;i++)if(!down[i])active[i]=false;changed=true;if(!anyActive())stop(out);}}
 template<class Out> void reset(Out&out){release(out);down.fill(false);active.fill(false);running=false;pedal=false;changed=true;}
 template<class Out> void release(Out&out){for(int i=0;i<count;i++)out.noteOff(sounding[i]);count=0;noteLeft=-1;}
 void align(double ppqNow){if(!running)return;double step=arpRateBeats(s.rate);double f=ppqNow/step;pos=(f-std::floor(f))*stepSamples();}
 // Samples until the next arpeggiator event (at most limit).
 int until(int limit)const{if(!running)return limit;double next=stepSamples()-pos;if(noteLeft>=0)next=std::min(next,noteLeft);return std::clamp((int)std::ceil(next),1,limit);}
 template<class Out> void advance(int frames,Out&out){if(!running)return;pos+=frames;sinceStart+=frames;
  if(noteLeft>=0){noteLeft-=frames;if(noteLeft<=0)release(out);}
  if(pos+1e-9>=stepSamples()){pos-=stepSamples();if(pos<0)pos=0;fire(out);}
 }
 private:
 std::array<bool,128> down{},active{};std::array<double,128> vel{};std::array<unsigned,128> when{};unsigned clock=0;bool pedal=false,running=false,changed=true;
 std::array<int,64> list{};int n=0;std::array<int,64> perm{};int last=-1,drift=0;long step=0;int cycles=0;double pos=0,sinceStart=0,noteLeft=-1;
 std::array<int,64> sounding{};int count=0;uint32_t rng=0x2545f491u;
 bool anyKeyDown()const{for(bool d:down)if(d)return true;return false;}
 bool anyActive()const{for(bool a:active)if(a)return true;return false;}
 template<class Out> void clearLatched(Out&out){bool any=false;for(int i=0;i<128;i++)if(active[i]&&!down[i]){active[i]=false;any=true;}if(any){changed=true;if(!anyActive()){release(out);running=false;}}}
 double stepSamples()const{return std::max(16.,arpRateBeats(s.rate)*60/std::max(20.,bpm)*sr);}
 uint32_t random(){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
 void start(){running=true;step=0;cycles=0;pos=0;sinceStart=0;noteLeft=-1;pendingFirst=true;}
 bool pendingFirst=false;
 template<class Out> void stop(Out&out){release(out);running=false;}
 void rebuild(){int base[128];int m=0;
  if(s.mode==Played){int idx[128];int k=0;for(int i=0;i<128;i++)if(active[i])idx[k++]=i;std::sort(idx,idx+k,[&](int a,int b){return when[a]<when[b];});for(int i=0;i<k;i++)base[m++]=idx[i];}
  else for(int i=0;i<128;i++)if(active[i])base[m++]=i;
  n=0;int oct=std::clamp(s.octaves,0,3)+1;for(int o=0;o<oct;o++)for(int i=0;i<m&&n<64;i++){int note=base[i]+12*o;if(note<=127)list[n++]=note;}
  for(int i=0;i<n;i++)perm[i]=i;for(int i=n-1;i>0;i--){int j=random()%(i+1);std::swap(perm[i],perm[j]);}
  changed=false;
 }
 int length()const{if(n<=1)return 1;switch(s.mode){case UpDown:case DownUp:return 2*n-2;case UpPlusDown:case DownPlusUp:case ConDiverge:return 2*n;case ThumbUp:case PinkyUp:return 2*(n-1);case ThumbUD:case PinkyUD:return n>2?2*(2*(n-1)-2):2;default:return n;}}
 static int bounce(int u,int m){if(m<=1)return 0;int c=2*m-2;u%=c;return u<m?u:c-u;}
 int converge(int k)const{return k%2==0?k/2:n-1-k/2;}
 int pick(long st){int L=length();int u=(int)(((st%L)+L)%L);
  switch(s.mode){
   case Up:case Played:case Chord:return u;case Down:return n-1-u;
   case UpDown:return bounce(u,n);case DownUp:return n-1-bounce(u,n);
   case UpPlusDown:return u<n?u:2*n-1-u;case DownPlusUp:return u<n?n-1-u:u-n;
   case ThumbUp:return u%2==0?0:1+(u/2)%(n-1);case PinkyUp:return u%2==0?n-1:(u/2)%(n-1);
   case ThumbUD:return u%2==0?0:1+bounce(u/2,n-1);case PinkyUD:return u%2==0?n-1:bounce(u/2,n-1);
   case Converge:return converge(u);case Diverge:return converge(n-1-u);
   case ConDiverge:return u<n?converge(u):converge(2*n-1-u);
   case Random:return (int)(random()%n);
   case RandNoDup:{int r=(int)(random()%n);if(n>1&&r==last)r=(r+1+(int)(random()%(n-1)))%n;return r;}
   case RandDrift:{int d=(int)(random()%3)-1;drift=std::clamp(drift+d,0,n-1);return drift;}
   case RandOnce:return perm[u];
  }
  return u;
 }
 template<class Out> void fire(Out&out){
  if(changed)rebuild();if(n==0){release(out);return;}
  double rt=arpRetrigBeats(s.retrig);if(rt>0&&sinceStart>=rt*60/std::max(20.,bpm)*sr){sinceStart=0;step=0;cycles=0;}
  int L=length();if(s.repeats>0&&cycles>=s.repeats){release(out);return;}
  release(out);
  long st=step+(s.offset-8);step++;if(step%L==0)cycles++;
  if(s.chance<1&&(random()%10000)/10000.>=s.chance)return;
  if(s.mode==Chord){for(int i=0;i<n&&count<64;i++)play(list[i],out);}
  else{int i=std::clamp(pick(st),0,n-1);last=i;play(list[i],out);}
  noteLeft=std::max(sr*.003,stepSamples()*std::clamp(s.gate,0.,1.)*.98);
 }
 template<class Out> void play(int note,Out&out){int src=note;while(src>127)src-=12;double v=vel[src%128]>0?vel[src%128]:.8;
  for(int o=0;o<4;o++){int k=note-12*o;if(k>=0&&k<128&&active[k]){v=vel[k];break;}}
  out.noteOn(note,v);sounding[count++]=note;}
 public:
 // First step sounds immediately when the arpeggio starts.
 template<class Out> void kick(Out&out){if(running&&pendingFirst){pendingFirst=false;pos=0;fire(out);}}
};
}
