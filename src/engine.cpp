// Native MPC adapter; host work is serialized by the pinned wrapper.
#include "synth.h"
#include "import_browser.h"
#include "import_wavetable.h"
#include "lfo_preview.h"
#include "filter_preview.h"
#include "trajectory_preview.h"
#include "trajectory_draft.h"
extern "C" {
#include "engine.h"
}
#include "metadata.h"
#include "params.h"
#include "bank_names.h"
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <string_view>
#include <unordered_map>
#include <cstring>
#include <cstdio>
#include <cstdlib>
using namespace pimorph;
namespace {
constexpr int Banks=sizeof(bank_names)/sizeof(bank_names[0]);
using Values=std::array<double,pmmeta::Count>;
const auto& indexMap(){static const auto map=[](){std::unordered_map<std::string_view,int> m;for(int i=0;i<pmmeta::Count;i++)m.emplace(pmmeta::params[i].key,i);return m;}();return map;}
int id(std::string_view key){auto it=indexMap().find(key);return it==indexMap().end()?-1:it->second;}
const auto& importIndices(){static const auto a=[](){std::array<int,12>a{};const char*names[]={"import_open","import_phase","import_status","table_name","browse_path","browse_page","browse_item0","browse_item1","browse_item2","browse_item3","browse_item4","browse_item5"};for(int i=0;i<12;i++)a[i]=id(names[i]);return a;}();return a;}
const auto& filterIndices(){static const auto indices=[](){std::array<int,FilterCodes> a{};char key[40];for(int i=0;i<FilterCodes;i++){snprintf(key,sizeof key,"filterline%d",i);a[i]=id(key);}return a;}();return indices;}
struct PlotIndices{std::array<int,PathTiles> path{};std::array<int,192>wave{};std::array<int,PathRows>rows{};std::array<int,4>markers{};};
const auto& plotIndices(){static const auto result=[](){PlotIndices p;char k[48];for(int i=0;i<PathTiles;i++){if(i<384)snprintf(k,sizeof k,"pathseg%d_%d",i/32,i%32);else snprintf(k,sizeof k,"pathtile%d",i);p.path[i]=id(k);}for(int i=0;i<192;i++){snprintf(k,sizeof k,i<64?"wave%d":"wavefine%d",i);p.wave[i]=id(k);}for(int i=0;i<PathRows;i++){snprintf(k,sizeof k,"pathrow%d",i);p.rows[i]=id(k);}const char*names[]={"path_start_x","path_start_y","path_selected_x","path_selected_y"};for(int i=0;i<4;i++)p.markers[i]=id(names[i]);return p;}();return result;}
const auto& draftIndices(){static const auto result=[](){std::array<int,DraftTiles> a;const char* names[]={"path","ampgraph","filtergraph","response","lfo1graph","lfo2graph"};char key[40];for(int i=0;i<512;i++){if(i<384)snprintf(key,sizeof key,"%s%d",names[i/64],i%64);else snprintf(key,sizeof key,"pathdraft%d",i);a[i]=id(key);}return a;}();return result;}
constexpr int PlotMode=PICN_PLOT_MODE_PARAM;
struct Snapshot{Values values{};std::string external;unsigned source=1;int bank=0;unsigned revision=0;bool draft=false;};
struct Publication{std::shared_ptr<pf::Wavetable> table;std::string external,message;unsigned source=1;TrajectoryPreview trajectory{};WavePreview waveLine{};std::array<LfoPreview,2> lfoLine{};FilterPreview filterLine{};TrajectoryDraft draftPath{};int bank=0,error=0;unsigned revision=0;bool draft=false;};
struct ImportCheck{unsigned serial=0;bool valid=false;std::string path,message;};
struct Instance{
 Synth synth;Values p{};std::bitset<pmmeta::Count> changed;int notifyCursor=0,bank=0;unsigned revision=1,publishedRevision=0;int loaded=-1,error=0;bool stateValid=true,wasPlaying=false;
 std::mutex checkMutex;std::string checkPath,selectedPath;bool selectedValid=false;std::atomic<unsigned> checkSerial{0};std::atomic<bool> checkCancel{false},checkEnabled{false};std::atomic<ImportCheck*> checkReady{nullptr},checkRetired{nullptr};
 ImportBrowser browser;std::string requestedPath,externalPath,importMessage;std::atomic<unsigned> sourceSerial{1};bool importing=false;
 std::string directory;std::thread worker;std::atomic<bool> stop{false},cancel{false};std::atomic<int> desiredBank{0};
 std::mutex jobs;Snapshot snapshot;std::atomic<unsigned> requested{0};std::atomic<Publication*> ready{nullptr},retired{nullptr};Publication* active=nullptr;
 int plotQuietSamples=100000;bool editingPlot=false;
 int submittedSamples=0;double bend=0;float buffer[256]{};
 Instance(const char*dir):directory(dir?dir:"."){selectedPath.reserve(1024);checkPath.reserve(1024);requestedPath.reserve(1024);externalPath.reserve(1024);snapshot.external.reserve(1024);importMessage.reserve(256);indexMap();importIndices();filterIndices();plotIndices();draftIndices();for(int i=0;i<pmmeta::Count;i++)p[i]=pmmeta::params[i].def;worker=std::thread([this]{work();});}
 ~Instance(){stop=true;cancel=true;checkCancel=true;if(worker.joinable())worker.join();delete checkReady.exchange(nullptr);delete checkRetired.exchange(nullptr);delete ready.exchange(nullptr);delete retired.exchange(nullptr);delete active;}
 double value(const char*k)const{int i=id(k);return i>=0?p[i]:0;}
 void mark(const char*k){int i=id(k);if(i>=0)changed.set(i);}
 void touch(){revision++;}
 void plotEdit(){editingPlot=true;plotQuietSamples=0;}
 bool plotField(const char*k)const{return !strcmp(k,"x")||!strcmp(k,"y")||(!strncmp(k,"path_",5))||(!strncmp(k,"orbit_",6))||(!strncmp(k,"point",5)&&strcmp(k,"point_select"));}
 bool validExternal(const std::string& path)const{return !path.empty()&&path.size()<=1024&&path[0]=='/'&&path.find_first_of("\n\r")==std::string::npos;}
 void importStatus(const std::string& text,int phase){importMessage=phase==4?"ERROR : "+text:text;int i=id("import_phase");if(p[i]!=phase){p[i]=phase;changed.set(i);}mark("import_status");}
 void clearSelection(){checkEnabled=false;selectedPath.clear();selectedValid=false;checkCancel=true;checkSerial++;if(!importing)importStatus("Choose a WAV to validate",0);}
 void validateFile(const std::string& path){if(!validExternal(path)){importStatus("Choose an absolute WAV path",4);return;}
  std::unique_lock<std::mutex>l(checkMutex,std::try_to_lock);if(!l)return;
  selectedPath=path;selectedValid=false;checkCancel=true;checkPath=path;checkEnabled=true;checkSerial++;importStatus("CHECKING...",1);
 }
 void importFile(const std::string& path){if(!validExternal(path)){importStatus("Choose an absolute WAV path",4);return;}requestedPath=path;sourceSerial++;cancel=true;importing=true;selectedValid=false;importStatus("LOADING",3);mark("load_status");editingPlot=false;touch();}
 void setbank(int b){bank=b;desiredBank=b;requestedPath.clear();externalPath.clear();sourceSerial++;cancel=true;error=0;importing=false;clearSelection();touch();mark("load_status");mark("table_name");}
 bool browserSet(const char*k,const char*v){
  if(!strcmp(k,"import_path")){importFile(v);return true;}
  if(!strcmp(k,"browse_path_set")){if(!importing){clearSelection();browser.navigate(v);}return true;}
  if(strcmp(k,"import_open")&&strncmp(k,"browse_",7))return false;
  if(!strcmp(k,"browse_path")||!strcmp(k,"browse_page"))return true;
  double x=strtod(v,nullptr);
  if(!strcmp(k,"import_open")){p[id(k)]=x>.5;mark(k);if(!importing)clearSelection();if(x>.5)browser.reopen();return true;}
  if(x<=.5||importing)return true;
  if(!strcmp(k,"browse_up")){clearSelection();browser.up();}else if(!strcmp(k,"browse_root")){clearSelection();browser.navigate("/media");}else if(!strcmp(k,"browse_user")){clearSelection();browser.navigate("/sdcard");}else if(!strcmp(k,"browse_prev")){clearSelection();browser.step(-1);}else if(!strcmp(k,"browse_next")){clearSelection();browser.step(1);}
  else if(strlen(k)==12&&!strncmp(k,"browse_item",11)&&k[11]>='0'&&k[11]<='5'){
   auto path=browser.click(k[11]-'0');if(path.empty())clearSelection();
   else if(path==selectedPath&&selectedValid)importFile(path);else if(path!=selectedPath||value("import_phase")!=1)validateFile(path);
  }
  return true;
 }
 void apply(){auto& q=synth.patch;
  q.x=value("x")/100;q.y=value("y")/100;q.mode=(int)value("path_mode");static const double beats[]={.25,.5,1,2,4,8,16};
  q.pathBars=beats[(int)value("path_rate")]/4;q.orbitBars=beats[(int)value("orbit_rate")]/4;q.pathDepth=value("path_depth")/100;q.orbitWidth=value("orbit_width")/100;q.orbitHeight=value("orbit_height")/100;q.orbitPhase=value("orbit_phase")/360;q.pathOffset=value("path_offset")/100;
  q.attack=value("amp_attack")/1000;q.decay=value("amp_decay")/1000;q.sustain=value("amp_sustain")/100;q.release=value("amp_release")/1000;
  q.filterAttack=value("filter_attack")/1000;q.filterDecay=value("filter_decay")/1000;q.filterSustain=value("filter_sustain")/100;q.filterRelease=value("filter_release")/1000;
  q.hp=value("hp_cutoff");q.lp=value("lp_cutoff");q.hpPeak=value("hp_res")/100;q.lpPeak=value("lp_res")/100;q.drive=std::exp2(value("drive")/25);q.hpEnv=value("hp_env")/25;q.lpEnv=value("lp_env")/25;q.keytrack=value("keytrack")/100;
  q.filterOn=value("filter_model")>0;q.filterModel=(int)value("filter_model")-1;q.level=value("level")/100;q.pan=value("pan")/100;q.transpose=value("transpose");q.bend=bend*value("bend_range");q.glide=value("glide")/1000;q.voiceLimit=(int)value("voice_limit");q.playMode=(int)value("play_mode");
  q.pointCount=(int)value("point_count");char key[48];for(int i=0;i<12;i++){snprintf(key,sizeof key,"point%d_x",i);q.points[i].x=value(key)/100;snprintf(key,sizeof key,"point%d_y",i);q.points[i].y=value(key)/100;}
  for(int j=0;j<2;j++){
   auto get=[&](const char*s){snprintf(key,sizeof key,"lfo%d_%s",j+1,s);return value(key);};auto& l=synth.lfo[j];int shape=(int)get("wave");if(l.spec.f[1]!=shape)l.walk=ModWalk{};l.spec.f[1]=shape;
   q.lfoOn[j]=get("on")>0;q.lfoDepth[j]=get("depth")/100;q.lfoOffset[j]=get("offset")/100;q.lfoReset[j]=get("reset")>0;
   double phase=get("phase")/360;l.phase+=phase-q.lfoPhase[j];q.lfoPhase[j]=phase;
   double rate=get("rate");q.lfoHz[j]=get("sync")>0?q.bpm/(60*beats[(int)std::lround(rate*6)]):.02*std::pow(1000.,rate);
   for(int k=0;k<4;k++){snprintf(key,sizeof key,"lfo%d_extra%d",j+1,k);l.spec.extra[k]=(int)value(key);}for(int k=0;k<8;k++){snprintf(key,sizeof key,"lfo%d_c%d",j+1,k);l.spec.f[10+k]=(int)value(key);}
  }
  for(int i=0;i<8;i++){snprintf(key,sizeof key,"mod%d_source",i);q.source[i]=(int)value(key);snprintf(key,sizeof key,"mod%d_dest",i);q.dest[i]=(int)value(key);snprintf(key,sizeof key,"mod%d_amount",i);q.amount[i]=value(key)/100;}
 }
 int alias(int i)const{std::string_view k=pmmeta::params[i].key;char b[48];
  if(k=="point_x"||k=="point_y"){snprintf(b,sizeof b,"point%d_%c",(int)value("point_select"),k.back());return id(b);}
  if(k=="slot_source"||k=="slot_dest"||k=="slot_amount"){snprintf(b,sizeof b,"mod%d_%s",(int)value("slot_select"),pmmeta::params[i].key+5);return id(b);}return i;
 }
 void aliases(){for(const char*k:{"point_x","point_y","slot_source","slot_dest","slot_amount"})mark(k);}
 void aliasesFor(int written,int target){
  // Host already knows its own write. Only synchronize a genuinely related alias.
  if(target==id("point_select")){mark("point_x");mark("point_y");}
  if(target==id("slot_select")){mark("slot_source");mark("slot_dest");mark("slot_amount");}
  for(const char*key:{"point_x","point_y","slot_source","slot_dest","slot_amount"}){int i=id(key);if(i!=written&&alias(i)==target)changed.set(i);}
 }
 bool restore(const char*text){if(strlen(text)>30000)return false;std::istringstream in(text);std::string key,filename,external;bool foundExternal=false;in>>key;if(key!="PMORPH1")return false;Values next{};for(int i=0;i<pmmeta::Count;i++)next[i]=pmmeta::params[i].def;std::bitset<pmmeta::Count> seen;bool foundBank=false,ended=false;int b=-1;
  while(in>>key){if(key=="END"){ended=true;break;}if(key=="bank"){if(foundBank||!(in>>std::quoted(filename)))return false;foundBank=true;for(int j=0;j<Banks;j++)if(filename==bank_names[j])b=j;if(b<0)return false;}else if(key=="external"){if(foundExternal||!(in>>std::quoted(external))||!validExternal(external))return false;foundExternal=true;}else{
    int i=id(key);double v;if(i<0||i>=pmmeta::Canonical||seen[i]||!(in>>v)||!std::isfinite(v))return false;const auto&m=pmmeta::params[i];if(v<m.low||v>m.high)return false;if(m.options&&v!=std::round(v))return false;seen[i]=true;next[i]=v;}}
  if(!ended||!foundBank||(in>>key))return false;
  for(int i=0;i<pmmeta::Count;i++){if(PICN_VISUAL_CLASS[i])next[i]=p[i];else if(next[i]!=p[i])changed.set(i);}p=next;aliases();editingPlot=false;plotQuietSamples=100000;synth.panic();apply();setbank(b);if(foundExternal){importFile(external);externalPath=external;}return true;
 }
 std::string state()const{std::ostringstream o;o<<"PMORPH1\nbank "<<std::quoted(bank_names[bank])<<'\n'<<std::setprecision(17);for(int i=0;i<pmmeta::Canonical;i++)o<<pmmeta::params[i].key<<' '<<p[i]<<'\n';if(!externalPath.empty())o<<"external "<<std::quoted(externalPath)<<'\n';o<<"END\n";return o.str();}
 void set(const char*k,const char*v){if(browserSet(k,v))return;if(!strcmp(k,"ui_plot_finish")){if(p[PlotMode]!=0){p[PlotMode]=0;changed.set(PlotMode);}return;}if(!strcmp(k,"state")){stateValid=restore(v);return;}int i=id(k);if(i<0)return;char*end;double x=strtod(v,&end);if(end==v||*end||!std::isfinite(x))return;
  const auto&m=pmmeta::params[i];x=bound(x,m.low,m.high);if(m.options||!strcmp(k,"point_count")||!strcmp(k,"voice_limit")||!strcmp(k,"transpose"))x=std::round(x);
  if(!strcmp(k,"panic")){if(x>0)synth.panic();return;}
  if(!strcmp(k,"add_point")||!strcmp(k,"remove_point")){if(x<=0)return;int n=(int)value("point_count");if(k[0]=='a'&&n<12){p[id("point_count")]=n+1;p[id("point_select")]=n;}if(k[0]=='r'&&n>2){p[id("point_count")]=n-1;p[id("point_select")]=std::min(n-2,(int)value("point_select"));}mark("point_count");mark("point_select");aliases();apply();plotEdit();touch();return;}
  if(i>=pmmeta::Canonical&&strstr(k,"__open")==nullptr&&strcmp(k,"point_x")&&strcmp(k,"point_y")&&strncmp(k,"slot_",5))return;
  int target=alias(i);if(target>=pmmeta::Canonical)return;if(p[target]==x)return;p[target]=x;if(target!=i)changed.set(target);
  if(!strcmp(k,"play_mode")||!strcmp(k,"voice_limit"))synth.panic();apply();aliasesFor(i,target);if(plotField(k))plotEdit();touch();
 }
 void submit(){if(editingPlot)return;if(publishedRevision==revision)return;if(!jobs.try_lock())return;snapshot.values=p;snapshot.bank=bank;snapshot.external=requestedPath;snapshot.source=sourceSerial.load();snapshot.revision=revision;snapshot.draft=editingPlot;requested=revision;jobs.unlock();publishedRevision=revision;}
 void consume(){if(!checkRetired.load()){auto*r=checkReady.exchange(nullptr);if(r){if(r->serial==checkSerial&&r->path==selectedPath){selectedValid=r->valid;importStatus(r->valid?"VALID":r->message,r->valid?2:4);}checkRetired=r;}}
 if(browser.dirty.exchange(false)){mark("browse_path");mark("browse_page");for(int i=0;i<6;i++){char k[32];snprintf(k,sizeof k,"browse_item%d",i);mark(k);}}if(retired.load())return;Publication*n=ready.exchange(nullptr);if(!n)return;
  if(n->bank!=bank||n->source!=sourceSerial){retired=n;return;}
  if(n->error){error=n->error;importing=false;importStatus(n->message,4);requestedPath=externalPath;sourceSerial++;mark("import_status");mark("load_status");retired=n;return;}
  if(importing){externalPath=n->external;importing=false;importStatus("Loaded",0);mark("table_name");p[id("import_open")]=0;mark("import_open");}

  if(n->revision==revision){
   auto publish=[&](int i,double value){if(p[i]!=value){p[i]=value;changed.set(i);}};
   const auto& pi=plotIndices();bool plotDirty=false;
   auto plotPublish=[&](int i,double value){if(p[i]!=value){publish(i,value);plotDirty=true;}};
   if(n->draft){const auto& di=draftIndices();for(int g=0;g<DraftTiles;g++)publish(di[g],n->draftPath.cells[g]);}
   else{for(int g=0;g<PathTiles;g++)plotPublish(pi.path[g],n->trajectory.codes[g]);for(int g=0;g<192;g++)plotPublish(pi.wave[g],n->waveLine[g]);for(int g=0;g<PathRows;g++)plotPublish(pi.rows[g],n->trajectory.activeRows[g]);if(plotDirty)publish(PlotMode,1);}
   double marks[]={n->trajectory.start.x,n->trajectory.start.y,n->trajectory.selected.x,n->trajectory.selected.y};for(int g=0;g<4;g++)publish(pi.markers[g],marks[g]*100);
   const auto& fi=filterIndices();for(int g=0;g<FilterCodes;g++)publish(fi[g],n->filterLine[g]);
   for(int j=0;j<2;j++)for(int g=0;g<PreviewColumns;g++){char key[40];snprintf(key,sizeof key,"lfo%dline%d",j+1,g);publish(id(key),n->lfoLine[j][g]);}
  }
  int previousLoaded=loaded,previousError=error;
  if(!active||active->bank!=n->bank||active->table.get()!=n->table.get()){synth.setTable(n->table.get());mark("table_name");loaded=n->table?n->bank:-1;}
  error=n->error;if(loaded!=previousLoaded||error!=previousError)mark("load_status");if(active)retired=active;active=n;
 }
 static void graphs(Publication&o,const Snapshot&s,const FilterPreview* filterCached=nullptr,const TrajectoryPreview* pathCached=nullptr,const WavePreview* waveCached=nullptr,TrajectoryScratch* pathScratch=nullptr,const TrajectoryDraft* draftCached=nullptr){
  auto get=[&](const char*k){return s.values[id(k)];};
  for(int j=0;j<2;j++){
   char key[40];auto field=[&](const char*suffix){snprintf(key,sizeof key,"lfo%d_%s",j+1,suffix);return get(key);};
   Lfo preview;preview.spec.f[1]=(int)field("wave");
   for(int k=0;k<4;k++){snprintf(key,sizeof key,"lfo%d_extra%d",j+1,k);preview.spec.extra[k]=(int)get(key);}
   for(int k=0;k<8;k++){snprintf(key,sizeof key,"lfo%d_c%d",j+1,k);preview.spec.f[10+k]=(int)get(key);}
   o.lfoLine[j]=lfoPreview(preview,field("depth")/100,field("offset")/100,field("phase")/360);
  }
  o.filterLine=filterCached?*filterCached:filterPreview(get("hp_cutoff"),get("hp_res")/100,get("lp_cutoff"),get("lp_res")/100,(int)get("filter_model"),std::exp2(get("drive")/25));
  auto makePatch=[&](){Patch q;static const double beats[]={.25,.5,1,2,4,8,16};q.x=get("x")/100;q.y=get("y")/100;q.mode=(int)get("path_mode");q.pathBars=beats[(int)get("path_rate")]/4;q.orbitBars=beats[(int)get("orbit_rate")]/4;q.pathDepth=get("path_depth")/100;q.orbitWidth=get("orbit_width")/100;q.orbitHeight=get("orbit_height")/100;q.orbitPhase=get("orbit_phase")/360;q.pathOffset=get("path_offset")/100;q.pointCount=(int)get("point_count");char key[40];for(int i=0;i<12;i++){snprintf(key,sizeof key,"point%d_x",i);q.points[i].x=get(key)/100;snprintf(key,sizeof key,"point%d_y",i);q.points[i].y=get(key)/100;}return q;};
  if(s.draft){o.draftPath=draftCached?*draftCached:trajectoryDraft(makePatch(),(int)get("point_select"));o.trajectory.start=o.draftPath.start;o.trajectory.selected=o.draftPath.selected;}
  else{if(pathCached)o.trajectory=*pathCached;else o.trajectory=trajectoryPreview(makePatch(),(int)get("point_select"),pathScratch);o.waveLine=waveCached?*waveCached:wavePreview(o.table.get(),get("x")/100,get("y")/100);}

 }
 void work(){unsigned checked=0;std::shared_ptr<pf::Wavetable> checkedTable;std::string checkedPath;std::uintmax_t checkedSize=0;std::filesystem::file_time_type checkedTime{};
  auto fingerprint=[](const std::string&path,std::uintmax_t&size,std::filesystem::file_time_type&time){std::error_code ec;size=std::filesystem::file_size(path,ec);if(ec)return false;time=std::filesystem::last_write_time(path,ec);return !ec;};
  unsigned cacheSource=0;unsigned done=0;int cacheBank=-1;std::string cachePath;std::shared_ptr<pf::Wavetable> cache;std::array<double,6> filterSettings{};FilterPreview filterCache{};bool filterValid=false;std::array<double,36> pathSettings{};TrajectoryPreview pathCache;bool pathValid=false;std::array<double,36> draftSettings{};TrajectoryDraft draftCache;bool draftValid=false;std::array<double,2> waveXY{};WavePreview waveCache{};std::shared_ptr<pf::Wavetable>waveTable;bool waveValid=false;auto pathScratch=std::make_unique<TrajectoryScratch>();
  while(!stop){delete checkRetired.exchange(nullptr);delete retired.exchange(nullptr);browser.work();
   unsigned serial=checkSerial.load();if(serial!=checked){std::string path;{std::lock_guard<std::mutex>l(checkMutex);path=checkPath;}checked=serial;
    if(!path.empty()&&checkEnabled){checkCancel=false;auto result=std::make_unique<ImportCheck>();result->serial=serial;result->path=path;auto table=std::make_shared<pf::Wavetable>();int frame=0;std::string why;std::uintmax_t size=0;std::filesystem::file_time_type time{};
     bool ok=fingerprint(path,size,time)&&inspectImport(path,frame,why);
     if(ok)ok=pf::loadWavetable(path,*table,&why,&checkCancel,frame);
     if(ok){std::uintmax_t afterSize=0;std::filesystem::file_time_type afterTime{};if(!fingerprint(path,afterSize,afterTime)||size!=afterSize||time!=afterTime){ok=false;why="File changed during validation; select it again";}}
     if(stop)break;if(serial!=checkSerial||!checkEnabled)continue;
     if(ok){checkedTable=table;checkedPath=path;checkedSize=size;checkedTime=time;}
     else{checkedTable.reset();checkedPath.clear();}
     result->valid=ok;result->message=why=="silent"?"Silent table (check stereo phase)":why.empty()?"File missing or unreadable":why;
     delete checkReady.exchange(result.release());
    }
   }
   unsigned requestedNow=requested.load();if(requestedNow==done){std::this_thread::sleep_for(std::chrono::milliseconds(8));continue;}
   Snapshot job;{std::lock_guard<std::mutex>guard(jobs);job=snapshot;}done=job.revision;
   auto pub=std::make_unique<Publication>();pub->bank=job.bank;pub->revision=job.revision;pub->draft=job.draft;
   pub->source=job.source;pub->external=job.external;
   std::string path=job.external.empty()?directory+"/Wavetables/"+bank_names[job.bank]:job.external;
   if(cachePath!=path||(!job.external.empty()&&cacheSource!=job.source)){cancel=false;auto t=std::make_shared<pf::Wavetable>();std::string why;int frame=0;
    std::uintmax_t size=0;std::filesystem::file_time_type time{};
    bool reused=!job.external.empty()&&checkedTable&&checkedPath==path&&fingerprint(path,size,time)&&size==checkedSize&&time==checkedTime;
    bool ok=reused||job.external.empty()||inspectImport(path,frame,why);
    if(reused)t=checkedTable;else if(ok)ok=pf::loadWavetable(path,*t,&why,&cancel,frame);
    if(stop)break;if(sourceSerial.load()!=job.source)continue;
    if(ok){cache=t;cacheBank=job.bank;cachePath=path;cacheSource=job.source;}else{pub->error=1;pub->message=why=="silent"?"Silent table (check stereo phase)":why.empty()?"Table unavailable":why;Publication*old=ready.exchange(pub.release());delete old;continue;}}
   std::array<double,6> fs{};const char* fk[]={"hp_cutoff","hp_res","lp_cutoff","lp_res","filter_model","drive"};for(int i=0;i<6;i++)fs[i]=job.values[id(fk[i])];
   std::array<double,36> ps{};const char* pk[]={"x","y","path_mode","path_rate","path_depth","orbit_rate","orbit_width","orbit_height","orbit_phase","path_offset","point_count","point_select"};for(int i=0;i<12;i++)ps[i]=job.values[id(pk[i])];char key[40];for(int i=0;i<24;i++){snprintf(key,sizeof key,"point%d_%c",i/2,i%2?'y':'x');ps[12+i]=job.values[id(key)];}
   std::array<double,2> xy={job.values[id("x")],job.values[id("y")]};
   pub->table=cache;graphs(*pub,job,filterValid&&fs==filterSettings?&filterCache:nullptr,pathValid&&ps==pathSettings?&pathCache:nullptr,waveValid&&xy==waveXY&&waveTable==cache?&waveCache:nullptr,pathScratch.get(),draftValid&&ps==draftSettings?&draftCache:nullptr);
   filterCache=pub->filterLine;filterSettings=fs;filterValid=true;if(job.draft){draftCache=pub->draftPath;draftSettings=ps;draftValid=true;}else{pathCache=pub->trajectory;pathSettings=ps;pathValid=true;waveCache=pub->waveLine;waveXY=xy;waveTable=cache;waveValid=true;}Publication*old=ready.exchange(pub.release());delete old;
   std::this_thread::sleep_for(std::chrono::milliseconds(70));
  }
 }
};
void* create(const char*dir){try{return new Instance(dir);}catch(...){return nullptr;}}
void destroy(void*p){delete static_cast<Instance*>(p);}
void midi(void*p,const uint8_t*m,int len){auto&e=*static_cast<Instance*>(p);if(len<2)return;int op=m[0]&0xf0;
 if(op==0x90&&len>=3){if(m[2])e.synth.noteOn(m[1],m[2]/127.);else e.synth.noteOff(m[1]);}
 else if(op==0x80)e.synth.noteOff(m[1]);else if(op==0xe0&&len>=3){e.bend=((m[2]*128+m[1])-8192)/8192.;e.apply();}
 else if(op==0xd0)e.synth.patch.pressure=m[1]/127.;else if(op==0xa0&&len>=3)e.synth.patch.pressure=m[2]/127.;
 else if(op==0xb0&&len>=3){if(m[1]==1)e.synth.patch.modwheel=m[2]/127.;else if(m[1]==64)e.synth.sustain(m[2]>=64);else if(m[1]==120||m[1]==123)e.synth.panic();}}
void set(void*p,const char*k,const char*v){try{static_cast<Instance*>(p)->set(k,v);}catch(...){static_cast<Instance*>(p)->stateValid=false;}}
int get(void*p,const char*k,char*b,int n){auto&e=*static_cast<Instance*>(p);if(n<=0)return 0;
 if(!strcmp(k,"table_frames"))return snprintf(b,n,"%d",e.active&&e.active->table?e.active->table->frames:0);
 if(!strcmp(k,"table_name")||!strcmp(k,"table_name_display")){
  const auto*a=e.active;std::string name=a&&a->table?(a->external.empty()?bank_names[a->bank]:std::filesystem::path(a->external).filename().string()):bank_names[e.bank];return snprintf(b,n,"%s",name.c_str());}
 if(!strcmp(k,"import_path"))return snprintf(b,n,"%s",e.externalPath.c_str());
 if(!strcmp(k,"import_status")||!strcmp(k,"import_status_display"))return snprintf(b,n,"%s",e.importMessage.empty()?"Choose a WAV to validate":e.importMessage.c_str());
 if(!strncmp(k,"browse_item",11)||!strncmp(k,"browse_path",11)||!strncmp(k,"browse_page",11)){std::string key(k);if(key.size()>8&&key.substr(key.size()-8)=="_display")key.resize(key.size()-8);auto text=e.browser.read(key);return snprintf(b,n,"%s",text.c_str());}
 if(!strcmp(k,"state_valid"))return snprintf(b,n,"%d",e.stateValid);if(!strcmp(k,"state")){try{auto s=e.state();if((int)s.size()+1>n)return 0;memcpy(b,s.c_str(),s.size()+1);return (int)s.size();}catch(...){return 0;}}
 size_t len=strlen(k);bool display=len>8&&!strcmp(k+len-8,"_display"),name=len>5&&!strcmp(k+len-5,"_name");int i=id(std::string_view(k,len-(display?8:name?5:0)));if(i<0)return 0;
 if(!strcmp(pmmeta::params[i].key,"load_status")){if(display)return snprintf(b,n,"%s",e.error?e.importMessage.c_str():e.importing?"Loading…":!e.externalPath.empty()?std::filesystem::path(e.externalPath).filename().c_str():e.loaded==e.bank?bank_names[e.bank]:"Loading…");return snprintf(b,n,"0");}
 int a=e.alias(i);double v=e.p[a];const auto&m=pmmeta::params[a];if(name)return snprintf(b,n,"%s",m.name);
 if(display){if(m.options&&PARAMS[a].nopts)return snprintf(b,n,"%s",PARAMS[a].opts[std::clamp((int)v,0,PARAMS[a].nopts-1)]);
  if(strstr(m.key,"lfo")&&strstr(m.key,"_rate")){int j=m.key[3]-'0';char key[40];snprintf(key,sizeof key,"lfo%d_sync",j);if(e.value(key)>0){static const char*div[]={"1/16","1/8","1/4","1/2","1 BAR","2 BARS","4 BARS"};return snprintf(b,n,"%s",div[(int)std::lround(v*6)]);}return snprintf(b,n,"%.3g Hz",.02*std::pow(1000.,v));}
  if(!strcmp(m.unit,"ms")&&v>=1000)return snprintf(b,n,"%.2g s",v/1000);return snprintf(b,n,"%.4g%s%s",v,*m.unit?" ":"",m.unit);}
 return snprintf(b,n,"%.12g",v);
}
void render(void*p,int16_t*out,int frames){auto&e=*static_cast<Instance*>(p);e.consume();if(e.editingPlot){e.plotQuietSamples+=frames;if(e.plotQuietSamples>=8820){e.editingPlot=false;e.touch();}}e.submittedSamples+=frames;if(e.submittedSamples>=512){e.submit();e.submittedSamples=0;}
 while(frames>0){int count=std::min(128,frames);e.synth.render(e.buffer,count);for(int i=0;i<count*2;i++)out[i]=(int16_t)std::lround(bound(e.buffer[i],-1,1)*32767);out+=count*2;frames-=count;}}
void song(void*p,double ppq,double bpm,int playing,int){auto&e=*static_cast<Instance*>(p);if(std::isfinite(bpm)&&bpm>=20&&bpm<=400&&bpm!=e.synth.patch.bpm){e.synth.patch.bpm=bpm;e.apply();}
 if(e.wasPlaying&&!playing)e.synth.panic();e.wasPlaying=playing;
 static const double div[]={.25,.5,1,2,4,8,16};if(playing&&std::isfinite(ppq)&&ppq>=0)for(int j=0;j<2;j++){char k[40];snprintf(k,sizeof k,"lfo%d_sync",j+1);if(e.value(k)>0&&!e.synth.patch.lfoReset[j]){snprintf(k,sizeof k,"lfo%d_rate",j+1);e.synth.lfo[j].phase=ppq/div[(int)std::lround(e.value(k)*6)]+e.synth.patch.lfoPhase[j];}}}
const mpc_engine_t api={create,destroy,midi,set,get,render,nullptr,song};
}
extern "C" const mpc_engine_t* mpc_engine(){return &api;}
extern "C" int mpc_engine_changed(void*p,char*k,int n){auto&e=*static_cast<Instance*>(p);if(e.changed[PlotMode]){e.changed.reset(PlotMode);snprintf(k,n,"ui_plot_mode");return 1;}for(int a:importIndices())if(e.changed[a]){e.changed.reset(a);snprintf(k,n,"%s",pmmeta::params[a].key);return 1;}if(e.changed.none())return 0;for(int i=0;i<pmmeta::Count;i++){int a=e.notifyCursor++%pmmeta::Count;if(e.changed[a]){e.changed.reset(a);snprintf(k,n,"%s",pmmeta::params[a].key);return 1;}}return 0;}
