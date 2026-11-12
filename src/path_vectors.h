// Single-stroke vector cache with a complete raster fallback for complex crossings.
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>
namespace pimorph {
constexpr int PathVectorPoints=130,PathVectorCount=8385,PathMaskBase=8386,PathFrames=73922,PathMaxCode=PathFrames-1;
struct Vec2{double x,y;};
struct PathCellGeometry {
 struct Segment{float ax,ay,bx,by;};std::array<Segment,64> segments;int count=0;bool complex=false;
 void reset(){count=0;complex=false;}
 void add(Vec2 a,Vec2 b){if(count>=64){complex=true;return;}segments[count++]={(float)a.x,(float)a.y,(float)b.x,(float)b.y};}
};
inline bool clipPathSegment(Vec2 a,Vec2 b,double x,double y,Vec2&aa,Vec2&bb){
 double dx=b.x-a.x,dy=b.y-a.y,lo=0,hi=1;
 double p[]={-dx,dx,-dy,dy},q[]={a.x-x,x+9-a.x,a.y-y,y+12-a.y};
 for(int k=0;k<4;k++){if(std::abs(p[k])<1e-12){if(q[k]<0)return false;}else{double t=q[k]/p[k];if(p[k]<0)lo=std::max(lo,t);else hi=std::min(hi,t);if(lo>hi)return false;}}
 if(hi-lo<1e-10)return false;aa={a.x+lo*dx-x,a.y+lo*dy-y};bb={a.x+hi*dx-x,a.y+hi*dy-y};return true;
}
inline uint32_t pathVectorCode(const PathCellGeometry&cell){
 if(cell.complex||cell.count<1)return 0;double mx=0,my=0,xx=0,xy=0,yy=0;int n=cell.count*2;
 for(int i=0;i<cell.count;i++){auto&s=cell.segments[i];for(auto p:{Vec2{s.ax,s.ay},Vec2{s.bx,s.by}}){mx+=p.x;my+=p.y;xx+=p.x*p.x;xy+=p.x*p.y;yy+=p.y*p.y;}}
 mx/=n;my/=n;xx=xx/n-mx*mx;xy=xy/n-mx*my;yy=yy/n-my*my;
 double angle=.5*std::atan2(2*xy,xx-yy),dx=std::cos(angle),dy=std::sin(angle),low=1000,high=-1000;
 std::array<std::array<double,2>,64> intervals;
 for(int i=0;i<cell.count;i++){auto&s=cell.segments[i];int end=0;for(auto p:{Vec2{s.ax,s.ay},Vec2{s.bx,s.by}}){double x=p.x-mx,y=p.y-my;if(std::abs(x*dy-y*dx)>.45)return 0;double d=x*dx+y*dy;low=std::min(low,d);high=std::max(high,d);intervals[i][end++]=d;}if(intervals[i][0]>intervals[i][1])std::swap(intervals[i][0],intervals[i][1]);}
 if(high-low<.6)return 0;std::sort(intervals.begin(),intervals.begin()+cell.count);double covered=intervals[0][1];for(int i=1;i<cell.count;i++){if(intervals[i][0]>covered+.75)return 0;covered=std::max(covered,intervals[i][1]);}
 auto quant=[](double x,double y){int a=(int)std::lround(std::clamp(x,0.,9.)),b=(int)std::lround(std::clamp(y,0.,12.));return a*13+b;};
 int a=quant(mx+low*dx,my+low*dy),b=quant(mx+high*dx,my+high*dy);if(a==b)return 0;if(a>b)std::swap(a,b);
 return 1+a*(2*PathVectorPoints-a-1)/2+(b-a-1);
}
}
