//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "Lights_backend.h"
#include "Coronas.h"
#include "Frontend.h"
#include "MBlur.h"
#ifdef EXTENDED_COLOURFILTER
#include "postfx.h"
#endif
#include <stddef.h>
static_assert(sizeof(LightsColour)==sizeof(RwRGBAReal),"Preserve native colour storage");
static_assert(alignof(LightsColour)==alignof(RwRGBAReal),"Preserve native colour alignment");
static_assert(sizeof(LightsMatrix)==sizeof(RwMatrix),"Preserve native matrix storage");
static_assert(alignof(LightsMatrix)==alignof(RwMatrix),"Preserve native matrix alignment");
static_assert(offsetof(LightsColour,red)==offsetof(RwRGBAReal,red),"Preserve red offset");
static_assert(offsetof(LightsColour,green)==offsetof(RwRGBAReal,green),"Preserve green offset");
static_assert(offsetof(LightsColour,blue)==offsetof(RwRGBAReal,blue),"Preserve blue offset");
static_assert(offsetof(LightsColour,alpha)==offsetof(RwRGBAReal,alpha),"Preserve alpha offset");
static_assert(offsetof(LightsMatrix,right)==offsetof(RwMatrix,right),"Preserve right offset");
static_assert(offsetof(LightsMatrix,flags)==offsetof(RwMatrix,flags),"Preserve flags offset");
static_assert(offsetof(LightsMatrix,up)==offsetof(RwMatrix,up),"Preserve up offset");
static_assert(offsetof(LightsMatrix,pad1)==offsetof(RwMatrix,pad1),"Preserve pad1 offset");
static_assert(offsetof(LightsMatrix,at)==offsetof(RwMatrix,at),"Preserve at offset");
static_assert(offsetof(LightsMatrix,pad2)==offsetof(RwMatrix,pad2),"Preserve pad2 offset");
static_assert(offsetof(LightsMatrix,pos)==offsetof(RwMatrix,pos),"Preserve pos offset");
static_assert(offsetof(LightsMatrix,pad3)==offsetof(RwMatrix,pad3),"Preserve pad3 offset");

void Lights_HostColour(void *light,const LightsColour *colour)
{
    // Copy native colour bytes without reading the original uninitialized alpha as a scalar
    RwRGBAReal native;memcpy(&native,colour,sizeof(native));RpLightSetColor((RpLight*)light,&native);
}
void Lights_HostRotate(void *frame,const LightsVector *axis,float angle)
{
    // Retain the original SDK rotation and preconcatenation mode
    const RwV3d native={axis->x,axis->y,axis->z};RwFrameRotate((RwFrame*)frame,&native,angle,rwCOMBINEPRECONCAT);
}
void Lights_HostTransform(void *frame,const LightsMatrix *matrix)
{
    // Preserve all original matrix bytes and the SDK replacement operation without aliasing record types
    RwMatrix native;memcpy(&native,matrix,sizeof(native));RwFrameTransform((RwFrame*)frame,&native,rwCOMBINEREPLACE);
}
LightsVectorOut Lights_HostAt(void *matrix)
{
    // Borrow the actual native direction components while keeping scalar writes in the C owner
    RwV3d *at=RwMatrixGetAt((RwMatrix*)matrix);const LightsVectorOut value={&at->x,&at->y,&at->z};return value;
}
float Lights_HostMultiplier(void)
{
    // Preserve the original SDK primitive or external owner metadata access
    return CCoronas::LightsMult;
}
int Lights_HostBrightness(void)
{
    // Preserve the original SDK primitive or external owner metadata access
    return FrontEndMenuManager.m_PrefsBrightness;
}
void * Lights_HostCreate(int kind)
{
    // Preserve the original SDK primitive or external owner metadata access
    return RpLightCreate(kind==LIGHTS_AMBIENT?rpLIGHTAMBIENT:rpLIGHTDIRECTIONAL);
}
void Lights_HostDestroy(void *light)
{
    // Preserve the original SDK primitive or external owner metadata access
    RpLightDestroy((RpLight*)light);
}
void Lights_HostFlags(void *light,int active)
{
    // Preserve the original SDK primitive or external owner metadata access
    RpLightSetFlags((RpLight*)light,active?rpLIGHTLIGHTATOMICS:0);
}
void Lights_HostRadius(void *light,float radius)
{
    // Preserve the original SDK primitive or external owner metadata access
    RpLightSetRadius((RpLight*)light,radius);
}
void * Lights_HostCreateFrame(void)
{
    // Preserve the original SDK primitive or external owner metadata access
    return RwFrameCreate();
}
void Lights_HostDestroyFrame(void *frame)
{
    // Preserve the original SDK primitive or external owner metadata access
    RwFrameDestroy((RwFrame*)frame);
}
void * Lights_HostFrame(void *light)
{
    // Preserve the original SDK primitive or external owner metadata access
    return RpLightGetFrame((RpLight*)light);
}
void Lights_HostSetFrame(void *light,void *frame)
{
    // Preserve the original SDK primitive or external owner metadata access
    RpLightSetFrame((RpLight*)light,(RwFrame*)frame);
}
void * Lights_HostMatrix(void *frame)
{
    // Preserve the original SDK primitive or external owner metadata access
    return RwFrameGetMatrix((RwFrame*)frame);
}
void Lights_HostUpdateMatrix(void *matrix)
{
    // Preserve the original SDK primitive or external owner metadata access
    RwMatrixUpdate((RwMatrix*)matrix);
}
void Lights_HostUpdateObjects(void *frame)
{
    // Preserve the original SDK primitive or external owner metadata access
    RwFrameUpdateObjects((RwFrame*)frame);
}
void Lights_HostAdd(void *world,void *light)
{
    // Preserve the original SDK primitive or external owner metadata access
    RpWorldAddLight((RpWorld*)world,(RpLight*)light);
}
void Lights_HostRemove(void *world,void *light)
{
    // Preserve the original SDK primitive or external owner metadata access
    RpWorldRemoveLight((RpWorld*)world,(RpLight*)light);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
