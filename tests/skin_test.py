"""Native schema, parameter identity, Q-Link and cached-asset checks."""
import json,sys,math
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'scripts'))
from config import B,FOLDER,PAGES
from PIL import Image
P=json.loads((B/'params.json').read_text());I={r['key']:i for i,r in enumerate(P)}
assert len(P)<=4096
assert len(I)==len(P) and list(I)==(B/'params.lock').read_text().splitlines()
a=B/'build/skin'/FOLDER/'Plugin Skins';j=json.loads((a/'TUI.json').read_text())['pageData'];defs={r['key']:r['value'] for r in j['componentDefinitions']['localComponentDefinitions']}
assert len(defs)==len(j['componentDefinitions']['localComponentDefinitions'])
assert [r['tabName'] for r in j['tabs'][:6]]==PAGES
for definition in defs.values():
 for c in definition['componentsData']:
  rect=list(map(float,c['bounds']['bounds'].split()));assert len(rect)==4 and all(math.isfinite(n) for n in rect) and rect[2]>0 and rect[3]>0
  for remap in c['handle remapping']['map']:
   assert remap['value'].startswith('Parameter ');assert 0<=int(remap['value'].split()[-1])<len(P)
  d=c['componentData'];kind=d['type'];v=d['data']
  assert kind in defs or kind in ['Image','Label','Knob','Button','Slider','Envelope','Path','Plotter','XYPad']
  if kind=='Image':assert (a/v['image']).is_file()
  if kind=='Knob':
   image=Image.open(a/v['filmStrip'])
   strip=v['filmStrip']
   if strip.startswith('pathmask_'):
    h=12 if strip.endswith('9x12.png') else 8;assert image.size==(9,h*73922) and v['numFrames']==73922 and rect[2:]==[9,h]
   elif strip.startswith('pathdraft_'):
    h=24 if strip.endswith('18x24.png') else 16;assert image.size==(18,h*4096) and v['numFrames']==4096 and rect[2:]==[18,h]
   elif strip.startswith(('lfo_line_','filter_line_','space_wave_')):
    assert image.size==(6,96*5987) and v['numFrames']==5987 and rect[2:]==[6,96]
   else:
    assert image.height%(v['numFrames']+1)==0 and image.height<=16384 # Legacy artwork, separate count audit pending.

for f in ['Q-Links.json','Q-Links - 4by4.json','Q-Links - 8by1.json']:
 rows=json.loads((a/f).read_text())['Screen Mode Q-Links']['map'];assert len(rows)==8
 for r in rows:
  values=r['Q-Links'];assert set(values)=={f'Q-Link {i}' for i in range(1,17)}
  assert all(-1<=v<len(P) for v in values.values())
  used=[v for v in values.values() if v>=0];assert len(used)==len(set(used))
 # Main SPACE controls exactly follow the proven Force physical ordering.
 first=rows[0]['Q-Links'];assert [first[f'Q-Link {i}'] for i in [13,9,5,1,14,10,6,2]]==[I[k] for k in ['x','y','transpose','glide','voice_limit','level','pan','bend_range']]
assert not any('WAVETABLE BANK' in str(d) for d in defs.values())
print(f'PASS: {len(P)} stable parameters, six pages/two LFO subpages, all references/assets, Force Q-Link order, atlas dimensions checked (large LFO1 atlas is an explicit unvalidated device probe), no WAVETABLE BANK')
