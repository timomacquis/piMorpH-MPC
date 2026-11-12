#include "../src/lfo_preview.h"
#include <cassert>
#include <iostream>
#include <set>
using namespace pimorph;
int main(){
 Lfo l;auto a=lfoPreview(l,1,0,0),b=lfoPreview(l,1,0,.25),silent=lfoPreview(l,0,0,0);
 assert(a!=b);for(int i=0;i<96;i++){assert(a[i]>=0&&a[i]<4096);if(i<95)assert(a[i]%64==a[i+1]/64);assert(silent[i]==32*64+32);}
 double error=0;for(int i=0;i<=96;i++){int y=i==96?a[95]%64:a[i]/64;double ideal=(1-sin(i*4*6.283185307179586/96))*.5*95;error=std::max(error,fabs(y*95./63-ideal));}assert(error<=.76);
 for(int shape=0;shape<11;shape++){l.spec.f[1]=shape;for(double phase:{0.,.2,.999})for(double depth:{0.,.5,1.})for(double offset:{-1.,0.,1.}){auto c=lfoPreview(l,depth,offset,phase);for(int v:c)assert(v>=0&&v<4096);assert(c==lfoPreview(l,depth,offset,phase));}}
 // Random previews span several successive cycles, periodic forms show four cycles.
 for(int shape=0;shape<11;shape++)assert(lfoPreviewCycles(shape)==((shape==5||shape==6||shape==8||shape==9)?8:4));
 l.spec.f[1]=5;auto held=lfoPreview(l,1,0,0);std::set<int> plateaus;for(int i=0;i<96;i+=12)plateaus.insert(held[i]/64);assert(plateaus.size()>=4);
 l.spec.f[1]=8;auto walk=lfoPreview(l,1,0,0);assert(walk!=lfoPreview(l,1,0,.5));
 l.spec.f[1]=7;for(int&v:l.spec.f)v=0;l.spec.f[1]=7;auto flat=lfoPreview(l,1,0,0);l.spec.f[10]=100;assert(flat!=lfoPreview(l,1,0,0));
 std::cout<<"PASS: continuous endpoints, phase/depth/offset, custom anchors, 11 shapes bounded/reproducible; sine endpoint error "<<error<<" px. Not hardware/performance proof.\n";
}
