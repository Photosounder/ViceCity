"""Move complete shadow render/effect policy into C while retaining only typed SDK storage and primitives."""
import re
from memory_heap_migration import body
from mblur_schema import SETTERS
import shadow_cameras_resources as resources

CONSTANTS=['rpGEOMETRYPRELIT','rpGEOMETRYLIGHT','rpGEOMETRYTEXTURED','rpGEOMETRYTEXTURED2','rpGEOMETRYMODULATEMATERIALCOLOR','rpGEOMETRYNORMALS','rwCAMERACLEARZ','rwCAMERACLEARIMAGE','rwRENDERSTATEZTESTENABLE','rwRENDERSTATETEXTURERASTER','rwRENDERSTATEVERTEXALPHAENABLE','rwRENDERSTATESRCBLEND','rwRENDERSTATEDESTBLEND','rwRENDERSTATETEXTUREFILTER','rwRENDERSTATESHADEMODE','rwBLENDINVDESTCOLOR','rwBLENDZERO','rwBLENDSRCALPHA','rwBLENDINVSRCALPHA','rwBLENDONE','rwBLENDSRCCOLOR','rwFILTERLINEAR','rwFILTERNAFILTERMODE','rwSHADEMODEFLAT','rwSHADEMODEGOURAUD']
ALIASES={'RpAtomicGetGeometry':'RwHelper_HostAtomicGeometry','RpGeometryGetFlags':'Shadow_HostGeometryFlags','RpGeometrySetFlags':'Shadow_HostGeometrySetFlags','RpClumpForAllAtomics':'RwHelper_HostAtomics','RwCameraClear':'Shadow_HostClear','RwCameraBeginUpdate':'Shadow_HostBeginUpdate','RwCameraEndUpdate':'Shadow_HostEndUpdate','RwCameraGetNearClipPlane':'Shadow_HostGetNearClip','RwRasterGetWidth':'RwHelper_HostRasterWidth','RwRasterGetHeight':'RwHelper_HostRasterHeight','RwIm2DGetNearScreenZ':'Shadow_HostNearScreenZ','RwRenderStateSet':'RwHelper_HostRenderState','ShadowRenderCallBack':'Shadow_RenderAtomic'}
METHODS=[('UpdateClump','Update(RpClump *clump)','void *',',void *clump'),('UpdateAtomic','Update(RpAtomic *atomic)','void *',',void *atomic'),('InvertRaster','InvertRaster()','void',''),('MakeGradientRaster','MakeGradientRaster()','void *',''),('RasterResample','RasterResample(RwRaster *dstRaster)','void *',',void *dstRaster'),('RasterBlur','RasterBlur(RwRaster *dstRaster, int32 numPasses)','void *',',void *dstRaster,int32_t numPasses'),('RasterGradient','RasterGradient(RwRaster *dstRaster)','void *',',void *dstRaster'),('DrawOutlineBorder','DrawOutlineBorder(RwRGBA const& color)','void *',',const ShadowColour *color')]

