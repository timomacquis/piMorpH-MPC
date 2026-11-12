"""Bank-wide real-engine rendering and full-state roundtrips, offline only."""
import ctypes as C,json,time,math,sys,os
from pathlib import Path
B=Path(__file__).resolve().parents[1];sys.path.insert(0,str(B/'scripts'))
from config import FIELDS
from factory_sounds import make_sound,init_sound
bank_variant=False
bank=B/'Banks/AdventureKid-Animoog'
program_file=bank/'programs.json' if bank_variant else B/'presets/programs.json'
expected=295
rows=json.loads(program_file.read_text());assert len(rows)==expected and rows[-1]['name']=='INIT'
assert len({json.dumps(r['parameters'],sort_keys=True) for r in rows})==expected
for i,r in enumerate(rows[:-1]):assert r['parameters']==make_sound(r['filename'],i)[1]
CREATE=C.CFUNCTYPE(C.c_void_p,C.c_char_p);DESTROY=C.CFUNCTYPE(None,C.c_void_p);MIDI=C.CFUNCTYPE(None,C.c_void_p,C.POINTER(C.c_uint8),C.c_int);SET=C.CFUNCTYPE(None,C.c_void_p,C.c_char_p,C.c_char_p);GET=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_char_p,C.c_char_p,C.c_int);RENDER=C.CFUNCTYPE(None,C.c_void_p,C.POINTER(C.c_int16),C.c_int)
class API(C.Structure):_fields_=[('create',CREATE),('destroy',DESTROY),('midi',MIDI),('set',SET),('get',GET),('render',RENDER),('process',C.c_void_p),('song',C.c_void_p)]
so=C.CDLL(str(bank/'Compiled/pimorph-x86_64.so' if bank_variant else B/'build/pimorph-x86_64.so'));so.mpc_engine.restype=C.POINTER(API);a=so.mpc_engine().contents;p=a.create(str(bank/'Runtime' if bank_variant else B/'build').encode());audio=(C.c_int16*256)();buf=C.create_string_buffer(32768)
def get(key):
 assert a.get(p,key.encode(),buf,len(buf))>0,key
 return buf.value.decode()
def midi(status,n,v):a.midi(p,(C.c_uint8*3)(status,n,v),3)
results=[]
try:
 for i,r in enumerate(rows):
  a.set(p,b'state',r['state'].encode());assert get('state_valid')=='1'
  for f in FIELDS:assert float(get(f['key']))==r['parameters'][f['key']],(r['name'],f['key'])
  saved=get('state');a.set(p,b'state',saved.encode());assert get('state')==saved
  for wait in range(3000):
   a.render(p,audio,128)
   if get('load_status_display')==r['filename']:break
   time.sleep(.001)
  else:raise AssertionError('table timeout '+r['filename'])
  a.set(p,b'panic',b'1');a.render(p,audio,128)
  notes=[48] if r['family'] in ['Bass','Lead'] else [60,64,67]
  for note in notes:midi(0x90,note,100)
  peak=0;energy=0;clipped=0;count=0
  for block in range(520):
   a.render(p,audio,128)
   for v in audio:peak=max(peak,abs(v));energy+=v*v;clipped+=abs(v)>=32760;count+=1
  for note in notes:midi(0x80,note,0)
  for block in range(40):a.render(p,audio,128)
  assert peak>12,(r['name'],'silent',peak)
  assert clipped==0,(r['name'],'clipped',clipped)
  results.append(dict(name=r['name'],family=r['family'],peak=peak,rms=round(math.sqrt(energy/count),2),clipped_samples=clipped))
  if i%30==0:print('Rendered',i+1,'/',len(rows),flush=True)
finally:a.destroy(p)
out=bank if bank_variant else B/'Validation/Factory-Sounds';out.mkdir(exist_ok=True)
(out/'AUDIO-CHECK.json').write_text(json.dumps(dict(offline_only=True,sample_rate=44100,velocity=100,duration_seconds=round(520*128/44100,3),musical_listening_validated=False,results=results),indent=2)+'\n')
print(f'PASS: {expected} distinct deterministic sounds, exact canonical parameter and state roundtrips, table loading, audible renders, no clipped samples in test scenario')
