#define _GNU_SOURCE
#include <assert.h>
#include <unistd.h>
#include "../third_party/mpc-wrapper/wrapper/vst2_wrap.c"
static intptr_t host(AEffect*a,int32_t op,int32_t i,intptr_t v,void*p,float f){(void)a;(void)op;(void)i;(void)v;(void)p;(void)f;return 0;}
static void waitpath(AEffect*a,const char*path){float l[128],r[128];float*out[]={l,r};char text[2048];for(int i=0;i<10000;i++){a->processReplacing(a,0,out,128);eng_get(a->object,"import_path",text,sizeof text);if(!strcmp(text,path))return;usleep(2000);}assert(!"import timed out");}
int main(){AEffect*a=VSTPluginMain(host),*b=VSTPluginMain(host);assert(a&&b);a->dispatcher(a,effOpen,0,0,0,0);b->dispatcher(b,effOpen,0,0,0,0);
 char path[4096],text[4096];assert(realpath("build/Wavetables/Velvet-Field.wav",path));eng_set(a->object,"import_path",path);waitpath(a,path);
 void*chunk=0;intptr_t n=a->dispatcher(a,effGetChunk,0,0,&chunk,0);assert(n>0&&n<32768);void*copy=malloc(n);memcpy(copy,chunk,n);assert(strstr(copy,"external \""));
 b->dispatcher(b,effSetChunk,0,n,copy,0);free(copy);waitpath(b,path);eng_get(b->object,"load_status_display",text,sizeof text);
 // Wait for successful worker completion as the restored path is persisted immediately.
 for(int i=0;i<10000&&strcmp(text,"Velvet-Field.wav");i++){float l[128],r[128];float*out[]={l,r};b->processReplacing(b,0,out,128);usleep(2000);eng_get(b->object,"load_status_display",text,sizeof text);}assert(!strcmp(text,"Velvet-Field.wav"));
 eng_get(b->object,"state_valid",text,sizeof text);assert(!strcmp(text,"1"));
 b->dispatcher(b,effSetProgram,0,1,0,0);eng_get(b->object,"import_path",text,sizeof text);assert(!*text);
 a->dispatcher(a,effClose,0,0,0,0);b->dispatcher(b,effClose,0,0,0,0);puts("PASS: actual VST chunk saves/restores external table path across instances; native factory preset clears import");
}
