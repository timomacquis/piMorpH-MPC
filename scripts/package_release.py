"""Standard Open MPC public Gen1 package; no device access."""
from pathlib import Path
import json,hashlib,zipfile,shutil,subprocess,sys,xml.etree.ElementTree as E
from config import B,FOLDER,VERSION
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
assert VERSION=='0.1.0'
for name in ['NATIVE-TEST.txt','HOST-TEST.txt']:assert 'PASS:' in (B/'Validation'/name).read_text()
audio=json.loads((B/'Validation/Factory-Sounds/AUDIO-CHECK.json').read_text());assert len(audio['results'])==295 and all(r['clipped_samples']==0 and r['peak']>12 for r in audio['results'])
built=json.loads((B/'build/build-receipt.json').read_text())
for name,h in built['inputs'].items():assert sha(B/name)==h,'Stale input: '+name
assert sha(B/'build/pimorph.so')==built['outputs']['pimorph.so']
stage=B/'build/public-stage'/FOLDER
if stage.exists():shutil.rmtree(stage)
shutil.copytree(B/'build/skin'/FOLDER,stage)
shutil.copytree(B/'Wavetables',stage/'Wavetables');shutil.copytree(B/'docs',stage/'docs');shutil.copytree(B/'licenses',stage/'licenses')
shutil.copy2(B/'LICENSE',stage/'licenses/piMorpH-GPL-3.0.txt');shutil.copy2(B/'THIRD-PARTY.md',stage/'licenses/THIRD-PARTY.md')
for src,dst in [('third_party/mpc-wrapper/LICENSE','wrapper-MIT.txt'),('third_party/PolyWerk/LICENSE','PolyWerk-MIT.txt'),('third_party/MS20XXL/LICENSE','MS20XXL-GPL.txt'),('third_party/overcast/LICENSE','Overcast-MIT.txt'),('reference/fonts/OFL.txt','Titillium-OFL.txt')]:shutil.copy2(B/src,stage/'licenses'/dst)
shutil.copy2(B/'presets/programs.json',stage/'factory-programmes.json')
(stage/'PROVENANCE.json').write_text(json.dumps(dict(version=VERSION,architecture='armv7',engine_sha256=sha(B/'build/pimorph.so'),factory_programmes=295,tables=294,source_repo='timomacquis/piMorpH-MPC',build_inputs=built['inputs'],hardware_qualified=False),indent=2)+'\n')
entry=B/'build/release-entry.xml';entry.write_text(E.tostring(E.Element('PLUGIN',name='πMorpH',descriptiveName='πMorpH',format='VST',category='Synth',manufacturer='Timo',version=VERSION,file='%payload-path%/'+FOLDER+'/pimorph.so',uid='50694d48',isInstrument='1',numInputs='0',numOutputs='2'),encoding='unicode'))
out=B/'dist';out.mkdir(exist_ok=True);tools=B/'third_party/open-mpc-release/tools'
subprocess.run([sys.executable,str(tools/'release.py'),'--so',str(B/'build/pimorph.so'),'--skin',str(stage),'--entry',str(entry),'--version',VERSION,'--id','pimorph','--repo','timomacquis/piMorpH-MPC','--license','GPL-3.0-or-later AND CC0-1.0','--about','Polyphonic XY wavetable synthesizer with paths, orbit, dual envelopes, HP/LP filters, dual LFOs, modulation matrix and 294 Adventure Kid wavetable sounds.','-o',str(out)],check=True)
p=out/('MorpH-'+VERSION+'-mpc-armv7.zip');target=out/('piMorpH-'+VERSION+'-mpc-armv7.zip');p.replace(target)
subprocess.run([sys.executable,str(tools/'catalog_check.py'),str(target),'--catalog','--require-portable','--expect-id','pimorph','--expect-repo','timomacquis/piMorpH-MPC'],check=True)
with zipfile.ZipFile(target) as z:
 assert z.testzip() is None
 sums=next(n for n in z.namelist() if n.endswith('/SHA256SUMS'));top=sums.rsplit('/',1)[0]
 for line in z.read(sums).decode().splitlines():
  h,n=line.split('  ',1);assert hashlib.sha256(z.read(top+'/'+n)).hexdigest()==h,n
 assert len([n for n in z.namelist() if '/Wavetables/' in n and n.endswith('.wav')])==294
 assert not any(x in n for n in z.namelist() for x in ['References-privees','AC-EBassShift','MMDOctaveSaws','mywavetables.zip'])
Path(str(target)+'.sha256').write_text(sha(target)+'  '+target.name+'\n')
(B/'Validation/PUBLIC-PACKAGE.json').write_text(json.dumps(dict(path=str(target),sha256=sha(target),catalog_check=True,tables=294,programmes=295,installed=False,published=False),indent=2)+'\n');print(target)
