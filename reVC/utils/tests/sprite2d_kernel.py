"""Project every original 2D sprite body and explicit array lifecycle into genuine C."""
import re
from memory_heap_migration import body
from rect_migration import mask,closing,arguments
from sprite2d_schema import signatures,STATE,INSTANCE
from mblur_schema import SETTERS


def producer(source):
    # Keep all complete algorithms while replacing only native class and SDK storage representations
    assert 'RwIm2DVertex CSprite2d::maVertices[8];' in source
    text="""//+ rouz edit (ChatGPT)
#include "Sprite2d.h"
#include "Sprite2d_backend.h"
#include "RenderBuffer.h"
#include <stdint.h>
#define nil NULL
#define TRUE 1
#define FALSE 0
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
"""
    text+='\n'.join('static '+('int32_t ' if name.startswith('next') else 'float ')+'Sprite2d_'+name+';' for name in STATE)+'\n'
    text+="""void Sprite2d_Init(Sprite2d *self)
{
    // Initialize the sole instance field at the former constructor boundary
    self->m_pTexture=NULL;
}
void Sprite2d_Deinit(Sprite2d *self)
{
    // Retain the complete original destructor behavior through the C resource operation
    Sprite2d_Delete(self);
}
void *Sprite2d_GetVertices(void)
{
    // Return the same actual SDK array rather than a copied C vertex buffer
    return Sprite2d_HostVertices();
}
void Sprite2d_InitArray(Sprite2d *objects,size_t count)
{
    // Retain the original ascending array-construction order
    for(size_t i=0;i<count;++i)Sprite2d_Init(&objects[i]);
}
void Sprite2d_DeinitArray(Sprite2d *objects,size_t count)
{
    // Retain the original reverse array-destruction order
    while(count!=0)Sprite2d_Deinit(&objects[--count]);
}
"""
    names={r[3] for r in signatures(source)}
    for kind,name,params,old,original,start,opening in signatures(source):
        # Select each entire original definition before converting its actual scalar references and resource handle
        value=body(source,source[start:opening].strip()).replace('CSprite2d::','')
        if name=='DrawXYWH':
            # Keep the original rectangle temporary alive for its complete synchronous C vertex-building operation
            originalCall='SetVertices(Rect_Make(x, y, x + w, y + h), col, col, col, col);';assert originalCall in value;value=value.replace(originalCall,'CRect drawRect=Rect_Make(x, y, x + w, y + h);\n\tSetVertices(&drawRect, col, col, col, col);')
        changes=[]
        for match in re.finditer(r'\bSetVertices\(',mask(value)):
            # Resolve all internal overloads by their complete original argument list before changing SDK pointer syntax
            end=closing(value,value.index('(',match.start()));count=len(arguments(value[match.end():end-1]));target={5:'SetRectVertices',13:'SetRectUVVertices',12:'SetQuadVertices',14:'SetExternalVertices'}[count];changes.append((match.start(),match.start()+len('SetVertices'),'Sprite2d_'+target))
        for first,last,new in reversed(changes):value=value[:first]+new+value[last:]
        for field in re.findall(r'const CRect &(\w+)',original):value=re.sub(r'\b'+field+r'\.(left|right|top|bottom)',field+r'->\1',value)
        for colour in re.findall(r'const CRGBA &(\w+)',original):
            # Retain each actual colour byte read at its original expression point
            value=re.sub(r'\b'+colour+r'\.(alpha|red|green|blue|r|g|b|a)\b',lambda m:'(*'+colour+'.'+{'alpha':'a','red':'r','green':'g','blue':'b'}.get(m[1],m[1])+')',value)
        value=re.sub(r'&TempVertexBuffer\.im2d\[([^\]]+)\]',r'RwHelper_HostVertexAt(Sprite2d_HostTempVertices(),\1)',value)
        value=value.replace('TempVertexBuffer.im2d','Sprite2d_HostTempVertices()')
        value=re.sub(r'&maVertices\[([^\]]+)\]',r'RwHelper_HostVertexAt(Sprite2d_HostVertices(),\1)',value)
        value=re.sub(r'&verts\[([^\]]+)\]',r'RwHelper_HostVertexAt(verts,\1)',value)
        value=re.sub(r'\bmaVertices\b','Sprite2d_HostVertices()',value)
        value=value.replace('RwImVertexIndex *pIndexList','uint16_t *pIndexList')
        for setter in SETTERS:value=value.replace('RwIm2DVertexSet'+setter+'(','RwHelper_HostVertex'+setter+'(')
        value=value.replace('RwCameraGetNearClipPlane(Scene.camera)','Sprite_HostNearClip(Sprite_HostSceneCamera())').replace('RwIm2DGetNearScreenZ()','Sprite_HostNearScreenZ()').replace('RwTextureDestroy(','Shadow_HostTextureDestroy(').replace('RwTextureSetAddressing(','Shadow_HostTextureAddress(').replace('RwTextureRead(','Sprite2d_HostTextureRead(').replace('RwTextureGetRaster(','Sprite2d_HostTextureRaster(').replace('RwRenderStateSet(','RwHelper_HostRenderState(').replace('RwIm2DRenderPrimitive(','Sprite_HostDraw(').replace('RwIm2DRenderIndexedPrimitive(','Sprite2d_HostIndexedDraw(')
        for method in names-{'SetVertices','Draw','SetTexture'}:
            # Retain explicit instance receivers and leave static operation calls as ordinary C functions
            target=method;prefix='self,' if target in INSTANCE else '';value=re.sub(r'(?<![\w])'+method+r'\(', 'Sprite2d_'+target+'('+prefix,value);value=value.replace('(self,)','(self)')
        value=value.replace('nextBufferIndex > ARRAY_SIZE','(size_t)nextBufferIndex > ARRAY_SIZE')
        for field in STATE:value=re.sub(r'\b'+field+r'\b','Sprite2d_'+field,value)
        value=re.sub(r'\bm_pTexture\b','self->m_pTexture',value)
        for constant in sorted(set(re.findall(r'\brw(?:RENDERSTATE\w+|SHADEMODE\w+|BLEND\w+|FILTER\w+|PRIMTYPE\w+)\b',value))):value=re.sub(r'\b'+constant+r'\b','Sprite2d_HostConstant(SPRITE2D_CONST_'+constant+')',value)
        value=value.replace('(void*)','(void*)(uintptr_t)')
        text+=kind+' Sprite2d_'+name+'('+params+')\n{\n    // Execute the complete original 2D sprite algorithm with canonical C instance and private state\n'+value+'\n}\n'
    return text+'//- rouz edit (ChatGPT)\n'
