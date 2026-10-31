"""Move obsolete native sprite forwarding and point lifetimes into genuine C peers."""
import re
from memory_heap_migration import body
from rect_migration import mask
SOURCES=['Clouds','PointLights','MoneyMessages','Fluff']
NAMES=['Clouds_BackendProject','Clouds_BackendInitSprites','Clouds_BackendFlushSprites','Clouds_BackendRenderOneXLUSprite','Clouds_BackendRenderBufferedOneXLUSprite','Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Dimension','Clouds_BackendRenderBufferedOneXLUSprite_Rotate_Aspect','Clouds_BackendRenderBufferedOneXLUSprite_Rotate_2Colours','PointLights_BackendSpriteInit','PointLights_BackendSpriteFlush','PointLights_BackendScreen','PointLights_BackendSprite','MoneyMessages_BackendProject','ScrollBar_BackendInitSprites','ScrollBar_BackendFlushSprites','ScrollBar_BackendWithPoints','ScrollBar_BackendProject','ScrollBar_BackendSprite']


def definitions(source):
    # Locate only the complete existing forwarding operations whose owner is now C
    code=mask(source);rows=[]
    for match in re.finditer(r'(?m)^(?:bool|void|float)\s+('+ '|'.join(NAMES)+r')\([^;{}]*\)\s*\{',code):
        # Retain the full declaration and body rather than matching a call expression
        signature=source[match.start():match.end()-1].strip();value=body(source,signature);rows.append((match[1],signature,value,match.start(),match.end()+len(value)+1))
    return rows


def remove(source):
    # Remove only complete obsolete native definitions while retaining all unrelated SDK operations
    for _,_,_,first,last in reversed(definitions(source)):source=source[:first]+source[last:]
    return source


def bindings(originals):
    # Replace each former C++ forwarder with a C peer using live scalar references to C-owned storage
    text='//+ rouz edit (ChatGPT)\n#include "Sprite.h"\n#include "Clouds_backend.h"\n#include "PointLights_backend.h"\n#include "MoneyMessages_backend.h"\n#include "ScrollBar_backend.h"\n';seen=[]
    projects={'Clouds_BackendProject':('CloudsVector','value','out'),'PointLights_BackendScreen':('PointLightsVector','coors','screen'),'MoneyMessages_BackendProject':('MoneyMessageVector','position','out')}
    for original in originals:
        # Preserve every original forwarding signature except the private point-view API that no longer needs SDK owners
        for name,signature,value,_,_ in definitions(original):
            seen.append(name)
            if name in projects:
                # Keep conditional output publication while the complete projection algorithm now runs in its C owner
                kind,inputName,outputName=projects[name];value='\n    // Publish the projected point only on the original successful path\n    SpriteVector input={'+inputName+'.x,'+inputName+'.y,'+inputName+'.z},projected;\n    bool visible=Sprite_CalcScreenCoors(Sprite_VectorRef(&input.x,&input.y,&input.z),Sprite_VectorOut(&projected.x,&projected.y,&projected.z),width,height,farclip);\n    if(visible){\n        // Copy the defined C result into the actual peer receiver\n        '+outputName+'->x=projected.x;'+outputName+'->y=projected.y;'+outputName+'->z=projected.z;\n    }\n    return visible;\n'
            elif name=='ScrollBar_BackendProject':
                # Borrow existing point-view fields instead of casting an opaque C record as a native vector
                signature=signature.replace('void *input,void *output','ScrollBarPoint input,ScrollBarPoint output');value='\n    // Project directly through the live point scalar views\n    return Sprite_CalcScreenCoors(Sprite_VectorRef(input.x,input.y,input.z),Sprite_VectorOut(output.x,output.y,output.z),width,height,farclip);\n'
            elif name=='ScrollBar_BackendWithPoints':
                # Keep per-invocation point lifetime without constructing native SDK vectors for a C operation
                value=value.replace('CVector input,output;','SpriteVector input,output;')
            else:value=value.replace('CSprite::','Sprite_')
            value=re.sub(r'(?m)^([ \t]*)// Preserve the actual typed owner operation and original native object lifetime$',r'\1// Forward to the complete C sprite owner with the original argument values',value)
            text+=signature+'\n{'+value+'}\n'
    assert len(seen)==len(NAMES) and set(seen)==set(NAMES),seen
    return text+'//- rouz edit (ChatGPT)\n'
