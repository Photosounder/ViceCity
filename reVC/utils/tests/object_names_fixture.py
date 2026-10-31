"""Verify actual MSBuild object outputs remain unique in every project configuration."""
from pathlib import Path,PureWindowsPath
from collections import defaultdict
import json,os,subprocess,xml.etree.ElementTree as ET
root=Path(__file__).resolve().parents[2];out=root/'build/object-collision-tests';out.mkdir(exist_ok=True)
msbuild='C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe'
ns={'m':'http://schemas.microsoft.com/developer/msbuild/2003'}
tree=ET.parse(root/'build/reVC.vcxproj');results=[]
for item in tree.findall('.//m:ProjectConfiguration',ns):
    configuration,platform=item.get('Include').split('|')
    args=[msbuild,'build/reVC.vcxproj','-nologo','-p:Configuration='+configuration,'-p:Platform='+platform,'-getItem:ClCompile']
    run=subprocess.run(args,cwd=root,env=dict(os.environ),capture_output=True,text=True)
    assert run.returncode==0,run.stdout+run.stderr
    sources=json.loads(run.stdout)['Items']['ClCompile'];objects=defaultdict(list);pairs=defaultdict(list)
    for source in sources:
        if source.get('ExcludedFromBuild','').lower()=='true':continue
        path=PureWindowsPath(source['Identity']);target=source.get('ObjectFileName','')
        if not target or target.endswith(('\\','/')):target+=path.stem+'.obj'
        objects[str(PureWindowsPath(target)).lower()].append(source['Identity'])
        pairs[path.stem.lower()].append(source)
    duplicates={target:files for target,files in objects.items() if len(files)>1}
    assert not duplicates,(configuration,platform,duplicates)
    paired=[files for files in pairs.values() if {'.c','.cpp'}<=set(PureWindowsPath(f['Identity']).suffix.lower() for f in files)]
    assert len(paired)==28
    for files in paired:
        assert len({f['ObjectFileName'].lower() for f in files})==len(files)
    results.append({'configuration':configuration,'platform':platform,'sources':len(objects),'pairs':len(paired)})
(out/'evaluated-configurations.json').write_text(json.dumps(results,indent=2),newline='\n')
print('Verified unique MSBuild object paths in',len(results),'configurations, including all twenty-eight C/C++ pairs')
