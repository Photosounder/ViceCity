//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"

#include "RenderBuffer.h"
#include "Entity.h"
#include "Fluff.h"
#include "Plane.h" // rouz edit (ChatGPT)
#include "Camera.h"
#include "Sprite.h"
#include "Coronas.h"
#include "PointLights.h"
#include "Rubbish.h"
#include "Rubbish_backend.h"
#include "Timecycle.h"
#include "General.h"
#include "Timer.h"
#include "Clock.h"
#include "Weather.h"
#include "Stats.h"
#include "maths.h"
#include "Frontend.h"
#include "CutsceneMgr.h"
#include "PlayerPed.h"
#include "Bones.h"
#include "World.h"
#include "Replay.h"
#include "Coronas.h"
#include "SaveBuf.h"
#include "Pools.h" // rouz edit (ChatGPT)
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
#include "custompipes.h" // rouz edit (ChatGPT)
#endif




//+ rouz edit (ChatGPT)
static bool
IsEffectSphereVisible(const CVector &center, float radius)
{
	// Let the active auxiliary camera clip effect geometry during environment rendering
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(CustomPipes::bRenderingEnvMap)
		return true;
#endif
	return TheCamera.IsSphereVisible(center, radius);
}


//- rouz edit (ChatGPT)









// ---------- CMovingThings ----------
enum eScrollBarTypes
{
	SCROLL_ARENA_STRING
};






 








//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS

#endif
//- rouz edit (ChatGPT)





// ---------- CMovingThing ----------













// ---------- CScrollBar ----------






#ifdef REVC_SOFTWARE_POLYGONS
//+ rouz edit (ChatGPT)

#endif
//- rouz edit (ChatGPT)













//+ rouz edit (ChatGPT)
static CMatrix EscalatorMatrices[NUM_ESCALATORS];
//- rouz edit (ChatGPT)



















































