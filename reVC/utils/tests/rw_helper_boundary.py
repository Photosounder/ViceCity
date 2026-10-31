"""Build the private C ABI for individual SDK primitives and scoped native rendering-helper storage."""
from pathlib import Path
import hashlib,json,shutil
from rw_helper_schema import HOST,LIB_HOST,ITERATORS

root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rw-helper-c-tests';saved=out/'before/src';tree=out/'overlay/src'
for name,digest in json.loads((out/'baseline-source-digests.json').read_text()).items():
    # Reject source drift before constructing a coherent candidate against the immutable baseline
    assert hashlib.sha256((root/name).read_bytes()).hexdigest()==digest,name
if not tree.exists():shutil.copytree(saved,tree)
from rw_helper_native_generator import build
header,native=build((saved/'rw/RwHelper.cpp').read_text())
for name,value in [('RwHelper_backend.h',header),('RwHelper_native.cpp',native)]:
    # Materialize only the private primitive boundary until the complete C producer is ready
    target=tree/'rw'/name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
print('Prepared',len(HOST)+len(LIB_HOST),'individual primitives, four SDK traversal adapters and four native storage scopes; production unchanged')
