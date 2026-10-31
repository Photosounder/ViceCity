"""Prepare exact original geometry, independent C callers and literal native initialization sites."""
from pathlib import Path
import re,json,sys
import rect_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/rect-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
assert re.search(r'CRGBA\(void\)\s*\{\s*\}',(saved/'src/core/common.h').read_text())
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
for name in ['Rect.h','Rect.c']:(out/name).write_text((tree/'src/math'/name).read_text(),newline='\n')
(out/'original-Rect.h').write_text((saved/'src/math/Rect.h').read_text(),newline='\n')
(out/'owners.h').write_text('''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>
struct CVector {float x,y,z;};
struct CVector2D {float x,y;};
struct CRGBA {uint8_t r,g,b,a;CRGBA(void) { }};
typedef int16_t int16;
#ifdef ORIGINAL
#include "original-Rect.h"
#else
#include "Rect.h"
#endif
''',newline='\n')
entry='''#pragma once
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/math/Rect.cpp').read_text(),flags=re.M)+'\n'
caller='#include "entry.h"\n#ifndef ORIGINAL\n#include "Rect.h"\n#include <stdint.h>\n#endif\n'
signatures=[('Init','void','void *self','self'),('Make','void','void *self,float l,float t,float r,float b','self,l,t,r,b'),('CopyValue','void','void *self,const void *other','self,other'),('ContainPoint','void','void *self,const void *point','self,point'),('ContainRect','void','void *self,const void *other','self,other'),('IsPointInside','bool','void *self,const void *point','self,point'),('IsPointInsideRadius','bool','void *self,const void *point,float radius','self,point,radius'),('Translate','void','void *self,float x,float y','self,x,y'),('Grow','void','void *self,float radius','self,radius'),('GrowSides','void','void *self,float l,float r','self,l,r'),('GrowEdges','void','void *self,float l,float r,float t,float b','self,l,r,t,b'),('GetWidth','float','void *self','self'),('GetHeight','float','void *self','self')]
for name,result,args,values in signatures:
    for prefix in ['Run','Before']:entry+=result+' '+prefix+'_'+name+'('+args+');\n'
    old={'Init':'new(self) CRect();','Make':'new(self) CRect(l,t,r,b);','CopyValue':'new(self) CRect(*(const CRect*)other);','ContainPoint':'((CRect*)self)->ContainPoint(*(const CVector*)point);','ContainRect':'((CRect*)self)->ContainRect(*(const CRect*)other);','IsPointInside':'return ((CRect*)self)->IsPointInside(*(const CVector2D*)point);','IsPointInsideRadius':'return ((CRect*)self)->IsPointInside(*(const CVector2D*)point,radius);','Translate':'((CRect*)self)->Translate(x,y);','Grow':'((CRect*)self)->Grow(radius);','GrowSides':'((CRect*)self)->Grow(l,r);','GrowEdges':'((CRect*)self)->Grow(l,r,t,b);','GetWidth':'return ((CRect*)self)->GetWidth();','GetHeight':'return ((CRect*)self)->GetHeight();'}[name]
    original+='extern "C" '+result+' Before_'+name+'('+args+')\n{\n    // Invoke the complete original rectangle operation\n    '+old+'\n}\n'
    new='Rect_'+name+'((CRect*)self'+(','+values[values.index(',')+1:] if ',' in values else '')+');'
    if name=='Make':new='*(CRect*)self=Rect_Make(l,t,r,b);'
    if name=='CopyValue':new='*(CRect*)self=Rect_CopyValue(*(const CRect*)other);'
    if name=='ContainRect':new='Rect_ContainRect((CRect*)self,(const CRect*)other);'
    if name in ['ContainPoint','IsPointInside','IsPointInsideRadius']:new='Rect_'+name+'((CRect*)self,(const float*)point,(const float*)((const uint8_t*)point+4)'+(',radius' if name=='IsPointInsideRadius' else '')+');'
    caller+=result+' Run_'+name+'('+args+')\n{\n    // Exercise each public operation through an independent strict C11 caller\n#ifdef ORIGINAL\n    '+('return ' if result!='void' else '')+'Before_'+name+'('+values+');\n#else\n    '+('return ' if result!='void' else '')+new+'\n#endif\n}\n'
entry+='#ifdef __cplusplus\n}\n#endif\n'
globals='#ifdef ORIGINAL\n'
for phase,base in [('before',saved),('after',tree)]:
    script=(base/'src/control/Script.h').read_text();intro=script[script.index('struct intro_script_rectangle'):script.index('\nVALIDATE_SIZE(intro_script_rectangle')]
    assert '~intro_script_rectangle() { }' in intro
    main=(base/'src/core/main.cpp').read_text();zones=main[main.index('struct tZonePrint\n'):main.index('\n\nvoid\nPrintMemoryUsage',main.index('struct tZonePrint\n'))]
    water='CRect CWaterLevel::ms_aWaterRects[48];'+(m.WATER_INIT if phase=='after' else '')
    globals+=intro+'\nclass CTheScripts {public:static intro_script_rectangle IntroRectangles[16];};\nintro_script_rectangle CTheScripts::IntroRectangles[16];\nclass CWaterLevel {public:static CRect ms_aWaterRects[48];};\n'+water+'\n'+zones+'\n'
    globals+='#else\n' if phase=='before' else '#endif\n'
fixture=(root/'utils/tests/rect.cpp.in').read_text().replace('@GLOBALS@',globals)
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('fixture.cpp',fixture)]:(out/name).write_text(value,newline='\n')
print('Prepared all thirteen original/C operations, water array, overlay constructors and nineteen debug-zone initializers')
