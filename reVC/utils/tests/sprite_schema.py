"""Describe all original sprite operations and C scalar-reference representations."""
import re
RENAMES={'src/renderer/Sprite.cpp':'src/renderer/Sprite.c'}
OPERATIONS=['GetNearScreenZ','GetFarScreenZ','CalcHorizonCoors','CalcScreenCoors','InitSpriteBuffer','InitSpriteBuffer2D','FlushSpriteBuffer','RenderOneXLUSprite','RenderOneXLUSprite_Rotate_Aspect','RenderBufferedOneXLUSprite','RenderBufferedOneXLUSprite_Rotate_Dimension','RenderBufferedOneXLUSprite_Rotate_Aspect','RenderBufferedOneXLUSprite_Rotate_2Colours','Set6Vertices2DRect','Set6Vertices2DCoords','RenderBufferedOneXLUSprite2D','RenderBufferedOneXLUSprite2D_Rotate_Dimension']
STATE=['m_f2DNearScreenZ','m_f2DFarScreenZ','m_fRecipNearClipPlane','m_bFlushSpriteBufferSwitchZTest','nSpriteBufferIndex']

def signatures(source):
    # Retain every complete original signature and distinguish both coordinate-building overloads
    rows=[]
    for match in re.finditer(r'(?m)^(float|bool|void)\s*\nCSprite::(\w+)\(([^{}]*?)\)\s*\{',source):
        # Convert only public representations while preserving every scalar argument type and order
        kind,name,args=match.groups();original=name
        if name=='Set6Vertices2D':name+='Rect' if 'const CRect &r' in args else 'Coords'
        args=args.replace('const RwV3d &in','SpriteVectorRef in').replace('RwV3d *out','SpriteVectorOut out').replace('RwIm2DVertex *verts','void *verts').replace('const CRect &r','const CRect *r').replace('const RwRGBA &colour','SpriteColourRef colour')
        args=re.sub(r'const CRGBA &(c[0-3])',r'SpriteColourRef \1',args)
        for old,new in [('uint8','uint8_t'),('int16','int16_t'),('int32','int32_t')]:args=re.sub(r'\b'+old+r'\b',new,args)
        rows.append((kind,name,' '.join(args.split()),original,match.start(),match.end()-1))
    assert len(rows)==15 and {r[1] for r in rows}==set(OPERATIONS)-{'GetNearScreenZ','GetFarScreenZ'}
    return rows


def header(source):
    # Expose the complete owner as C functions without any SDK or native class declaration
    text="""//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "Rect.h"
typedef struct SpriteVector {float x,y,z;} SpriteVector;
typedef struct SpriteVectorRef {const float *x,*y,*z;} SpriteVectorRef;
typedef struct SpriteVectorOut {float *x,*y,*z;} SpriteVectorOut;
typedef struct SpriteColour {uint8_t r,g,b,a;} SpriteColour;
typedef struct SpriteColourRef {const uint8_t *r,*g,*b,*a;} SpriteColourRef;
static inline SpriteVectorRef Sprite_VectorRef(const float *x,const float *y,const float *z)
{
    // Borrow actual live vector scalar subobjects for the complete synchronous C operation
    SpriteVectorRef value={x,y,z};return value;
}
static inline SpriteVectorOut Sprite_VectorOut(float *x,float *y,float *z)
{
    // Publish directly into the caller's actual scalar receiver fields
    SpriteVectorOut value={x,y,z};return value;
}
static inline SpriteColourRef Sprite_ColourRef(const uint8_t *r,const uint8_t *g,const uint8_t *b,const uint8_t *a)
{
    // Retain the original live colour component reads across intervening SDK vertex setters
    SpriteColourRef value={r,g,b,a};return value;
}
#ifdef __cplusplus
extern "C" {
#endif
float Sprite_GetNearScreenZ(void);
float Sprite_GetFarScreenZ(void);
"""
    text+='\n'.join(kind+' Sprite_'+name+'('+args+');' for kind,name,args,*_ in signatures(source))
    return text+'\n#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)\n'
