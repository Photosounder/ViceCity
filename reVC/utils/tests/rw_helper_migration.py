"""Project the complete rendering-helper owner into C while retaining native SDK storage scopes."""
import re
from rect_migration import mask
from memory_heap_migration import body
from rw_helper_schema import OPAQUE,SDK_MAP,ITERATORS

RENAMES={'src/rw/RwHelper.cpp':'src/rw/RwHelper.c'}

def definition(source,name):
    # Select the full original definition rather than a call or a forward declaration
    match=re.search(r'(?m)^(?:static\s+)?(?:void|bool|int32|RwBool|RwFrame|RwObject|RpAtomic|RwTexture|RpHAnimHierarchy|RpHAnimAnimation|RpClump|RwCamera)\b\s*\*?\s*'+name+r'\([^;{}]*\)\s*\{',mask(source));assert match,name
    opening=match.end()-1;value=body(source,source[match.start():opening].strip());return match.start(),opening+len(value)+2,value

def scoped(source,name,result,params,storage,locals,changes):
    # Keep the complete original body in C while borrowing actual SDK objects synchronously
    start,end,value=definition(source,name)
    for old,new in changes:
        assert old in value,(name,old)
        value=value.replace(old,new)
    context='RwHelper_'+name+'Scope';callback='RwHelper_'+name+'Body';fields=';'.join(kind+' '+arg for kind,arg in params)+';'+(result+' result;' if result!='void' else '')
    if result!='void':value=re.sub(r'\breturn\s+([^;]+);',r'{scope->result=\1;return;}',value)
    code='typedef struct '+context+' {'+fields+'} '+context+';\nstatic void '+callback+'(void *data,'+locals+')\n{\n    // Execute the entire original algorithm with per-invocation typed SDK storage\n    '+context+' *scope=('+context+'*)data;\n'+''.join('    '+kind+' '+arg+'=scope->'+arg+';\n' for kind,arg in params)+value+'\n}\n'
    code+=result+' '+name+'('+','.join(kind+' '+arg for kind,arg in params)+')\n{\n    // Keep native stack objects alive for the complete synchronous C operation\n    '+context+' scope={'+','.join(arg for _,arg in params)+(',0' if result!='void' else '')+'};RwHelper_Host'+storage+'('+callback+ ',&scope);'+('return scope.result;' if result!='void' else '')+'\n}\n'
    return source[:start]+code+source[end:]

