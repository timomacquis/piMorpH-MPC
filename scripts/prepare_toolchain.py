"""Fetch pinned old runtime headers/libs for Force; never modify the shared compiler."""
import json,hashlib,urllib.request,subprocess
from config import B
for name,row in json.loads((B/'reference/GEN1-TOOLCHAIN.json').read_text()).items():
 dest=B/'build/toolchain-downloads'/name;dest.parent.mkdir(parents=True,exist_ok=True)
 if not dest.exists():urllib.request.urlretrieve(row['url'],dest)
 assert hashlib.sha256(dest.read_bytes()).hexdigest()==row['sha256'],name
 root=B/'build/gen1-sysroot';root.mkdir(exist_ok=True)
 subprocess.run(['dpkg-deb','-x',str(dest),str(root)],check=True)
print('Pinned Gen1 runtime prepared.')
