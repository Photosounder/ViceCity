"""Move the complete extended colour-filter owner and all C forwarding boundaries into C."""
import re
from rect_migration import mask,closing
from memory_heap_migration import body
from mblur_schema import SETTERS,RW_STATES,BLENDS

RENAMES={'src/extras/postfx.cpp':'src/extras/postfx.c'}
OPERATIONS=['InitOnce','Open','Close','RenderOverlayBlur','RenderOverlaySniper','RenderOverlayShader','RenderMotionBlur','Render','SmoothColor','NeedBackBuffer','NeedFrontBuffer','GetBackBuffer','UseBlurColours']
VARIABLES={'pFrontBuffer':'void *','pBackBuffer':'void *','bJustInitialised':'bool','EffectSwitch':'int','BlurOn':'bool','MotionBlurOn':'bool','Intensity':'float','PrevRed':'int','PrevGreen':'int','PrevBlue':'int','PrevAlpha':'int','AvgRed':'int','AvgGreen':'int','AvgBlue':'int','AvgAlpha':'int','Next':'int','NumValues':'int'}
GPU=['colourfilterVC_PS','contrast_PS','u_blurcolor','u_contrastAdd','u_contrastMult','colourFilterVC','contrast']
SDK=[('void *','CameraRaster','void *camera','RwCameraGetRaster((RwCamera*)camera)'),('float','NearClip','void *camera','RwCameraGetNearClipPlane((RwCamera*)camera)'),('int32_t','Width','void *raster','RwRasterGetWidth((RwRaster*)raster)'),('int32_t','Height','void *raster','RwRasterGetHeight((RwRaster*)raster)'),('int32_t','Depth','void *raster','RwRasterGetDepth((RwRaster*)raster)'),('void','DestroyRaster','void *raster','RwRasterDestroy((RwRaster*)raster)'),('void','PushContext','void *raster','RwRasterPushContext((RwRaster*)raster)'),('void','PopContext','void','RwRasterPopContext()'),('void','RenderFast','void *raster,int32_t x,int32_t y','RwRasterRenderFast((RwRaster*)raster,x,y)'),('float','NearScreenZ','void','RwIm2DGetNearScreenZ()'),('float','HalfPixel','void','HALFPX'),('void','DefinedState','void','DefinedState()')]

def header():
    # Expose every application-owned variable and complete operation without SDK classes or static members
    value='''//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#ifdef EXTENDED_COLOURFILTER
enum {POSTFX_OFF,POSTFX_SIMPLE,POSTFX_NORMAL,POSTFX_MOBILE,POSTFX_NUMAVERAGE=20};
#ifdef __cplusplus
extern "C" {
#endif
'''
    for name,kind in VARIABLES.items():value+='extern '+kind+' PostFX_'+name+('[POSTFX_NUMAVERAGE]' if name.startswith('Prev') else '')+';\n'
    value+='''void PostFX_InitOnce(void);
void PostFX_Open(void *camera);
void PostFX_Close(void);
void PostFX_RenderOverlayBlur(void *camera,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void PostFX_RenderOverlaySniper(void *camera,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void PostFX_RenderOverlayShader(void *camera,int32_t red,int32_t green,int32_t blue,int32_t alpha);
void PostFX_RenderMotionBlur(void *camera,uint32_t blur);
void PostFX_Render(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha);
void PostFX_SmoothColor(uint32_t red,uint32_t green,uint32_t blue,uint32_t alpha);
bool PostFX_NeedBackBuffer(void);
bool PostFX_NeedFrontBuffer(int32_t type);
void PostFX_GetBackBuffer(void *camera);
bool PostFX_UseBlurColours(void);
#ifdef __cplusplus
}
#endif
#endif
//- rouz edit (ChatGPT)
'''
    return value