def kernel(original):
    # Preserve all original operation bodies and configuration branches while replacing only SDK representations
    source=original
    source=scoped(source,'SkinGetBonePositionsToTable','void',[('void *','clump'),('void *','boneTable')],'BoneMatrices','void *m,void *invmat',[
        ('RwMatrix m, invmat;',''),('&RpSkinGetSkinToBoneMatrices(skin)[i]','RwHelper_HostMatrixAt(RpSkinGetSkinToBoneMatrices(skin),i)'),
        ('&invmat.pos','RwHelper_HostMatrixPosition(invmat)'),('&invmat','invmat'),('&m','m'),
        ('const RwMatrix *x','const void *x'),('&x[parent]','RwHelper_HostMatrixAt(x,parent)'),
        ('&boneTable[i]','RwHelper_HostVectorAt(boneTable,i)'),
        ('boneTable[0].x','(*RwHelper_HostVectorField(RwHelper_HostVectorAt(boneTable,0),0))'),
        ('boneTable[0].y','(*RwHelper_HostVectorField(RwHelper_HostVectorAt(boneTable,0),1))'),
        ('boneTable[0].z','(*RwHelper_HostVectorField(RwHelper_HostVectorAt(boneTable,0),2))')])
    source=scoped(source,'Im2DRenderQuad','int32_t',[( 'float',v) for v in ['x1','y1','x2','y2','z','recipCamZ','uvOffset']],'QuadVertices','void *vx',[('RwIm2DVertex        vx[4];','')])
    for name in ['cbsCalcMeanBSphereRadiusCB','cbsCalcMeanBSphereCenterCB']:
        # Retain the SDK callback's original return value while its temporary vectors remain live
        changes=[('RwV3d atomicPos;',''),('&atomicPos','atomicPos')]
        if name.endswith('RadiusCB'):changes += [('RwV3d temp;',''),('&temp','temp')]
        source=scoped(source,name,'void *',[('void *','atomic'),('void *','originalData')],'AtomicVectors','void *atomicPos,void *temp',changes)
        # Restore the original callback data expression without shadowing the storage callback argument
        start,end,value=definition(source,name)
        callbackStart=source.rfind('static void RwHelper_'+name+'Body',0,start)
        piece=source[callbackStart:start].replace('void *data,void *atomicPos','void *storage,void *atomicPos').replace('*)data;','*)storage;').replace('void * originalData=scope->originalData;','void * data=scope->originalData;')
        source=source[:callbackStart]+piece+source[start:]
    source=scoped(source,'RpClumpGetBoundingSphere','void *',[('void *','clump'),('void *','sphere'),('bool','useLTM')],'BoundingSphere','void *matrix,void *result',[
        ('RwMatrix matrix;',''),('RwSphere result = { 0.0f, 0.0f, 0.0f, 0.0f };','RwHelper_HostStoreSphere(result,0.0f,0.0f,0.0f,0.0f);'),
        ('&result.center','RwHelper_HostSphereCenter(result)'),('&result','result'),('&matrix','matrix'),('*sphere = result;','RwHelper_HostCopySphere(sphere,result);')])
    source=re.sub(r'^#include[^\n]*\n','',source,flags=re.M)
    source=source.replace('#define WITHD3D','')
    source=source.replace('CVector p1, p2;','Debug_Vector p1, p2;').replace('RwMatrix *mats','void *mats')
    source=re.sub(r'mats\[([^]]+)\]\.pos',r'RwHelper_HostVectorValue(RwHelper_HostMatrixPosition(RwHelper_HostMatrixAt(mats,\1)))',source)
    source=re.sub(r'Debug_FromVector\((p[12])\)',r'\1',source)
    source=re.sub(r'HIERNODEINFO\(hier\)\[([^]]+)\]\.flags',r'RwHelper_HostNodeFlags(hier,\1)',source)
    source=source.replace('hier->numNodes','RwHelper_HostNumNodes(hier)').replace('HANIMFRAME(anim, i)','RwHelper_HostAnimationFrame(anim,i)')
    for field,index in [('q.real',0),('q.imag.x',1),('q.imag.y',2),('q.imag.z',3),('t.x',4),('t.y',5),('t.z',6),('time',7)]:source=source.replace('frame->'+field,'(*RwHelper_HostKeyFrameField(frame,'+str(index)+'))')
    source=source.replace('frame->prevFrame = nil;','RwHelper_HostPreviousFrame(frame,nil);')
    source=source.replace('atomic->clump','RwHelper_HostAtomicClump(atomic)')
    source=source.replace('&RpAtomicGetBoundingSphere(atomic)->center','RwHelper_HostSphereCenter(RpAtomicGetBoundingSphere(atomic))').replace('RpAtomicGetBoundingSphere(atomic)->radius','(*RwHelper_HostSphereRadius(RpAtomicGetBoundingSphere(atomic)))')
    source=re.sub(r'&\(\(RwSphere\s*\*\)data\)->center','RwHelper_HostSphereCenter(data)',source)
    source=re.sub(r'\(\(RwSphere\s*\*\)data\)->radius','(*RwHelper_HostSphereRadius(data))',source)
    source=source.replace('&sphere->center','RwHelper_HostSphereCenter(sphere)').replace('sphere->radius','(*RwHelper_HostSphereRadius(sphere))')
    for axis,index in [('x',0),('y',1),('z',2)]:source=source.replace('sphere->center.'+axis,'(*RwHelper_HostVectorField(RwHelper_HostSphereCenter(sphere),'+str(index)+'))')
    source=camera(source)
    source=platform(source)
    source=source.replace('RwRGBA color','RwHelperColour color').replace('RwRGBA colorbg','RwHelperColour colorbg').replace('RtCharsetCreate(&color, &colorbg)','RwHelper_HostCharsetCreate(&color,&colorbg)')
    source=source.replace('static WCHAR tmp[256];','void *tmp=RwHelper_HostWideBuffer();').replace('MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, name, -1, tmp, sizeof(tmp));','RwHelper_HostToWide(name,tmp,256*sizeof(uint16_t));').replace('D3DPERF_BeginEvent(0xFFFFFFFF, tmp);','RwHelper_HostPerfBegin(0xFFFFFFFF,tmp);').replace('D3DPERF_EndEvent();','RwHelper_HostPerfEnd();')
    source=source.replace('GLAD_GL_KHR_debug','RwHelper_HostGLDebugAvailable()').replace('glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, name)','RwHelper_HostGLDebugPush(name)').replace('glPopDebugGroup()','RwHelper_HostGLDebugPop()')
    for name,_,operation,_ in ITERATORS:source=re.sub(r'\b'+operation+r'\b','RwHelper_Host'+name,source)
    for old,new in SDK_MAP.items():source=re.sub(r'\b'+old+r'\b','RwHelper_Host'+new,source)
    source=re.sub(r'&vx\[([^]]+)\]',r'RwHelper_HostVertexAt(vx,\1)',source)
    source=source.replace('RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vx, 4)','RwHelper_HostDrawStrip(vx,4)')
    for name in ['ScreenX','ScreenY','ScreenZ','IntRGBA','RecipCameraZ','U','V']:source=source.replace('RwIm2DVertexSet'+name,'RwHelper_HostVertex'+name)
    source=source.replace('RwRenderStateSet(','RwHelper_HostRenderState(').replace('RWRGBALONG(','RwHelper_PackColour(')
    source=source.replace('RwD3D8SetRenderState(','RwHelper_HostD3DSetState(').replace('RwD3D8GetRenderState(','RwHelper_HostD3DGetState(')
    for kind in OPAQUE:source=re.sub(r'\b'+kind+r'\b','void',source)
    for old,new in [('RwReal','float'),('RwInt32','int32_t'),('RwUInt32','uint32_t'),('RwBool','int32_t'),('uint32','uint32_t'),('int32','int32_t')]:source=re.sub(r'\b'+old+r'\b',new,source)
    constants=sorted(set(re.findall(r'\b(?:rw[A-Z]\w*|rpHANIM\w*|rpMATFX\w*|D3DRS_\w+|D3DCMP_\w+)\b',mask(source))))
    for name in constants:source=re.sub(r'\b'+name+r'\b','RwHelper_HostConstant(RWH_CONST_'+name+')',source)
    # Add one explanatory comment at each original and generated function body boundary
    source=re.sub(r'(\)\n\{\n)(?!\s*//)',r'\1    // Preserve the original rendering-helper operation and its live application state\n',source)
    return '//+ rouz edit (ChatGPT)\n#include "RwHelper.h"\n#include "RwHelper_backend.h"\n#include "Timecycle.h"\n#include "Debug.h"\n#include "MBlur.h"\n#include <stddef.h>\n#define nil NULL\n#define TRUE 1\n'+source+'\n//- rouz edit (ChatGPT)\n',constants

