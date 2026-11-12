#define _GNU_SOURCE
#include <assert.h>
#include <unistd.h>
#include "../third_party/mpc-wrapper/wrapper/vst2_wrap.c"
static int updates,notifications,alias_notifications,mode_events;
static float last_mode;
static int chart_events;
static int test_frames,last_heavy_frame=-100000,heavy_events;
static VstTimeInfo ti={.tempo=120,.flags=kVstTempoValid};
static intptr_t host(AEffect*a,int32_t op,int32_t i,intptr_t v,void*p,float f){(void)a;(void)i;(void)v;(void)p;(void)f;if(op==42)updates++;if(op==audioMasterAutomate){notifications++;if(PICN_PLOT_VALUE[i]){assert(last_mode==1);chart_events++;}
#ifdef PICN_GRAPHICS_REPORT_FRAMES
if(PICN_VISUAL_CLASS[i]==1){assert(test_frames-last_heavy_frame>=PICN_GRAPHICS_REPORT_FRAMES);last_heavy_frame=test_frames;heavy_events++;}
#endif
if(i==PICN_PLOT_MODE_PARAM){mode_events++;last_mode=f;if(f<.5f){wrap_t*w=a->object;for(int j=0;j<NPARAMS;j++)assert(!PICN_PLOT_VALUE[j]||!w->cc_changed[j]);}}if(!strcmp(PARAMS[i].key,"point_x")||!strcmp(PARAMS[i].key,"point_y")||!strncmp(PARAMS[i].key,"slot_",5))alias_notifications++;if(PICN_PLOT_VALUE[i]){assert(last_mode==1);chart_events++;}}if(op==audioMasterGetTime)return (intptr_t)&ti;return 0;}
static int param(const char*key){for(int i=0;i<NPARAMS;i++)if(!strcmp(PARAMS[i].key,key))return i;assert(0);return -1;}
static void block(AEffect*a,float**out){test_frames+=128;a->processReplacing(a,0,out,128);}
int main(){AEffect*a=VSTPluginMain(host),*b=VSTPluginMain(host);assert(a&&b&&a->numPrograms==PIMORPH_FACTORY_COUNT&&a->numParams==NPARAMS);a->dispatcher(a,effOpen,0,0,0,0);b->dispatcher(b,effOpen,0,0,0,0);float l[128],r[128];float*out[]={l,r};
 for(int i=0;i<100;i++){block(a,out);usleep(2000);}char status[128];eng_get(a->object,"load_status_display",status,sizeof status);assert(strstr(status,".wav"));
 char names[PIMORPH_FACTORY_COUNT][32];for(int j=0;j<PIMORPH_FACTORY_COUNT;j++){a->dispatcher(a,effGetProgramNameIndexed,j,0,names[j],0);assert(names[j][0]);for(int k=0;k<j;k++)assert(strcmp(names[j],names[k]));}
 for(int i=0;i<NPARAMS;i++)if(PIMORPH_CURVE[i])for(int j=0;j<=16;j++){char temp[64];float n=j/16.f;norm_to_str(&PARAMS[i],n,temp,sizeof temp);assert(fabsf(str_to_norm(&PARAMS[i],temp)-n)<.0001f);}
 a->setParameter(a,param("filter_model"),0);a->setParameter(a,param("amp_attack"),0);
 VstMidiEvent m={.type=1,.byteSize=sizeof m,.deltaFrames=48,.midiData={0x90,60,127,0}};VstEvents ev={.numEvents=1,.events={(VstEvent*)&m}};a->dispatcher(a,effProcessEvents,0,0,&ev,0);block(a,out);double energy=0;for(int i=0;i<128;i++){assert(isfinite(l[i])&&isfinite(r[i]));if(i<48)assert(l[i]==0&&r[i]==0);else energy+=fabs(l[i])+fabs(r[i]);}assert(energy>0);
 a->setParameter(a,param("x"),.27f);void*chunk=0;intptr_t size=a->dispatcher(a,effGetChunk,0,0,&chunk,0);assert(size>0&&size<32768);void*copy=malloc(size);memcpy(copy,chunk,size);b->dispatcher(b,effSetChunk,0,size,copy,0);free(copy);assert(fabs(b->getParameter(b,param("x"))-.27)<.0001);
 // Exact factory programme identity survives edited project chunks.
 a->dispatcher(a,effSetProgram,0,177,0,0);assert(a->dispatcher(a,effGetProgram,0,0,0,0)==177);a->setParameter(a,param("x"),.42f);size=a->dispatcher(a,effGetChunk,0,0,&chunk,0);copy=malloc(size);memcpy(copy,chunk,size);b->dispatcher(b,effSetChunk,0,size,copy,0);free(copy);assert(b->dispatcher(b,effGetProgram,0,0,0,0)==177);assert(fabs(b->getParameter(b,param("x"))-.42)<.0001);
 for(int i=0;i<120;i++){block(a,out);usleep(1000);}updates=0;
 for(int k=0;k<32;k++){a->setParameter(a,param("x"),k/31.f);a->setParameter(a,param("filter_model"),(k%3)/2.f);block(a,out);}assert(updates==0);
 // Sustained filter drag after startup/preset queues have drained.
 for(int i=0;i<2400;i++){block(a,out);usleep(1000);}
 int max_callbacks=0,total_callbacks=0;alias_notifications=0;updates=0;
 for(int i=0;i<360;i++){
  if(i%4==0)a->setParameter(a,param("hp_cutoff"),(i%100)/100.f);
  notifications=0;block(a,out);if(notifications>max_callbacks)max_callbacks=notifications;total_callbacks+=notifications;usleep(2000);
 }
 printf("FILTER notification stress: max/block=%d total=%d unrelated-alias=%d update-display=%d\n",max_callbacks,total_callbacks,alias_notifications,updates);
 assert(updates==0);
#if PICN_NOTIFICATION_BUDGET <= 2
 assert(max_callbacks<=2);assert(alias_notifications==0);
#endif
 // Diagnostic hold: no spatial graphics callback during a sustained XY drag.
 for(int i=0;i<2400;i++){block(a,out);usleep(1000);}
 mode_events=0;updates=0;max_callbacks=0;total_callbacks=0;
 for(int i=0;i<300;i++){if(i%4==0){a->setParameter(a,param("x"),(i%100)/100.f);a->setParameter(a,param("y"),1-(i%100)/100.f);}notifications=0;block(a,out);if(notifications>max_callbacks)max_callbacks=notifications;total_callbacks+=notifications;usleep(2000);}
 assert(mode_events==0&&max_callbacks==0&&updates==0);
 for(int i=0;i<2400;i++){notifications=0;block(a,out);assert(notifications<=2);usleep(1000);}
 assert(last_mode==0&&mode_events>=2&&chart_events>0);
 printf("SPACE reveal stress: hidden during chart callbacks; reveal after all chart values delivered; transitions=%d\n",mode_events);
 printf("SPACE hold stress: during-drag callbacks=%d mode-events=%d update-display=%d; precise recovery budget<=2\n",total_callbacks,mode_events,updates);
 a->dispatcher(a,effClose,0,0,0,0);b->dispatcher(b,effClose,0,0,0,0);puts("PASS: actual wrapper; 295 unique native programmes; tapered value mapping; sample-accurate MIDI; edited chunk roundtrip/programme identity; no full UI rebuild during parameter edits; two instances");}