def backend_header():
    # Retain SDK storage and shader operations behind individual typed native primitives
    value='//+ rouz edit (ChatGPT)\n#pragma once\n#include "postfx.h"\n#include <stddef.h>\n'
    value+='enum {'+','.join('POSTFX_STATE_'+name for name in RW_STATES)+'};\n'
    value+='enum {'+','.join('POSTFX_BLEND_'+name for name in BLENDS)+'};\n'
    value+='enum {POSTFX_SHADER_DECL,POSTFX_VERTEX_HEADER,POSTFX_FRAGMENT_HEADER};\n#ifdef __cplusplus\nextern "C" {\n#endif\n'
    value+=''.join(result+' PostFX_Host'+name+'('+args+');\n' for result,name,args,_ in SDK)
    for name,(args,_) in SETTERS.items():value+='void PostFX_Host'+name+'(void *vertex,'+args+');\n'
    value+='''void *PostFX_HostVertices(int32_t set);
void *PostFX_HostVertex(void *vertices,int32_t index);
void PostFX_HostDraw(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount);
void *PostFX_HostCreateRaster(int32_t width,int32_t height,int32_t depth);
void PostFX_HostState(int32_t state,void *value);
void PostFX_HostPushGroup(const char *name);
void PostFX_HostPopGroup(void);
void *PostFX_HostD3DCreate(void *bytes);
void PostFX_HostD3DDestroy(void *shader);
void PostFX_HostD3DConstants(uint32_t first,const float *values,uint32_t count);
void PostFX_HostD3DOverride(void *shader);
int32_t PostFX_HostGLRegister(const char *name);
const char *PostFX_HostGLText(int32_t kind);
void *PostFX_HostGLCreate(const char **vertices,const char **fragments);
void PostFX_HostGLDestroy(void *shader);
void PostFX_HostGLUse(void *shader);
void PostFX_HostGLOverride(void *shader);
int32_t PostFX_HostGLLocation(void *shader,int32_t uniform);
void PostFX_HostGLUniform3(int32_t location,int32_t count,const float *values);
void PostFX_HostGLUniform4(int32_t location,int32_t count,const float *values);
void re3_assert(const char *expression,const char *filename,unsigned int line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
'''
    return value

def bindings():
    # Remove application-to-application forwarding from C++ once both callers and callees have C interfaces
    return '''//+ rouz edit (ChatGPT)
#include "config.h"
#include "postfx.h"
#include "MBlur.h"
#include "MBlur_backend.h"
#include "Lights_backend.h"
bool Lights_HostBlurColours(void)
{
    // Read the original active colour-filter preference or classic C blur state at each call site
#ifdef EXTENDED_COLOURFILTER
    return PostFX_UseBlurColours();
#else
    return MBlur_BlurOn;
#endif
}
#ifdef EXTENDED_COLOURFILTER
void MBlur_HostPostOpen(void *camera)
{
    // Forward the complete C blur operation directly into the complete C post-processing owner
    PostFX_Open(camera);
}
void MBlur_HostPostClose(void)
{
    // Retain the original delegation without routing application policy through C++
    PostFX_Close();
}
void MBlur_HostPostRender(void *camera,uint32_t red,uint32_t green,uint32_t blue,uint32_t blur,int32_t type,uint32_t alpha)
{
    // Retain all original C argument conversions before the complete post-processing operation
    PostFX_Render(camera,red,green,blue,blur,type,alpha);
}
#endif
//- rouz edit (ChatGPT)
'''

