"""Build/package Gen2 from the released Gen1 skin and identical engine inputs. No device access."""
from pathlib import Path
import os,sys,json,hashlib,subprocess,zipfile,shutil,re,xml.etree.ElementTree as ET
from PIL import Image
from config import B,FOLDER,VERSION
DESCRIPTION="Polyphonic XY wavetable synthesizer with paths, orbit, envelopes, filters, dual LFOs, modulation matrix and Custom Wavetable import."
os.chdir(B)
sha=lambda p:hashlib.sha256(Path(p).read_bytes()).hexdigest()
receipt=json.loads((B/'build/build-receipt.json').read_text())
source_hashes={n:h for n,h in receipt['inputs'].items() if n.startswith(('src/','third_party/','Wavetables/')) or n in ('params.json','params.lock','vst.json','presets/factory-bank.json','build/params.h','build/pimorph_factory_programs.h','build/bank_names.h')}
for name,digest in source_hashes.items():assert sha(B/name)==digest,'Changed published input: '+name
out=B/'build/gen2';out.mkdir(parents=True,exist_ok=True)
root=Path(os.environ.get('PIMORPH_AARCH64_SYSROOT','/home/greg/forcework/gen2-toolchain/root'))
env=dict(os.environ,PATH=str(root/'usr/bin')+':'+os.environ['PATH'],LD_LIBRARY_PATH=str(root/'usr/lib/x86_64-linux-gnu')+':'+os.environ.get('LD_LIBRARY_PATH',''))
compat=Path(os.environ.get('PIMORPH_AARCH64_COMPAT','/home/greg/forcework/gen2-toolchain/compat'))
extra=['--sysroot='+str(compat),'-B'+str(compat/'usr/lib/gcc-cross/aarch64-linux-gnu/10')+'/', '-B'+str(compat/'usr/aarch64-linux-gnu/lib')+'/', '-L'+str(compat/'usr/aarch64-linux-gnu/lib'),'-L'+str(compat/'usr/lib/gcc-cross/aarch64-linux-gnu/10'),'-O2','-fPIC','-pthread','-Ibuild','-Ithird_party/mpc-wrapper/wrapper']
objects=[]
for source,c in [('third_party/mpc-wrapper/wrapper/vst2_wrap.c',True),('src/engine.cpp',False),('third_party/PolyWerk/dsp/wavetable.cpp',False)]:
 obj=out/(Path(source).stem+'.o');objects.append(str(obj))
 subprocess.run([str(root/'usr/bin'/('aarch64-linux-gnu-gcc-12' if c else 'aarch64-linux-gnu-g++-12')),*extra,*([] if c else ['-nostdinc++','-isystem',str(compat/'usr/aarch64-linux-gnu/include/c++/10'),'-isystem',str(compat/'usr/aarch64-linux-gnu/include/c++/10/aarch64-linux-gnu')]),'-isystem',str(compat/'usr/aarch64-linux-gnu/include'),'-std=c11' if c else '-std=c++17','-D_POSIX_C_SOURCE=200809L','-c',source,'-o',str(obj)],env=env,check=True)
so=out/'pimorph.so'
subprocess.run([str(root/'usr/bin/aarch64-linux-gnu-g++-12'),*extra,'-shared',*objects,'-static-libstdc++','-static-libgcc','-pthread','-lm','-ldl','-Wl,--no-undefined','-Wl,-z,relro,-z,now,-z,noexecstack','-o',str(so)],env=env,check=True)
subprocess.run([str(root/'usr/bin/aarch64-linux-gnu-strip'),str(so)],env=env,check=True)
report=subprocess.check_output([str(root/'usr/bin/aarch64-linux-gnu-readelf'),'-h','-d','--version-info',str(so)],env=env,text=True)
(out/'ELF.txt').write_text(report)
raw=so.read_bytes();assert raw[:5]==b'\x7fELF\x02' and int.from_bytes(raw[18:20],'little')==183
versions=set(re.findall(r'GLIBC_([0-9.]+)',report));assert all(tuple(map(int,v.split('.')))<=(2,34) for v in versions),versions
source=B/f'dist/piMorpH-{VERSION}-mpc-armv7.zip';original=sha(source)
stage=out/FOLDER
if stage.exists():shutil.rmtree(stage)
stage.mkdir()
with zipfile.ZipFile(source) as z:
 prefix=next(n[:-len('Plugin Skins/TUI.json')] for n in z.namelist() if n.endswith('/Plugin Skins/TUI.json'))
 for n in z.namelist():
  if n.startswith(prefix) and not n.endswith('/'):
   rel=n[len(prefix):]
   if rel=='pimorph.so':continue
   dest=stage/rel;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(z.read(n))
