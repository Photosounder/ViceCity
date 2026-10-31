"""Keep C algorithms and same-name C++ bridges in distinct MSVC objects."""
import re
from pathlib import PureWindowsPath
def project(source):
    # Preserve every existing project entry and disambiguate only real paired sources
    anchor='  <Import Project="$(VCTargetsPath)\\Microsoft.Cpp.targets" />'
    if anchor not in source:return source
    pattern=r'<ClCompile Include="([^"]+)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
    entries=list(re.finditer(pattern,source));groups={}
    for entry in entries:
        path=PureWindowsPath(entry[1]);groups.setdefault(path.stem.lower(),set()).add(path.suffix.lower())
    paired={name for name,extensions in groups.items() if {'.c','.cpp'}<=extensions}
    def rename(entry):
        # Give each paired compilation unit an explicit extension-bearing object name
        path=PureWindowsPath(entry[1])
        if path.stem.lower() not in paired:return entry[0]
        metadata='<ObjectFileName>$(IntDir)'+path.name+'.obj</ObjectFileName>'
        body=entry[2] or '\n    '
        if '<ObjectFileName>' in body:body=re.sub(r'<ObjectFileName>[^<]*</ObjectFileName>',lambda _:metadata,body)
        else:body=body.rstrip()+'\n      '+metadata+'\n    '
        return '<ClCompile Include="'+entry[1]+'">'+body+'</ClCompile>'
    return re.sub(pattern,rename,source)
