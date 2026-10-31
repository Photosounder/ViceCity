"""Project all original sprite algorithms into C without altering render and clipping policy."""
import re
from memory_heap_migration import body
from rect_migration import closing,arguments
from sprite_schema import signatures,OPERATIONS,STATE
from mblur_schema import SETTERS


def producer(source):
    # Keep every complete original operation body while replacing only class and SDK representations
    text="""//+ rouz edit (ChatGPT)
#include "Sprite.h"
#include "Sprite_backend.h"
#include "Draw.h"
#include "Draw_constants.h"
#include <math.h>
#include <string.h>
#define SPRITEBUFFERSIZE 64
#define TRUE 1
#define FALSE 0
#define Clamp(v,low,high) ((v)<(low)?(low):(v)>(high)?(high):(v))
#define Sin(x) sinf(x)
#define Cos(x) cosf(x)
#ifdef GTA_PS2
#ifdef GTA_PAL
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)512)
#else
#define SCREEN_WIDTH ((float)640)
#define SCREEN_HEIGHT ((float)448)
#endif
#else
#define SCREEN_WIDTH ((float)Sprite_HostScreenWidth())
#define SCREEN_HEIGHT ((float)Sprite_HostScreenHeight())
#endif
#ifdef ASPECT_RATIO_SCALE
#define SCREEN_SCALE_AR(a) ((a)*DEFAULT_ASPECT_RATIO/Draw_GetAspectRatio())
#else
#define SCREEN_SCALE_AR(a) (a)
#endif
"""
    for state in STATE:text+='static '+('int32_t ' if state in ['m_bFlushSpriteBufferSwitchZTest','nSpriteBufferIndex'] else 'float ')+'Sprite_'+state+';\n'
    text+='float Sprite_GetNearScreenZ(void)\n{\n    // Retain the original near-depth getter on the canonical C state\n    return Sprite_m_f2DNearScreenZ;\n}\nfloat Sprite_GetFarScreenZ(void)\n{\n    // Retain the original far-depth getter on the canonical C state\n    return Sprite_m_f2DFarScreenZ;\n}\n'
    for kind,name,args,old,start,opening in signatures(source):
        # Select the exact complete original definition before converting its borrowed scalar fields
        value=body(source,source[start:opening].strip())
        if name=='CalcHorizonCoors':
            # Keep position construction and horizon selection in C while invoking the separately owned matrix primitive
            original='CVector p = TheCamera.GetPosition() + CVector(TheCamera.CamFrontXNorm, TheCamera.CamFrontYNorm, 0.0f)*3000.0f;';assert original in value
            value=value.replace(original,'SpriteVectorRef position=Sprite_HostCameraPosition();\n\tSpriteVector p={*position.x+Sprite_HostFrontX()*3000.0f,*position.y+Sprite_HostFrontY()*3000.0f,*position.z+0.0f*3000.0f};').replace('p = TheCamera.m_viewMatrix * p;','p = Sprite_HostViewTransform(&p);')
        if name=='CalcScreenCoors':
            # Preserve point publication before either rejection and retain aliased input values through a local C snapshot
            value=value.replace('CVector viewvec = TheCamera.m_viewMatrix * in;','SpriteVector input={*in.x,*in.y,*in.z};\n\tSpriteVector viewvec=Sprite_HostViewTransform(&input);').replace('*out = viewvec;','memcpy(out.x,&viewvec.x,sizeof(float));memcpy(out.y,&viewvec.y,sizeof(float));memcpy(out.z,&viewvec.z,sizeof(float));')
            value=re.sub(r'out->([xyz])',r'(*out.\1)',value).replace('DefaultFOV','Sprite_HostDefaultFOV()').replace('SoftwarePolygons::CapturingWorldEffects()','Sprite_HostCapturingWorldEffects()')
        if old=='Set6Vertices2D':
            # Borrow actual colour and rectangle scalar fields at each original setter expression
            value=re.sub(r'\b(c[0-3])\.([rgba])',r'(*\1.\2)',value)
            if name.endswith('Rect'):value=re.sub(r'\br\.(left|top|right|bottom)',r'r->\1',value)
        value=value.replace('RwIm2DVertex *vert','void *vert')
        value=re.sub(r'CRGBA col\(([^;]+)\);',r'SpriteColour col={\1};',value)
        if old.startswith('RenderBufferedOneXLUSprite2D'):
            # Preserve the original narrowed local colour and all repeated references to that live local object
            for oldfield,newfield in [('red','r'),('green','g'),('blue','b'),('alpha','a')]:value=value.replace('colour.'+oldfield,'(*colour.'+newfield+')')
            value=value.replace('col, col, col, col','Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a), Sprite_ColourRef(&col.r,&col.g,&col.b,&col.a)')
            if old=='RenderBufferedOneXLUSprite2D':value=value.replace(', rect,',', &rect,')
        changes=[]
        for match in re.finditer(r'\bSet6Vertices2D\(',value):
            # Disambiguate the former overload using its complete original argument list
            end=closing(value,value.index('(',match.start()));call=value[match.end():end-1];count=len(arguments(call));assert count in [6,13],count;changes.append((match.start(),match.start()+len('Set6Vertices2D'),'Sprite_Set6Vertices2D'+('Rect' if count==6 else 'Coords')))
        for first,last,replacement in reversed(changes):value=value[:first]+replacement+value[last:]
        value=re.sub(r'&SpriteBufferVerts\[([^\]]+)\]',r'RwHelper_HostVertexAt(Sprite_HostBufferedVertices(),\1)',value)
        value=re.sub(r'&(verts|vert)\[([^\]]+)\]',lambda m:'RwHelper_HostVertexAt('+('Sprite_HostImmediateVertices()' if m[1]=='verts' and old!='Set6Vertices2D' else m[1])+','+m[2]+')',value)
        value=re.sub(r'\bSpriteBufferVerts\b','Sprite_HostBufferedVertices()',value)
        if old!='Set6Vertices2D':value=re.sub(r'\bverts\b','Sprite_HostImmediateVertices()',value)
        for setter in SETTERS:value=value.replace('RwIm2DVertexSet'+setter+'(','RwHelper_HostVertex'+setter+'(')
        value=value.replace('RwCameraGetNearClipPlane(Scene.camera)','Sprite_HostNearClip(Sprite_HostSceneCamera())').replace('RwIm2DGetNearScreenZ()','Sprite_HostNearScreenZ()').replace('RwIm2DGetFarScreenZ()','Sprite_HostFarScreenZ()').replace('RwIm2DRenderPrimitive(','Sprite_HostDraw(').replace('RwRenderStateSet(','RwHelper_HostRenderState(')
        for oldstate in STATE:value=re.sub(r'\b'+oldstate+r'\b','Sprite_'+oldstate,value)
        for op in OPERATIONS:
            # Replace only whole unqualified calls after public overload names have been resolved
            value=re.sub(r'(?<![\w])'+op+r'\(', 'Sprite_'+op+'(',value)
        for oldtype,newtype in [('uint8','uint8_t'),('int16','int16_t'),('int32','int32_t')]:value=re.sub(r'\b'+oldtype+r'\b',newtype,value)
        constants=sorted(set(re.findall(r'\brw(?:PRIMTYPE\w+|RENDERSTATE\w+)\b',value)))
        for constant in constants:value=re.sub(r'\b'+constant+r'\b','Sprite_HostConstant(SPRITE_CONST_'+constant+')',value)
        if name=='RenderOneXLUSprite':value=value.replace('static short indices[] = { 0, 1, 2, 3 };','static short indices[] = { 0, 1, 2, 3 };\n\t// Retain the original unused static table without introducing an observable operation\n\t(void)indices;')
        text+=kind+' Sprite_'+name+'('+args+')\n{\n    // Execute the complete original sprite algorithm using canonical C policy state and typed SDK primitives\n'+value+'\n}\n'
    return text+'//- rouz edit (ChatGPT)\n'