def camera(source):
    # Preserve live caller rectangle fields and the original resize fallback sequence without SDK record aliasing
    start,end,value=definition(source,'CameraSize')
    assert 'if (camera)\n\t{' in value
    value=value.replace('if (camera)\n\t{','if (camera)\n\t{\n        RwHelperRectRef rect=RwHelper_HostRect(rectangle);')
    value=value.replace('RwVideoMode','RwHelperVideoMode').replace('RwRect','RwHelperRect').replace('RwV2d','RwHelperVector2')
    value=value.replace('if (!rect)','if (!rect.w)').replace('rect = &r;','rect = RwHelper_LocalRect(&r);')
    for field in ['x','y','w','h']:value=value.replace('rect->'+field,'(*rect.'+field+')')
    value=value.replace('RwEngineGetVideoModeInfo','RwHelper_HostVideoMode').replace('RwEngineGetCurrentVideoMode','RwHelper_HostCurrentVideoMode').replace('RwCameraSetViewWindow','RwHelper_HostViewWindow')
    value=value.replace('camera->frameBuffer = nil;','RwHelper_HostStoreCameraRaster(camera,nil);').replace('camera->zBuffer = nil;','RwHelper_HostStoreCameraZRaster(camera,nil);')
    for field in ['width','height']:
        for raster in ['raster','zRaster']:value=value.replace(raster+'->'+field,'(*RwHelper_HostRaster'+field.title()+'Slot('+raster+'))')
    value=re.sub(r'RsGlobal\.(width|height)\s*=\s*([^;]+);',lambda m:'RwHelper_HostScreen'+m[1].title()+'('+m[2]+');',value)
    return source[:start]+'void CameraSize(void *camera,void *rectangle,float viewWindow,float aspectRatio)\n{'+value+'}\n'+source[end:]

