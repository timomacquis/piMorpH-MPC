// UI-only sampled preview. Call from the existing background worker, never DSP.
#pragma once
#include "lfo.h"
#include "line_codes.h"
#include <array>
#include <algorithm>
namespace pimorph {
constexpr int PreviewColumns=96,PreviewLevels=64,PreviewWidth=576,PreviewHeight=96;
constexpr int PreviewBlank=LineBlank;
using LfoPreview=std::array<int,PreviewColumns>;
inline int lfoPreviewCycles(int shape){return shape==5||shape==6||shape==8||shape==9?8:4;}
inline LfoPreview lfoPreview(Lfo l,double depth,double offset,double phase){
 LfoPreview out{};std::array<int,PreviewColumns+1> y{};
 l.phase=std::max(0.,phase);l.walk=ModWalk{};
 for(int x=0;x<=PreviewColumns;x++){
  double v=std::clamp(l.sample(double(lfoPreviewCycles(l.spec.f[1]))/PreviewColumns)*depth+offset,-1.,1.);
  y[x]=(int)std::lround((1-v)*.5*(PreviewLevels-1));
 }
 for(int x=0;x<PreviewColumns;x++)out[x]=y[x]*PreviewLevels+y[x+1];
 return out;
}
}