def native():
    # Keep only native SDK storage, single graphics operations and shader representation access outside C
    value='''//+ rouz edit (ChatGPT)
#define WITHD3D
#include "common.h"
#include "main.h"
#include "PostFX_backend.h"
#include "RwHelper.h"
#ifdef EXTENDED_COLOURFILTER
static RwIm2DVertex nativeVertices[2][4];
void *PostFX_HostVertices(int32_t set)
{
    // Borrow the same zero-initialized SDK vertex objects used by the original two arrays
    return nativeVertices[set];
}
void *PostFX_HostVertex(void *vertices,int32_t index)
{
    // Index actual SDK objects without exposing their backend-specific representation to C
    return &((RwIm2DVertex*)vertices)[index];
}
void PostFX_HostDraw(void *vertices,int32_t count,uint16_t *indices,int32_t indexCount)
{
    // Forward one original indexed triangle-list draw after C has selected its vertices and state
    RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST,(RwIm2DVertex*)vertices,count,indices,indexCount);
}
void *PostFX_HostCreateRaster(int32_t width,int32_t height,int32_t depth)
{
    // Retain the original SDK camera-texture allocation primitive
    return RwRasterCreate(width,height,depth,rwRASTERTYPECAMERATEXTURE);
}
void PostFX_HostState(int32_t state,void *value)
{
    // Translate only SDK state and blend identities while C retains all rendering decisions
'''
    value+='    static const RwRenderState states[]={'+','.join('rwRENDERSTATE'+name for name in RW_STATES)+'};\n    static const uintptr_t blends[]={'+','.join('rwBLEND'+name for name in BLENDS)+'};\n'
    value+='''    if(state==POSTFX_STATE_TEXTUREFILTER)value=(void*)rwFILTERNEAREST;
    else if(state==POSTFX_STATE_SRCBLEND||state==POSTFX_STATE_DESTBLEND)value=(void*)blends[(uintptr_t)value];
    RwRenderStateSet(states[state],value);
}
void PostFX_HostPushGroup(const char *name)
{
    // Preserve the original debug-build group primitive and incoming literal label
    PUSH_RENDERGROUP(name);
}
void PostFX_HostPopGroup(void)
{
    // Preserve the original debug-build group exit primitive
    POP_RENDERGROUP();
}
#ifdef RW_D3D9
void *PostFX_HostD3DCreate(void *bytes)
{
    // Pass the original mutable compiled shader bytes directly to the SDK factory
    return rw::d3d::createPixelShader(bytes);
}
void PostFX_HostD3DDestroy(void *shader)
{
    // Destroy the actual SDK shader selected by C lifetime policy
    rw::d3d::destroyPixelShader(shader);
}
void PostFX_HostD3DConstants(uint32_t first,const float *values,uint32_t count)
{
    // Preserve the original register count and borrowed data pointer including its original three-float mobile inputs
    rw::d3d::d3ddevice->SetPixelShaderConstantF(first,values,count);
}
void PostFX_HostD3DOverride(void *shader)
{
    // Write the actual SDK override slot at each original C-selected binding point
    rw::d3d::im2dOverridePS=shader;
}
#endif
#ifdef RW_OPENGL
int32_t PostFX_HostGLRegister(const char *name)
{
    // Register the original external uniform name without moving shader setup policy out of C
    return rw::gl3::registerUniform(name);
}
const char *PostFX_HostGLText(int32_t kind)
{
    // Borrow the actual SDK shader preamble and stage headers
    switch(kind){case POSTFX_SHADER_DECL:return rw::gl3::shaderDecl;case POSTFX_VERTEX_HEADER:return rw::gl3::header_vert_src;case POSTFX_FRAGMENT_HEADER:return rw::gl3::header_frag_src;}return NULL;
}
void *PostFX_HostGLCreate(const char **vertices,const char **fragments)
{
    // Forward the original C-owned source-pointer arrays directly to the SDK shader factory
    return rw::gl3::Shader::create(vertices,fragments);
}
void PostFX_HostGLDestroy(void *shader)
{
    // Destroy the actual SDK shader object selected by C lifetime policy
    ((rw::gl3::Shader*)shader)->destroy();
}
void PostFX_HostGLUse(void *shader)
{
    // Invoke the original SDK shader binding primitive
    ((rw::gl3::Shader*)shader)->use();
}
void PostFX_HostGLOverride(void *shader)
{
    // Retain the original live SDK override slot
    rw::gl3::im2dOverrideShader=(rw::gl3::Shader*)shader;
}
int32_t PostFX_HostGLLocation(void *shader,int32_t uniform)
{
    // Read the original live SDK uniform-location metadata
    return ((rw::gl3::Shader*)shader)->uniformLocations[uniform];
}
void PostFX_HostGLUniform3(int32_t location,int32_t count,const float *values)
{
    // Forward the original vector-three uniform primitive and borrowed C coefficient array
    glUniform3fv(location,count,values);
}
void PostFX_HostGLUniform4(int32_t location,int32_t count,const float *values)
{
    // Forward the original vector-four uniform primitive and borrowed C coefficient array
    glUniform4fv(location,count,values);
}
#endif
'''
    for result,name,args,expression in SDK:
        value+=result+' PostFX_Host'+name+'('+args+')\n{\n    // Preserve the original individual SDK primitive\n    '+('' if result=='void' else 'return ')+expression+';\n}\n'
    for name,(args,values) in SETTERS.items():
        value+='void PostFX_Host'+name+'(void *vertex,'+args+')\n{\n    // Apply the original SDK setter to the actual native vertex object\n    RwIm2DVertexSet'+name+'((RwIm2DVertex*)vertex,'+values+');\n}\n'
    return value+'#endif\n//- rouz edit (ChatGPT)\n'

