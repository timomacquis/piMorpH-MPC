"""Actual-engine import, browser, failure fallback and saved-state regression tests."""
from pathlib import Path
import ctypes as C, tempfile, wave, math, array, time, json, struct, hashlib
B=Path(__file__).resolve().parents[1]
CREATE=C.CFUNCTYPE(C.c_void_p,C.c_char_p);DESTROY=C.CFUNCTYPE(None,C.c_void_p);MIDI=C.CFUNCTYPE(None,C.c_void_p,C.POINTER(C.c_uint8),C.c_int);SET=C.CFUNCTYPE(None,C.c_void_p,C.c_char_p,C.c_char_p);GET=C.CFUNCTYPE(C.c_int,C.c_void_p,C.c_char_p,C.c_char_p,C.c_int);RENDER=C.CFUNCTYPE(None,C.c_void_p,C.POINTER(C.c_int16),C.c_int)
class API(C.Structure):_fields_=[('create',CREATE),('destroy',DESTROY),('midi',MIDI),('set',SET),('get',GET),('render',RENDER),('process',C.c_void_p),('song',C.c_void_p)]
lib=C.CDLL(str(B/'build/pimorph-x86_64.so'));lib.mpc_engine.restype=C.POINTER(API);api=lib.mpc_engine().contents
p=api.create(str(B/'build').encode());q=api.create(str(B/'build').encode());assert p and q
audio=(C.c_int16*256)();buf=C.create_string_buffer(32768)
def get(k,inst=p):assert api.get(inst,k.encode(),buf,len(buf))>=0,k;return buf.value.decode()
def set(k,v,inst=p):api.set(inst,k.encode(),str(v).encode())
def wait(fn,inst=p):
 until=time.monotonic()+20
 while time.monotonic()<until:
  api.render(inst,audio,128)
  if fn():return
  time.sleep(.002)
 raise AssertionError('timeout: '+get('import_status',inst))
def make(path,frame,channels=1,rate=44100,count=256,kind='pcm16'):
 if kind=='float32':cycle=array.array('f',(.37*math.sin(2*math.pi*i/frame) for i in range(frame)))
 else:cycle=array.array('h' if kind=='pcm16' else 'i',(int((12000 if kind=='pcm16' else 800000000)*math.sin(2*math.pi*i/frame)) for i in range(frame)))
 if channels==2:cycle=array.array(cycle.typecode,(v for value in cycle for v in (value,value)))
 raw=cycle.tobytes()*count;bits=16 if kind=='pcm16' else 32;align=channels*bits//8
 path.write_bytes(struct.pack('<4sI4s4sIHHIIHH4sI',b'RIFF',36+len(raw),b'WAVE',b'fmt ',16,3 if kind=='float32' else 1,channels,rate,rate*align,align,bits,b'data',len(raw))+raw)

try:
 wait(lambda:get('load_status_display').endswith('.wav'))
 factory=get('state');x=get('x');results=[]
 with tempfile.TemporaryDirectory(prefix='pimorph import ') as tmp:
  d=Path(tmp);(d/'Folder').mkdir();(d/'ignored.txt').write_text('not a table')
  for kind in ['pcm16','pcm32','float32']:
   for frame in [512,1024,2048]:
    f=d/f'table {frame} {kind}.WAV';make(f,frame,kind=kind);digest=hashlib.sha256(f.read_bytes()).hexdigest();set('import_path',f)
    wait(lambda:get('import_path')==str(f));assert get('x')==x;assert hashlib.sha256(f.read_bytes()).hexdigest()==digest
    assert f'external "{f}"' in get('state');saved=get('state')
    set('state',saved,q);wait(lambda:get('load_status_display',q)==f.name,q);assert get('state',q)==saved
    api.midi(p,(C.c_uint8*3)(0x90,60,100),3);energy=0
    for _ in range(50):api.render(p,audio,128);energy+=sum(v*v for v in audio)
    assert energy>1000;api.midi(p,(C.c_uint8*3)(0x80,60,0),3);results.append(dict(samples_per_waveform=frame,format=kind))
  for kind in ['pcm16','pcm32','float32']:
   for frame in [512,1024,2048]:
    f=d/f'stereo {frame} {kind}.wav';make(f,frame,channels=2,rate=48000,kind=kind);digest=hashlib.sha256(f.read_bytes()).hexdigest();set('import_path',f)
    wait(lambda:get('import_path')==str(f));assert get('x')==x;assert hashlib.sha256(f.read_bytes()).hexdigest()==digest
    api.midi(p,(C.c_uint8*3)(0x90,60,100),3);energy=0
    for _ in range(25):api.render(p,audio,128);energy+=sum(v*v for v in audio)
    assert energy>1000;results.append(dict(samples_per_waveform=frame,format=kind,channels=2,rate=48000))
  before=get('state')
  cancel=d/'opposed.wav';make(cancel,1024,channels=2)
  raw=bytearray(cancel.read_bytes());values=array.array('h');values.frombytes(raw[44:])
  for i in range(1,len(values),2):values[i]=-values[i]
  cancel.write_bytes(bytes(raw[:44])+values.tobytes());set('import_path',cancel);wait(lambda:get('import_status')!='Loading wavetable...');assert get('state')==before and 'Silent' in get('import_status')
  for name,ch,rate,count in [('surround.wav',3,44100,256),('short.wav',1,44100,10)]:
   f=d/name;make(f,1024,ch,rate,count);set('import_path',f);wait(lambda:get('import_status')!='Loading wavetable...');assert get('state')==before,(name,'old table/state lost')
  set('import_path',d/'missing.wav');wait(lambda:get('import_status')!='Loading wavetable...');assert get('state')==before
  set('browse_path_set',d);wait(lambda:get('browse_path_display')==str(d));assert '[DIR] Folder'==get('browse_item0_display')
  entries=[get(f'browse_item{i}_display') for i in range(6)];assert not any('ignored.txt' in e for e in entries)
  set('browse_item0',1);wait(lambda:get('browse_path_display')==str(d/'Folder'));set('browse_up',1);wait(lambda:get('browse_path_display')==str(d))
  # Clicking a listed WAV uses the same import path and closes the panel on success.
  set('import_open',1);set('browse_path_set',d);wait(lambda:get('browse_path_display')==str(d))
  pick=d/'Pick';pick.mkdir();f=pick/'chosen.wav';make(f,512);set('browse_path_set',pick);wait(lambda:get('browse_item0_display')=='chosen.wav');set('browse_item0',1);wait(lambda:get('import_path')==str(f));assert get('import_open')=='0'
  # Latest request wins; no stale loaded table can overwrite it.
  for frame in [512,1024,2048]:set('import_path',d/f'table {frame} pcm16.WAV')
  wait(lambda:get('import_path')==str(d/'table 2048 pcm16.WAV'))
  set('state',factory);wait(lambda:get('load_status_display')=='Velvet-Field.wav');assert get('import_path')=='' and get('state')==factory
 (B/'Validation/IMPORT-TEST.json').write_text(json.dumps(dict(formats=results,old_state_compatible=True,import_state_roundtrip=True,invalid_input_preserves_sound_state=True,browser_navigation=True,latest_request_wins=True,hardware_validated=False),indent=2)+'\n')
 print('PASS: PCM16/PCM32/float32 512/1024/2048 x256 imports, audible rendering, state restore, invalid/missing file fallback, browser, rapid cancellation and original preset return')
finally:api.destroy(p);api.destroy(q)
