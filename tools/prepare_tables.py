from pathlib import Path
import json,hashlib
B=Path(__file__).resolve().parents[1];out=B/'build/tables';out.mkdir(parents=True,exist_ok=True);rows=[]
for stale in out.glob('*.wav'):
 if not (B/'Wavetables'/stale.name).exists():stale.unlink()
for p in sorted((B/'Wavetables').glob('*.wav')):
 data=p.read_bytes();q=out/p.name;q.write_bytes(data);rows.append(dict(name=p.stem,path=str(q.relative_to(B)),sha256=hashlib.sha256(data).hexdigest()))
assert len(rows)==100
(B/'build/prepared-tables.json').write_text(json.dumps(rows,indent=2)+'\n')
link=B/'build/Wavetables'
if not link.exists():link.symlink_to('tables',target_is_directory=True)
print('Prepared 100 Greg original tables')