def kernel(original):
    # Retain every complete original body including smoothing, shader coefficients and buffer selection policy
    source=re.sub(r'^#include "(?!shaders/)[^\n]*\n','',original,flags=re.M)
    source=source.replace('#define WITHD3D\n','')
    source=source.replace('static RwIm2DVertex Vertex[4];\n','').replace('static RwIm2DVertex Vertex2[4];\n','')
    source=source.replace('rw::gl3::Shader *colourFilterVC;','void *colourFilterVC;').replace('rw::gl3::Shader *contrast;','void *contrast;')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bCPostFX::',code))):source=source[:match.start()]+'PostFX_'+source[match.end():]
    names={name:'PostFX_'+name for name in list(VARIABLES)+OPERATIONS+GPU};names['NUMAVERAGE']='POSTFX_NUMAVERAGE';code=mask(source)
    for match in reversed(list(re.finditer(r'\b(?:'+'|'.join(names)+r')\b',code))):
        # Rename only code identifiers while preserving shader include paths and registered uniform strings
        source=source[:match.start()]+names[match[0]]+source[match.end():]
    source=source.replace('using namespace rw::gl3;','')
    for name,kind in [('shaderDecl','POSTFX_SHADER_DECL'),('header_vert_src','POSTFX_VERTEX_HEADER'),('header_frag_src','POSTFX_FRAGMENT_HEADER')]:source=re.sub(r'\b'+name+r'\b','PostFX_HostGLText('+kind+')',source)
    source=source.replace('Shader::create(','PostFX_HostGLCreate(').replace('rw::d3d::createPixelShader(','PostFX_HostD3DCreate(').replace('rw::d3d::destroyPixelShader(','PostFX_HostD3DDestroy(')
    for name in ['PostFX_colourFilterVC','PostFX_contrast']:
        source=source.replace(name+'->destroy()','PostFX_HostGLDestroy('+name+')').replace(name+'->use()','PostFX_HostGLUse('+name+')')
        source=re.sub(re.escape(name)+r'->uniformLocations\[([^\]]+)\]',r'PostFX_HostGLLocation('+name+r',\1)',source)
    source=source.replace('rw::gl3::registerUniform(','PostFX_HostGLRegister(').replace('glUniform3fv(','PostFX_HostGLUniform3(').replace('glUniform4fv(','PostFX_HostGLUniform4(').replace('rw::d3d::d3ddevice->SetPixelShaderConstantF(','PostFX_HostD3DConstants(')
    source=re.sub(r'rw::d3d::im2dOverridePS = ([^;]+);',r'PostFX_HostD3DOverride(\1);',source)
    source=re.sub(r'rw::gl3::im2dOverrideShader = ([^;]+);',r'PostFX_HostGLOverride(\1);',source)
    for name,(args,values) in SETTERS.items():source=source.replace('RwIm2DVertexSet'+name+'(','PostFX_Host'+name+'(')
    source=re.sub(r'&(Vertex2|Vertex)\[([^\]]+)\]',lambda match:'PostFX_HostVertex(PostFX_HostVertices('+('1' if match[1]=='Vertex2' else '0')+'),'+match[2]+')',source)
    source=re.sub(r'\bVertex2\b','PostFX_HostVertices(1)',source);source=re.sub(r'\bVertex\b','PostFX_HostVertices(0)',source)
    source=source.replace('RwIm2DRenderIndexedPrimitive(rwPRIMTYPETRILIST,','PostFX_HostDraw(').replace('RwRenderStateSet(','PostFX_HostState(')
    for name in RW_STATES:source=source.replace('rwRENDERSTATE'+name,'POSTFX_STATE_'+name)
    for name in BLENDS:source=source.replace('rwBLEND'+name,'POSTFX_BLEND_'+name)
    source=source.replace('rwFILTERNEAREST','0')
    sdk={'RwCameraGetRaster':'CameraRaster','RwCameraGetNearClipPlane':'NearClip','RwRasterGetWidth':'Width','RwRasterGetHeight':'Height','RwRasterGetDepth':'Depth','RwRasterDestroy':'DestroyRaster','RwRasterPushContext':'PushContext','RwRasterPopContext':'PopContext','RwRasterRenderFast':'RenderFast','RwIm2DGetNearScreenZ':'NearScreenZ','DefinedState':'DefinedState'}
    for name,operation in sdk.items():source=re.sub(r'\b'+name+r'\b','PostFX_Host'+operation,source)
    source=source.replace('RwRasterCreate(width, height, depth, rwRASTERTYPECAMERATEXTURE)','PostFX_HostCreateRaster(width,height,depth)')
    source=source.replace('PUSH_RENDERGROUP(','PostFX_HostPushGroup(').replace('POP_RENDERGROUP(','PostFX_HostPopGroup(').replace('Pow(','powf(')
    code=mask(source)
    for match in reversed(list(re.finditer(r'\bint32\(',code))):
        end=closing(code,match.end()-1);source=source[:match.start()]+'(int32_t)'+source[match.end()-1:end]+source[end:]
    for old,new in [('RwCamera','void'),('RwRaster','void'),('RwImVertexIndex','uint16_t'),('uint32','uint32_t'),('int32','int32_t'),('nil','NULL'),('TRUE','1'),('FALSE','0')]:source=re.sub(r'\b'+old+r'\b',new,source)
    source+='''
#ifdef EXTENDED_COLOURFILTER
bool PostFX_UseBlurColours(void)
{
    // Preserve the complete original inline predicate using its live C effect preference
    return PostFX_EffectSwitch!=POSTFX_SIMPLE;
}
#endif
'''
    for name in OPERATIONS:
        source=re.sub(r'(?m)^(PostFX_'+name+r'\([^\n]*\)\n\{\n)',lambda match:match[1]+'    // Preserve the complete original post-processing operation and SDK call order\n',source)
    prefix='''//+ rouz edit (ChatGPT)
#include "config.h"
#include "postfx.h"
#include "PostFX_backend.h"
#include "MBlur.h"
#include "Camera_effects.h"
#include <math.h>
#include <stddef.h>
#if defined(RW_GL3) && !defined(RW_OPENGL)
#define RW_OPENGL
#endif
#define HALFPX PostFX_HostHalfPixel()
#define Min(a,b) ((a)<(b)?(a):(b))
#ifndef MASTER
#define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
#else
#define assert(expression) (expression)
#endif
'''
    return prefix+source+'\n//- rouz edit (ChatGPT)\n'