def methods(original):
    # Keep every original render and effect branch in C including the exact failed-begin state behavior
    result=[]
    for name,signature,returns,parameters in METHODS:
        value=body(original,'CShadowCamera::'+signature)
        for field in ['m_pCamera','m_pTexture']:value=re.sub(r'\b'+field+r'\b','self->'+field,value)
        value=value.replace('InvertRaster();','ShadowCamera_InvertRaster(self);')
        value=value.replace('BeginSoftwareShadowPass(self->m_pCamera, bgColor)','Shadow_BeginSoftwareClear(self->m_pCamera,&bgColor)').replace('BeginSoftwareShadowPass(self->m_pCamera, clearColor)','Shadow_BeginSoftwareClear(self->m_pCamera,&clearColor)').replace('BeginSoftwareShadowPass(self->m_pCamera, raster)','Shadow_BeginSoftwareRaster(self->m_pCamera,raster)').replace('EndSoftwareShadowPass(','Shadow_EndSoftware(')
        value=value.replace('RwRGBA','ShadowColour').replace('RwImVertexIndex','uint16_t')
        for old,new in {**resources.ALIASES,**ALIASES}.items():value=re.sub(r'\b'+old+r'\b',new,value)
        for old in ['RwRaster','RpGeometry','RpClump','RpAtomic']:value=re.sub(r'\b'+old+r'\b','void',value)
        for old,new in [('RwUInt32','uint32_t'),('uint32','uint32_t'),('int32','int32_t')]:value=re.sub(r'\b'+old+r'\b',new,value)
        value=value.replace('Im2DRenderQuad(','Im2DRenderQuad(')
        for constant in CONSTANTS:value=re.sub(r'\b'+constant+r'\b','Shadow_HostEffectConstant(SHADOW_EFFECT_CONST_'+constant+')',value)
        value=re.sub(r'\(void\s*\*\)\s*(?=Shadow_HostEffectConstant)','(void*)(uintptr_t)',value)
        for field in ['red','green','blue','alpha']:value=value.replace('color.'+field,'color->'+field)
        if name in ['InvertRaster','MakeGradientRaster','DrawOutlineBorder']:
            # Borrow real SDK vertex arrays while retaining the complete construction and draw algorithm in C
            count=2 if name=='MakeGradientRaster' else 4;pattern=r'RwIm2DVertex\s+vx\['+str(count)+r'\];';assert re.search(pattern,value),name;value=re.sub(pattern,'',value)
            value=re.sub(r'&vx\[([^]]+)\]',r'Shadow_HostVertex(vx,\1)',value)
            for setter in SETTERS:value=re.sub(r'\bRwIm2DVertexSet'+setter+r'\b','Shadow_HostVertex'+setter,value)
            value=value.replace('RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP, vx, 4)','Shadow_HostDrawStrip(vx,4)').replace('RwIm2DRenderLine(vx, 2, 0, 1)','Shadow_HostDrawLine(vx,2,0,1)').replace('RwIm2DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE, vx, 4, ix, 5)','Shadow_HostDrawPolyline(vx,4,ix,5)')
            context='Shadow_'+name+'Scope';fields='ShadowCamera *self;'+('const ShadowColour *color;' if name=='DrawOutlineBorder' else '')+(returns+' result;' if returns!='void' else '')
            if returns!='void':value=re.sub(r'\breturn\s+([^;]+);',r'{scope->result=\1;return;}',value)
            result.append('typedef struct '+context+' {'+fields+'} '+context+';\nstatic void Shadow_'+name+'Body(void *data,void *vx)\n{\n    // Execute complete original effect policy with native vertex storage alive throughout the callback\n    '+context+' *scope=('+context+'*)data;ShadowCamera *self=scope->self;'+('const ShadowColour *color=scope->color;' if name=='DrawOutlineBorder' else '')+value+'\n}\n'+returns+' ShadowCamera_'+name+'(ShadowCamera *self'+parameters+')\n{\n    // Preserve distinct per-call SDK vertices and the original conditional return control flow\n    '+context+' scope={self'+(',color' if name=='DrawOutlineBorder' else '')+(',NULL' if returns!='void' else '')+'};'+('Shadow_HostTwoVertices' if count==2 else 'RwHelper_HostQuadVertices')+'(Shadow_'+name+'Body,&scope);'+('return scope.result;' if returns!='void' else '')+'\n}\n')
        else:result.append(returns+' ShadowCamera_'+name+'(ShadowCamera *self'+parameters+')\n{\n    // Preserve the complete original render or raster-effect operation and its canonical camera handle\n'+value+'\n}\n')
    return '#define TEXELOFFSET 0.5f\n#define TRUE 1\n#define FALSE 0\n'+helpers()+'\n'+'\n'.join(result)

def helpers():
    # Keep software configuration branches and callback save/restore decisions in C
    return '''bool Shadow_BeginSoftwareClear(void *camera,const ShadowColour *colour)
{
    // Select the original software colour overload only when software rasterization is configured
#ifdef REVC_SOFTWARE_POLYGONS
    return Shadow_HostSoftwareClear(camera,colour);
#else
    (void)camera;(void)colour;return false;
#endif
}
bool Shadow_BeginSoftwareRaster(void *camera,void *raster)
{
    // Select the original software initial-image overload without changing raster ownership
#ifdef REVC_SOFTWARE_POLYGONS
    return Shadow_HostSoftwareRaster(camera,raster);
#else
    (void)camera;(void)raster;return false;
#endif
}
void Shadow_EndSoftware(void *camera,bool active)
{
    // Query the current camera raster only at the original active-pass upload boundary
#ifdef REVC_SOFTWARE_POLYGONS
    if(active)Shadow_HostSoftwareEnd(RwHelper_HostCameraRaster(camera));
#else
    (void)camera;(void)active;
#endif
}
void *Shadow_RenderAtomic(void *atomic,void *data)
{
    // Save and restore the actual SDK render callback without converting a function pointer into a data pointer
    (void)data;ShadowRenderToken saved=Shadow_HostGetRenderCallback(atomic);Shadow_HostSetRenderCallback(atomic,Shadow_HostDefaultRenderCallback());Shadow_HostAtomicRender(atomic);Shadow_HostSetRenderCallback(atomic,saved);return atomic;
}
'''

