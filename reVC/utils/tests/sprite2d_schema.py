"""Describe every original 2D sprite overload and explicit C lifetime operation."""
import re
from rect_migration import arguments
RENAMES={'src/renderer/Sprite2d.cpp':'src/renderer/Sprite2d.c'}
STATE=['RecipNearClip','NearScreenZ','NearCamZ','nextBufferVertex','nextBufferIndex']
INSTANCE={'Delete','SetRenderState','SetTexture','SetTextureMasked','SetAddressing','DrawXYWH','DrawRect','DrawRectUV','DrawRectColours','DrawQuad'}


def operation(name,params):
    # Distinguish complete original overloads without retaining a C++ dispatch wrapper
    args=arguments(params);count=len(args)
    if name=='SetTexture':return 'SetTextureMasked' if count==2 else 'SetTexture'
    if name=='Draw':
        # Preserve all five instance draw argument orders
        if args[0].startswith('const CRect'):return {2:'DrawRect',5:'DrawRectColours',10:'DrawRectUV'}[count]
        return {5:'DrawXYWH',9:'DrawQuad'}[count]
    if name=='SetVertices':
        # Preserve all five static vertex-building argument orders
        if args[0].startswith('const CRect'):return {5:'SetRectVertices',13:'SetRectUVVertices'}[count]
        if args[0].startswith('RwIm2DVertex'):return 'SetExternalVertices'
        if args[0].startswith('int '):return 'SetArrayVertices'
        assert count==12;return 'SetQuadVertices'
    if name=='DrawRect':return {2:'DrawSolidRect',5:'DrawFourColourRect'}[count]
    return name


def signatures(source):
    # Include both split-line and same-line definitions so no original operation escapes the full conversion
    rows=[]
    for match in re.finditer(r'(?m)^(void|bool)\s+CSprite2d::(\w+)\(([^{}]*?)\)\s*\{',source):
        # Replace only native representations while preserving original scalar type and argument order
        kind,old,original=match.groups();name=operation(old,original);args=(original.strip() or 'void').replace('RwTextureAddressMode','int32_t').replace('RwIm2DVertex *verts','void *verts')
        args=re.sub(r'const CRect &(\w+)',r'const CRect *\1',args);args=re.sub(r'const CRGBA &(\w+)',r'SpriteColourRef \1',args)
        if name in INSTANCE:args='Sprite2d *self'+('' if args.strip()=='void' else ','+args)
        rows.append((kind,name,' '.join(args.split()),old,original,match.start(),match.end()-1))
    assert len(rows)==26 and len({row[1] for row in rows})==26
    return rows


def header(source):
    # Expose an actual C record and the complete original operation set without SDK class fields
    text='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stddef.h>\n#include "Sprite.h"\ntypedef struct Sprite2d {void *m_pTexture;} Sprite2d;\n#ifdef __cplusplus\nextern "C" {\n#endif\nvoid Sprite2d_Init(Sprite2d *self);\nvoid Sprite2d_Deinit(Sprite2d *self);\nvoid *Sprite2d_GetVertices(void);\nvoid Sprite2d_InitArray(Sprite2d *objects,size_t count);\nvoid Sprite2d_DeinitArray(Sprite2d *objects,size_t count);\n'
    text+='\n'.join(kind+' Sprite2d_'+name+'('+args+');' for kind,name,args,*_ in signatures(source))
    return text+'\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
