"""Meaningful sanitized engine/wrapper tests; no device access."""
import subprocess,sys,os
from config import B
os.chdir(B)
(B/'Validation').mkdir(exist_ok=True)
(B/'build').mkdir(exist_ok=True)
def run(args,out=None):
 result=subprocess.run(args,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True)
 if out:(B/'Validation'/out).write_text(result.stdout)
 print(result.stdout,end='');result.check_returncode()
flags=['-O1','-g','-fno-omit-frame-pointer','-fsanitize=address,undefined','-Ibuild','-Ithird_party/mpc-wrapper/wrapper']
sources=['src/engine.cpp','third_party/PolyWerk/dsp/wavetable.cpp']
run(['g++','-std=c++17','-pthread',*flags,'tests/native_test.cpp',*sources,'-o','build/native-test'])
run(['./build/native-test'],'NATIVE-TEST.txt')
run(['gcc','-std=c11',*flags,'-c','tests/host_test.c','-o','build/host-test.o'])
run(['g++','-std=c++17','-pthread',*flags,'build/host-test.o',*sources,'-ldl','-lm','-o','build/host-test'])
run(['./build/host-test'],'HOST-TEST.txt')
# 0.2.0 : arpégiateur (logique seule, puis dans le moteur).
run(['g++','-std=c++17',*flags,'-Isrc','tests/arp_test.cpp','-o','build/arp-test'])
run(['./build/arp-test'],'ARP-TEST.txt')
run(['g++','-std=c++17','-pthread',*flags,'tests/arp_engine_test.cpp',*sources,'-o','build/arp-engine-test'])
run(['./build/arp-engine-test'],'ARP-ENGINE-TEST.txt')
# 0.2.0 (candidat 6) : DELAY Dub Station + REVERB Tidal Verb.
run(['g++','-std=c++17','-pthread',*flags,'tests/fx_engine_test.cpp',*sources,'-o','build/fx-engine-test'])
run(['./build/fx-engine-test'],'FX-ENGINE-TEST.txt')
# 0.2.0 (candidat 2) : effets REVERB + DELAY repris de πToniC.
