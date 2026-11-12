#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <cstdlib>
namespace pimorph {
// Inspect on the background worker only; never read a file from an audio callback.
inline bool inspectImport(const std::string& path,int& frame,std::string& error){
 auto fail=[&](const char*s){error=s;return false;};
 std::ifstream f(path,std::ios::binary|std::ios::ate);if(!f)return fail("File missing or unreadable");
 auto size=f.tellg();if(size<44||size>8*1024*1024)return fail("Unsupported wavetable file size");
 std::vector<unsigned char>b((size_t)size);f.seekg(0);if(!f.read((char*)b.data(),size))return fail("Cannot read WAV");
 if(memcmp(b.data(),"RIFF",4)||memcmp(b.data()+8,"WAVE",4))return fail("Not a WAV file");
 auto u16=[&](size_t p){return unsigned(b[p])|(unsigned(b[p+1])<<8);};
 auto u32=[&](size_t p){return u16(p)|(u16(p+2)<<16);};
 unsigned tag=0,channels=0,rate=0,bits=0,align=0;size_t samples=0;int marker=0;bool fmt=false,data=false;
 for(size_t p=12;p+8<=b.size();){size_t n=u32(p+4),body=p+8;if(n>b.size()-body)return fail("Truncated WAV chunk");
  if(!memcmp(&b[p],"fmt ",4)&&n>=16){tag=u16(body);channels=u16(body+2);rate=u32(body+4);align=u16(body+12);bits=u16(body+14);if(tag==65534&&n>=40)tag=u16(body+24);fmt=true;}
  if(!memcmp(&b[p],"data",4)){if(data)return fail("Multiple WAV data chunks");samples=n;data=true;}
  if(!memcmp(&b[p],"clm ",4)&&n>3&&!memcmp(&b[body],"<!>",3)){std::string digits((char*)&b[body+3],std::min<size_t>(n-3,8));marker=atoi(digits.c_str());}
  p=body+n+(n&1);
 }
 if(!fmt||!data||(channels!=1&&channels!=2)||rate==0||!((tag==1&&(bits==16||bits==32))||(tag==3&&bits==32))||align!=channels*bits/8)return fail("Use mono/stereo PCM16, PCM32 or float32 WAV");
 if(samples%align)return fail("Incomplete WAV sample");samples/=align;
 // Honor valid cycle metadata first: 64 x 2048 is a valid table, not
 // an erroneous 256 x 512 export. Never trim or reinterpret complete cycles.
 if(marker){
  if(marker!=512&&marker!=1024&&marker!=2048)return fail("Unsupported cycle metadata (512 / 1024 / 2048)");
  if(samples%marker||samples/marker<1||samples/marker>256)return fail("Cycle metadata conflicts with WAV length");
  frame=marker;
 }else{
  // Existing metadata-free export convention: 256 waveforms.
  frame=samples==512*256?512:samples==1024*256?1024:samples==2048*256?2048:0;
  if(!frame){int matches=0;for(int n:{512,1024,2048})if(samples%n==0&&samples/n>=1&&samples/n<=256){frame=n;matches++;}
   if(matches!=1)return fail("Cycle size missing or ambiguous (512 / 1024 / 2048)");}
 }

 return true;
}
}
