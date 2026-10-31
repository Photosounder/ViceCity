//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "SpecialFX.h"
#include "Markers3D_backend.h"
#include "World.h"
#include "ColStore.h"
#include "Camera.h"
#include "TxdStore.h"
#include "FileLoader.h"
#include "Lights.h"
#include "Coronas.h"
#include "Script.h"
#include "VisibilityPlugins.h"
static RpAtomic *
MarkerAtomicCB(RpAtomic *atomic,void *data)
{
    // Retain the exact typed callback slot used to select the last atomic
    *(RpAtomic **)data=atomic;return atomic;
}
static_assert(MARKERS3D_GEOMETRY_MODULATE_MATERIAL_COLOR==rpGEOMETRYMODULATEMATERIALCOLOR,"Original geometry modulation flag");
void Markers3D_BackendUnity(void *marker)
{
    // Retain original typed matrix initialization
    ((C3dMarker *)marker)->m_Matrix.SetUnity();
}
void * Markers3D_BackendLastAtomic(void *clump)
{
    // Run the exact typed callback with an actual typed atomic slot
    RpAtomic *atomic=nil;RpClumpForAllAtomics((RpClump *)clump,MarkerAtomicCB,&atomic);return atomic;
}
void * Markers3D_BackendCloneAtomic(void *atomic)
{
    // Clone the originally selected atomic
    return RpAtomicClone((RpAtomic *)atomic);
}
void * Markers3D_BackendCreateFrame(void)
{
    // Create the original resource frame
    return RwFrameCreate();
}
void Markers3D_BackendSetFrame(void *atomic,void *frame)
{
    // Attach the original atomic and frame
    RpAtomicSetFrame((RpAtomic *)atomic,(RwFrame *)frame);
}
void Markers3D_BackendDefaultCallback(void *atomic)
{
    // Retain the original null atomic-render callback
    CVisibilityPlugins::SetAtomicRenderCallback((RpAtomic *)atomic,nil);
}
void * Markers3D_BackendAtomicGeometry(void *atomic)
{
    // Read the original typed atomic geometry
    return RpAtomicGetGeometry((RpAtomic *)atomic);
}
uint32_t Markers3D_BackendGeometryFlags(void *geometry)
{
    // Read the actual geometry flags
    return RpGeometryGetFlags((RpGeometry *)geometry);
}
void Markers3D_BackendSetGeometryFlags(void *geometry,uint32_t flags)
{
    // Publish the C-computed geometry flags
    RpGeometrySetFlags((RpGeometry *)geometry,flags);
}
void * Markers3D_BackendGeometryMaterial(void *geometry,int32_t index)
{
    // Read the original typed material slot
    return RpGeometryGetMaterial((RpGeometry *)geometry,index);
}
void Markers3D_BackendSetMaterial(void *marker,void *material)
{
    // Publish the original material through its actual pointer type
    ((C3dMarker *)marker)->m_pMaterial=(RpMaterial *)material;
}
void Markers3D_BackendAttachMarker(void *marker,void *matrix)
{
    // Preserve the original embedded matrix attachment and ownership
    ((C3dMarker *)marker)->m_Matrix.Attach((RwMatrix *)matrix);
}
void * Markers3D_BackendAtomicFrame(void *atomic)
{
    // Read the current atomic frame at the original call point
    return RpAtomicGetFrame((RpAtomic *)atomic);
}
void Markers3D_BackendDestroyAtomic(void *atomic)
{
    // Destroy the original live atomic
    RpAtomicDestroy((RpAtomic *)atomic);
}
void Markers3D_BackendDestroyFrame(void *frame)
{
    // Destroy the original captured frame
    RwFrameDestroy((RwFrame *)frame);
}
void Markers3D_BackendMaterialColor(void *marker)
{
    // Borrow the actual live material color without copying its bytes
    C3dMarker &m=*(C3dMarker *)marker;RpMaterialSetColor(m.m_pMaterial,&m.m_Color);
}
void * Markers3D_BackendAttachment(void *marker)
{
    // Read the current embedded matrix attachment
    return ((C3dMarker *)marker)->m_Matrix.m_attachment;
}
void Markers3D_BackendAttachMatrix(void *matrix,void *attachment)
{
    // Attach the independent original stack matrix
    ((CMatrix *)matrix)->Attach((RwMatrix *)attachment);
}
Markers3DBasis Markers3D_BackendMatrixBasis(void *matrix)
{
    // Borrow all nine actual matrix basis coefficients through typed pointers
    CMatrix &m=*(CMatrix *)matrix;Markers3DBasis result={{{&m.rx,&m.ry,&m.rz},{&m.fx,&m.fy,&m.fz},{&m.ux,&m.uy,&m.uz}}};return result;
}
void Markers3D_BackendPublishMatrix(void *matrix)
{
    // Publish the independent original render matrix
    ((CMatrix *)matrix)->UpdateRW();
}
void Markers3D_BackendUpdateFrame(void *frame)
{
    // Update the original frame objects
    RwFrameUpdateObjects((RwFrame *)frame);
}
void Markers3D_BackendBright(float value)
{
    // Set the original live marker brightness
    // Use the complete C lighting owner and its borrowed SDK handles
    Lights_SetBrightMarkerColours(value);
}
void Markers3D_BackendZWrite(bool enabled)
{
    // Set the original depth-write render state
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr)(enabled?TRUE:FALSE));
}
void Markers3D_BackendDrawAtomic(void *atomic)
{
    // Render the current original atomic
    RpAtomicRender((RpAtomic *)atomic);
}
void Markers3D_BackendRestoreLighting(void)
{
    // Restore original ambient and directional lighting
    // Use the complete C lighting owner and its borrowed SDK handles
    Lights_ReSetAmbientAndDirectionalColours();
}
void Markers3D_BackendWithRenderMatrix(void *marker,void (*render)(void *,void *))
{
    // Keep each temporary CMatrix alive until its complete C render sequence returns
    CMatrix matrix;render(marker,&matrix);
}
static C3dMarker MarkerOwners[NUM3DMARKERS];
static RpClump *MarkerClumps[NUMMARKERTYPES];
void * Markers3D_BackendMarker(int32_t index)
{
    // Address the actual typed marker pool without assuming its stride
    return &MarkerOwners[index];
}
Markers3DFields Markers3D_BackendFields(void *marker)
{
    // Borrow correctly typed scalar field addresses without copying live values
    C3dMarker &m=*(C3dMarker *)marker;
    Markers3DFields result={&m.m_nType,&m.m_bIsUsed,&m.m_bFindZOnNextPlacement,&m.m_nIdentifier,&m.m_Color.red,&m.m_Color.green,&m.m_Color.blue,&m.m_Color.alpha,&m.m_nPulsePeriod,&m.m_nRotateRate,&m.m_nStartTime,&m.m_fPulseFraction,&m.m_fStdSize,&m.m_fSize,&m.m_fBrightness,&m.m_fCameraRange};return result;
}
void * Markers3D_BackendAtomic(void *marker)
{
    // Read the actual typed atomic pointer at its original use
    return ((C3dMarker *)marker)->m_pAtomic;
}
void Markers3D_BackendSetAtomic(void *marker,void *atomic)
{
    // Write the actual atomic field without a pointer-to-pointer cast
    ((C3dMarker *)marker)->m_pAtomic=(RpAtomic *)atomic;
}
void * Markers3D_BackendClump(int32_t index)
{
    // Read each original live typed resource slot
    return MarkerClumps[index];
}
void Markers3D_BackendSetClump(int32_t index,void *clump)
{
    // Publish a resource through its actual typed pointer slot
    MarkerClumps[index]=(RpClump *)clump;
}
void Markers3D_BackendDestroyClump(void *clump)
{
    // Destroy the original typed clump without clearing its published slot
    RpClumpDestroy((RpClump *)clump);
}
Markers3DVector2D Markers3D_BackendPlayer(void)
{
    // Retain the original player query and two-component conversion
    CVector2D value=FindPlayerCentreOfWorld(0);
    Markers3DVector2D result={value.x,value.y};return result;
}
bool Markers3D_BackendCollision(Markers3DVector2D pos)
{
    // Enter the exact original collision-loaded predicate
    return CColStore::HasCollisionLoaded(CVector2D(pos.x,pos.y));
}
float Markers3D_BackendGround(float x,float y,float z)
{
    // Retain the original ground query and null found pointer
    return CWorld::FindGroundZFor3DCoord(x,y,z,nil);
}
float Markers3D_BackendReadLegacy(const void *pos,int32_t axis)
{
    // Read only the original live legacy coordinate component
    const CVector &v=*(const CVector *)pos;
    return axis==0?v.x:axis==1?v.y:v.z;
}
void Markers3D_BackendWriteLegacy(void *pos,int32_t axis,float value)
{
    // Write the original borrowed vector component through its actual type
    CVector &v=*(CVector *)pos;
    if(axis==0)v.x=value;else if(axis==1)v.y=value;else v.z=value;
}
Markers3DVector Markers3D_BackendPosition(void *marker)
{
    // Read actual marker matrix-position fields at their original use
    const CVector &v=((C3dMarker *)marker)->m_Matrix.GetPosition();
    Markers3DVector result={v.x,v.y,v.z};return result;
}
void Markers3D_BackendSetPosition(void *marker,Markers3DVector pos)
{
    // Retain the original typed matrix-position assignment
    ((C3dMarker *)marker)->m_Matrix.GetPosition()=CVector(pos.x,pos.y,pos.z);
}
void Markers3D_BackendSetZ(void *marker,float z)
{
    // Write only the original live matrix-height component
    ((C3dMarker *)marker)->m_Matrix.GetPosition().z=z;
}
void Markers3D_BackendSetTranslate(void *marker,Markers3DVector pos)
{
    // Enter the existing typed matrix translation operation
    ((C3dMarker *)marker)->m_Matrix.SetTranslate(pos.x,pos.y,pos.z);
}
void Markers3D_BackendRotateZ(void *marker,float angle)
{
    // Retain the original typed Z rotation
    ((C3dMarker *)marker)->m_Matrix.RotateZ(angle);
}
void Markers3D_BackendRotateX(void *marker,float angle)
{
    // Retain the original typed X rotation
    ((C3dMarker *)marker)->m_Matrix.RotateX(angle);
}
void Markers3D_BackendUpdateRW(void *marker)
{
    // Publish the original matrix to its attached RenderWare owner
    ((C3dMarker *)marker)->m_Matrix.UpdateRW();
}
void Markers3D_BackendDirectional(void)
{
    // Activate the original directional lighting at the same call point
    // Use the complete C lighting owner and its borrowed SDK handles
    Lights_ActivateDirectional();
}
float Markers3D_BackendLOD(void)
{
    // Read the original live camera distance multiplier
    return TheCamera.LODDistMultiplier;
}
void Markers3D_BackendCorona(void *marker,float size,float drawDist)
{
    // Retain the exact corona overload, borrowed matrix position, ID narrowing and defaults
    CCoronas::RegisterCorona((uintptr)marker,SPHERE_MARKER_R,SPHERE_MARKER_G,SPHERE_MARKER_B,192,((C3dMarker *)marker)->m_Matrix.GetPosition(),size,drawDist,CCoronas::TYPE_STAR,CCoronas::FLARE_NONE,CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_ON,0.0f,false);
}




