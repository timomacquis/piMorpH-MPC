extern "C" {
#include "../third_party/mpc-wrapper/wrapper/engine.h"
}
#include <cassert>
#include <cmath>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>
static std::string get(const mpc_engine_t*a,void*p,const char*k){char b[32768];int n=a->get_param(p,k,b,sizeof b);assert(n>0&&n<(int)sizeof b);return b;}
int main(){const auto*a=mpc_engine();void*p=a->create("build");void*q=a->create("build");assert(p&&q);int16_t out[256];
 auto render=[&](void*t,int n){for(int i=0;i<n;i++){a->render(t,out,128);std::this_thread::sleep_for(std::chrono::milliseconds(1));}};
 for(int wait=0;wait<2000&&get(a,p,"load_status_display").find(".wav")==std::string::npos;wait++)render(p,1);assert(get(a,p,"load_status_display").find(".wav")!=std::string::npos);a->set_param(p,"filter_model","0");a->set_param(p,"amp_attack","1");uint8_t note[]={0x90,60,110};a->midi(p,note,3);long long energy=0;for(int i=0;i<30;i++){a->render(p,out,128);for(auto v:out)energy+=(long long)v*v;}assert(energy>10000);
 a->set_param(p,"x","27");a->set_param(p,"point_select","3");a->set_param(p,"point_x","11");assert(get(a,p,"point3_x")=="11");a->set_param(p,"slot_select","2");a->set_param(p,"slot_source","1");assert(get(a,p,"mod2_source")=="1");
 std::string saved=get(a,p,"state");a->set_param(q,"state",saved.c_str());assert(get(a,q,"state")==saved);assert(get(a,q,"state_valid")=="1");
 for(const char*bad:{"PMORPH1\nbank \"../../oops.wav\"\nEND", "PMORPH1\nbank \"Velvet-Field.wav\"\nx nan\nEND", "PMORPH1\nbank \"Velvet-Field.wav\"\nx 15\nx 20\nEND", "PMORPH1\nbank \"Velvet-Field.wav\"\nx 200\nEND"}){a->set_param(q,"state",bad);assert(get(a,q,"state_valid")=="0");assert(get(a,q,"state")==saved);}
 a->set_param(q,"x","99");assert(get(a,p,"x")=="27");assert(get(a,q,"x")=="99");
 uint8_t off[]={0x80,60,0};a->midi(p,off,3);a->set_param(p,"panic","1");a->render(p,out,128);for(auto v:out)assert(v==0);
 for(int i=0;i<20;i++){a->set_param(p,"state",i%2?"PMORPH1\nbank \"Velvet-Field.wav\"\nEND":"PMORPH1\nbank \"Lunar-Wind.wav\"\nEND");render(p,5);}a->set_param(p,"state","PMORPH1\nbank \"Lunar-Wind.wav\"\nEND");render(p,3000);assert(get(a,p,"load_status_display")=="Lunar-Wind.wav");
 for(int i=0;i<64;i++){char k[32];snprintf(k,sizeof k,"path%d",i);double v=std::stod(get(a,p,k));assert(v>=0&&v<=4095);}
 // Filter preview responds to its own controls and stays out of the musical state.
 auto filterLine=[&](){std::vector<int> v;for(int i=0;i<864;i++){char k[40];snprintf(k,sizeof k,"filterline%d",i);v.push_back(std::stoi(get(a,p,k)));}return v;};
 auto originalFilter=filterLine();a->set_param(p,"hp_res","60");render(p,120);assert(originalFilter!=filterLine());{auto f=filterLine();for(int g=0;g<576;g++)assert(f[g]==5986);}/* PERF-CANDIDATE.3: HPF and LPF curves no longer published */auto heldFilter=filterLine();a->set_param(p,"lfo2_depth","66");render(p,100);assert(heldFilter==filterLine());
 // Both passive previews follow their own canonical controls without cross-talk.
 auto line=[&](const char*prefix){std::vector<int> v;for(int i=0;i<96;i++){char k[40];snprintf(k,sizeof k,"%s%d",prefix,i);v.push_back(std::stoi(get(a,p,k)));}return v;};
 auto first=line("lfo1line"),second=line("lfo2line");a->set_param(p,"lfo2_phase","90");render(p,100);
 assert(first==line("lfo1line"));assert(second!=line("lfo2line"));
 auto changedSecond=line("lfo2line");a->set_param(p,"lfo1_depth","0");render(p,100);
 assert(changedSecond==line("lfo2line"));for(int code:line("lfo1line"))assert(code==32*64+32);
 saved=get(a,p,"state");assert(saved.find("line0")==std::string::npos);a->set_param(q,"state",saved.c_str());assert(get(a,q,"state")==saved);
 std::cout<<"PASS: dual LFO preview isolation, control changes and state roundtrip without passive graphics parameters\n";
 // PERF-CANDIDATE.3: the path raster, its active rows and the gesture draft are no longer computed nor published.
 auto trajectory=[&](){std::vector<int> v;for(int i=0;i<2048;i++){char k[40];if(i<384)snprintf(k,sizeof k,"pathseg%d_%d",i/32,i%32);else snprintf(k,sizeof k,"pathtile%d",i);v.push_back(std::stoi(get(a,p,k)));}return v;};
 auto rows=[&](){std::vector<int> v;for(int i=0;i<32;i++){char k[40];snprintf(k,sizeof k,"pathrow%d",i);v.push_back(std::stoi(get(a,p,k)));}return v;};
 auto draft=[&](){std::vector<int> v;const char* names[]={"path","ampgraph","filtergraph","response","lfo1graph","lfo2graph"};for(int i=0;i<512;i++){char k[40];if(i<384)snprintf(k,sizeof k,"%s%d",names[i/64],i%64);else snprintf(k,sizeof k,"pathdraft%d",i);v.push_back((int)std::stod(get(a,p,k)));}return v;};
 auto quiet=[](const std::vector<int>&v){return std::all_of(v.begin(),v.end(),[](int x){return x==0;});};
 auto num=[&](const char*k){return std::stod(get(a,p,k));};
 // MOTION ring: same formula as pathAnchor (X/Y + (point - .5) * PATH DEPTH, clamped).
 auto expectSelected=[&](){int s=(int)num("point_select");char kx[32],ky[32];snprintf(kx,sizeof kx,"point%d_x",s);snprintf(ky,sizeof ky,"point%d_y",s);double depth=num("path_depth")/100;
  double ex=std::clamp(num("x")/100+(num(kx)/100-.5)*depth,0.,1.)*100,ey=std::clamp(num("y")/100+(num(ky)/100-.5)*depth,0.,1.)*100;
  assert(std::fabs(num("path_selected_x")-ex)<1e-3&&std::fabs(num("path_selected_y")-ey)<1e-3);};
 render(p,150);assert(quiet(trajectory())&&quiet(rows())&&quiet(draft()));expectSelected();
 a->set_param(p,"path_depth","80");a->set_param(p,"point_select","5");render(p,150);expectSelected();
 a->set_param(p,"path_depth","0");render(p,150);expectSelected();assert(quiet(trajectory())&&quiet(rows())&&quiet(draft()));
 saved=get(a,p,"state");assert(saved.find("pathtile")==std::string::npos&&saved.find("wavefine")==std::string::npos&&saved.find("pathrow")==std::string::npos);
 std::cout<<"PASS: no path raster/rows/draft published, selected-point ring follows X/Y/point/PATH DEPTH, no passive graphics in saved musical state\n";
 // Diagnostic hold: the SPACE wave stays still during a sustained XY gesture, then refreshes after quiet.
 auto wave=[&](){std::vector<int> v;for(int i=0;i<192;i++){char k[40];snprintf(k,sizeof k,i<64?"wave%d":"wavefine%d",i);v.push_back(std::stoi(get(a,p,k)));}return v;};
 render(p,400);a->set_param(p,"ui_plot_finish","1");auto heldWave=wave();auto heldCommit=get(a,p,"ui_plot_mode");
 for(int i=0;i<40;i++){a->set_param(p,"x",i%2?"42":"57");render(p,5);assert(get(a,p,"ui_plot_mode")==heldCommit);assert(heldWave==wave());assert(quiet(draft()));}
 a->set_param(p,"x","85");render(p,400);assert(get(a,p,"ui_plot_mode")!=heldCommit);assert(heldWave!=wave());expectSelected();a->set_param(p,"ui_plot_finish","1");assert(get(a,p,"ui_plot_mode")=="0");
 saved=get(a,p,"state");assert(saved.find("ui_plot_mode")==std::string::npos&&saved.find("pathdraft")==std::string::npos);
 std::cout<<"PASS: wave held throughout XY gesture, canonical X updates and wave refreshes after quiet; no saved helper state\n";
 a->destroy(p);a->destroy(q);for(int i=0;i<12;i++){p=a->create("build");a->render(p,out,128);a->destroy(p);}std::cout<<"PASS: native engine, async loading, sound, state roundtrip/invalid state, aliases, isolation, rapid table changes, panic and worker lifecycle\n";
}