def extend(header,native):
    # Add typed SDK render primitives and software-owner forwarding without moving shadow policy out of C
    extraHeader='''typedef void (*ShadowRenderToken)(void);
bool Shadow_BeginSoftwareClear(void *camera,const ShadowColour *colour);
bool Shadow_BeginSoftwareRaster(void *camera,void *raster);
void Shadow_EndSoftware(void *camera,bool active);
void *Shadow_RenderAtomic(void *atomic,void *data);
'''
    extraNative='#ifdef REVC_SOFTWARE_POLYGONS\n#include "SoftwarePolygons.h"\n#endif\n'
    primitives=[
('uint32_t','GeometryFlags','void *geometry','return RpGeometryGetFlags((RpGeometry*)geometry);'),
('void','GeometrySetFlags','void *geometry,uint32_t flags','RpGeometrySetFlags((RpGeometry*)geometry,flags);'),
('bool','BeginUpdate','void *camera','return RwCameraBeginUpdate((RwCamera*)camera)!=NULL;'),
('void','EndUpdate','void *camera','RwCameraEndUpdate((RwCamera*)camera);'),
('void','Clear','void *camera,const ShadowColour *colour,int32_t flags','RwRGBA native={colour->red,colour->green,colour->blue,colour->alpha};RwCameraClear((RwCamera*)camera,&native,flags);'),
('float','GetNearClip','void *camera','return RwCameraGetNearClipPlane((RwCamera*)camera);'),
('float','NearScreenZ','void','return RwIm2DGetNearScreenZ();'),
('ShadowRenderToken','GetRenderCallback','void *atomic','return reinterpret_cast<ShadowRenderToken>(RpAtomicGetRenderCallBack((RpAtomic*)atomic));'),
('void','SetRenderCallback','void *atomic,ShadowRenderToken callback','RpAtomicSetRenderCallBack((RpAtomic*)atomic,reinterpret_cast<RpAtomicCallBackRender>(callback));'),
('ShadowRenderToken','DefaultRenderCallback','void','return reinterpret_cast<ShadowRenderToken>(AtomicDefaultRenderCallBack);'),
('void','AtomicRender','void *atomic','RpAtomicRender((RpAtomic*)atomic);'),
('void *','Vertex','void *vertices,int32_t index','return &((RwIm2DVertex*)vertices)[index];'),
('void','DrawStrip','void *vertices,int32_t count','RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP,(RwIm2DVertex*)vertices,count);'),
('void','DrawLine','void *vertices,int32_t count,int32_t first,int32_t second','RwIm2DRenderLine((RwIm2DVertex*)vertices,count,first,second);'),
('void','DrawPolyline','void *vertices,int32_t count,uint16_t *indices,int32_t indexCount','RwIm2DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE,(RwIm2DVertex*)vertices,count,indices,indexCount);'),
('void','TwoVertices','RwHelperVertexScope callback,void *data','RwIm2DVertex vertices[2];callback(data,vertices);')]
    for result,name,args,value in primitives:
        # Forward individual SDK calls and retain actual native callback and vertex representations
        declaration=result+' Shadow_Host'+name+'('+args+')';extraHeader+=declaration+';\n';extraNative+=declaration+'\n{\n    // Preserve the exact typed SDK primitive and its borrowed arguments\n    '+value+'\n}\n'
    for name,(args,values) in SETTERS.items():
        # Retain backend-specific vertex setters while C computes every vertex value and draw order
        declaration='void Shadow_HostVertex'+name+'(void *vertex,'+args+')';extraHeader+=declaration+';\n';extraNative+=declaration+'\n{\n    // Apply one original SDK vertex setter without exposing SDK record layout to C\n    RwIm2DVertexSet'+name+'((RwIm2DVertex*)vertex,'+values+');\n}\n'
    extraHeader+='#ifdef REVC_SOFTWARE_POLYGONS\nbool Shadow_HostSoftwareClear(void *camera,const ShadowColour *colour);\nbool Shadow_HostSoftwareRaster(void *camera,void *raster);\nvoid Shadow_HostSoftwareEnd(void *raster);\n#endif\n'
    extraNative+='''#ifdef REVC_SOFTWARE_POLYGONS
bool Shadow_HostSoftwareClear(void *camera,const ShadowColour *colour)
{
    // Preserve the existing software owner's colour overload with a real native RGBA input
    rw::RGBA native={colour->red,colour->green,colour->blue,colour->alpha};return SoftwarePolygons::BeginOffscreenFrame((RwCamera*)camera,native);
}
bool Shadow_HostSoftwareRaster(void *camera,void *raster)
{
    // Preserve the existing software owner's initial-image overload
    return SoftwarePolygons::BeginOffscreenFrame((RwCamera*)camera,(RwRaster*)raster);
}
void Shadow_HostSoftwareEnd(void *raster)
{
    // Upload the C-selected current raster through the unchanged software owner
    SoftwarePolygons::EndOffscreenFrame((RwRaster*)raster);
}
#endif
static_assert(sizeof(ShadowRenderToken)==sizeof(RpAtomicCallBackRender),"Preserve native render callback transport");
'''
    extraHeader+='enum {'+','.join('SHADOW_EFFECT_CONST_'+name for name in CONSTANTS)+'};\nint32_t Shadow_HostEffectConstant(int32_t identity);\n'
    extraNative+='int32_t Shadow_HostEffectConstant(int32_t identity)\n{\n    // Translate SDK enum identities while every render-state choice remains in C\n    switch(identity){'+''.join('case SHADOW_EFFECT_CONST_'+name+':return '+name+';' for name in CONSTANTS)+'}return 0;\n}\n'
    header=header.replace('#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)',extraHeader+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)')
    return header,native+'\n//+ rouz edit (ChatGPT)\n'+extraNative+'//- rouz edit (ChatGPT)\n'
