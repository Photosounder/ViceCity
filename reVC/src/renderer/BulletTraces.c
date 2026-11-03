//+ rouz edit (ChatGPT)
#include "RwHelper.h"
#include "BulletTraces_backend.h"
#include "Timer.h"
#include "DMAudio.h"
#include "WeaponType.h"
#include "CameraModes.h"
#include <math.h>
#include <stddef.h>

#if defined(_MSC_VER)
float fast_absf(float value);
#define Abs fast_absf
#else
#define Abs fabsf
#endif
#define Sqrt sqrtf
#define SQR(a) ((a)*(a))
typedef BulletTraceVector (*BulletTracesRead)(const void *);
typedef const void *(*BulletTracesCamera)(void);
static BulletTraceVector BulletTraces_Read(const void *value)
{
    // Read actual C vector storage at the original live input use
    return *(const BulletTraceVector *)value;
}
static BulletTraceVector BulletTraces_AddVector(BulletTraceVector a,BulletTraceVector b)
{
    // Retain the original component addition order
    BulletTraceVector result={a.x+b.x,a.y+b.y,a.z+b.z};return result;
}
static BulletTraceVector BulletTraces_Subtract(BulletTraceVector a,BulletTraceVector b)
{
    // Retain the original component subtraction order
    BulletTraceVector result={a.x-b.x,a.y-b.y,a.z-b.z};return result;
}
static BulletTraceVector BulletTraces_LeftScale(float a,BulletTraceVector b)
{
    // Retain original scalar-times-vector operand order
    BulletTraceVector result={a*b.x,a*b.y,a*b.z};return result;
}
static BulletTraceVector BulletTraces_Divide(BulletTraceVector a,float b)
{
    // Retain the original three separate divisions
    BulletTraceVector result={a.x/b,a.y/b,a.z/b};return result;
}
static float BulletTraces_Dot(BulletTraceVector a,BulletTraceVector b)
{
    // Retain the original dot products and additions
    return a.x*b.x+a.y*b.y+a.z*b.z;
}
static float BulletTraces_Magnitude(BulletTraceVector a)
{
    // Retain the original float square-root wrapper
    return Sqrt(a.x*a.x+a.y*a.y+a.z*a.z);
}
static void BulletTraces_Normalise(BulletTraceVector *v)
{
    // Retain the original float reciprocal root and x-only fallback
    float square=v->x*v->x+v->y*v->y+v->z*v->z;
    if(square>0.0f){
        // Scale each original component in order
        float scale=1.0f/Sqrt(square);v->x*=scale;v->y*=scale;v->z*=scale;
    }else v->x=1.0f;
}
static float BulletTraces_Project(const void *input,BulletTracesRead read,BulletTracesCamera axis)
{
    // Preserve borrowed camera references and the compiler-selected original argument order
#if defined(__clang__)
    const void *basis=axis();
#endif
    const void *position=BulletTraces_BackendCameraPosition();
    BulletTraceVector delta=BulletTraces_Subtract(read(input),BulletTraces_BackendReadVector(position));
#if !defined(__clang__)
    const void *basis=axis();
#endif
    return BulletTraces_Dot(BulletTraces_BackendReadVector(basis),delta);
}
static uint16_t TraceIndexList[48] = {0, 5, 7, 0, 7, 2, 0, 7, 5, 0, 2, 7, 0, 4, 9, 0,
										9, 5, 0, 9, 4, 0, 5, 9, 0, 1, 6, 0, 6, 5, 0, 6,
										1, 0, 5, 6, 0, 3, 8, 0, 8, 5, 0, 8, 3, 0, 5, 8 };

