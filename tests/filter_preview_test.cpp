#include "../src/filter_preview.h"
#include <cassert>
#include <iostream>
using namespace pimorph;
int main(){
 assert(clippedLine(-48,48)==LineIncoming+32*7+3);
 assert(clippedLine(48,144)==LineOutgoing+448+32*7+3);
 assert(clippedLine(-48,144)==LineCrossing+2*7+5);
 assert(clippedLine(-100,-1)==LineBlank);
 for(int a=-400;a<=400;a+=7)for(int b=-400;b<=400;b+=11){int code=clippedLine(a,b);assert(code>=0&&code<=LineBlank);}
 double worst=0;
 for(bool high:{false,true})for(int model:{0,1})for(double peak:{0.,.4})for(int bin:{19,186,1486}){
  double hz=44100.*bin/8192.,phase=0,cs=0,sn=0;ms20::Ms20Hpf hp;ms20::Ms20Lpf lp;hp.set(1200,peak,model);lp.set(1200,peak,model);
  constexpr double amp=.001;for(int i=0;i<40960;i++){double angle=i*6.283185307179586*bin/8192.;float input=amp*std::sin(angle);double out=high?hp.tick(input):lp.tick(input);if(i>=32768){cs+=out*std::cos(angle);sn+=out*std::sin(angle);}}
  double measured=2*std::hypot(cs,sn)/8192/amp,predicted=std::abs(linearFilter(high,1200,peak,model).response(hz));double rel=std::abs(measured-predicted)/std::max(.001,predicted);worst=std::max(worst,rel);assert(rel<.06);
 }
 auto bypass=filterPreview(80,.1,6000,.15,0,1),plain=filterPreview(80,0,6000,0,2,1),res=filterPreview(80,.6,6000,.6,2,1);
 assert(plain!=res);assert(plain!=filterPreview(800,0,1000,0,2,1));assert(plain!=filterPreview(80,0,6000,0,2,2));
 for(int c=0;c<3;c++)for(int row=0;row<3;row++)for(int i=1;i<96;i++)assert(bypass[(c*3+row)*96+i]==bypass[(c*3+row)*96]);
 for(int model=0;model<3;model++)for(double peak:{0.,.5,1.})for(double cutoff:{20.,80.,15000.}){auto a=filterPreview(cutoff,peak,cutoff,peak,model,16);for(int v:a)assert(v>=0&&v<=LineBlank);}
 std::cout<<"PASS: small-signal response against actual HP/LP tick, both models; worst relative error "<<worst<<"; cutoff/resonance/drive and bounds. No nonlinear/live voice response claim.\n";
}