def postfx_calls(source):
    # Replace the complete static class and every existing direct client or C forwarding boundary
    if 'CPostFX::InitOnce(void)\n{' in source:return kernel(source)
    if 'class CPostFX\n' in source:return header()
    original=source
    if 'bool Lights_HostBlurColours(void)\n{' in source and 'CPostFX::UseBlurColours()' in source:
        opening=source.index('bool Lights_HostBlurColours(void)');value=body(source,'bool Lights_HostBlurColours(void)');end=source.index('{',opening)+len(value)+2;source=source[:opening]+source[end:]
    for name in ['Open','Close','Render']:
        signature='void MBlur_HostPost'+name+'('
        if re.search(r'(?m)^void MBlur_HostPost'+name+r'\([^;\n]*\)\n\{',source):
            opening=source.index(signature);value=body(source,signature)
            if 'CPostFX::'+name+'(' not in value:continue
            end=source.index('{',opening)+len(value)+2;source=source[:opening]+source[end:]
    code=mask(source);changes=[]
    for match in re.finditer(r'\bCPostFX::(\w+)',code):
        name=match[1];assert name in VARIABLES or name in OPERATIONS or name.startswith('POSTFX_') or name=='NUMAVERAGE',name;value=name if name.startswith('POSTFX_') else 'POSTFX_NUMAVERAGE' if name=='NUMAVERAGE' else 'PostFX_'+name;changes.append((match.start(),match.end(),value))
    for start,end,value in reversed(changes):source=source[:start]+value+source[end:]
    if source!=original:
        lines=source.splitlines(keepends=True);code=mask(source).splitlines(keepends=True);directives=set();continuation=False
        for index,line in enumerate(lines):
            # Keep preprocessor continuations intact instead of inserting function comments into global selector macros
            if continuation or line.lstrip().startswith('#'):directives.add(index);continuation=line.rstrip().endswith('\\')
        for index in reversed(range(len(lines))):
            if index not in directives and re.search(r'\bPostFX_\w+',code[index]):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the complete C post-processing owner and its live shared state\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source

def postfx_project(source):
    # Select strict C11 for the complete policy and C forwarding units and add only typed SDK primitives as native
    import object_names_migration as objects
    source=source.replace('src\\extras\\postfx.cpp','src\\extras\\postfx.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\extras\\postfx\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve original per-file options and select C for both complete application units
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';base=match[1][:-len('postfx.c')]
            if '<ClCompile Include="'+base+'PostFX_native.cpp"' not in source:result+='\n    <ClCompile Include="'+base+'PostFX_native.cpp" />'
            if '<ClCompile Include="'+base+'PostFX_bindings.c"' not in source:result+='\n    <ClCompile Include="'+base+'PostFX_bindings.c">\n      <CompileAs>CompileAsC</CompileAs>\n      <LanguageStandard_C>stdc11</LanguageStandard_C>\n    </ClCompile>'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)


def calls(source):
    # Retain every preceding C owner before applying the complete rendering-helper successor
    import rw_helper_migration as helper
    return helper.calls(postfx_calls(source))

def project(source):
    # Select the complete C helper and its individual SDK primitives after preceding C metadata
    import rw_helper_migration as helper
    return helper.project(postfx_project(source))