//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "PlaneTrails_backend.h"
PlaneTrailVector PlaneTrails_BackendVector(const void *position)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    const CVector &v=*(const CVector *)position;PlaneTrailVector result={v.x,v.y,v.z};return result;
}
void PlaneTrails_BackendWithPosition(PlaneTrailsPositionCallback callback)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    CVector value;PlaneTrailsPosition fields={&value,&value.x,&value.y,&value.z};callback(fields);
}
bool PlaneTrails_BackendRenderingEnvMap(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
bool PlaneTrails_BackendSphereVisible(PlaneTrailVector position,float radius)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return IsEffectSphereVisible(CVector(position.x,position.y,position.z),radius);
}
void PlaneTrails_BackendSetRGBA(int32_t index,uint8_t alpha)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],255,255,255,alpha);
}
void PlaneTrails_BackendSetPos(int32_t index,float x,float y,float z)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);
}
void PlaneTrails_BackendVertexAlpha(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)TRUE);
}
void PlaneTrails_BackendSrcBlend(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);
}
void PlaneTrails_BackendDestBlend(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);
}
void PlaneTrails_BackendRasterNull(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);
}
bool PlaneTrails_BackendTransform(int32_t count)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return RwIm3DTransform(TempBufferRenderVertices,count,nil,rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)!=nil;
}
void PlaneTrails_BackendRenderIndexed(uint16_t *indices,int32_t count)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPELINELIST,indices,count);
}
void PlaneTrails_BackendEnd(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    RwIm3DEnd();
}
void PlaneTrails_BackendRegisterCorona(uint32_t id,void *position)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    CCoronas::RegisterCorona(id,255,0,0,255,*(CVector *)position,5.0f,2000.0f,CCoronas::TYPE_NORMAL,CCoronas::FLARE_NONE,CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);
}
void PlaneTrails_BackendUpdateCorona(uint32_t id,void *position)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    CCoronas::UpdateCoronaCoors(id,*(CVector *)position,2000.0f,0.0f);
}
float PlaneTrails_BackendFoggyness(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return Weather_Foggyness;
}
float PlaneTrails_BackendCloudCoverage(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return Weather_CloudCoverage;
}
float PlaneTrails_BackendRain(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return Weather_Rain;
}
int32_t PlaneTrails_BackendSkyTopRed(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return TimeCycle_GetSkyTopRed();
}
int32_t PlaneTrails_BackendSkyTopGreen(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return TimeCycle_GetSkyTopGreen();
}
int32_t PlaneTrails_BackendSkyTopBlue(void)
{
    // Preserve the actual typed owner operation and original borrowed-vector lifetime
    return TimeCycle_GetSkyTopBlue();
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "PlaneBanners_backend.h"
PlaneBannerVector PlaneBanners_BackendVector(const void *position)
{
    // Preserve the actual typed owner operation and original native field access
    const CVector &v=*(const CVector *)position;PlaneBannerVector result={v.x,v.y,v.z};return result;
}
PlaneBannerVector PlaneBanners_BackendCameraPosition(void)
{
    // Preserve the actual typed owner operation and original native field access
    const CVector &v=TheCamera.GetPosition();PlaneBannerVector result={v.x,v.y,v.z};return result;
}
bool PlaneBanners_BackendEnvMap(void)
{
    // Preserve the actual typed owner operation and original native field access
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
void * PlaneBanners_BackendEnvCamera(void)
{
    // Preserve the actual typed owner operation and original native field access
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::EnvMapCam;
#else
    return NULL;
#endif
}
void * PlaneBanners_BackendCameraFrame(void *camera)
{
    // Preserve the actual typed owner operation and original native field access
    return RwCameraGetFrame((RwCamera *)camera);
}
void * PlaneBanners_BackendFrameMatrix(void *frame)
{
    // Preserve the actual typed owner operation and original native field access
    return RwFrameGetMatrix((RwFrame *)frame);
}
PlaneBannerVector PlaneBanners_BackendMatrixPosition(void *matrix)
{
    // Preserve the actual typed owner operation and original native field access
    const RwV3d &v=((RwMatrix *)matrix)->pos;PlaneBannerVector result={v.x,v.y,v.z};return result;
}
bool PlaneBanners_BackendSphereVisible(PlaneBannerVector position,float radius)
{
    // Preserve the actual typed owner operation and original native field access
    return TheCamera.IsSphereVisible(CVector(position.x,position.y,position.z),radius);
}
void PlaneBanners_BackendSetRGBA(int32_t index,uint8_t alpha)
{
    // Preserve the actual typed owner operation and original native field access
    RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],255,255,255,alpha);
}
void PlaneBanners_BackendSetPos(int32_t index,float x,float y,float z)
{
    // Preserve the actual typed owner operation and original native field access
    RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);
}
void PlaneBanners_BackendSetU(int32_t index,float u)
{
    // Preserve the actual typed owner operation and original native field access
    RwIm3DVertexSetU(&TempBufferRenderVertices[index],u);
}
void PlaneBanners_BackendSetV(int32_t index,float v)
{
    // Preserve the actual typed owner operation and original native field access
    RwIm3DVertexSetV(&TempBufferRenderVertices[index],v);
}
void PlaneBanners_BackendZWrite(bool enabled)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);
}
void PlaneBanners_BackendZTest(bool enabled)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void *)(uintptr_t)enabled);
}
void PlaneBanners_BackendFog(bool enabled)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)enabled);
}
void PlaneBanners_BackendVertexAlpha(bool enabled)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void PlaneBanners_BackendSrcBlend(void)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);
}
void PlaneBanners_BackendDestBlend(void)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);
}
void PlaneBanners_BackendRaster(void *raster)
{
    // Preserve the actual typed owner operation and original native field access
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);
}
bool PlaneBanners_BackendTransform(int32_t count)
{
    // Preserve the actual typed owner operation and original native field access
    return RwIm3DTransform(TempBufferRenderVertices,count,nil,
#ifdef FIX_BUGS
        rwIM3D_VERTEXXYZ|rwIM3D_VERTEXUV|rwIM3D_VERTEXRGBA
#else
        0
#endif
    )!=nil;
}
void PlaneBanners_BackendRenderIndexed(int32_t count)
{
    // Preserve the actual typed owner operation and original native field access
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPETRILIST,TempBufferRenderIndexList,count);
}
void PlaneBanners_BackendEnd(void)
{
    // Preserve the actual typed owner operation and original native field access
    RwIm3DEnd();
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "SmokeTrails_backend.h"
bool SmokeTrails_BackendEnvMap(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    #if defined(LIBRW) && defined(EXTENDED_PIPELINES)
    return CustomPipes::bRenderingEnvMap;
#else
    return false;
#endif
}
bool SmokeTrails_BackendSphereVisible(SmokeTrailVector position,float radius)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return TheCamera.IsSphereVisible(CVector(position.x,position.y,position.z),radius);
}
bool SmokeTrails_BackendFirstPerson(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return TheCamera.Using1stPersonWeaponMode();
}
void * SmokeTrails_BackendPlayerPed(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return FindPlayerPed();
}
void * SmokeTrails_BackendPlayerVehicle(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return FindPlayerVehicle();
}
bool SmokeTrails_BackendCutscene(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return CCutsceneMgr::IsRunning();
}
void * SmokeTrails_BackendClump(void *ped)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return ((CPlayerPed *)ped)->GetClump();
}
void * SmokeTrails_BackendHierarchy(void *clump)
{
    // Preserve the actual typed owner operation and original native point lifetime
    // Use the C helper and retain the original typed SDK result
    return ((RpHAnimHierarchy*)GetAnimHierarchyFromSkinClump((RpClump *)clump));
}
int32_t SmokeTrails_BackendHeadTag(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return ConvertPedNode2BoneTag(PED_HEAD);
}
int32_t SmokeTrails_BackendBoneIndex(void *hier,int32_t tag)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return RpHAnimIDGetIndex((RpHAnimHierarchy *)hier,tag);
}
void * SmokeTrails_BackendMatrixAt(void *hier,int32_t index)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return &RpHAnimHierarchyGetMatrixArray((RpHAnimHierarchy *)hier)[index];
}
void SmokeTrails_BackendWithPoints(SmokeTrailPointsCallback callback)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwV3d start,end;SmokeTrailPoints points={{&start,&start.x,&start.y,&start.z},{&end,&end.x,&end.y,&end.z}};callback(points);
}
void SmokeTrails_BackendTransformPoint(void *point,void *matrix)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwV3dTransformPoints((RwV3d *)point,(RwV3d *)point,1,(RwMatrix *)matrix);
}
float SmokeTrails_BackendWind(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return Weather_Wind;
}
void SmokeTrails_BackendSetRGBA(int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwIm3DVertexSetRGBA(&TempBufferRenderVertices[index],r,g,b,a);
}
void SmokeTrails_BackendSetPos(int32_t index,float x,float y,float z)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwIm3DVertexSetPos(&TempBufferRenderVertices[index],x,y,z);
}
void SmokeTrails_BackendVertexAlpha(bool enabled)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void SmokeTrails_BackendSrcBlend(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)rwBLENDSRCALPHA);
}
void SmokeTrails_BackendDestBlend(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)rwBLENDINVSRCALPHA);
}
void SmokeTrails_BackendRasterNull(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,nil);
}
bool SmokeTrails_BackendTransform(int32_t count)
{
    // Preserve the actual typed owner operation and original native point lifetime
    return RwIm3DTransform(TempBufferRenderVertices,count,nil,rwIM3D_VERTEXXYZ|rwIM3D_VERTEXRGBA)!=nil;
}
void SmokeTrails_BackendRenderIndexed(uint16_t *indices,int32_t count)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwIm3DRenderIndexedPrimitive(rwPRIMTYPEPOLYLINE,indices,count);
}
void SmokeTrails_BackendEnd(void)
{
    // Preserve the actual typed owner operation and original native point lifetime
    RwIm3DEnd();
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "MovingThings_backend.h"
int32_t MovingThings_BackendSectorsX(void)
{
    // Preserve the original typed owner operation and live native field access
    return NUMSECTORS_X;
}
int32_t MovingThings_BackendSectorsY(void)
{
    // Preserve the original typed owner operation and live native field access
    return NUMSECTORS_Y;
}
void * MovingThings_BackendSectorFirst(int32_t x,int32_t y)
{
    // Preserve the original typed owner operation and live native field access
    return CWorld::GetSector(x,y)->m_lists[ENTITYLIST_BUILDINGS].first;
}
void * MovingThings_BackendBigFirst(int32_t level)
{
    // Preserve the original typed owner operation and live native field access
    return CWorld::GetBigBuildingList((eLevelName)level).first;
}
void * MovingThings_BackendNodeNext(void *node)
{
    // Preserve the original typed owner operation and live native field access
    return ((CPtrNode *)node)->next;
}
CEntity * MovingThings_BackendNodeEntity(void *node)
{
    // Preserve the original typed owner operation and live native field access
    return (CEntity *)((CPtrNode *)node)->item;
}










MovingThingVector MovingThings_BackendPosition(CEntity *entity)
{
    // Preserve the original typed owner operation and live native field access
    const CVector &v=entity->GetPosition();MovingThingVector result={v.x,v.y,v.z};return result;
}
MovingThingVector MovingThings_BackendCameraPosition(void)
{
    // Preserve the original typed owner operation and live native field access
    const CVector &v=TheCamera.GetPosition();MovingThingVector result={v.x,v.y,v.z};return result;
}
float MovingThings_BackendPositionAxis(CEntity *entity,int32_t axis)
{
    // Preserve the original typed owner operation and live native field access
    const CVector &v=entity->GetPosition();return axis==0?v.x:axis==1?v.y:v.z;
}
float MovingThings_BackendCameraAxis(int32_t axis)
{
    // Preserve the original typed owner operation and live native field access
    const CVector &v=TheCamera.GetPosition();return axis==0?v.x:axis==1?v.y:v.z;
}
void MovingThings_BackendSetRight(CEntity *entity,float x,float y,float z)
{
    // Preserve the original typed owner operation and live native field access
    entity->GetRight()=CVector(x,y,z);
}
void MovingThings_BackendSetForward(CEntity *entity,float x,float y,float z)
{
    // Preserve the original typed owner operation and live native field access
    entity->GetForward()=CVector(x,y,z);
}
void MovingThings_BackendSetUp(CEntity *entity,float x,float y,float z)
{
    // Preserve the original typed owner operation and live native field access
    entity->GetUp()=CVector(x,y,z);
}
void MovingThings_BackendSetPosition(CEntity *entity,float x,float y,float z)
{
    // Preserve the original typed owner operation and live native field access
    entity->SetPosition(CVector(x,y,z));
}
void MovingThings_BackendUpdateRW(CEntity *entity)
{
    // Preserve the original typed owner operation and live native field access
    entity->GetMatrix().UpdateRW();
}
void MovingThings_BackendUpdateFrame(CEntity *entity)
{
    // Preserve the original typed owner operation and live native field access
    entity->UpdateRwFrame();
}
int16_t MovingThings_BackendModelIndex(CEntity *entity)
{
    // Preserve the original typed owner operation and live native field access
    return entity->GetModelIndex();
}
void MovingThings_BackendCorona(MovingThingVector position)
{
    // Preserve the original typed owner operation and live native field access
    CVector coors(position.x,position.y,position.z);CCoronas::RegisterCorona(43,128,128,100,255,coors,70.0f,600.0f,0.0f,CCoronas::TYPE_STAR,CCoronas::REFLECTION_OFF,CCoronas::LOSCHECK_OFF,CCoronas::STREAK_OFF,0.0f);
}
void MovingThings_BackendPush(const char *name)
{
    // Preserve the original typed owner operation and live native field access
    PUSH_RENDERGROUP(name);
}
void MovingThings_BackendPop(void)
{
    // Preserve the original typed owner operation and live native field access
    POP_RENDERGROUP();
}
int16_t MovingThings_BackendModelLIGHTBEAM(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_LIGHTBEAM;
}
int16_t MovingThings_BackendModelAIRPORTRADAR(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_AIRPORTRADAR;
}
int16_t MovingThings_BackendModelMALLFAN(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_MALLFAN;
}
int16_t MovingThings_BackendModelHOTELFAN_NIGHT(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_HOTELFAN_NIGHT;
}
int16_t MovingThings_BackendModelHOTELFAN_DAY(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_HOTELFAN_DAY;
}
int16_t MovingThings_BackendModelHOTROOMFAN(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_HOTROOMFAN;
}
int16_t MovingThings_BackendModelBLIMP_NIGHT(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_BLIMP_NIGHT;
}
int16_t MovingThings_BackendModelBLIMP_DAY(void)
{
    // Preserve the original typed owner operation and live native field access
    return MI_BLIMP_DAY;
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "ScrollBar_backend.h"
ScrollBarVector ScrollBar_BackendCameraPosition(void)
{
    // Preserve the actual typed owner operation and original native object lifetime
    const CVector &v=TheCamera.GetPosition();ScrollBarVector result={v.x,v.y,v.z};return result;
}
bool ScrollBar_BackendSphereVisible(ScrollBarVector value,float radius)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return TheCamera.IsSphereVisible(CVector(value.x,value.y,value.z),radius);
}





void * ScrollBar_BackendTexture(void)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return gpCoronaTexture[0];
}
void * ScrollBar_BackendRaster(void *texture)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return RwTextureGetRaster((RwTexture *)texture);
}
void ScrollBar_BackendSetRaster(void *raster)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATETEXTURERASTER,raster);
}
void ScrollBar_BackendZWrite(bool enabled)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATEZWRITEENABLE,(void *)(uintptr_t)enabled);
}
void ScrollBar_BackendZTest(bool enabled)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATEZTESTENABLE,(void *)(uintptr_t)enabled);
}
void ScrollBar_BackendVertexAlpha(bool enabled)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE,(void *)(uintptr_t)enabled);
}
void ScrollBar_BackendSrcBlend(bool additive)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATESRCBLEND,(void *)(additive?rwBLENDONE:rwBLENDSRCALPHA));
}
void ScrollBar_BackendDestBlend(bool additive)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATEDESTBLEND,(void *)(additive?rwBLENDONE:rwBLENDINVSRCALPHA));
}
void ScrollBar_BackendLinear(void)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATETEXTUREFILTER,(void *)rwFILTERLINEAR);
}
void ScrollBar_BackendFog(bool enabled)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATEFOGENABLE,(void *)(uintptr_t)enabled);
}
void ScrollBar_BackendCull(bool none)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwRenderStateSet(rwRENDERSTATECULLMODE,(void *)(none?rwCULLMODECULLNONE:rwCULLMODECULLBACK));
}
void ScrollBar_BackendPush(const char *name)
{
    // Preserve the actual typed owner operation and original native object lifetime
    PUSH_RENDERGROUP(name);
}
void ScrollBar_BackendPop(void)
{
    // Preserve the actual typed owner operation and original native object lifetime
    POP_RENDERGROUP();
}
void * ScrollBar_BackendCameraFrame(void *camera)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return RwCameraGetFrame((RwCamera *)camera);
}
void * ScrollBar_BackendFrameMatrix(void *frame)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return RwFrameGetMatrix((RwFrame *)frame);
}
ScrollBarBasis ScrollBar_BackendBasis(void *matrix)
{
    // Preserve the actual typed owner operation and original native object lifetime
    const RwMatrix &m=*(RwMatrix *)matrix;ScrollBarBasis result={{m.pos.x,m.pos.y,m.pos.z},{m.right.x,m.right.y,m.right.z},{m.up.x,m.up.y,m.up.z},{m.at.x,m.at.y,m.at.z}};return result;
}
float ScrollBar_BackendNearClip(void *camera)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return RwCameraGetNearClipPlane((RwCamera *)camera);
}
float ScrollBar_BackendFarClip(void *camera)
{
    // Preserve the actual typed owner operation and original native object lifetime
    return RwCameraGetFarClipPlane((RwCamera *)camera);
}
void ScrollBar_BackendSetPos(void *vertices,int32_t index,float x,float y,float z)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetPos(&v[index],x,y,z);
}
void ScrollBar_BackendSetRGBA(void *vertices,int32_t index,uint8_t r,uint8_t g,uint8_t b,uint8_t a)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetRGBA(&v[index],r,g,b,a);
}
void ScrollBar_BackendSetU(void *vertices,int32_t index,float u)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetU(&v[index],u);
}
void ScrollBar_BackendSetV(void *vertices,int32_t index,float value)
{
    // Preserve the actual typed owner operation and original native object lifetime
    RwIm3DVertex *v=(RwIm3DVertex *)vertices;RwIm3DVertexSetV(&v[index],value);
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "Escalators_backend.h"
bool Escalators_BackendReplay(void)
{
    // Retain the original typed record operation at its C-selected index
    return CReplay::IsPlayingBack();
}




//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "Escalator_backend.h"
static_assert(ESCALATOR_TEMP_OBJECT==TEMP_OBJECT && ESCALATOR_CONTROLLED_SUB_OBJECT==CONTROLLED_SUB_OBJECT,"Original object creation tags");
float Escalator_BackendStepHeight(void)
{
    // Preserve the actual typed object or matrix operation at its original call point
    return CModelInfo::GetColModel(MI_ESCALATORSTEP)->boundingBox.max.z;
}
EscalatorVector Escalator_BackendCameraPosition(void)
{
    // Preserve the actual typed object or matrix operation at its original call point
    const CVector &v=TheCamera.GetPosition();EscalatorVector result={v.x,v.y,v.z};return result;
}
bool Escalator_BackendSphereVisible(EscalatorVector center,float radius)
{
    // Preserve the actual typed object or matrix operation at its original call point
    return TheCamera.IsSphereVisible(CVector(center.x,center.y,center.z),radius);
}
CPool * Escalator_BackendObjectPool(void)
{
    // Preserve the actual typed object or matrix operation at its original call point
    return CPools::GetObjectPool();
}
int8_t Escalator_BackendCreatedBy(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    return object->ObjectCreatedBy;
}
void Escalator_BackendConstruct(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    std::allocator<CObject>().construct(object,MI_ESCALATORSTEP,TRUE);
}
void Escalator_BackendDestroy(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->~CObject();
}
void Escalator_BackendWorldAdd(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    CWorld::Add(object);
}
void Escalator_BackendWorldRemove(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    CWorld::Remove(object);
}
EscalatorVector Escalator_BackendPosition(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    const CVector &v=object->GetPosition();EscalatorVector result={v.x,v.y,v.z};return result;
}
void Escalator_BackendSetPosition(CObject *object,EscalatorVector value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->SetPosition(CVector(value.x,value.y,value.z));
}
void Escalator_BackendSetCreatedBy(CObject *object,int8_t value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->ObjectCreatedBy=value;
}
void Escalator_BackendAssignMatrix(CObject *object,int32_t index)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->GetMatrix()=EscalatorMatrices[index];
}
void Escalator_BackendSetMoveSpeed(CObject *object,EscalatorVector value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->m_vecMoveSpeed=CVector(value.x,value.y,value.z);
}
void Escalator_BackendUpdateRW(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->GetMatrix().UpdateRW();
}
void Escalator_BackendUpdateFrame(CObject *object)
{
    // Preserve the actual typed object or matrix operation at its original call point
    object->UpdateRwFrame();
}
void Escalator_BackendMatrixUp(int32_t index,EscalatorVector value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    EscalatorMatrices[index].GetUp()=CVector(value.x,value.y,value.z);
}
void Escalator_BackendMatrixForward(int32_t index,EscalatorVector value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    EscalatorMatrices[index].GetForward()=CVector(value.x,value.y,value.z);
}
void Escalator_BackendMatrixRight(int32_t index,EscalatorVector value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    EscalatorMatrices[index].GetRight()=CVector(value.x,value.y,value.z);
}
void Escalator_BackendMatrixPosition(int32_t index,EscalatorVector value)
{
    // Preserve the actual typed object or matrix operation at its original call point
    EscalatorMatrices[index].GetPosition()=CVector(value.x,value.y,value.z);
}
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
#include "ScriptPaths_backend.h"
static_assert(sizeof(CPlaneNode)==SCRIPT_PATH_NODE_SIZE,"Original native node serialization stride");
float ScriptPaths_BackendNodeT(ScriptPath *path,int32_t index)
{
    // Preserve the actual typed native operation and original borrowed field access
    return path->m_pNode[index].t;
}
float ScriptPaths_BackendNodeAxis(ScriptPath *path,int32_t index,int32_t axis)
{
    // Preserve the actual typed native operation and original borrowed field access
    const CVector &v=path->m_pNode[index].p;return axis==0?v.x:axis==1?v.y:v.z;
}
void ScriptPaths_BackendFreeNodes(CPlaneNode *nodes)
{
    // Preserve the actual typed native operation and original borrowed field access
    free(nodes);
}
CPlaneNode * ScriptPaths_BackendLoadNodes(const char *file,int32_t *count,float *length)
{
    // Preserve the actual typed native operation and original borrowed field access
    return CPlane::LoadPath(file,*count,*length,false);
}
CPlaneNode * ScriptPaths_BackendAllocateNodes(int32_t count)
{
    // Preserve the actual typed native operation and original borrowed field access
    return (CPlaneNode *)malloc(sizeof(CPlaneNode)*count);
}
void ScriptPaths_BackendReadNode(CPlaneNode *nodes,int32_t index,const void *buffer)
{
    // Preserve the actual typed native operation and original borrowed field access
    nodes[index]=*(const CPlaneNode *)buffer;
}
void ScriptPaths_BackendWriteNode(void *buffer,CPlaneNode *nodes,int32_t index)
{
    // Preserve the actual typed native operation and original borrowed field access
    *(CPlaneNode *)buffer=nodes[index];
}
CPool * ScriptPaths_BackendObjectPool(void)
{
    // Preserve the actual typed native operation and original borrowed field access
    return CPools::GetObjectPool();
}
void ScriptPaths_BackendClearPhysics(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->m_phy_flagA08=false;
}
void ScriptPaths_BackendRegister(CObject *object,CObject **slot)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->RegisterReference((CEntity **)slot);
}
ScriptPathVector ScriptPaths_BackendPosition(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    const CVector &v=object->GetPosition();ScriptPathVector result={v.x,v.y,v.z};return result;
}
ScriptPathVector ScriptPaths_BackendForward(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    const CVector &v=object->GetForward();ScriptPathVector result={v.x,v.y,v.z};return result;
}
void ScriptPaths_BackendSetPosition(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->SetPosition(CVector(value.x,value.y,value.z));
}
void ScriptPaths_BackendUpdateRW(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->GetMatrix().UpdateRW();
}
void ScriptPaths_BackendUpdateFrame(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->UpdateRwFrame();
}
bool ScriptPaths_BackendBigBuilding(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    return object->bIsBIGBuilding;
}
void ScriptPaths_BackendRemoveAndAdd(CObject *object)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->RemoveAndAdd();
}
ScriptPathVector ScriptPaths_BackendPreviousPosition(void *matrix)
{
    // Preserve the actual typed native operation and original borrowed field access
    const CVector &v=((CMatrix *)matrix)->GetPosition();ScriptPathVector result={v.x,v.y,v.z};return result;
}
ScriptPathVector ScriptPaths_BackendPreviousForward(void *matrix)
{
    // Preserve the actual typed native operation and original borrowed field access
    const CVector &v=((CMatrix *)matrix)->GetForward();ScriptPathVector result={v.x,v.y,v.z};return result;
}
void ScriptPaths_BackendSetRight(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->GetRight()=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendSetUp(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->GetUp()=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendSetForward(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->GetForward()=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendSetMoveSpeed(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->m_vecMoveSpeed=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendSetTurnSpeed(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->m_vecTurnSpeed=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendSetMoveFriction(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->m_vecMoveFriction=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendSetTurnFriction(CObject *object,ScriptPathVector value)
{
    // Preserve the actual typed native operation and original borrowed field access
    object->m_vecTurnFriction=CVector(value.x,value.y,value.z);
}
void ScriptPaths_BackendWithPrevious(ScriptPath *path,int32_t index,ScriptPathGeometry geometry,ScriptPathObjectCallback callback)
{
    // Keep the original native matrix copy alive through the complete per-object update sequence
    CMatrix previous(path->m_pObjects[index]->GetMatrix());callback(path,index,geometry,&previous);
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
