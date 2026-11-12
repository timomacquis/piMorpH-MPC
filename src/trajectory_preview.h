// Static path/orbit plan from Synth::position, rasterized in a shared compact tile grid.
#pragma once
#include "synth.h"
#include "line_codes.h"
#include "path_vectors.h"
#include <memory>
#include <array>
#include <cstdint>
namespace pimorph {
constexpr int PathCols=64,PathRows=32,PathBitsX=4,PathBitsY=4;
constexpr int PathWidth=PathCols*PathBitsX,PathHeight=PathRows*PathBitsY,PathTiles=PathCols*PathRows;
using PathRaster=std::array<uint16_t,PathTiles>;
struct TrajectoryScratch{std::array<PathCellGeometry,PathTiles> geometry;void reset(){for(auto&c:geometry)c.reset();}};
struct TrajectoryPreview{PathRaster cells{};std::array<uint32_t,PathTiles> codes{};std::array<uint8_t,PathRows> activeRows{};Point start{},selected{};};
inline void pathPixel(PathRaster& a,int x,int y){
 if(x<0||x>=PathWidth||y<0||y>=PathHeight)return;
 int tx=x/PathBitsX,ty=y/PathBitsY,bit=(y%PathBitsY)*PathBitsX+x%PathBitsX;
 a[ty*PathCols+tx]|=uint16_t(1u<<bit);
}
inline void pathLine(PathRaster& p,Point a,Point b){
 int x=(int)std::lround(bound(a.x,0,1)*(PathWidth-1)),y=(int)std::lround((1-bound(a.y,0,1))*(PathHeight-1));
 int ex=(int)std::lround(bound(b.x,0,1)*(PathWidth-1)),ey=(int)std::lround((1-bound(b.y,0,1))*(PathHeight-1));
 int dx=std::abs(ex-x),sx=x<ex?1:-1,dy=-std::abs(ey-y),sy=y<ey?1:-1,error=dx+dy;
 for(int n=0;n<PathWidth+PathHeight+1;n++){
  pathPixel(p,x,y);if(x==ex&&y==ey)break;int e=2*error;if(e>=dy){error+=dy;x+=sx;}if(e<=dx){error+=dx;y+=sy;}
 }
}
inline Point pathAnchor(const Patch&p,int i){return {bound(p.x+(p.points[i].x-.5)*p.pathDepth,0,1),bound(p.y+(p.points[i].y-.5)*p.pathDepth,0,1)};}
inline TrajectoryPreview trajectoryPreview(const Patch&patch,int selected,TrajectoryScratch* scratch=nullptr){
 std::unique_ptr<TrajectoryScratch> own;if(!scratch){own=std::make_unique<TrajectoryScratch>();scratch=own.get();}scratch->reset();
 Synth projection;projection.patch=patch;TrajectoryPreview out;out.start=projection.position(0);out.selected=pathAnchor(patch,std::clamp(selected,0,11));
 double path=240*patch.pathBars/patch.bpm,orbit=240*patch.orbitBars/patch.bpm;
 double duration=patch.mode==0?orbit:path*(patch.mode==3?2:1);
 int count=std::clamp((int)std::ceil(duration/std::max(.0001,std::min(path,orbit)))*std::max(2,patch.pointCount)*16,256,8192);
 Point previous=out.start;
 for(int k=1;k<=count;k++){Point next=projection.position(duration*k/count);pathLine(out.cells,previous,next);
  Vec2 a={previous.x*575,(1-previous.y)*383},b={next.x*575,(1-next.y)*383};
  int x0=std::clamp((int)(std::min(a.x,b.x)/9),0,PathCols-1),x1=std::clamp((int)(std::max(a.x,b.x)/9),0,PathCols-1),y0=std::clamp((int)(std::min(a.y,b.y)/12),0,PathRows-1),y1=std::clamp((int)(std::max(a.y,b.y)/12),0,PathRows-1);
  for(int ty=y0;ty<=y1;ty++)for(int tx=x0;tx<=x1;tx++){Vec2 aa,bb;if(clipPathSegment(a,b,tx*9,ty*12,aa,bb))scratch->geometry[ty*PathCols+tx].add(aa,bb);}
  previous=next;}
 if(patch.mode!=0)for(int i=0;i<patch.pointCount;i++){
  auto a=pathAnchor(patch,i);int x=(int)std::lround(a.x*(PathWidth-1)),y=(int)std::lround((1-a.y)*(PathHeight-1));
  for(int dy=-1;dy<=1;dy++)for(int dx=-1;dx<=1;dx++)if(dx*dx+dy*dy<=1){pathPixel(out.cells,x+dx,y+dy);int xx=x+dx,yy=y+dy;if(xx>=0&&xx<PathWidth&&yy>=0&&yy<PathHeight)scratch->geometry[yy/PathBitsY*PathCols+xx/PathBitsX].complex=true;}
 }
 for(int i=0;i<PathTiles;i++){uint32_t code=pathVectorCode(scratch->geometry[i]);out.codes[i]=code?code:out.cells[i]?PathMaskBase+out.cells[i]:0;if(out.codes[i])out.activeRows[i/PathCols]=1;}
 return out;
}
}

namespace pimorph {
using WavePreview=std::array<int,192>;
inline WavePreview wavePreview(const pf::Wavetable* table,double px,double py){
 WavePreview out;out.fill(LineBlank);if(!table||table->frames<1)return out;
 auto&wt=*table;int cols=(int)std::ceil(std::sqrt(wt.frames)),rows=(wt.frames+cols-1)/cols;
 double x=bound(px,0,1)*(cols-1),y=bound(py,0,1)*(rows-1),fx=x-(int)x,fy=y-(int)y;int a=(int)x,b=(int)y,aa=std::min(cols-1,a+1),bb=std::min(rows-1,b+1);
 double ys[97];for(int i=0;i<=96;i++){int k=(int)std::lround(double(i)/96*(pf::kTableSize-1));double v=(wt.at(std::min(wt.frames-1,b*cols+a),0,k)*(1-fx)+wt.at(std::min(wt.frames-1,b*cols+aa),0,k)*fx)*(1-fy)+(wt.at(std::min(wt.frames-1,bb*cols+a),0,k)*(1-fx)+wt.at(std::min(wt.frames-1,bb*cols+aa),0,k)*fx)*fy;ys[i]=192-bound(v,-1,1)*80;}
 for(int row=0;row<2;row++)for(int i=0;i<96;i++)out[row*96+i]=clippedLine(ys[i]-(row+1)*96,ys[i+1]-(row+1)*96);
 return out;
}
}