void * Markers3D_BackendLoad(const char *name)
{
    // Enter the original typed atomic-file loader
    return CFileLoader::LoadAtomicFile2Return(name);
}
void * Markers3D_BackendCameraFrame(void *camera)
{
    // Read the original reflection camera frame
    return RwCameraGetFrame((RwCamera *)camera);
}
void * Markers3D_BackendFrameMatrix(void *frame)
{
    // Retain every original matrix null check and subsequent matrix read
    return RwFrameGetMatrix((RwFrame *)frame);
}
float Markers3D_BackendNearClip(void *camera)
{
    // Read the original live near clip plane
    return RwCameraGetNearClipPlane((RwCamera *)camera);
}
float Markers3D_BackendFarClip(void *camera)
{
    // Read the original live far clip plane
    return RwCameraGetFarClipPlane((RwCamera *)camera);
}
Markers3DVector Markers3D_BackendMatrixPosition(void *matrix)
{
    // Copy actual camera matrix pos components
    const RwV3d &v=((RwMatrix *)matrix)->pos;
    Markers3DVector result={v.x,v.y,v.z};return result;
}
Markers3DVector Markers3D_BackendMatrixForward(void *matrix)
{
    // Copy actual camera matrix at components
    const RwV3d &v=((RwMatrix *)matrix)->at;
    Markers3DVector result={v.x,v.y,v.z};return result;
}
C3dMarker * Markers3D_PlaceMarkerLegacy(uint32_t identifier,uint16_t type,CVector &pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Retain the original mutable CVector reference across all placement callbacks
    return (C3dMarker *)Markers3D_PlaceMarkerLegacyInput(identifier,type,&pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate);
}
void Markers3D_PlaceMarkerSetLegacy(uint32_t identifier,uint16_t type,CVector &pos,float size,uint8_t r,uint8_t g,uint8_t b,uint8_t a,uint16_t pulsePeriod,float pulseFraction,int16_t rotateRate)
{
    // Retain the original mutable CVector reference across all placement callbacks
    Markers3D_PlaceMarkerSetLegacyInput(identifier,type,&pos,size,r,g,b,a,pulsePeriod,pulseFraction,rotateRate);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
