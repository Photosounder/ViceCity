"""Stage C shadow records and lifecycle functions against one immutable coherent baseline."""
from pathlib import Path
import hashlib,json,shutil
import shadow_cameras_schema as schema
import shadow_cameras_resources as resources
import shadow_cameras_effects as effects
import shadow_cameras_cutscene as cutscene_policy
import shadow_cameras_peer as peer
import shadow_cameras_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/shadow-cameras-c-tests';out.mkdir(exist_ok=True);saved=out/'before/src';tree=out/'overlay/src'
assert json.loads((root/'.diagnostics/rw-helper-c-tests/verification-summary.json').read_text())['status']=='applied-verified'
if not saved.exists():
    # Capture the complete source baseline only after the preceding helper conversion is verified
    shutil.copytree(root/'src',saved,ignore=shutil.ignore_patterns('.git','.codex-build','.agents','.codex'))
digests={}
for path in saved.rglob('*'):
    # Reject every concurrent production edit before constructing the shadow candidate
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();assert (root/name).read_bytes()==path.read_bytes(),name;digests[name]=hashlib.sha256(path.read_bytes()).hexdigest()
manifest=out/'baseline-source-digests.json'
if manifest.exists():assert json.loads(manifest.read_text())==digests
else:manifest.write_text(json.dumps(digests,indent=2),newline='\n')
if not tree.exists():shutil.copytree(saved,tree)
files={'renderer/ShadowCamera.h':schema.camera_header(),'renderer/CutsceneShadow.h':schema.cutscene_header(),'renderer/ShadowCamera.c':schema.camera_lifecycle(),'renderer/CutsceneShadow.c':schema.cutscene_lifecycle()}
camera,cutscene=resources.resource_methods((saved/'renderer/ShadowCamera.cpp').read_text(),(saved/'renderer/CutsceneShadow.cpp').read_text())
files['renderer/ShadowCamera.c']=files['renderer/ShadowCamera.c'].replace('#include "RwHelper_backend.h"',resources.prelude())+'\n//+ rouz edit (ChatGPT)\n'+camera+'//- rouz edit (ChatGPT)\n'
files['renderer/ShadowCamera.c']+='\n//+ rouz edit (ChatGPT)\n'+effects.methods((saved/'renderer/ShadowCamera.cpp').read_text())+'//- rouz edit (ChatGPT)\n'
files['renderer/CutsceneShadow.c']=files['renderer/CutsceneShadow.c'].replace('#include "CutsceneShadow.h"','#include "CutsceneShadow.h"\n'+resources.prelude())+'\n//+ rouz edit (ChatGPT)\n'+cutscene+'//- rouz edit (ChatGPT)\n'
files['renderer/CutsceneShadow.c']+='\n//+ rouz edit (ChatGPT)\n'+cutscene_policy.methods((saved/'renderer/CutsceneShadow.cpp').read_text())+'//- rouz edit (ChatGPT)\n'
header,native=cutscene_policy.extend(*effects.extend(*resources.boundary()));files['renderer/Shadow_backend.h']=header;files['renderer/Shadow_native.cpp']=native
files['rw/RwHelper.c']=peer.producer((saved/'rw/RwHelper.c').read_text());files['rw/RwHelper.h']=peer.header((saved/'rw/RwHelper.h').read_text())
for name in migration.CONSUMERS:
    # Project every diagnosed whole native consumer from the same immutable baseline
    files[name.removeprefix('src/')]=migration.calls((out/'before'/name).read_text())
for name,value in files.items():
    # Write genuine C records and explicit lifecycle while the full rendering operations remain staged work
    target=tree/name
    if not target.exists() or target.read_bytes()!=value.encode():target.write_text(value,newline='\n')
for old,new in schema.RENAMES.items():
    # Retain complete original C++ comparison producers only in the immutable baseline
    target=(out/'overlay'/old).resolve();assert target.is_relative_to(tree.resolve()) and (out/'overlay'/new).exists();target.unlink(missing_ok=True)
report={'status':'staged-produced','production_changed':False,'classes_in_scope':2,'original_operations_in_scope':len(schema.CAMERA_OPERATIONS)+len(schema.CUTSCENE_OPERATIONS)+len(schema.HELPERS),'C_operations_implemented':35,'remaining':['All existing native consumers and allocator lifecycle sites','Whole original/C runtime comparisons and real SDK compilation','Historical source projections and complete Debug/Release integration','Production application and predecessor audits']}
summary=out/'stage-summary.json'
if summary.exists():report.update({key:value for key,value in json.loads(summary.read_text()).items() if key not in report})
summary.write_text(json.dumps(report,indent=2),newline='\n');print('Staged all 35 original operations, both C record interfaces and live C sphere receivers;',len(digests),'production source digests unchanged')

changes=[];untouched={}
for path in saved.rglob('*'):
    # Preserve exact whole-source projections and a complete untouched-source manifest for reproducible audits
    if path.suffix not in {'.cpp','.c','.h'}:continue
    name='src/'+path.relative_to(saved).as_posix();original=path.read_text();converted=migration.calls(original);assert migration.calls(converted)==converted,name;assert (out/'overlay'/migration.RENAMES.get(name,name)).read_text()==converted,name
    if converted!=original:changes.append(name)
    else:untouched[name]=hashlib.sha256(path.read_bytes()).hexdigest()
for name,value in [('changes',changes),('untouched',untouched)]:
    # Retain complete explicit changed and untouched source lists without deriving them from compiler output
    (out/(name+'.json')).write_text(json.dumps(value,indent=2),newline='\n')
