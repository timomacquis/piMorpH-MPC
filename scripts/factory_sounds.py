"""Deterministic, bounded musical starting points. No runtime random generation."""
import hashlib,random,json
from config import FIELDS
FAMILIES=['Bass','Pad','Pluck','Lead','Texture','Keys']

def make_sound(filename,index):
 rng=random.Random(int.from_bytes(hashlib.sha256(filename.encode()).digest()[:8],'big'))
 n=filename.lower()
 family=('Bass' if 'bass' in n else 'Pad' if any(x in n for x in ['vox','vowel','aah','ooh','choir','string']) else 'Pluck' if any(x in n for x in ['bell','pluck','lpg']) else FAMILIES[index%6])
 p={f['key']:f['default'] for f in FIELDS}
 # Each preset includes every canonical value, including unused modulation slots.
 p.update(x=rng.randint(22,78),y=rng.randint(22,78),hp_res=rng.randint(3,12),lp_res=rng.randint(8,23),drive=rng.randint(0,8),level=58,pan=0,point_select=0,slot_select=0)
 recipes={
 'Bass':dict(amp_attack=3,amp_decay=380,amp_sustain=62,amp_release=100,filter_attack=2,filter_decay=330,filter_sustain=15,filter_release=120,hp_cutoff=25,lp_cutoff=2100,lp_env=27,keytrack=45,play_mode=1,voice_limit=1,transpose=-12,glide=22,path_mode=0,path_depth=10,orbit_width=1,orbit_height=1),
 'Pad':dict(amp_attack=1100,amp_decay=2900,amp_sustain=78,amp_release=2200,filter_attack=1800,filter_decay=2400,filter_sustain=65,filter_release=2300,hp_cutoff=110,lp_cutoff=4800,lp_env=12,keytrack=25,path_mode=2,path_rate=6,path_depth=32,orbit_width=4,orbit_height=3),
 'Pluck':dict(amp_attack=2,amp_decay=520,amp_sustain=0,amp_release=190,filter_attack=1,filter_decay=440,filter_sustain=0,filter_release=160,hp_cutoff=65,lp_cutoff=3300,lp_env=30,keytrack=40,path_mode=1,path_rate=3,path_depth=18,orbit_width=0,orbit_height=0),
 'Lead':dict(amp_attack=12,amp_decay=700,amp_sustain=76,amp_release=220,filter_attack=18,filter_decay=630,filter_sustain=50,filter_release=240,hp_cutoff=70,lp_cutoff=5200,lp_env=14,keytrack=40,play_mode=2,voice_limit=1,glide=45,path_mode=0,path_depth=16,orbit_width=2,orbit_height=1),
 'Texture':dict(amp_attack=450,amp_decay=3200,amp_sustain=68,amp_release=1600,filter_attack=900,filter_decay=2700,filter_sustain=40,filter_release=1500,hp_cutoff=180,lp_cutoff=6500,lp_env=10,keytrack=20,path_mode=3,path_rate=6,path_depth=44,orbit_width=6,orbit_height=5),
 'Keys':dict(amp_attack=7,amp_decay=1250,amp_sustain=24,amp_release=450,filter_attack=5,filter_decay=900,filter_sustain=20,filter_release=400,hp_cutoff=85,lp_cutoff=4100,lp_env=22,keytrack=50,path_mode=1,path_rate=4,path_depth=23,orbit_width=2,orbit_height=1)}
 p.update(recipes[family])
 for key in ['amp_attack','amp_decay','amp_release','filter_attack','filter_decay','filter_release','lp_cutoff']:
  p[key]=round(p[key]*rng.uniform(.8,1.2))
 p['filter_model']=rng.choice([1,2]);p['orbit_phase']=rng.randint(0,359)
 count=rng.randint(3,6);p['point_count']=count
 for i in range(12):
  p[f'point{i}_x']=rng.randint(15,85);p[f'point{i}_y']=rng.randint(15,85)
 def route(slot,source,dest,amount):p.update({f'mod{slot}_source':source,f'mod{slot}_dest':dest,f'mod{slot}_amount':amount})
 # Velocity opens the filter; mod wheel adds modest position movement.
 route(0,5,9,6 if family=='Bass' else 10);route(1,7,1,12)
 if family in ['Pad','Texture','Lead']:
  p.update(lfo1_on=1,lfo1_wave=0,lfo1_rate=1 if family!='Lead' else .5,lfo1_depth=40,lfo1_reset=1)
  route(2,1,2,6 if family!='Lead' else 3)
 if family=='Texture':
  p.update(lfo2_on=1,lfo2_wave=6,lfo2_rate=1,lfo2_depth=35,lfo2_reset=1)
  route(3,2,9,9)
 validate(p)
 return family,p

def init_sound():
 p={f['key']:f['default'] for f in FIELDS}
 p.update(path_mode=0,path_depth=0,orbit_width=0,orbit_height=0,filter_model=0,amp_attack=5,amp_decay=500,amp_sustain=80,amp_release=200,level=58)
 validate(p);return p

def validate(p):
 assert set(p)=={f['key'] for f in FIELDS}
 for f in FIELDS:
  v=p[f['key']];assert f['min']<=v<=f['max'],(f['key'],v)
  if f['options']:assert v==int(v)

def state(filename,p):
 return 'PMORPH1\nbank '+json.dumps(filename,ensure_ascii=False)+'\n'+''.join(f'{f["key"]} {p[f["key"]]}\n' for f in FIELDS)+'END\n'
