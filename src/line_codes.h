// Shared cached line fragments: full bands, clipped ends, and band crossings.
#pragma once
#include <algorithm>
#include <cmath>
namespace pimorph {
constexpr int LineLevels=64,LineOutgoing=4096,LineIncoming=4992,LineCrossing=5888,LineBlank=5986,LineFrames=5987;
inline int lineY(double v){return (int)std::lround(std::clamp(v,0.,95.)*63/95);}
inline int clippedLine(double a,double b){
 if(std::max(a,b)<0||std::min(a,b)>96)return LineBlank;
 double dy=b-a,t0=0,t1=1;
 if(std::abs(dy)>1e-12){double u=(0-a)/dy,v=(96-a)/dy;if(u>v)std::swap(u,v);t0=std::max(0.,u);t1=std::min(1.,v);}
 if(t1<=t0)return LineBlank;
 bool in=t0>1e-10,out=t1<1-1e-10;
 int x0=(int)std::lround(t0*6),x1=(int)std::lround(t1*6);double y0=a+t0*dy,y1=a+t1*dy;
 if(in&&out)return LineCrossing+(y0>48?49:0)+x0*7+x1;
 if(out)return LineOutgoing+(y1>48?448:0)+lineY(y0)*7+x1;
 if(in)return LineIncoming+(y0>48?448:0)+lineY(y1)*7+x0;
 return lineY(y0)*64+lineY(y1);
}
}