static void *gpSmokeTrailTexture;
void BulletTraces_InitTexture(void)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	BulletTraces_BackendPushTxd();
	BulletTraces_BackendSetTxd(BulletTraces_BackendFindTxd("particle"));
	if(gpSmokeTrailTexture == NULL)
		gpSmokeTrailTexture = RwHelper_ReadTexture("smoketrail", NULL);
	BulletTraces_BackendPopTxd();
}
void BulletTraces_ShutdownTexture(void)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	if (gpSmokeTrailTexture) {
		// Retain original eligibility, arithmetic and write order
		RwHelper_DestroyTexture(gpSmokeTrailTexture);
		gpSmokeTrailTexture = NULL;
	}
}

void BulletTraces_InitGeometry(void)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	BulletTraces_BackendSetU(0, 0.0);
	BulletTraces_BackendSetV(0, 0.0);
	BulletTraces_BackendSetU(1, 1.0);
	BulletTraces_BackendSetV(1, 0.0);
	BulletTraces_BackendSetU(2, 1.0);
	BulletTraces_BackendSetV(2, 0.0);
	BulletTraces_BackendSetU(3, 1.0);
	BulletTraces_BackendSetV(3, 0.0);
	BulletTraces_BackendSetU(4, 1.0);
	BulletTraces_BackendSetV(4, 0.0);
	BulletTraces_BackendSetU(5, 0.0);
	BulletTraces_BackendSetU(6, 1.0);
	BulletTraces_BackendSetU(7, 1.0);
	BulletTraces_BackendSetU(8, 1.0);
	BulletTraces_BackendSetU(9, 1.0);
}

BulletTrace BulletTraces_aTraces[NUMBULLETTRACES];

void BulletTraces_Init(void)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	for (int i = 0; i < NUMBULLETTRACES; i++)
		BulletTraces_aTraces[i].m_bInUse = false;
}

static void BulletTraces_AddInput(const void *start,const void *end,float thickness,uint32_t lifeTime,uint8_t visibility,BulletTracesRead read)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	int32_t enabledCount;
	uint32_t modifiedLifeTime;
	int32_t nextSlot;

	enabledCount = 0;
	for (int i = 0; i < NUMBULLETTRACES; i++)
		if (BulletTraces_aTraces[i].m_bInUse)
			enabledCount++;
	if (enabledCount >= 10)
		modifiedLifeTime = lifeTime / 4;
	else if (enabledCount >= 5)
		modifiedLifeTime = lifeTime / 2;
	else
		modifiedLifeTime = lifeTime;

	nextSlot = 0;
	for (int i = 0; nextSlot < NUMBULLETTRACES && BulletTraces_aTraces[i].m_bInUse; i++)
		nextSlot++;
	if (nextSlot < 16) {
		// Retain original eligibility, arithmetic and write order
		BulletTraces_aTraces[nextSlot].m_vecStartPos = read(start);
		BulletTraces_aTraces[nextSlot].m_vecEndPos = read(end);
		BulletTraces_aTraces[nextSlot].m_bInUse = true;
		BulletTraces_aTraces[nextSlot].m_nCreationTime = (*Timer_GetTimeInMilliseconds());
		BulletTraces_aTraces[nextSlot].m_fVisibility = visibility;
		BulletTraces_aTraces[nextSlot].m_fThickness = thickness;
		BulletTraces_aTraces[nextSlot].m_nLifeTime = modifiedLifeTime;
	}

	float startProjFwd = BulletTraces_Project(start,read,BulletTraces_BackendCameraForward);
	float endProjFwd = BulletTraces_Project(end,read,BulletTraces_BackendCameraForward);
	if (startProjFwd * endProjFwd < 0.0f) { //if one of point behind us and second before us
		// Retain original eligibility, arithmetic and write order
		float fStartDistFwd = Abs(startProjFwd) / (Abs(startProjFwd) + Abs(endProjFwd));

		float startProjUp = BulletTraces_Project(start,read,BulletTraces_BackendCameraUp);
		float endProjUp = BulletTraces_Project(end,read,BulletTraces_BackendCameraUp);
		float distUp = (endProjUp - startProjUp) * fStartDistFwd + startProjUp;

		float startProjRight = BulletTraces_Project(start,read,BulletTraces_BackendCameraRight);
		float endProjRight = BulletTraces_Project(end,read,BulletTraces_BackendCameraRight);
		float distRight = (endProjRight - startProjRight) * fStartDistFwd + startProjRight;

		float dist = Sqrt(SQR(distUp) + SQR(distRight));
		if (dist < 2.0f) {
			// Retain original eligibility, arithmetic and write order
			if(distRight < 0.0f)
				DMAudio_PlayFrontEndSound(SOUND_BULLETTRACE_2, 127 * (1.0f - dist * 0.5f));
			else
				DMAudio_PlayFrontEndSound(SOUND_BULLETTRACE_1, 127 * (1.0f - dist * 0.5f));
		}
	}
}

