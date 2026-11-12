import sys,json,hashlib,math,struct
from config import *
sys.path.insert(0,str(B/'third_party/mpc-wrapper/tools'));import gen_vst
P=[]
for f in FIELDS:
 p={k:v for k,v in f.items() if k not in ['unit','options']};p.update(dynamic_display=True,poll=False)
 if f['options']:p['options']=f['options'];p['display']='int'
 if f['key'].endswith(('attack','decay','release','cutoff')):p['taper']='log'
 P.append(p)
for k,name,lo,hi in [('point_x','POINT X',0,100),('point_y','POINT Y',0,100),('slot_source','SOURCE',0,len(SOURCES)-1),('slot_dest','DESTINATION',0,len(DEST)-1),('slot_amount','AMOUNT',-100,100)]:
 p=dict(key=k,name=name,min=lo,max=hi,default=0,dynamic_display=True,poll=False)
 if k=='slot_source':p['options']=SOURCES
 if k=='slot_dest':p['options']=DEST
 P.append(p)
for key in ['add_point','remove_point','panic']:
 P.append(dict(key=key,name=key.upper(),min=0,max=1,default=0,momentary=True,poll=False))
# Static native picture slices. Each screen uses cached 15x15 min/max glyphs.
for kind in ['wave','path','ampgraph','filtergraph','response','lfo1graph','lfo2graph']:
 for i in range(64):P.append(dict(key=f'{kind}{i}',name='Graph',min=0,max=5986 if kind=='wave' else 4095,default=5986 if kind=='wave' else 0,poll=False))
P.append(dict(key='load_status',name='TABLE STATUS',min=0,max=1,default=0,dynamic_display=True,poll=False))
for p in list(P):
 if 'options' in p:
  P.append(dict(key=p['key']+'__open',name='Options',min=0,max=1,default=0,popup_of=p['key']))
for group in range(12):
 for i in range(32):P.append(dict(key=f'pathseg{group}_{i}',name='Path preview',min=0,max=73921,default=0,poll=False))
# Append-only, passive LFO1 line preview; never part of saved musical state.
for i in range(96):P.append(dict(key=f'lfo1line{i}',name='LFO preview',min=0,max=5986,default=5986,poll=False))
for i in range(96):P.append(dict(key=f'lfo2line{i}',name='LFO 2 preview',min=0,max=5986,default=5986,poll=False))
# Three cached filter traces, each split into three 96px bands.
for i in range(864):P.append(dict(key=f'filterline{i}',name='Filter preview',min=0,max=5986,default=5986,poll=False))
# Extend passive plot slots; reuse existing wave/path helpers without moving an index.
for i in range(384,2048):P.append(dict(key=f'pathtile{i}',name='Path tile',min=0,max=73921,default=0,poll=False))
for i in range(64,192):P.append(dict(key=f'wavefine{i}',name='Wave line',min=0,max=5986,default=5986,poll=False))
for i in range(32):P.append(dict(key=f'pathrow{i}',name='Path row',min=0,max=1,default=0,poll=False))
for k in ['start_x','start_y','selected_x','selected_y']:P.append(dict(key='path_'+k,name='Path marker',min=0,max=100,default=0,poll=False))
for i in range(384,512):P.append(dict(key=f'pathdraft{i}',name='Gesture preview',min=0,max=4095,default=0,poll=False))
P.append(dict(key='ui_plot_mode',name='Preview mode',min=0,max=1,default=0,poll=False))
# Append-only import controls; never serialized as musical numeric parameters.
for key in ['import_open','browse_up','browse_root','browse_user','browse_prev','browse_next']+[f'browse_item{i}' for i in range(6)]+['browse_path','browse_page','import_status']:
 P.append(dict(key=key,name=key.upper(),min=0,max=1,default=0,dynamic_display=True,poll=False,momentary=key.startswith('browse_') and key not in ['browse_path','browse_page']))
