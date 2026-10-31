"""Supply typed record field access and SDK constants without moving helper policy out of C."""
import re
from mblur_schema import SETTERS

EXTRA=[
('float *','VectorField','void *vector,int32_t index','RwV3d *v=(RwV3d*)vector;float *fields[]={&v->x,&v->y,&v->z};return fields[index];',None),
('Debug_Vector','VectorValue','const void *vector','const RwV3d *v=(const RwV3d*)vector;Debug_Vector value={v->x,v->y,v->z};return value;',None),
('float *','KeyFrameField','void *frame,int32_t index','RpHAnimStdKeyFrame *f=(RpHAnimStdKeyFrame*)frame;float *fields[]={&f->q.real,&f->q.imag.x,&f->q.imag.y,&f->q.imag.z,&f->t.x,&f->t.y,&f->t.z,&f->time};return fields[index];',None),
('void','StoreSphere','void *sphere,float x,float y,float z,float radius','RwSphere *s=(RwSphere*)sphere;s->center.x=x;s->center.y=y;s->center.z=z;s->radius=radius;',None),
('RwHelperRectRef','Rect','void *rectangle','RwRect *r=(RwRect*)rectangle;RwHelperRectRef value={NULL,NULL,NULL,NULL};if(r){value.x=&r->x;value.y=&r->y;value.w=&r->w;value.h=&r->h;}return value;',None),
('int32_t','CurrentVideoMode','void','return RwEngineGetCurrentVideoMode();',None),
('void','VideoMode','RwHelperVideoMode *mode,int32_t index','RwVideoMode native;RwEngineGetVideoModeInfo(&native,index);mode->width=native.width;mode->height=native.height;mode->flags=native.flags;',None),
('void','ViewWindow','void *camera,const RwHelperVector2 *window','RwV2d native={window->x,window->y};RwCameraSetViewWindow((RwCamera*)camera,&native);',None),
('void *','CharsetCreate','const RwHelperColour *foreground,const RwHelperColour *background','RwRGBA fg={foreground->red,foreground->green,foreground->blue,foreground->alpha};RwRGBA bg={background->red,background->green,background->blue,background->alpha};return RtCharsetCreate(&fg,&bg);','!defined(FINAL) || defined(DEBUGMENU)'),
('void *','WideBuffer','void','static_assert(sizeof(WCHAR)==sizeof(uint16_t),"Preserve original wide storage element size");static WCHAR buffer[256];return buffer;','defined(RW_D3D9)'),
('void','ToWide','const char *name,void *buffer,uint32_t count','MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,name,-1,(WCHAR*)buffer,count);','defined(RW_D3D9)'),
('void','PerfBegin','uint32_t colour,void *buffer','D3DPERF_BeginEvent(colour,(WCHAR*)buffer);','defined(RW_D3D9)'),
('void','PerfEnd','void','D3DPERF_EndEvent();','defined(RW_D3D9)'),
('bool','GLDebugAvailable','void','return GLAD_GL_KHR_debug!=0;','defined(RW_OPENGL)'),
('void','GLDebugPush','const char *name','glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION,0,-1,name);','defined(RW_OPENGL)'),
('void','GLDebugPop','void','glPopDebugGroup();','defined(RW_OPENGL)'),
('void *','VertexAt','void *vertices,int32_t index','return &((RwIm2DVertex*)vertices)[index];',None),
('void','DrawStrip','void *vertices,int32_t count','RwIm2DRenderPrimitive(rwPRIMTYPETRISTRIP,(RwIm2DVertex*)vertices,count);',None),
('void','RenderState','uint32_t state,void *value','RwRenderStateSet((RwRenderState)state,value);',None),
('void','D3DSetState','uint32_t state,uint32_t value','RwD3D8SetRenderState(state,value);','!defined(LIBRW)'),
('void','D3DGetState','uint32_t state,uint32_t *value','RwD3D8GetRenderState(state,value);','!defined(LIBRW)'),
]

def extend(header,native,kernel,constants):
    # Keep C record declarations independent of SDK headers and borrow actual scalar subobjects explicitly
    records='''#include "Debug.h"
typedef struct RwHelperColour {uint8_t red,green,blue,alpha;} RwHelperColour;
typedef struct RwHelperRect {int32_t x,y,w,h;} RwHelperRect;
typedef struct RwHelperRectRef {int32_t *x,*y,*w,*h;} RwHelperRectRef;
typedef struct RwHelperVector2 {float x,y;} RwHelperVector2;
typedef struct RwHelperVideoMode {int32_t width,height;uint32_t flags;} RwHelperVideoMode;
'''
    header=header.replace('typedef void *(*RwHelperVisit)',records+'typedef void *(*RwHelperVisit)')
    extraHeader='';extraNative=''
    for result,name,args,code,guard in EXTRA:
        # Translate one SDK operation or representation access per native entry point
        if guard:extraHeader+='#if '+guard+'\n';extraNative+='#if '+guard+'\n'
        declaration=result+' RwHelper_Host'+name+'('+args+')';extraHeader+=declaration+';\n';extraNative+=declaration+'\n{\n    // Access the actual SDK primitive or typed record at the C-selected point\n    '+code+'\n}\n'
        if guard:extraHeader+='#endif\n';extraNative+='#endif\n'
    for name,(args,call) in SETTERS.items():
        # Keep vertex construction order and values in C while retaining backend-specific SDK layout
        declaration='void RwHelper_HostVertex'+name+'(void *vertex,'+args+')';extraHeader+=declaration+';\n';extraNative+=declaration+'\n{\n    // Forward the original SDK vertex setter and its exact C arguments\n    RwIm2DVertexSet'+name+'((RwIm2DVertex*)vertex,'+call+');\n}\n'
    identifiers=sorted(set(re.findall(r'\bRWH_CONST_(\w+)',kernel)));extraHeader+='enum {'+','.join('RWH_CONST_'+name for name in identifiers)+'};\nuint32_t RwHelper_HostConstant(int32_t identity);\n'
    extraNative+='uint32_t RwHelper_HostConstant(int32_t identity)\n{\n    // Translate SDK constant identities without embedding backend enum values in C policy\n    switch(identity){\n'
    for name in identifiers:
        # Compile proprietary D3D8 identities only on their original legacy SDK branch
        guard='defined(LIBRW)' if name.startswith('LIB_') or name=='rpMATFXEFFECTNULL' else '!defined(LIBRW)' if name.startswith('D3D') else None
        value='rw::'+name[4:].replace('Geometry_NATIVE','Geometry::NATIVE') if name.startswith('LIB_') else name
        if guard:extraNative+='#if '+guard+'\n'
        extraNative+='case RWH_CONST_'+name+':return '+value+';\n'
        if guard:extraNative+='#endif\n'
    extraNative+='}return 0;\n}\n'
    header=header.replace('#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)',extraHeader+'#ifdef __cplusplus\n}\n#endif\n//- rouz edit (ChatGPT)')
    extraNative+='''#include "RwHelper.h"
#ifdef LIBRW
RpAtomic *ConvertPlatformAtomic(RpAtomic *atomic,void *data)
{
    // Enter the complete C policy through the original typed SDK iterator callback ABI
    return (RpAtomic*)RwHelper_ConvertPlatformAtomic(atomic,data);
}
#endif
'''
    return header,native+'\n//+ rouz edit (ChatGPT)\n'+extraNative+'//- rouz edit (ChatGPT)\n'
