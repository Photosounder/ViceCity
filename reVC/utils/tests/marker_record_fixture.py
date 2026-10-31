"""Compare all original controller and record methods with the complete C implementation."""
from pathlib import Path
import json,re,runpy
import marker_record_migration as m
root=Path(__file__).resolve().parents[2];out=root/'build/marker-record-c-tests'
original=(out/'before/src/renderer/SpecialFX.cpp').read_text()
for path in json.loads((out/'changes.json').read_text()):
    old=(out/'before'/path).read_text();expected=m.kernel(old,original) if path.endswith('Markers3D.c') else m.calls(old)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert len(re.findall(r'\nC3dMarker::\w+\(',m.record_section(original)))==3
runpy.run_path(str(root/'utils/tests/markers_3d_fixture.py'))
previous=root/'build/markers-3d-c-tests'
for path in previous.iterdir():
    if path.suffix in ['.c','.cpp','.h']:(out/path.name).write_text(path.read_text(),newline='\n')
extra='''bool TestMk_ObjectAdd(void *marker,uint32_t id,uint16_t type,float size);
void TestMk_ObjectDelete(void *marker);
void TestMk_ObjectRender(void *marker);
'''
caller=(out/'caller.h').read_text();caller=caller.replace('#ifdef __cplusplus\n}\n#endif',extra+'#ifdef __cplusplus\n}\n#endif');(out/'caller.h').write_text(caller,newline='\n')
args='void *marker,uint32_t id,uint16_t type,float size'
for old in [True,False]:
    path=out/('old-adapter.cpp' if old else 'caller.c');text=path.read_text()
    if not old:text+='\n#include "Markers3D_backend.h"\n'
    for result,name,params,invoke in [('bool','Add',args,'id,type,size,23,71,199,228,1024,.2f,5'),('void','Delete','void *marker',''),('void','Render','void *marker','')]:
        call=('((C3dMarker *)marker)->'+{'Add':'AddMarker','Delete':'DeleteMarkerObject','Render':'Render'}[name]+'('+invoke+')') if old else 'Markers3D_Record'+name+'(marker'+(','+invoke if invoke else '')+')'
        text+=('extern "C" ' if old else '')+result+' TestMk_Object'+name+'('+params+')\n{\n    // Enter the complete original or C record method through a separate caller\n    '+('return ' if result!='void' else '')+call+';\n}\n'
    path.write_text(text,newline='\n')
template=(root/'utils/tests/markers_3d.cpp.in').read_text()
hooks=[
('event(18);require(index==0);','event(18);require(index==0);if(!triggered && hook==13){triggered=true;marker(0).m_pAtomic=NULL;}'),
('if(!triggered && hook==6){triggered=true;changeInput();}','if(!triggered && hook==6){triggered=true;changeInput();}\n    if(!triggered && hook==14){triggered=true;marker(0).m_fSize=3.125f;marker(0).m_fBrightness=.4375f;}'),
('event(27);draws++;word(atomicIndex(value));','event(27);draws++;word(atomicIndex(value));\n    if(!triggered && hook==15){triggered=true;TestMk_ObjectRender(TestMk_Marker(1));}'),
('event(23);word(color->red);word(color->green);word(color->blue);word(color->alpha);return value;','event(23);word(color->red);word(color->green);word(color->blue);word(color->alpha);\n    if(!triggered && hook==16){triggered=true;marker(0).m_Color.red=211;marker(0).m_Color.alpha=113;word(color->red);word(color->alpha);}\n    return value;'),
('event(19);require(value!=NULL);return frameToken(atomicFrames[atomicIndex(value)]);','event(19);require(value!=NULL);\n    if(!triggered && hook==17){triggered=true;marker(0).m_pAtomic=marker(1).m_pAtomic;}\n    return frameToken(atomicFrames[atomicIndex(value)]);'),
('event(26);word(state);word((uint32_t)(uintptr_t)value);return 1;','event(26);word(state);word((uint32_t)(uintptr_t)value);\n    if(!triggered && hook==18){triggered=true;marker(0).m_nType=MARKERTYPE_ARROW;}\n    return 1;')]
for old,new in hooks:
    assert old in template
    template=template.replace(old,new)
template=template.replace('require(states>1600 && placements>3000', (root/'utils/tests/marker_record_cases.cpp.in').read_text()+'\n    require(states>1600 && placements>3000')
(out/'test.cpp').write_text(template,newline='\n')
print('Audited all three original record methods, six complete changed files and the full combined original/C oracle')