def platform(source):
    # Keep every pipeline-selection decision and temporary global-platform transition in C
    source=source.replace('rw::Atomic','void').replace('rw::Geometry *g = a->geometry;','void *g=RwHelper_HostAtomicGeometryField(a);').replace('g->instData->platform','RwHelper_HostInstancePlatform(RwHelper_HostInstanceData(g))').replace('g->instData','RwHelper_HostInstanceData(g)')
    source=source.replace('atomic->pipeline = rw::skinGlobals.pipelines[rw::platform];','RwHelper_HostSetPipeline(atomic,RwHelper_HostSkinPipeline(RwHelper_HostPlatform()));')
    source=source.replace('RpGeometryForAllMaterials(RpAtomicGetGeometry(atomic), CVehicleModelInfo::GetMatFXEffectMaterialCB, &fx);','RwHelper_HostForMaterials(RpAtomicGetGeometry(atomic),&fx);').replace('RpMatFXAtomicEnableEffects(atomic)','RwHelper_HostMaterialEffects(atomic)')
    source=source.replace('a->pipeline = rw::skinGlobals.pipelines[platform];','RwHelper_HostSetPipeline(a,RwHelper_HostSkinPipeline(platform));').replace('a->pipeline = rw::matFXGlobals.pipelines[platform];','RwHelper_HostSetPipeline(a,RwHelper_HostMatFXPipeline(platform));')
    source=source.replace('a->pipeline->platform','RwHelper_HostPipelinePlatform(RwHelper_HostPipeline(a))').replace('a->pipeline->pluginID','RwHelper_HostPipelinePlugin(RwHelper_HostPipeline(a))').replace('a->pipeline','RwHelper_HostPipeline(a)')
    source=source.replace('rw::ObjPipeline *origPipe = atomic->pipeline;','void *origPipe=RwHelper_HostPipeline(atomic);').replace('rw::platform = platform;','RwHelper_HostStorePlatform(platform);').replace('rw::platform = driver;','RwHelper_HostStorePlatform(driver);').replace('atomic->pipeline = origPipe;','RwHelper_HostSetPipeline(atomic,origPipe);')
    source=source.replace('atomic->geometry->flags','RwHelper_HostGeometryFlags(RwHelper_HostAtomicGeometryField(atomic))').replace('atomic->uninstance()','RwHelper_HostUninstance(atomic)').replace('rw::platform','RwHelper_HostPlatform()')
    source=re.sub(r'switch\(plgid\)\{[\s\S]*?case rw::ID_SKIN:([\s\S]*?)break;\s*case rw::ID_MATFX:([\s\S]*?)break;\s*\}',lambda m:'if(plgid==rw::ID_SKIN){'+m[1]+'}else if(plgid==rw::ID_MATFX){'+m[2]+'}',source)
    for name in ['ID_SKIN','ID_MATFX','Geometry::NATIVE','ALPHATESTFUNC','ALPHATESTREF','GSALPHATEST','ALPHAGREATEREQUAL']:source=source.replace('rw::'+name,'RwHelper_HostConstant(RWH_CONST_LIB_'+name.replace('::','_')+')')
    return source.replace('rw::SetRenderState(','RwHelper_HostLibSetState(').replace('rw::GetRenderState(','RwHelper_HostLibGetState(').replace('ConvertPlatformAtomic(','RwHelper_ConvertPlatformAtomic(')


