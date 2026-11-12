// Static small-signal response of the actual filter stages. Background worker only.
#pragma once
#include "../third_party/MS20XXL/src/ms20_dsp.h"
#include "line_codes.h"
#include <array>
#include <complex>
#include <algorithm>
#include <cmath>
namespace pimorph {
using Complex=std::complex<double>;
struct LinearFilter {
 double A[3][3]{},B[3]{},C[3]{},D=0;
 Complex response(double hz)const{
  Complex m[3][4];Complex z=std::exp(Complex(0,6.283185307179586*hz/44100.));
  for(int i=0;i<3;i++){for(int j=0;j<3;j++)m[i][j]=(i==j?z:Complex(0))-A[i][j];m[i][3]=B[i];}
  for(int col=0;col<3;col++){
   int pivot=col;for(int i=col+1;i<3;i++)if(std::abs(m[i][col])>std::abs(m[pivot][col]))pivot=i;
   if(std::abs(m[pivot][col])<1e-14)return Complex(0);
   if(pivot!=col)for(int j=col;j<4;j++)std::swap(m[pivot][j],m[col][j]);
   auto v=m[col][col];for(int j=col;j<4;j++)m[col][j]/=v;
   for(int i=0;i<3;i++)if(i!=col){auto f=m[i][col];for(int j=col;j<4;j++)m[i][j]-=f*m[col][j];}
  }
  Complex result=D;for(int i=0;i<3;i++)result+=C[i]*m[i][3];return result;
 }
};
inline std::array<double,4> filterProbe(bool high,float hz,float peak,int model,int column,float value){
 std::array<double,4> out{};
 if(high){ms20::Ms20Hpf f;f.set(hz,peak,model);if(column==0)f.hp1.s=value;if(column==1)f.hp2.s=value;if(column==2)f.lp.s=value;out[3]=f.tick(column==3?value:0);out[0]=f.hp1.s;out[1]=f.hp2.s;out[2]=f.lp.s;}
 else {ms20::Ms20Lpf f;f.set(hz,peak,model);if(column==0)f.lp1.s=value;if(column==1)f.lp2.s=value;if(column==2)f.hp.s=value;out[3]=f.tick(column==3?value:0);out[0]=f.lp1.s;out[1]=f.lp2.s;out[2]=f.hp.s;}
 return out;
}
inline LinearFilter linearFilter(bool high,double hz,double peak,int model){
 LinearFilter f;constexpr float e=.001f;
 for(int col=0;col<4;col++){
  auto a=filterProbe(high,(float)hz,(float)peak,model,col,e),b=filterProbe(high,(float)hz,(float)peak,model,col,-e);
  for(int row=0;row<3;row++){double v=(a[row]-b[row])/(2*e);if(col<3)f.A[row][col]=v;else f.B[row]=v;}
  if(col<3)f.C[col]=(a[3]-b[3])/(2*e);else f.D=(a[3]-b[3])/(2*e);
 }
 return f;
}
constexpr int FilterColumns=96,FilterRows=3,FilterCurves=3,FilterCodes=FilterColumns*FilterRows*FilterCurves;
using FilterPreview=std::array<int,FilterCodes>;
inline FilterPreview filterPreview(double hp,double hpRes,double lp,double lpRes,int model,double drive){
 FilterPreview result{};result.fill(LineBlank);auto h=linearFilter(true,hp,hpRes,model-1),l=linearFilter(false,lp,lpRes,model-1);
 std::array<std::array<double,97>,3> y{};
 for(int i=0;i<=FilterColumns;i++){
  double freq=20*std::pow(1000.,double(i)/FilterColumns);
  auto a=model?std::abs(h.response(freq)):1.,b=model?std::abs(l.response(freq)):1.;double gain[3]={a,b,model?a*b*drive:1.};
  for(int c=0;c<3;c++){double db=20*std::log10(std::max(1e-9,gain[c]));y[c][i]=(18-std::clamp(db,-60.,18.))/78*287;}
 }
 for(int c=0;c<3;c++)for(int row=0;row<3;row++)for(int i=0;i<FilterColumns;i++){
  double a=y[c][i]-row*96,b=y[c][i+1]-row*96;
  result[(c*3+row)*FilterColumns+i]=clippedLine(a,b);
 }
 return result;
}
}
