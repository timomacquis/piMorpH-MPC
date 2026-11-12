#include "../src/trajectory_draft.h"
#include <cassert>
#include <iostream>
using namespace pimorph;
static bool pixel(const PathRaster& p,int x,int y){return p[y/PathBitsY*PathCols+x/PathBitsX]&(1u<<((y%PathBitsY)*PathBitsX+x%PathBitsX));}
static bool contains(const PathRaster& p,Point a){int x=(int)std::lround(bound(a.x,0,1)*(PathWidth-1)),y=(int)std::lround((1-bound(a.y,0,1))*(PathHeight-1));return pixel(p,x,y);}
int main(){
 PathCellGeometry straight;straight.add({0,0},{9,12});assert(pathVectorCode(straight)==129);
 PathCellGeometry crossed;crossed.add({0,0},{9,12});crossed.add({0,12},{9,0});assert(pathVectorCode(crossed)==0);
 PathCellGeometry gap;gap.add({0,5},{2,5});gap.add({7,5},{9,5});assert(pathVectorCode(gap)==0);
 PathRaster raster{};pathLine(raster,{0,0},{1,1});pathLine(raster,{0,1},{1,0});pathLine(raster,{.5,0},{.5,1});pathLine(raster,{0,.5},{1,.5});
 assert(contains(raster,{0,0})&&contains(raster,{1,1})&&contains(raster,{0,1})&&contains(raster,{1,0}));
 Patch inactive;inactive.points[11]={1,0};auto selected=trajectoryPreview(inactive,11);assert(selected.selected.x==pathAnchor(inactive,11).x&&selected.selected.y==pathAnchor(inactive,11).y);
 Patch p;Synth s;s.patch=p;auto base=trajectoryPreview(p,3);assert(contains(base.cells,s.position(0)));assert(std::abs(base.start.x-s.position(0).x)<1e-12);assert(base.cells!=trajectoryPreview([&]{auto q=p;q.orbitWidth=.25;return q;}(),3).cells);
 p.mode=0;p.orbitWidth=p.orbitHeight=0;auto fixed=trajectoryPreview(p,0);int pixels=0;for(auto mask:fixed.cells)pixels+=__builtin_popcount(mask);assert(pixels==1);
 for(int mode=0;mode<4;mode++)for(double rate:{.0625,4.})for(double depth:{0.,.65,1.}){p=Patch{};p.mode=mode;p.pathBars=rate;p.orbitBars=4;p.pathDepth=depth;auto r=trajectoryPreview(p,11);assert(r.start.x>=0&&r.start.x<=1&&r.start.y>=0&&r.start.y<=1);assert(contains(r.cells,r.start));assert(r.cells==trajectoryPreview(p,11).cells);for(auto code:r.codes)assert(code<=PathMaxCode);}
 p=Patch{};p.mode=1;p.orbitWidth=p.orbitHeight=0;s.patch=p;auto one=trajectoryPreview(p,2);assert(contains(one.cells,s.position(240*p.pathBars/p.bpm)));
 auto draft=trajectoryDraft(p,2);assert(draft.cells==trajectoryDraft(p,2).cells);assert(std::abs(draft.start.x-one.start.x)<1e-12);for(auto mask:draft.cells)assert(mask<=4095);
 std::cout<<"PASS: crossing/vertical/endpoints retained without layer limits; actual Synth position modes/rates/depth, bounded and deterministic. Not native render or audio validation.\n";
}