def producer(original):
    # Materialize complete C policy with explicit prototypes and original assertion behavior
    value,constants=kernel(original)
    helpers='''#define FALSE 0
    void re3_assert(const char *expression,const char *file,unsigned int line,const char *function);
    #ifndef MASTER
    #define assert(expression) (void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0))
    #else
    #define assert(expression) (expression)
    #endif
    #define RwHelper_PackColour(r,g,b,a) ((uint32_t)(((a)<<24)|((r)<<16)|((g)<<8)|(b)))
    static RwHelperRectRef RwHelper_LocalRect(RwHelperRect *rectangle)
    {
        // Borrow direct scalar subobjects of the C-owned fallback rectangle
        RwHelperRectRef value={&rectangle->x,&rectangle->y,&rectangle->w,&rectangle->h};return value;
    }
    '''
    helpers=''.join(line[4:] if line.startswith('    ') else line for line in helpers.splitlines(keepends=True))
    value=value.replace('#define TRUE 1\n','#define TRUE 1\n'+helpers)
    value=re.sub(r'(?m)^((?:void\s+)?(?:OpenCharsetSafe|CreateDebugFont|DestroyDebugFont|FlushObrsPrintfs|RestoreAlphaTest))\(\)',r'\1(void)',value)
    value=re.sub(r'\(void\*\)(?=RwHelper_HostConstant|RwHelper_PackColour|mode\b)','(void*)(uintptr_t)',value)
    value=re.sub(r'\bGetFirstChild\b','RwHelper_GetFirstChild',value)
    return value

def header(original):
    # Expose the complete C API while retaining only separately owned typed SDK callback declarations
    original=re.sub(r'//[^\n]*','',original);original=re.sub(r'^#pragma once\s*','',original);external=[]
    for line in original.splitlines():
        # Retain SDK callback declarations whose implementations remain in their respective native owners
        if 'AtomicRemoveAnimFromSkinCB(' in line or 'ConvertPlatformAtomic(' in line:external.append(line)
    original='\n'.join(line for line in original.splitlines() if line not in external)
    for kind in OPAQUE+['RwV3d','RwSphere','RwRect']:original=re.sub(r'\b'+kind+r'\b','void',original)
    for old,new in [('RwBool','int32_t'),('RwReal','float'),('RwInt32','int32_t'),('RwUInt32','uint32_t'),('uint32','uint32_t')]:original=re.sub(r'\b'+old+r'\b',new,original)
    header='//+ rouz edit (ChatGPT)\n#pragma once\n#include <stdint.h>\n#include <stdbool.h>\n#include "config.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+original+'\nvoid *RwHelper_ConvertPlatformAtomic(void *atomic,void *data);\n#ifdef __cplusplus\n}\n'+'\n'.join(external)+'\n#endif\n//- rouz edit (ChatGPT)\n'
    header=re.sub(r'(?m)^(void\s+\w+)\(\)',r'\1(void)',header)
    header=header.replace('#include "config.h"\n','#include "config.h"\n#if defined(LIBRW) && (defined(RW_GL3) || defined(RW_WDGL)) && !defined(RW_OPENGL)\n#define RW_OPENGL\n#endif\n')
    header=header.replace('#ifdef __cplusplus\n}\n','#ifdef __cplusplus\n}\n#ifdef RWCORE_H\n').replace('\n#endif\n//- rouz edit (ChatGPT)','\n#endif\n#endif\n//- rouz edit (ChatGPT)')
    header=header.replace('void *RwHelper_ConvertPlatformAtomic','void PushRendergroup(const char *name);\nvoid PopRendergroup(void);\nvoid *RwHelper_ConvertPlatformAtomic')
    header=re.sub(r'\bGetFirstChild\b','RwHelper_GetFirstChild',header)
    return header