static void BulletTraces_AddWeaponInput(const void *start,const void *end,int32_t weaponType,void *shooter,BulletTracesRead read)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	void *player;
	float speed;
	int16_t camMode;
	
	if (shooter == BulletTraces_BackendPlayerPedEntity() || (BulletTraces_BackendPlayerVehicle() != NULL && BulletTraces_BackendPlayerVehicle() == BulletTraces_BackendAsVehicle(shooter))) {
		// Retain original eligibility, arithmetic and write order
		camMode = BulletTraces_BackendCameraMode();
		if (camMode == CAMERA_MODE_M16_1STPERSON
			|| camMode == CAMERA_MODE_CAMERA
			|| camMode == CAMERA_MODE_SNIPER
			|| camMode == CAMERA_MODE_M16_1STPERSON_RUNABOUT
			|| camMode == CAMERA_MODE_ROCKETLAUNCHER
			|| camMode == CAMERA_MODE_ROCKETLAUNCHER_RUNABOUT
			|| camMode == CAMERA_MODE_SNIPER_RUNABOUT
			|| camMode == CAMERA_MODE_HELICANNON_1STPERSON) {
				// Retain original eligibility, arithmetic and write order

			player = BulletTraces_BackendPlayerVehicle() ? BulletTraces_BackendPlayerVehiclePhysical() : BulletTraces_BackendPlayerPedPhysical();
			speed = BulletTraces_Magnitude(BulletTraces_BackendMoveSpeed(player));
			if (speed < 0.05f)
				return;
		}
	}

	switch (weaponType) {
		// Retain original eligibility, arithmetic and write order
	case WEAPONTYPE_PYTHON:
	case WEAPONTYPE_SHOTGUN:
	case WEAPONTYPE_SPAS12_SHOTGUN:
	case WEAPONTYPE_STUBBY_SHOTGUN:
		BulletTraces_AddInput(start,end,0.7f, 1000, 200,read);
		break;
	case WEAPONTYPE_M4:
	case WEAPONTYPE_RUGER:
	case WEAPONTYPE_SNIPERRIFLE:
	case WEAPONTYPE_LASERSCOPE:
	case WEAPONTYPE_M60:
	case WEAPONTYPE_MINIGUN:
	case WEAPONTYPE_HELICANNON:
		BulletTraces_AddInput(start,end,1.0f, 2000, 220,read);
		break;
	default:
		BulletTraces_AddInput(start,end,0.4f, 750, 150,read);
		break;
	}
}

void BulletTraces_Render(void)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	// Clip reflected traces against the active environment camera instead of the player camera
		BulletTraceVector traceCameraPosition = BulletTraces_BackendReadVector(BulletTraces_BackendCameraPosition());
	BulletTraceVector traceCameraForward = BulletTraces_BackendReadVector(BulletTraces_BackendCameraForward());
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(BulletTraces_BackendEnvMap() && BulletTraces_BackendEnvCamera() != NULL){
		// Retain original eligibility, arithmetic and write order
		void *effectCamera = BulletTraces_BackendEnvCamera();
		void *effectFrame = BulletTraces_BackendCameraFrame(effectCamera);
		if(effectFrame && BulletTraces_BackendFrameMatrix(effectFrame)){
			// Retain original eligibility, arithmetic and write order
			void *effectMatrix = BulletTraces_BackendFrameMatrix(effectFrame);
			traceCameraPosition = BulletTraces_BackendMatrixPosition(effectMatrix);
			traceCameraForward = BulletTraces_BackendMatrixForward(effectMatrix);
			BulletTraces_Normalise(&traceCameraForward);
		}
	}
