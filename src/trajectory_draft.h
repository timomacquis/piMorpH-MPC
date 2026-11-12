// Low-cost gesture preview; same position function, fewer cells and no vector fit.
#pragma once
#include "trajectory_preview.h"
namespace pimorph {
constexpr int DraftCols=32,DraftRows=16,DraftWidth=96,DraftHeight=64,DraftTiles=512;
struct TrajectoryDraft{std::array<uint16_t,DraftTiles> cells{};Point start{},selected{};};
inline TrajectoryDraft trajectoryDraft(const Patch&p,int selected){
 Synth s;s.patch=p;TrajectoryDraft out;out.start=s.position(0);out.selected=pathAnchor(p,std::clamp(selected,0,11));
 auto pixel=[&](int x,int y){if(x<0||x>=96||y<0||y>=64)return;out.cells[y/4*32+x/3]|=uint16_t(1u<<((y%4)*3+x%3));};
 auto line=[&](Point a,Point b){int x=(int)std::lround(bound(a.x,0,1)*95),y=(int)std::lround((1-bound(a.y,0,1))*63),ex=(int)std::lround(bound(b.x,0,1)*95),ey=(int)std::lround((1-bound(b.y,0,1))*63);int dx=std::abs(ex-x),sx=x<ex?1:-1,dy=-std::abs(ey-y),sy=y<ey?1:-1,e=dx+dy;for(int i=0;i<162;i++){pixel(x,y);if(x==ex&&y==ey)break;int z=2*e;if(z>=dy){e+=dy;x+=sx;}if(z<=dx){e+=dx;y+=sy;}}};
 double path=240*p.pathBars/p.bpm,orbit=240*p.orbitBars/p.bpm,duration=p.mode==0?orbit:path*(p.mode==3?2:1);
 int count=std::clamp((int)std::ceil(duration/std::max(.0001,std::min(path,orbit)))*std::max(2,p.pointCount)*8,128,4096);
 Point previous=out.start;for(int i=1;i<=count;i++){Point next=s.position(duration*i/count);line(previous,next);previous=next;}
 return out;
}
}