assert len(P)<=4096, 'Force native skin handles must fit indices 0..4095'
keys=[p['key'] for p in P];lock=B/'params.lock'
if lock.exists():assert keys[:len(old:=lock.read_text().splitlines())]==old,'parameter prefix changed'
lock.write_text('\n'.join(keys)+'\n');(B/'params.json').write_text(json.dumps(P,indent=2)+'\n');(B/'build').mkdir(exist_ok=True)
config=dict(name='πMorpH',vendor='Timo',uid='PiMH',version=102,so='pimorph.so',params='params.json',custom_skin=True,cc=False,nrpn=False,defines=dict(SAMPLE_ACCURATE=1,HAS_SONG_POS=1,PARAM_TEXT_MAX=128,MPC_CHUNK_BYTES=32768,PICN_NOTIFICATION_BUDGET=2,PICN_GRAPHICS_REPORT_FRAMES=256,PICN_PLOT_MODE_PARAM=keys.index('ui_plot_mode'),MODULE_SUBDIR='"."',PIMORPH_FACTORY_PROGRAMS=1))
(B/'vst.json').write_text(json.dumps(config,ensure_ascii=False,indent=2)+'\n');cfg=gen_vst.load(str(B/'vst.json'));gen_vst.gen_params(cfg,P,str(B/'build/params.h'))
with (B/'build/params.h').open('a') as out:out.write('\nstatic const unsigned char PICN_AUTOMATABLE[NPARAMS]={'+','.join('1' if p['key'] in {f['key'] for f in FIELDS} and p['key'] not in ['point_select','slot_select'] else '0' for p in P)+'};\n')
# Separate fine graphics from lightweight markers without moving parameters.
import re
def visual_class(key):
 if key=='ui_plot_mode' or key in ['path_start_x','path_start_y','path_selected_x','path_selected_y']:return 2
 return int(bool(re.fullmatch(r'(?:wave|wavefine|path|ampgraph|filtergraph|response|lfo[12]graph|lfo[12]line|filterline|pathtile|pathrow|pathdraft)\d+|pathseg\d+_\d+',key)))
with (B/'build/params.h').open('a') as out:out.write('static const unsigned char PICN_VISUAL_CLASS[NPARAMS]={'+','.join(str(visual_class(p['key'])) for p in P)+'};\n')
def plot_value(key):return int(bool(re.fullmatch(r'(?:wave|wavefine|pathtile|pathrow)\d+|pathseg\d+_\d+',key)))
with (B/'build/params.h').open('a') as out:out.write('static const unsigned char PICN_PLOT_VALUE[NPARAMS]={'+','.join(str(plot_value(p['key'])) for p in P)+'};\n')
def cstr(s):return json.dumps(s,ensure_ascii=True)
meta=['#pragma once','#include <cstddef>','namespace pmmeta {','struct Param {const char* key;const char* name;double low,high,def;int options;const char* unit;};','static const Param params[]={']
for p in P:
 unit=next((f['unit'] for f in FIELDS if f['key']==p['key']),'');meta.append('{'+','.join([cstr(p['key']),cstr(p['name']),str(p['min']),str(p['max']),str(p['default']),str(len(p.get('options',[]))),cstr(unit)])+'},')
meta+=['};','constexpr int Count=sizeof(params)/sizeof(params[0]);','constexpr int Canonical='+str(len(FIELDS))+';','}'];(B/'build/metadata.h').write_text('\n'.join(meta)+'\n')
from factory_sounds import make_sound,init_sound,state
programs=json.loads((B/'presets/factory-bank.json').read_text());assert len(programs)==101 and programs[0]['name']=='Velvet Field' and programs[-1]['name']=='INIT'
from factory_sounds import validate
for p in programs:validate(p['parameters']);assert p['state']==state(p['filename'],p['parameters'])
header=['#pragma once',f'#define PIMORPH_FACTORY_COUNT {len(programs)}','typedef struct {const char* name;const char* state;} PiMorphFactory;','static const PiMorphFactory PIMORPH_FACTORY[]={']
for p in programs:header.append('{'+cstr(p['name'])+','+cstr(p['state'])+'},')
header.append('};');(B/'build/pimorph_factory_programs.h').write_text('\n'.join(header)+'\n')
(B/'presets/programs.json').write_text(json.dumps(programs,ensure_ascii=False,indent=2)+'\n')
(B/'build/bank_names.h').write_text('#pragma once\nstatic const char* bank_names[]={'+','.join(cstr(p['filename']) for p in programs[:-1])+'};\n')
print('Generated',len(P),'native parameters and',len(programs),'programs')

with (B/'build/params.h').open('a') as out:
 out.write('\nstatic const unsigned char PIMORPH_CURVE[NPARAMS]={'+','.join('1' if p.get('taper')=='log' else '0' for p in P)+'};\n')
 order=sorted(range(len(P)),key=lambda i:P[i]['key'])
 out.write('static const int PICN_PARAM_LOOKUP[NPARAMS]={'+','.join(map(str,order))+'};\n')