#endif
		for (int i = 0; i < NUMBULLETTRACES; i++) {
			// Retain original eligibility, arithmetic and write order
		if (!BulletTraces_aTraces[i].m_bInUse)
			continue;
		BulletTraces_BackendZWrite(false);
		BulletTraces_BackendSrcBlend();
		BulletTraces_BackendDestBlend();
		RwHelper_SetTextureRaster(RwHelper_TextureRaster(gpSmokeTrailTexture));

		float timeAlive = (*Timer_GetTimeInMilliseconds()) - BulletTraces_aTraces[i].m_nCreationTime;

		float traceThickness = BulletTraces_aTraces[i].m_fThickness * timeAlive / BulletTraces_aTraces[i].m_nLifeTime;
		BulletTraceVector horizontalOffset = BulletTraces_Subtract(BulletTraces_aTraces[i].m_vecEndPos,BulletTraces_aTraces[i].m_vecStartPos);
		BulletTraces_Normalise(&horizontalOffset);
		horizontalOffset.x *= traceThickness;horizontalOffset.y *= traceThickness;horizontalOffset.z *= traceThickness;

		//then closer trace to die then it more transparent
		uint8_t nAlphaValue = BulletTraces_aTraces[i].m_fVisibility * (BulletTraces_aTraces[i].m_nLifeTime - timeAlive) / BulletTraces_aTraces[i].m_nLifeTime;

		BulletTraceVector start = BulletTraces_aTraces[i].m_vecStartPos;
		BulletTraceVector end = BulletTraces_aTraces[i].m_vecEndPos;
		// Use camera-relative depth for clipping the trace endpoints
				float startProj = BulletTraces_Dot(BulletTraces_Subtract(start,traceCameraPosition),traceCameraForward) - 0.7f;
		float endProj = BulletTraces_Dot(BulletTraces_Subtract(end,traceCameraPosition),traceCameraForward) - 0.7f;
				if (startProj < 0.0f && endProj < 0.0f) //we dont need render trace behind us
			continue;

		if (startProj < 0.0f) { //if strat behind us move it closer
			// Retain original eligibility, arithmetic and write order
			float absStartProj = Abs(startProj);
			float absEndProj = Abs(endProj);
			start = BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(absEndProj,start),BulletTraces_LeftScale(absStartProj,end)),(absStartProj + absEndProj));
		} else if (endProj < 0.0f) {
			// Retain original eligibility, arithmetic and write order
			float absStartProj = Abs(startProj);
			float absEndProj = Abs(endProj);
			end = BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(absEndProj,start),BulletTraces_LeftScale(absStartProj,end)),(absStartProj + absEndProj));
		}

		//we divide trace at three parts
		BulletTraceVector start2 = BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(7.0f,start),end),8);
		BulletTraceVector end2 = BulletTraces_Divide(BulletTraces_AddVector(BulletTraces_LeftScale(7.0f,end),start),8);

		BulletTraces_BackendSetV(5, 10.0f);
		BulletTraces_BackendSetV(6, 10.0f);
		BulletTraces_BackendSetV(7, 10.0f);
		BulletTraces_BackendSetV(8, 10.0f);
		BulletTraces_BackendSetV(9, 10.0f);

		BulletTraces_BackendSetRGBA(0, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(1, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(2, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(3, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(4, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(5, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(6, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(7, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(8, 255, 255, 255, nAlphaValue);
		BulletTraces_BackendSetRGBA(9, 255, 255, 255, nAlphaValue);
		//two points in center
		BulletTraces_BackendSetPos(0, start2.x, start2.y, start2.z);
		BulletTraces_BackendSetPos(5, end2.x, end2.y, end2.z);
		//vertical planes
		BulletTraces_BackendSetPos(1, start2.x, start2.y, start2.z + traceThickness);
		BulletTraces_BackendSetPos(3, start2.x, start2.y, start2.z - traceThickness);
		BulletTraces_BackendSetPos(6, end2.x, end2.y, end2.z + traceThickness);
		BulletTraces_BackendSetPos(8, end2.x, end2.y, end2.z - traceThickness);
		//horizontal planes
		BulletTraces_BackendSetPos(2, start2.x + horizontalOffset.y, start2.y - horizontalOffset.x, start2.z);
		BulletTraces_BackendSetPos(7, end2.x + horizontalOffset.y, end2.y - horizontalOffset.x, end2.z);
#ifdef FIX_BUGS //this point calculated wrong for some reason
		BulletTraces_BackendSetPos(4, start2.x - horizontalOffset.y, start2.y + horizontalOffset.x, start2.z);
		BulletTraces_BackendSetPos(9, end2.x - horizontalOffset.y, end2.y + horizontalOffset.x, end2.z);
#else
		BulletTraces_BackendSetPos(4, start2.x - horizontalOffset.y, start2.y - horizontalOffset.y, start2.z);
		BulletTraces_BackendSetPos(9, end2.x - horizontalOffset.y, end2.y - horizontalOffset.y, end2.z);
#endif

		if (BulletTraces_BackendTransform(false)) {
			// Retain original eligibility, arithmetic and write order
			BulletTraces_BackendRenderIndexed(TraceIndexList);
			BulletTraces_BackendEnd();
		}

		BulletTraces_BackendSetV(5, 2.0f);
		BulletTraces_BackendSetV(6, 2.0f);
		BulletTraces_BackendSetV(7, 2.0f);
		BulletTraces_BackendSetV(8, 2.0f);
		BulletTraces_BackendSetV(9, 2.0f);
		BulletTraces_BackendSetRGBA(0, 255, 255, 255, 0);
		BulletTraces_BackendSetRGBA(1, 255, 255, 255, 0);
		BulletTraces_BackendSetRGBA(2, 255, 255, 255, 0);
		BulletTraces_BackendSetRGBA(3, 255, 255, 255, 0);
		BulletTraces_BackendSetRGBA(4, 255, 255, 255, 0);

		BulletTraces_BackendSetPos(0, start.x, start.y, start.z);
		BulletTraces_BackendSetPos(1, start.x, start.y, start.z + traceThickness);
		BulletTraces_BackendSetPos(3, start.x, start.y, start.z - traceThickness);
		BulletTraces_BackendSetPos(2, start.x + horizontalOffset.y, start.y - horizontalOffset.x, start.z);

		BulletTraces_BackendSetPos(5, start2.x, start2.y, start2.z);
		BulletTraces_BackendSetPos(6, start2.x, start2.y, start2.z + traceThickness);
		BulletTraces_BackendSetPos(8, start2.x, start2.y, start2.z - traceThickness);
		BulletTraces_BackendSetPos(7, start2.x + horizontalOffset.y, start2.y - horizontalOffset.x, start2.z);
#ifdef FIX_BUGS
		BulletTraces_BackendSetPos(4, start.x - horizontalOffset.y, start.y + horizontalOffset.x, start.z);
		BulletTraces_BackendSetPos(9, start2.x - horizontalOffset.y, start2.y + horizontalOffset.x, start2.z);
#else
		BulletTraces_BackendSetPos(4, start.x - horizontalOffset.y, start.y - horizontalOffset.y, start.z);
		BulletTraces_BackendSetPos(9, start2.x - horizontalOffset.y, start2.y - horizontalOffset.y, start2.z);
#endif

		if (BulletTraces_BackendTransform(true)) {
			// Retain original eligibility, arithmetic and write order
			BulletTraces_BackendRenderIndexed(TraceIndexList);
			BulletTraces_BackendEnd();
		}

		BulletTraces_BackendSetPos(1, end.x, end.y, end.z);
		BulletTraces_BackendSetPos(2, end.x, end.y, end.z + traceThickness);
		BulletTraces_BackendSetPos(4, end.x, end.y, end.z - traceThickness);
		BulletTraces_BackendSetPos(3, end.x + horizontalOffset.y, end.y - horizontalOffset.x, end.z);

		BulletTraces_BackendSetPos(5, end2.x, end2.y, end2.z);
		BulletTraces_BackendSetPos(6, end2.x, end2.y, end2.z + traceThickness);
		BulletTraces_BackendSetPos(8, end2.x, end2.y, end2.z - traceThickness);
		BulletTraces_BackendSetPos(7, end2.x + horizontalOffset.y, end2.y - horizontalOffset.x, end2.z);
#ifdef FIX_BUGS
		BulletTraces_BackendSetPos(5, end.x - horizontalOffset.y, end.y + horizontalOffset.x, end.z);
		BulletTraces_BackendSetPos(9, end2.x - horizontalOffset.y, end2.y + horizontalOffset.x, end2.z);
#else
		BulletTraces_BackendSetPos(5, end.x - horizontalOffset.y, end.y - horizontalOffset.y, end.z);
		BulletTraces_BackendSetPos(9, end2.x - horizontalOffset.y, end2.y - horizontalOffset.y, end2.z);
#endif

		if (BulletTraces_BackendTransform(true)) {
			// Retain original eligibility, arithmetic and write order
			BulletTraces_BackendRenderIndexed(TraceIndexList);
			BulletTraces_BackendEnd();
		}
	}
	BulletTraces_BackendZWrite(true);
	BulletTraces_BackendSrcBlend();
	BulletTraces_BackendDestBlend();
}

void BulletTraces_Update(void)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	for (int i = 0; i < NUMBULLETTRACES; i++) {
		// Retain original eligibility, arithmetic and write order
		if (BulletTraces_aTraces[i].m_bInUse)
			BulletTrace_Update(&BulletTraces_aTraces[i]);
	}
}

void BulletTrace_Update(BulletTrace *trace)
{
	// Preserve the complete original trace, clipping and shared-geometry operation order
	if ((*Timer_GetTimeInMilliseconds()) - trace->m_nCreationTime >= trace->m_nLifeTime)
		trace->m_bInUse = false;
}

void BulletTraces_Add(const BulletTraceVector *start,const BulletTraceVector *end,float thickness,uint32_t lifeTime,uint8_t visibility)
{
    // Preserve all original lazy input reads and overload-specific eligibility
    BulletTraces_AddInput(start,end,thickness,lifeTime,visibility,BulletTraces_Read);
}
void BulletTraces_AddLegacy(const void *start,const void *end,float thickness,uint32_t lifeTime,uint8_t visibility)
{
    // Preserve all original lazy input reads and overload-specific eligibility
    BulletTraces_AddInput(start,end,thickness,lifeTime,visibility,BulletTraces_BackendReadVector);
}
void BulletTraces_AddWeapon(const BulletTraceVector *start,const BulletTraceVector *end,int32_t weaponType,void *shooter)
{
    // Preserve all original lazy input reads and overload-specific eligibility
    BulletTraces_AddWeaponInput(start,end,weaponType,shooter,BulletTraces_Read);
}
void BulletTraces_AddWeaponLegacy(const void *start,const void *end,int32_t weaponType,void *shooter)
{
    // Preserve all original lazy input reads and overload-specific eligibility
    BulletTraces_AddWeaponInput(start,end,weaponType,shooter,BulletTraces_BackendReadVector);
}
//- rouz edit (ChatGPT)
