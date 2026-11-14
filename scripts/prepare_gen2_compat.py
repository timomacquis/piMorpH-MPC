"""Fetch pinned compatibility runtime without changing system packages."""
from pathlib import Path
import os,json,hashlib,urllib.request,subprocess
B=Path(__file__).resolve().parents[1]
root=Path(os.environ.get('PIMORPH_AARCH64_COMPAT','/home/greg/forcework/gen2-toolchain/compat'))
for name,row in json.loads((B/'reference/GEN2-TOOLCHAIN.json').read_text()).items():
 p=B/'build/gen2-downloads'/name;p.parent.mkdir(parents=True,exist_ok=True)
 if not p.exists():urllib.request.urlretrieve(row['url'],p)
 assert hashlib.sha256(p.read_bytes()).hexdigest()==row['sha256'],name
 subprocess.run(['dpkg-deb','-x',str(p),str(root)],check=True)
print('Pinned Gen2 compatibility runtime prepared.')