def rw_helper_calls(source):
    # Replace the complete owner and retain explicit typed SDK handles at every existing native return use
    from rw_helper_schema import RETURNS
    from rect_migration import closing
    if 'void *RwHelper_ConvertPlatformAtomic(void *atomic,void *data);' in source:return source
    if 'void OpenCharsetSafe();' in source and 'RwFrame *GetFirstChild(RwFrame *frame);' in source:return header(source)
    if 'RwMatrix m, invmat;' in source and 'SkinGetBonePositionsToTable(RpClump *clump, RwV3d *boneTable)' in source:return producer(source)
    if '#include "RwHelper_backend.h"' in source:return source
    original=source
    if '// defined in RwHelpder.cpp\nvoid PushRendergroup(const char *name);\nvoid PopRendergroup(void);' in source:
        source=source.replace('// defined in RwHelpder.cpp\nvoid PushRendergroup(const char *name);\nvoid PopRendergroup(void);','#include "RwHelper.h"')
    code=mask(source);changes=[]
    for match in re.finditer(r'\b('+ '|'.join(RETURNS)+r')\s*\(',code):
        # Preserve literal text and already converted call expressions when repeating the whole-source projection
        name=match[1];kind=RETURNS[name]
        if source[:match.start()].endswith('(('+kind+'*)'):continue
        opening=code.index('(',match.start());end=closing(code,opening);changes.append((match.start(),end,'(('+kind+'*)'+source[match.start():end]+')'))
    for start,end,value in reversed(changes):source=source[:start]+value+source[end:]
    if source!=original:
        # Mark complete native consumer edits without changing macro continuations or introducing comments into declarations
        lines=source.splitlines(keepends=True);codeLines=mask(source).splitlines(keepends=True);depth=0;continuation=False;commentLines=[]
        for index,line in enumerate(lines):
            directive=continuation or line.lstrip().startswith('#');continuation=line.rstrip().endswith('\\') if directive else False
            if not directive and depth>0 and any('(('+kind+'*)'+name+'(' in line for name,kind in RETURNS.items()):commentLines.append(index)
            depth+=codeLines[index].count('{')-codeLines[index].count('}')
        for index in reversed(commentLines):lines.insert(index,re.match(r'[ \t]*',lines[index])[0]+'// Use the C helper and retain the original typed SDK result\n')
        return '//+ rouz edit (ChatGPT)\n'+''.join(lines)+'//- rouz edit (ChatGPT)\n'
    return source


def rw_helper_project(source):
    # Select strict C11 for the complete policy and C forwarding units and add only typed SDK primitives as native
    import object_names_migration as objects
    source=source.replace('src\\rw\\RwHelper.cpp','src\\rw\\RwHelper.c')
    if 'Microsoft.Cpp.targets' in source:
        pattern=r'<ClCompile Include="([^"]*src\\rw\\RwHelper\.c)"\s*(?:/>|>([\s\S]*?)</ClCompile>)'
        def metadata(match):
            # Preserve original per-file options and select C for both complete application units
            body=match[2] or ''
            for tag,value in [('CompileAs','CompileAsC'),('LanguageStandard_C','stdc11')]:
                if '<'+tag+'>' in body:body=re.sub(r'<'+tag+r'>[^<]*</'+tag+'>','<'+tag+'>'+value+'</'+tag+'>',body)
                else:body+='\n      <'+tag+'>'+value+'</'+tag+'>'
            result='<ClCompile Include="'+match[1]+'">'+body.rstrip()+'\n    </ClCompile>';base=match[1][:-len('RwHelper.c')]
            if '<ClCompile Include="'+base+'RwHelper_native.cpp"' not in source:result+='\n    <ClCompile Include="'+base+'RwHelper_native.cpp" />'
            return result
        source=re.sub(pattern,metadata,source)
    return objects.project(source)

def native(original):
    # Reconstruct the entire SDK primitive boundary without depending on mutable generated files
    from rw_helper_native_generator import build
    return build(original)[1]

def backend_header(original):
    # Reconstruct all private C declarations from the same original owner and explicit SDK primitive schemas
    from rw_helper_native_generator import build
    return build(original)[0]



def calls(source):
    # Retain the complete preceding helper conversion before applying canonical C shadow policy and consumers
    import shadow_cameras_migration as shadow
    if '#include "Shadow_backend.h"' in source:return shadow.calls(source)
    return shadow.calls(rw_helper_calls(source))


def project(source):
    # Select the complete C shadow pair after all preceding C owner metadata
    import shadow_cameras_migration as shadow
    return shadow.project(rw_helper_project(source))