assets=stage/'Plugin Skins';tui=assets/'TUI.json';j=json.loads(tui.read_text());grown={}
def walk(x):
 if isinstance(x,dict):
  if x.get('type')=='Image' and isinstance(x.get('data'),dict) and x['data'].get('image'):
   name=x['data']['image']
   if name not in grown:
    im=Image.open(assets/name);w,h=im.size;new=name
    if w*h<6400:
     pw=max(w,80);ph=max(h,(6400+pw-1)//pw);canvas=Image.new(im.mode,(pw,ph),(0,0,0,0) if im.mode=='RGBA' else im.getpixel((0,0)));canvas.paste(im,(0,0));new='g2_'+name;canvas.save(assets/new)
    grown[name]=new
   x['data']['image']=grown[name]
  for v in x.values():walk(v)
 elif isinstance(x,list):
  for v in x:walk(v)
walk(j);tui.write_text(json.dumps(j,indent=2)+'\n')
notes='Gen2 ARM64 build of πMorpH '+VERSION+'. Compiled and package-checked on PC; NOT TESTED ON GEN2 HARDWARE. 100 factory sounds + INIT. Small Image assets padded for Gen2 rendering. Gen1 package unchanged. Please report device model, OS and results for sound, presets, Q-Links, samples and effects.\n'
(stage/'GEN2-STATUS.md').write_text(notes)
(stage/'PROVENANCE.json').write_text(json.dumps(dict(version=VERSION,architecture='aarch64',hardware_verified=False,gen1_package_sha256=original,engine_sha256=sha(so),source_hashes=source_hashes,small_images_padded=sum(k!=v for k,v in grown.items())),indent=2)+'\n')
entry=out/'entry.xml';entry.write_text(ET.tostring(ET.Element('PLUGIN',name='πMorpH',descriptiveName='πMorpH',format='VST',category='Synth',manufacturer='Timo',version=VERSION,file='%payload-path%/'+FOLDER+'/pimorph.so',uid='50694d48',isInstrument='1',numInputs='0',numOutputs='2'),encoding='unicode'))
dest=B/'dist'
subprocess.run([sys.executable,str(B/'third_party/open-mpc-release/tools/release.py'),'--so',str(so),'--skin',str(stage),'--entry',str(entry),'--version',VERSION,'--id','pimorph','--repo','timomacquis/piMorpH-MPC','--license','GPL-3.0-or-later','--about',DESCRIPTION,'-o',str(dest)],check=True)
z=dest/f'MorpH-{VERSION}-mpc-aarch64.zip';target=dest/f'piMorpH-{VERSION}-mpc-aarch64.zip';z.replace(target)
subprocess.run([sys.executable,str(B/'third_party/open-mpc-release/tools/catalog_check.py'),str(target),'--catalog','--require-portable','--expect-id','pimorph','--expect-repo','timomacquis/piMorpH-MPC'],check=True)
with zipfile.ZipFile(target) as z:
 assert z.testzip() is None
 assert sum('/Wavetables/' in n and n.endswith('.wav') for n in z.namelist())==100
 sums=next(n for n in z.namelist() if n.endswith('/SHA256SUMS'));top=sums.rsplit('/',1)[0]
 for line in z.read(sums).decode().splitlines():
  digest,name=line.split('  ',1);assert hashlib.sha256(z.read(top+'/'+name)).hexdigest()==digest,name
assert sha(source)==original
Path(str(target)+'.sha256').write_text(sha(target)+'  '+target.name+'\n')
(out/'RESULT.json').write_text(json.dumps(dict(package=str(target),sha256=sha(target),hardware_verified=False,gen1_unchanged=True,glibc_versions=sorted(versions),programs=101,tables=100,images_padded=sum(k!=v for k,v in grown.items())),indent=2)+'\n')
print(notes, target,sha(target))