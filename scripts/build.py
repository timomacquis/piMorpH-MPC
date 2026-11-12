"""Local x86 and Gen1 build. No device or publication action."""
import os,sys,subprocess,json,hashlib
from pathlib import Path
from config import B
os.chdir(B)
def run(args,**kw):subprocess.run([str(a) for a in args],check=True,**kw)
run([sys.executable,'scripts/generate.py'])
root=Path(os.environ.get('PIMORPH_SYSROOT',B.parent/'πClideaN/toolchain/root'))
env=dict(os.environ);env['PATH']=str(root/'usr/bin')+os.pathsep+env.get('PATH','');env['LD_LIBRARY_PATH']=str(root/'usr/lib/x86_64-linux-gnu')+os.pathsep+env.get('LD_LIBRARY_PATH','')
common=['-O2','-fPIC','-pthread','-Ibuild','-Ithird_party/mpc-wrapper/wrapper']
for arch in ['x86_64','armv7']:
 cross=arch=='armv7';cc=root/'usr/bin/arm-linux-gnueabihf-gcc-12' if cross else 'gcc';cxx=root/'usr/bin/arm-linux-gnueabihf-g++-12' if cross else 'g++'
 old=B/'build/gen1-sysroot';lib=old/'usr/arm-linux-gnueabihf/lib';stdlib=old/'usr/lib/gcc-cross/arm-linux-gnueabihf/10'
 extra=['--sysroot='+str(old),'-isystem',str(old/'usr/arm-linux-gnueabihf/include'),'-B'+str(lib)+'/', '-L'+str(lib),'-L'+str(stdlib),'-march=armv7-a','-mfpu=neon','-mfloat-abi=hard'] if cross else []
 work=B/'build'/arch;work.mkdir(parents=True,exist_ok=True)
 objects=[]
 for source,c in [('third_party/mpc-wrapper/wrapper/vst2_wrap.c',True),('src/engine.cpp',False),('third_party/PolyWerk/dsp/wavetable.cpp',False)]:
  obj=work/(Path(source).stem+'.o');objects.append(obj)
  cpp=['-nostdinc++','-isystem',str(old/'usr/arm-linux-gnueabihf/include/c++/10'),'-isystem',str(old/'usr/arm-linux-gnueabihf/include/c++/10/arm-linux-gnueabihf')] if cross and not c else []
  run([cc if c else cxx,*cpp,*extra,*common,'-std=c11' if c else '-std=c++17','-D_POSIX_C_SOURCE=200809L','-c',source,'-o',obj],env=env if cross else None)
 output=B/'build'/('pimorph.so' if cross else 'pimorph-x86_64.so')
 run([cxx,*extra,'-shared',*objects,'-pthread','-lm','-ldl','-static-libstdc++','-static-libgcc','-o',output],env=env if cross else None)
files=[]
for folder in ['src','scripts','tests','third_party','reference']:
 files += [p for p in (B/folder).rglob('*') if p.is_file() and '__pycache__' not in p.parts and p.suffix!='.pyc']
files += [B/'vst.json',B/'params.json',B/'params.lock']
receipt={'inputs':{str(f.relative_to(B)):hashlib.sha256(f.read_bytes()).hexdigest() for f in sorted(files)},'outputs':{n:hashlib.sha256((B/'build'/n).read_bytes()).hexdigest() for n in ['pimorph.so','pimorph-x86_64.so']}}
(B/'build/build-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
print('Built native x86_64 and ARMv7. Hardware not accessed.')
