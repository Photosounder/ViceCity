//+ rouz edit (ChatGPT)
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
    static const RwRenderState states[]={rwRENDERSTATETEXTUREFILTER,rwRENDERSTATEFOGENABLE,rwRENDERSTATEZTESTENABLE,rwRENDERSTATEZWRITEENABLE,rwRENDERSTATETEXTURERASTER,rwRENDERSTATEVERTEXALPHAENABLE,rwRENDERSTATESRCBLEND,rwRENDERSTATEDESTBLEND};
    static const uintptr_t blends[]={rwBLENDONE,rwBLENDSRCALPHA,rwBLENDINVSRCALPHA,rwBLENDDESTALPHA,rwBLENDINVDESTALPHA};
    if(state==POSTFX_STATE_TEXTUREFILTER)value=(void*)rwFILTERNEAREST;
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
void * PostFX_HostCameraRaster(void *camera)
{
    // Preserve the original individual SDK primitive
    return RwCameraGetRaster((RwCamera*)camera);
}
float PostFX_HostNearClip(void *camera)
{
    // Preserve the original individual SDK primitive
    return RwCameraGetNearClipPlane((RwCamera*)camera);
}
int32_t PostFX_HostWidth(void *raster)
{
    // Preserve the original individual SDK primitive
    return RwRasterGetWidth((RwRaster*)raster);
}
int32_t PostFX_HostHeight(void *raster)
{
    // Preserve the original individual SDK primitive
    return RwRasterGetHeight((RwRaster*)raster);
}
int32_t PostFX_HostDepth(void *raster)
{
    // Preserve the original individual SDK primitive
    return RwRasterGetDepth((RwRaster*)raster);
}
void PostFX_HostDestroyRaster(void *raster)
{
    // Preserve the original individual SDK primitive
    RwRasterDestroy((RwRaster*)raster);
}
void PostFX_HostPushContext(void *raster)
{
    // Preserve the original individual SDK primitive
    RwRasterPushContext((RwRaster*)raster);
}
void PostFX_HostPopContext(void)
{
    // Preserve the original individual SDK primitive
    RwRasterPopContext();
}
void PostFX_HostRenderFast(void *raster,int32_t x,int32_t y)
{
    // Preserve the original individual SDK primitive
    RwRasterRenderFast((RwRaster*)raster,x,y);
}
float PostFX_HostNearScreenZ(void)
{
    // Preserve the original individual SDK primitive
    return RwIm2DGetNearScreenZ();
}
float PostFX_HostHalfPixel(void)
{
    // Preserve the original individual SDK primitive
    return HALFPX;
}
void PostFX_HostDefinedState(void)
{
    // Preserve the original individual SDK primitive
    DefinedState();
}
void PostFX_HostScreenX(void *vertex,float value)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetScreenX((RwIm2DVertex*)vertex,value);
}
void PostFX_HostScreenY(void *vertex,float value)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetScreenY((RwIm2DVertex*)vertex,value);
}
void PostFX_HostScreenZ(void *vertex,float value)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetScreenZ((RwIm2DVertex*)vertex,value);
}
void PostFX_HostCameraZ(void *vertex,float value)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetCameraZ((RwIm2DVertex*)vertex,value);
}
void PostFX_HostRecipCameraZ(void *vertex,float value)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetRecipCameraZ((RwIm2DVertex*)vertex,value);
}
void PostFX_HostU(void *vertex,float value,float reciprocal)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetU((RwIm2DVertex*)vertex,value,reciprocal);
}
void PostFX_HostV(void *vertex,float value,float reciprocal)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetV((RwIm2DVertex*)vertex,value,reciprocal);
}
void PostFX_HostIntRGBA(void *vertex,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha)
{
    // Apply the original SDK setter to the actual native vertex object
    RwIm2DVertexSetIntRGBA((RwIm2DVertex*)vertex,red,green,blue,alpha);
}
#endif
//- rouz edit (ChatGPT)
