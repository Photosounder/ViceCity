//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "main.h"

#include "General.h"
#include "CutsceneMgr.h"
#include "ModelIndices.h"
#include "FileMgr.h"
#include "Streaming.h"
#include "Replay.h"
#include "Camera.h"
#include "DMAudio.h"
#include "Wanted.h"
#include "Coronas.h"
#include "Particle.h"
#include "Explosion.h"
#include "Fluff.h"
#include "World.h"
#include "HandlingMgr.h"
#include "Heli.h"
#include "Plane.h"
#include "PlaneMath.h"
#include "MemoryHeap.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "VehicleLogic.h"

CPlaneNode *pPathNodes;
CPlaneNode *pPath2Nodes;
CPlaneNode *pPath3Nodes;
CPlaneNode *pPath4Nodes;
int32 NumPathNodes;
int32 NumPath2Nodes;
int32 NumPath3Nodes;
int32 NumPath4Nodes;
float TotalLengthOfFlightPath;
float TotalLengthOfFlightPath2;
float TotalLengthOfFlightPath3;
float TotalLengthOfFlightPath4;
float TotalDurationOfFlightPath;
float TotalDurationOfFlightPath2;
float TotalDurationOfFlightPath3;
float TotalDurationOfFlightPath4;
float LandingPoint;
float TakeOffPoint;
CPlaneInterpolationLine aPlaneLineBits[6];

float PlanePathPosition[3];
float OldPlanePathPosition[3];
float PlanePathSpeed[3];
float PlanePath2Position[5];
float PlanePath3Position[4];
float PlanePath2Speed[5];
float PlanePath3Speed[4];


enum
{
	CESNA_STATUS_NONE,	// doesn't even exist
	CESNA_STATUS_FLYING,
	CESNA_STATUS_DESTROYED,
	CESNA_STATUS_LANDED,
};

static CVector
InterpolatePlanePathPosition(CPlaneNode *nodes, int32 numNodes, float totalLength, bool loop, int32 node, int32 next, float f)
{
	// Delegate to the C implementation
	float out[3];
	PlaneMath_InterpolatePathPosition(nodes, numNodes, totalLength, loop, node, next, f, out);
	return CVector(out[0], out[1], out[2]);
}

int32 CesnaMissionStatus;
int32 CesnaMissionStartTime;
CPlane *pDrugRunCesna;
int32 DropOffCesnaMissionStatus;
int32 DropOffCesnaMissionStartTime;
CPlane *pDropOffCesna;

CPlane::CPlane(int32 id, uint8 CreatedBy)
 : CVehicle(CreatedBy)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(id)); // rouz edit (ChatGPT)
	m_vehType = VEHICLE_TYPE_PLANE;
	pHandling = mod_HandlingManager.GetHandlingData((tVehicleType)mi->m_handlingId);
	SetModelIndex(id);

	m_fMass = 100000000.0f;
	m_fTurnMass = 100000000.0f;
	m_fAirResistance = 0.9994f;
	m_fElasticity = 0.05f;

		bUsesCollision = true;
	bInfiniteMass = true;
	bDrawFarAway = true;
		m_bHasBeenHit = false;
	m_bIsDrugRunCesna = false;
	m_bIsDropOffCesna = false;
	m_bTempPlane = false;
	m_bHasCollisionVelocity = false; // rouz edit (ChatGPT)

	SetStatus(STATUS_PLANE);
	m_level = LEVEL_GENERIC;

	m_isFarAway = false;
#ifdef CPLANE_ROTORS
	m_fRotorRotation = 0.0f;
#endif
}

CPlane::~CPlane()
{
	DeleteRwObject();
}

void
CPlane::SetModelIndex(uint32 id)
{
	CVehicle::SetModelIndex(id);
#ifdef CPLANE_ROTORS
	int i;
	for(i = 0; i < NUM_PLANE_NODES; i++)
		m_aPlaneNodes[i] = nil;
	if(GetModelIndex() == MI_CHOPPER){
		// This is surprisingly annoying...
		RwFrame *heliNodes[NUM_HELI_NODES];
		for(i = 0; i < NUM_HELI_NODES; i++)
			heliNodes[i] = nil;
		CClumpModelInfo::FillFrameArray(GetClump(), heliNodes);
		m_aPlaneNodes[PLANE_TOPROTOR] = heliNodes[HELI_TOPROTOR];
		m_aPlaneNodes[PLANE_BACKROTOR] = heliNodes[HELI_BACKROTOR];
	}else
		CClumpModelInfo::FillFrameArray(GetClump(), m_aPlaneNodes);
#endif
}

void
CPlane::DeleteRwObject(void)
{
	if(m_rwObject && RwObjectGetType(m_rwObject) == rpATOMIC){
		GetMatrix().Detach();
		if(RwObjectGetType(m_rwObject) == rpATOMIC){	// useless check
			RwFrame *f = RpAtomicGetFrame((RpAtomic*)m_rwObject);
			RpAtomicDestroy((RpAtomic*)m_rwObject);
			RwFrameDestroy(f);
		}
		m_rwObject = nil;
	}
	CEntity::DeleteRwObject();
}

// There's a LOT of copy and paste in here. Maybe this could be refactored somehow
void
CPlane::ProcessControl(void)
{
	int i;
	CVector pos;

	if(Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;

		bHasContacted = false;
	bIsInSafePosition = false;
	bWasPostponed = false;
	bHasHitWall = false;
	m_nCollisionRecords = 0;
	bHasCollided = false;
	m_nDamagePieceType = 0;
	m_fDamageImpulse = 0.0f;
	m_pDamageEntity = nil;
	
		CMatrix previousCollisionMatrix(GetMatrix());
	bool hadCollisionVelocity = m_bHasCollisionVelocity;
	bool updatedCollisionMatrix = false;
	
	if(GetModelIndex() == MI_AIRTRAIN){
		if(GetPosition().z > 100.0f)
			// Enter the C plane-trail algorithm with the original input capture
			PlaneTrails_RegisterPointVector(&GetPosition(), m_nPlaneId);
	}else if(GetModelIndex() == MI_DEADDODO)
		// Enter the C plane-banner algorithm with the original input capture
		PlaneBanners_RegisterPointVector(&GetPosition(), m_nPlaneId);

	// Explosion
	if(m_bHasBeenHit){
		// BUG: since this is all based on frames, you can skip the explosion processing when you go into the menu
		if(GetModelIndex() == MI_AIRTRAIN){
			int frm = (*Timer_GetFrameCounter()) - m_nFrameWhenHit;
			if(frm == 20){
				static int nFrameGen;
				CRGBA colors[8];

				Explosion_AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), 0);

				colors[0] = CRGBA(0, 0, 0, 255);
				colors[1] = CRGBA(224, 230, 238, 255);
				colors[2] = CRGBA(224, 230, 238, 255);
				colors[3] = CRGBA(0, 0, 0, 255);
				colors[4] = CRGBA(224, 230, 238, 255);
				colors[5] = CRGBA(0, 0, 0, 255);
				colors[6] = CRGBA(0, 0, 0, 255);
				colors[7] = CRGBA(224, 230, 238, 255);

				CVector dir;
				for(i = 0; i < 40; i++){
					dir.x = General_GetRandomNumberInRangeFloat(-2.0f, 2.0f);
					dir.y = General_GetRandomNumberInRangeFloat(-2.0f, 2.0f);
					dir.z = General_GetRandomNumberInRangeFloat(0.0f, 2.0f);
					int rotSpeed = General_GetRandomNumberInRangeInt(10, 30);
					if(General_GetRandomNumber() & 1)
						rotSpeed = -rotSpeed;
					int f = ++nFrameGen & 3;
					CParticle::AddParticle(PARTICLE_HELI_DEBRIS, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), dir,
						nil, General_GetRandomNumberInRangeFloat(0.1f, 1.0f),
						colors[nFrameGen&7], rotSpeed, 0, f, 0);
				}
			}
			if(frm >= 40 && frm <= 80 && frm & 1){
				if(frm & 1){
					pos.x = ((General_GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.z = ((General_GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.y = frm - 40;
					pos = GetMatrix() * pos;
				}else{
					pos.x = ((General_GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.z = ((General_GetRandomNumber() & 0x3F) - 32) * 0.2f;
					pos.y = 40 - frm;
					pos = GetMatrix() * pos;
				}
				Explosion_AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, pos, 0);
			}
			if(frm == 60)
				bRenderScorched = true;
			if(frm == 82){
				TheCamera.SetFadeColour(255, 255, 255);
				TheCamera.Fade(0.0f, FADE_OUT);
				TheCamera.ProcessFade();
				TheCamera.Fade(1.0f, FADE_IN);
				FlagToDestroyWhenNextProcessed();
			}
		}else{
			int frm = (*Timer_GetFrameCounter()) - m_nFrameWhenHit;
			if(frm == 20){
				static int nFrameGen;
				CRGBA colors[8];

				Explosion_AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), 0);

				colors[0] = CRGBA(0, 0, 0, 255);
				colors[1] = CRGBA(224, 230, 238, 255);
				colors[2] = CRGBA(224, 230, 238, 255);
				colors[3] = CRGBA(0, 0, 0, 255);
				colors[4] = CRGBA(252, 66, 66, 255);
				colors[5] = CRGBA(0, 0, 0, 255);
				colors[6] = CRGBA(0, 0, 0, 255);
				colors[7] = CRGBA(252, 66, 66, 255);

				CVector dir;
				for(i = 0; i < 40; i++){
					dir.x = General_GetRandomNumberInRangeFloat(-2.0f, 2.0f);
					dir.y = General_GetRandomNumberInRangeFloat(-2.0f, 2.0f);
					dir.z = General_GetRandomNumberInRangeFloat(0.0f, 2.0f);
					int rotSpeed = General_GetRandomNumberInRangeFloat(30.0f, 20.0f);
					if(General_GetRandomNumber() & 1)
						rotSpeed = -rotSpeed;
					int f = ++nFrameGen & 3;
					CParticle::AddParticle(PARTICLE_HELI_DEBRIS, GetMatrix() * CVector(0.0f, 0.0f, 0.0f), dir,
						nil, General_GetRandomNumberInRangeFloat(0.1f, 1.0f),
						colors[nFrameGen&7], rotSpeed, 0, f, 0);
				}
			}
			if(frm >= 40 && frm <= 60 && frm & 1){
				if(frm & 1){
					pos.x = ((General_GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.z = ((General_GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.y = (frm - 40)*0.3f;
					pos = GetMatrix() * pos;
				}else{
					pos.x = ((General_GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.z = ((General_GetRandomNumber() & 0x3F) - 32) * 0.1f;
					pos.y = (40 - frm)*0.3f;
					pos = GetMatrix() * pos;
				}
				Explosion_AddExplosion(nil, FindPlayerPed(), EXPLOSION_HELI, pos, 0);
			}
			if(frm == 30)
				bRenderScorched = true;
			if(frm == 62){
				TheCamera.SetFadeColour(200, 200, 200);
				TheCamera.Fade(0.0f, FADE_OUT);
				TheCamera.ProcessFade();
				TheCamera.Fade(1.0f, FADE_IN);
				if(m_bIsDrugRunCesna){
					CesnaMissionStatus = CESNA_STATUS_DESTROYED;
					pDrugRunCesna = nil;
				}
				if(m_bIsDropOffCesna){
					DropOffCesnaMissionStatus = CESNA_STATUS_DESTROYED;
					pDropOffCesna = nil;
				}
				FlagToDestroyWhenNextProcessed();
			}
		}
	}

	// Update plane position and speed
	if(GetModelIndex() == MI_AIRTRAIN || !m_isFarAway || (((*Timer_GetFrameCounter()) + m_randomSeed) & 7) == 0){
		updatedCollisionMatrix = true; // rouz edit (ChatGPT)
		if(GetModelIndex() == MI_AIRTRAIN){
			float pathPositionRear = PlanePathPosition[m_nPlaneId] - 30.0f;
			if(pathPositionRear < 0.0f)
				pathPositionRear += TotalLengthOfFlightPath;
			float pathPosition = pathPositionRear + 30.0f;

			// Delegate the pitch of the airtrain to the C implementation
			float pitch = PlaneMath_AirtrainPitch(pathPosition, TakeOffPoint, LandingPoint);

			// Advance current node to appropriate position
			// Delegate the search of the segment of the path to the C implementation
			int nextTrackNode;
			float f;
			int curNode = m_nCurPathNode;
			PlaneMath_LocatePathNode(pPathNodes, NumPathNodes, TotalLengthOfFlightPath, &curNode, pathPositionRear, &nextTrackNode, &f);
			m_nCurPathNode = curNode;
			bool bothOnGround = pPathNodes[m_nCurPathNode].bOnGround && pPathNodes[nextTrackNode].bOnGround;
			if(PlanePathPosition[m_nPlaneId] >= LandingPoint && OldPlanePathPosition[m_nPlaneId] < LandingPoint)
				DMAudio_PlayOneShot(m_audioEntityId, SOUND_PLANE_ON_GROUND, 0.0f);
			CVector posRear = InterpolatePlanePathPosition(pPathNodes, NumPathNodes, TotalLengthOfFlightPath, true, m_nCurPathNode, nextTrackNode, f); // rouz edit (ChatGPT)

			// Same for the front
			float pathPositionFront = pathPositionRear + 60.0f;
			if(pathPositionFront > TotalLengthOfFlightPath)
				pathPositionFront -= TotalLengthOfFlightPath;
			int curPathNodeFront = m_nCurPathNode;
			int nextPathNodeFront;
			PlaneMath_LocatePathNode(pPathNodes, NumPathNodes, TotalLengthOfFlightPath, &curPathNodeFront, pathPositionFront, &nextPathNodeFront, &f);
			CVector posFront = InterpolatePlanePathPosition(pPathNodes, NumPathNodes, TotalLengthOfFlightPath, true, curPathNodeFront, nextPathNodeFront, f); // rouz edit (ChatGPT)

			// And for another point 60 units in front of the plane, used to calculate roll
			float pathPositionFront2 = pathPositionFront + 60.0f;
			if(pathPositionFront2 > TotalLengthOfFlightPath)
				pathPositionFront2 -= TotalLengthOfFlightPath;
			int curPathNodeFront2 = m_nCurPathNode;
			int nextPathNodeFront2;
			PlaneMath_LocatePathNode(pPathNodes, NumPathNodes, TotalLengthOfFlightPath, &curPathNodeFront2, pathPositionFront2, &nextPathNodeFront2, &f);
			CVector posFront2 = InterpolatePlanePathPosition(pPathNodes, NumPathNodes, TotalLengthOfFlightPath, true, curPathNodeFront2, nextPathNodeFront2, f); // rouz edit (ChatGPT)

			// Now set matrix
			GetMatrix().SetTranslateOnly((posRear + posFront) / 2.0f);
			GetMatrix().GetPosition().z += 4.3f;
			// Delegate the orientation along the path to the C implementation
			CVector fwd, right, up;
			{
				float fr[3] = { posFront.x, posFront.y, posFront.z };
				float re[3] = { posRear.x, posRear.y, posRear.z };
				float fr2[3] = { posFront2.x, posFront2.y, posFront2.z };
				float f[3], r[3], u[3];
				PlaneMath_PathOrientation(fr, re, fr2, pitch, !bothOnGround, f, r, u);
				fwd = CVector(f[0], f[1], f[2]);
				right = CVector(r[0], r[1], r[2]);
				up = CVector(u[0], u[1], u[2]);
			}
			GetMatrix().GetRight() = right;
			GetMatrix().GetUp() = up;
			GetMatrix().GetForward() = fwd;
			// Set speed
			m_vecMoveSpeed = fwd*PlanePathSpeed[m_nPlaneId]/60.0f;
			m_fSpeed = PlanePathSpeed[m_nPlaneId]/60.0f;
			m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);

			m_isFarAway = !((posFront - TheCamera.GetPosition()).MagnitudeSqr2D() < sq(300.0f));
		}else{
			float planePathPosition;
			float totalLengthOfFlightPath;
			CPlaneNode *pathNodes;
			float planePathSpeed;
			int numPathNodes;
			bool planePathLoops; // rouz edit (ChatGPT)

			if(GetModelIndex() == MI_CHOPPER){
				planePathPosition = PlanePath3Position[m_nPlaneId];
				totalLengthOfFlightPath = TotalLengthOfFlightPath3;
				pathNodes = pPath3Nodes;
				planePathSpeed = PlanePath3Speed[m_nPlaneId];
				numPathNodes = NumPath3Nodes;
				planePathLoops = false; // rouz edit (ChatGPT)
			}else{
				planePathPosition = PlanePath2Position[m_nPlaneId];
				totalLengthOfFlightPath = TotalLengthOfFlightPath2;
				pathNodes = pPath2Nodes;
				planePathSpeed = PlanePath2Speed[m_nPlaneId];
				numPathNodes = NumPath2Nodes;
				planePathLoops = true; // rouz edit (ChatGPT)
			}

			// Advance current node to appropriate position
			float pathPositionRear = planePathPosition - 10.0f;
			if(pathPositionRear < 0.0f)
				pathPositionRear += totalLengthOfFlightPath;
			// Delegate the search of the segment of the path to the C implementation
			int nextTrackNode;
			float f;
			int curNode = m_nCurPathNode;
			PlaneMath_LocatePathNode(pathNodes, numPathNodes, totalLengthOfFlightPath, &curNode, pathPositionRear, &nextTrackNode, &f);
			m_nCurPathNode = curNode;
			CVector posRear = InterpolatePlanePathPosition(pathNodes, numPathNodes, totalLengthOfFlightPath, planePathLoops, m_nCurPathNode, nextTrackNode, f); // rouz edit (ChatGPT)

			// Same for the front
			float pathPositionFront = pathPositionRear + 20.0f;
			if(pathPositionFront > totalLengthOfFlightPath)
				pathPositionFront -= totalLengthOfFlightPath;
			int curPathNodeFront = m_nCurPathNode;
			int nextPathNodeFront;
			PlaneMath_LocatePathNode(pathNodes, numPathNodes, totalLengthOfFlightPath, &curPathNodeFront, pathPositionFront, &nextPathNodeFront, &f);
			CVector posFront = InterpolatePlanePathPosition(pathNodes, numPathNodes, totalLengthOfFlightPath, planePathLoops, curPathNodeFront, nextPathNodeFront, f); // rouz edit (ChatGPT)

			// And for another point 30 units in front of the plane, used to calculate roll
			float pathPositionFront2 = pathPositionFront + 30.0f;
			if(pathPositionFront2 > totalLengthOfFlightPath)
				pathPositionFront2 -= totalLengthOfFlightPath;
			int curPathNodeFront2 = m_nCurPathNode;
			int nextPathNodeFront2;
			PlaneMath_LocatePathNode(pathNodes, numPathNodes, totalLengthOfFlightPath, &curPathNodeFront2, pathPositionFront2, &nextPathNodeFront2, &f);
			CVector posFront2 = InterpolatePlanePathPosition(pathNodes, numPathNodes, totalLengthOfFlightPath, planePathLoops, curPathNodeFront2, nextPathNodeFront2, f); // rouz edit (ChatGPT)

			// Now set matrix
			GetMatrix().SetTranslateOnly((posRear + posFront) / 2.0f);
			GetMatrix().GetPosition().z += 1.0f;
			// Delegate the orientation along the path to the C implementation
			CVector fwd, right, up;
			{
				float fr[3] = { posFront.x, posFront.y, posFront.z };
				float re[3] = { posRear.x, posRear.y, posRear.z };
				float fr2[3] = { posFront2.x, posFront2.y, posFront2.z };
				float f[3], r[3], u[3];
				PlaneMath_PathOrientation(fr, re, fr2, 0.0f, true, f, r, u);
				fwd = CVector(f[0], f[1], f[2]);
				right = CVector(r[0], r[1], r[2]);
				up = CVector(u[0], u[1], u[2]);
			}
			GetMatrix().GetRight() = right;
			GetMatrix().GetUp() = up;
			GetMatrix().GetForward() = fwd;

			// Set speed
			m_vecMoveSpeed = fwd*planePathSpeed/60.0f;
			m_fSpeed = planePathSpeed/60.0f;
			m_vecTurnSpeed = CVector(0.0f, 0.0f, 0.0f);

			m_isFarAway = !((posFront - TheCamera.GetPosition()).MagnitudeSqr2D() < sq(300.0f));
		}
	}

		if(updatedCollisionMatrix){
		if(hadCollisionVelocity && (*Timer_GetTimeStep()) > 0.0f){
			float invTimeStep = 1.0f/(*Timer_GetTimeStep());
			m_vecMoveSpeed = (GetPosition() - previousCollisionMatrix.GetPosition()) * invTimeStep;
			m_vecTurnSpeed = (CrossProduct(previousCollisionMatrix.GetRight(), GetRight()) +
				CrossProduct(previousCollisionMatrix.GetForward(), GetForward()) +
				CrossProduct(previousCollisionMatrix.GetUp(), GetUp())) * (0.5f * invTimeStep);
		}
		m_bHasCollisionVelocity = true;
	}
	
	GetMatrix().UpdateRW();
	UpdateRwFrame();
	RemoveAndAdd(); // rouz edit (ChatGPT)

	// Handle streaming and such
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)
	if(m_isFarAway){
		// Switch to LOD model
		if(m_rwObject && RwObjectGetType(m_rwObject) == rpCLUMP){
			DeleteRwObject();
			if(mi->m_planeLodId != -1){
				PUSH_MEMID(MEMID_WORLD);
				m_rwObject = ((CBaseModelInfo*)ModelRegistry_Get(mi->m_planeLodId))->CreateInstance(); // rouz edit (ChatGPT)
				POP_MEMID();
				if(m_rwObject)
					GetMatrix().AttachRW(RwFrameGetMatrix(RpAtomicGetFrame((RpAtomic*)m_rwObject)));
			}
		}
	}else if(Streaming_HasModelLoaded(GetModelIndex())){ // rouz edit (ChatGPT)
		if(m_rwObject && RwObjectGetType(m_rwObject) == rpATOMIC){
			// Get rid of LOD model
			GetMatrix().Detach();
			if(m_rwObject){	// useless check
				if(RwObjectGetType(m_rwObject) == rpATOMIC){	// useless check
					RwFrame *f = RpAtomicGetFrame((RpAtomic*)m_rwObject);
					RpAtomicDestroy((RpAtomic*)m_rwObject);
					RwFrameDestroy(f);
				}
				m_rwObject = nil;
			}
		}
		// Set high detail model
		if(m_rwObject == nil){
			int id = GetModelIndex();
			m_modelIndex = -1;
			SetModelIndex(id);
		}
	}else{
		Streaming_RequestModel(GetModelIndex(), STREAMFLAGS_DEPENDENCY); // rouz edit (ChatGPT)
	}
}

void
CPlane::ProcessCollision(void)
{
	CMatrix savedMatrix(GetMatrix());
	CVector savedMoveSpeed = m_vecMoveSpeed;
	CVector savedTurnSpeed = m_vecTurnSpeed;
	CVector savedMoveFriction = m_vecMoveFriction;
	CVector savedTurnFriction = m_vecTurnFriction;

	CheckCollision();

	GetMatrix() = savedMatrix;
	m_vecMoveSpeed = savedMoveSpeed;
	m_vecTurnSpeed = savedTurnSpeed;
	m_vecMoveFriction = savedMoveFriction;
	m_vecTurnFriction = savedTurnFriction;
	bIsStuck = false;
	bIsInSafePosition = true;
	RemoveAndAdd();
}

void
CPlane::PreRender(void)
{
	CVehicleModelInfo *mi = (CVehicleModelInfo*)((CBaseModelInfo*)ModelRegistry_Get(GetModelIndex())); // rouz edit (ChatGPT)

	CVector lookVector = GetPosition() - TheCamera.GetPosition();
	float camDist = lookVector.Magnitude();
	if(camDist != 0.0f)
		lookVector *= 1.0f/camDist;
	else
		lookVector = CVector(1.0f, 0.0f, 0.0f);
	float behindness = DotProduct(lookVector, GetForward());

	// Wing lights
	if(behindness < 0.0f){
		// in front of plane
		CVector lightPos = mi->m_positions[PLANE_POS_LIGHT_RIGHT];
		CVector lightR = GetMatrix() * lightPos;
		CVector lightL = lightR;
		// Delegate the position of the left light to the C implementation
		{
			CVector right = GetRight();
			const float lr[3] = { lightL.x, lightL.y, lightL.z };
			const float rt[3] = { right.x, right.y, right.z };
			float ll[3];
			VehicleLogic_MirrorLight(lr, rt, lightPos.x, ll);
			lightL = CVector(ll[0], ll[1], ll[2]);
		}

		float intensity = -0.6f*behindness + 0.4f;
		float size = 1.0f - behindness;

		if(behindness < -0.9f && camDist < 50.0f){
			// directly in front
			CCoronas::RegisterCorona((uintptr)this + 10, 255*intensity, 255*intensity, 255*intensity, 255,
				lightL, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			CCoronas::RegisterCorona((uintptr)this + 11, 255*intensity, 255*intensity, 255*intensity, 255,
				lightR, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_HEADLIGHTS, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		}else{
			CCoronas::RegisterCorona((uintptr)this + 10, 255*intensity, 255*intensity, 255*intensity, 255,
				lightL, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
			CCoronas::RegisterCorona((uintptr)this + 11, 255*intensity, 255*intensity, 255*intensity, 255,
				lightR, size, 240.0f,
				CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
				CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
		}
	}

	// Tail light
	if((*Timer_GetTimeInMilliseconds()) & 0x200){
		CVector pos = GetMatrix() * mi->m_positions[PLANE_POS_LIGHT_TAIL];

		CCoronas::RegisterCorona((uintptr)this + 12, 255, 0, 0, 255,
			pos, 1.0f, 120.0f,
			CCoronas::TYPE_NORMAL, CCoronas::FLARE_NONE, CCoronas::REFLECTION_ON,
			CCoronas::LOSCHECK_OFF, CCoronas::STREAK_ON, 0.0f);
	}

#ifdef CPLANE_ROTORS
	CMatrix mat;
	CVector pos;
	m_fRotorRotation += 3.14f/6.5f;
	if(m_fRotorRotation > 6.28f)
		m_fRotorRotation -= 6.28f;

	if(m_aPlaneNodes[PLANE_TOPROTOR]){
		mat.Attach(RwFrameGetMatrix(m_aPlaneNodes[PLANE_TOPROTOR]));
		pos = mat.GetPosition();
		mat.SetRotateZ(m_fRotorRotation);
		mat.Translate(pos);
		mat.UpdateRW();
	}
	if(m_aPlaneNodes[PLANE_BACKROTOR]){
		mat.Attach(RwFrameGetMatrix(m_aPlaneNodes[PLANE_BACKROTOR]));
		pos = mat.GetPosition();
		mat.SetRotateX(m_fRotorRotation);
		mat.Translate(pos);
		mat.UpdateRW();
	}
#endif
}

void
CPlane::Render(void)
{
	if(!Cutscene_IsRunning()) // rouz edit (ChatGPT)
		CEntity::Render();
}

#define CRUISE_SPEED (50.0f)
#define TAXI_SPEED (5.0f)

void
CPlane::InitPlanes(void)
{
	int i;

	CesnaMissionStatus = CESNA_STATUS_NONE;

	// Jumbo
	if(pPathNodes == nil){
		pPathNodes = LoadPath("data\\paths\\flight.dat", NumPathNodes, TotalLengthOfFlightPath, true);

		// Delegate the search of the ground points and the timeline of the flight to the C implementation
		PlaneMath_FindGroundPoints(pPathNodes, NumPathNodes, &LandingPoint, &TakeOffPoint);
		PlaneMath_BuildFlightTimeline(aPlaneLineBits, TakeOffPoint, LandingPoint, TotalLengthOfFlightPath, CRUISE_SPEED, TAXI_SPEED, &TotalDurationOfFlightPath);
	}

	// Dodo
	if(pPath2Nodes == nil){
		pPath2Nodes = LoadPath("data\\paths\\flight2.dat", NumPath2Nodes, TotalLengthOfFlightPath2, true);
		TotalDurationOfFlightPath2 = TotalLengthOfFlightPath2/CRUISE_SPEED;
	}

	// Heli
	if(pPath3Nodes == nil){
		pPath3Nodes = LoadPath("data\\paths\\flight3.dat", NumPath3Nodes, TotalLengthOfFlightPath3, false);
		TotalDurationOfFlightPath3 = TotalLengthOfFlightPath3/CRUISE_SPEED;
	}

	CStreaming::LoadAllRequestedModels(false);
	Streaming_RequestModel(MI_AIRTRAIN, 0); // rouz edit (ChatGPT)
	CStreaming::LoadAllRequestedModels(false);

	// NB: 3 hardcoded also in CPlaneTrails
	for(i = 0; i < 3; i++){
		// Allocate the airtrain from the vehicle pool without invoking C++ new.
				// Access raw storage through the C store or pool API
		CPlane *plane = (CPlane*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
				assert(plane);
		std::allocator<CPlane>().construct(plane, MI_AIRTRAIN, PERMANENT_VEHICLE);
		plane->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
		plane->SetStatus(STATUS_ABANDONED);
		plane->bIsLocked = true;
		plane->m_nPlaneId = i;
		plane->m_nCurPathNode = 0;
		CWorld::Add(plane);
	}
}

void
CPlane::Shutdown(void)
{
	free(pPathNodes); // rouz edit (ChatGPT)
	free(pPath2Nodes); // rouz edit (ChatGPT)
	free(pPath3Nodes); // rouz edit (ChatGPT)
	free(pPath4Nodes); // rouz edit (ChatGPT)
	pPathNodes = nil;
	pPath2Nodes = nil;
	pPath3Nodes = nil;
	pPath4Nodes = nil;
}

CPlaneNode*
CPlane::LoadPath(char const *filename, int32 &numNodes, float &totalLength, bool loop)
{
	int bp, lp;
	int i;

	FileMgr_LoadFile(filename, work_buff, sizeof(work_buff), "r");
	*gString = '\0';
	for(bp = 0, lp = 0; work_buff[bp] != '\n'; bp++, lp++)
		gString[lp] = work_buff[bp];
	bp++;
	gString[lp] = '\0';
	sscanf(gString, "%d", &numNodes);
	CPlaneNode *nodes = (CPlaneNode*)malloc(sizeof(CPlaneNode)*numNodes); // rouz edit (ChatGPT)

	for(i = 0; i < numNodes; i++){
		for(lp = 0; work_buff[bp] != '\n' && work_buff[bp] != '\0'; bp++, lp++)
			gString[lp] = work_buff[bp];
		bp++;
		// BUG: game doesn't terminate string
		gString[lp] = '\0';
		sscanf(gString, "%f %f %f", &nodes[i].p.x, &nodes[i].p.y, &nodes[i].p.z);
	}

	// Delegate the calculation of the length of segments and path to the C implementation
	PlaneMath_ComputePathLengths(nodes, numNodes, loop, &totalLength);

	return nodes;
}

int32 LastTimeInPlane, LastTimeNotInPlane;
bool bCesnasActivated;
bool bHelisActivated;

void
CPlane::UpdatePlanes(void)
{
	int i;
	uint32 time;

	if(Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;

	// Jumbo jets
	time = (*Timer_GetTimeInMilliseconds());
	// Delegate the position and speed of the jumbo jets to the C implementation
	for(i = 0; i < 3; i++){
		OldPlanePathPosition[i] = PlanePathPosition[i];
		PlaneMath_JumboPath(time, TotalDurationOfFlightPath, aPlaneLineBits, &PlanePathPosition[i], &PlanePathSpeed[i]);

		// time offset for each plane
		time += 0x80000/3;
	}

	time = (*Timer_GetTimeInMilliseconds());

	// Delegate the position and speed of the other planes and of the choppers to the C implementation
	PlaneMath_CruisePath(time, TotalDurationOfFlightPath2, CRUISE_SPEED, 5, PlanePath2Position, PlanePath2Speed);
	PlaneMath_CruisePath(time, TotalDurationOfFlightPath3, CRUISE_SPEED, 4, PlanePath3Position, PlanePath3Speed);

	if(FindPlayerVehicle() && (FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_HELI ||
	                           FindPlayerVehicle()->GetVehicleAppearance() == VEHICLE_APPEARANCE_PLANE))
		LastTimeInPlane = (*Timer_GetTimeInMilliseconds());
	else
		LastTimeNotInPlane = (*Timer_GetTimeInMilliseconds());

	if((*Timer_GetTimeInMilliseconds()) - LastTimeNotInPlane > 10000){
		if(!bCesnasActivated){
			if(Streaming_HasModelLoaded(MI_DEADDODO)){ // rouz edit (ChatGPT)
				for(i = 0; i < 5; i++){
					// Allocate the temporary cesna from the vehicle pool without invoking C++ new.
										// Access raw storage through the C store or pool API
					CPlane *plane = (CPlane*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
										assert(plane);
					std::allocator<CPlane>().construct(plane, MI_DEADDODO, PERMANENT_VEHICLE);
					plane->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
					plane->SetStatus(STATUS_ABANDONED);
					plane->bIsLocked = true;
					plane->m_nPlaneId = i;
					plane->m_nCurPathNode = 0;
					plane->m_bTempPlane = true;
					CWorld::Add(plane);
				}
				bCesnasActivated = true;
			}else
				Streaming_RequestModel(MI_DEADDODO, 0); // rouz edit (ChatGPT)
		}

		if(!bHelisActivated){
			if(Streaming_HasModelLoaded(MI_CHOPPER)){ // rouz edit (ChatGPT)
				for(i = 0; i < 4; i++){
					// Allocate the temporary chopper from the vehicle pool without invoking C++ new.
										// Access raw storage through the C store or pool API
					CPlane *plane = (CPlane*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
										assert(plane);
					std::allocator<CPlane>().construct(plane, MI_CHOPPER, PERMANENT_VEHICLE);
					plane->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
					plane->SetStatus(STATUS_ABANDONED);
					plane->bIsLocked = true;
					plane->m_nPlaneId = i;
					plane->m_nCurPathNode = 0;
					plane->m_bTempPlane = true;
					CWorld::Add(plane);
				}
				bHelisActivated = true;
			}else
				Streaming_RequestModel(MI_CHOPPER, 0); // rouz edit (ChatGPT)
		}
	}else if((*Timer_GetTimeInMilliseconds()) - LastTimeInPlane > 10000)
		RemoveTemporaryPlanes();
}

void
CPlane::RemoveTemporaryPlanes(void)
{
	int i;
	if(!bHelisActivated && !bCesnasActivated)
		return;

		// Access raw storage through the C store or pool API
	i = CPool_GetSize(CPools::GetVehiclePool());
		while(--i >= 0){
				// Access raw storage through the C store or pool API
		CPlane *plane = (CPlane*)((CVehicle*)CPool_GetSlot(CPools::GetVehiclePool(), i));
				if(plane && plane->IsPlane() && plane->m_bTempPlane){
			CWorld::Remove(plane);
			// Destroy and release the temporary plane without invoking C++ delete.
			plane->~CPlane();
						// Access raw storage through the C store or pool API
			CPool_Delete(CPools::GetVehiclePool(), plane);
					}
	}
	bCesnasActivated = false;
	bHelisActivated = false;
}

bool
CPlane::TestRocketCollision(CVector *rocketPos)
{
	int i;

		// Access raw storage through the C store or pool API
	i = CPool_GetSize(CPools::GetVehiclePool());
		while(--i >= 0){
				// Access raw storage through the C store or pool API
		CPlane *plane = (CPlane*)((CVehicle*)CPool_GetSlot(CPools::GetVehiclePool(), i));
				if(plane &&
#ifdef EXPLODING_AIRTRAIN
		   (plane->GetModelIndex() == MI_AIRTRAIN || plane->GetModelIndex() == MI_DEADDODO) &&
#else
		   plane->GetModelIndex() != MI_AIRTRAIN && plane->GetModelIndex() == MI_DEADDODO &&	// strange check
#endif
		   !plane->m_bHasBeenHit && (*rocketPos - plane->GetPosition()).Magnitude() < 25.0f){
			plane->m_nFrameWhenHit = (*Timer_GetFrameCounter());
			plane->m_bHasBeenHit = true;
			Wanted_RegisterCrime_Immediately(CWorld::Players[CWorld::PlayerInFocus].m_pPed->m_pWanted,CRIME_DESTROYED_CESSNA,Wanted_ReferenceNative(plane->GetPosition()),i+1983,false);
			return true;
		}
	}
	return false;
}

// unused
// BUG: not in CPlane in the game
void
CPlane::CreateIncomingCesna(void)
{
	if(CesnaMissionStatus == CESNA_STATUS_FLYING){
		CWorld::Remove(pDrugRunCesna);
		// Destroy and release the drug-run cesna without invoking C++ delete.
		pDrugRunCesna->~CPlane();
				// Access raw storage through the C store or pool API
		CPool_Delete(CPools::GetVehiclePool(), pDrugRunCesna);
				pDrugRunCesna = nil;
	}
	// Allocate the drug-run cesna from the vehicle pool without invoking C++ new.
		// Access raw storage through the C store or pool API
	pDrugRunCesna = (CPlane*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
		assert(pDrugRunCesna);
	std::allocator<CPlane>().construct(pDrugRunCesna, MI_DEADDODO, PERMANENT_VEHICLE);
	pDrugRunCesna->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
	pDrugRunCesna->SetStatus(STATUS_ABANDONED);
	pDrugRunCesna->bIsLocked = true;
	pDrugRunCesna->m_nPlaneId = 0;
	pDrugRunCesna->m_nCurPathNode = 0;
	pDrugRunCesna->m_bIsDrugRunCesna = true;
	CWorld::Add(pDrugRunCesna);

        CesnaMissionStatus = CESNA_STATUS_FLYING;
        CesnaMissionStartTime = (*Timer_GetTimeInMilliseconds());
        printf("CPlane::CreateIncomingCesna(void)\n");
}

// unused
void
CPlane::CreateDropOffCesna(void)
{
	if(DropOffCesnaMissionStatus == CESNA_STATUS_FLYING){
		CWorld::Remove(pDropOffCesna);
		// Destroy and release the drop-off cesna without invoking C++ delete.
		pDropOffCesna->~CPlane();
				// Access raw storage through the C store or pool API
		CPool_Delete(CPools::GetVehiclePool(), pDropOffCesna);
				pDropOffCesna = nil;
	}
	// Allocate the drop-off cesna from the vehicle pool without invoking C++ new.
		// Access raw storage through the C store or pool API
	pDropOffCesna = (CPlane*)((CVehicle*)CPool_New(CPools::GetVehiclePool()));
		assert(pDropOffCesna);
	std::allocator<CPlane>().construct(pDropOffCesna, MI_DEADDODO, PERMANENT_VEHICLE);
	pDropOffCesna->GetMatrix().SetTranslate(0.0f, 0.0f, 0.0f);
	pDropOffCesna->SetStatus(STATUS_ABANDONED);
	pDropOffCesna->bIsLocked = true;
	pDropOffCesna->m_nPlaneId = 0;
	pDropOffCesna->m_nCurPathNode = 0;
	pDropOffCesna->m_bIsDropOffCesna = true;
	CWorld::Add(pDropOffCesna);

        DropOffCesnaMissionStatus = CESNA_STATUS_FLYING;
        DropOffCesnaMissionStartTime = (*Timer_GetTimeInMilliseconds());
        printf("CPlane::CreateDropOffCesna(void)\n");
}

// all unused
const CVector CPlane::FindDrugPlaneCoordinates(void) { return pDrugRunCesna->GetPosition(); }
const CVector CPlane::FindDropOffCesnaCoordinates(void) { return pDropOffCesna->GetPosition(); }
bool CPlane::HasCesnaLanded(void) { return CesnaMissionStatus == CESNA_STATUS_LANDED; }
bool CPlane::HasCesnaBeenDestroyed(void) { return CesnaMissionStatus == CESNA_STATUS_DESTROYED; }
bool CPlane::HasDropOffCesnaBeenShotDown(void) { return DropOffCesnaMissionStatus == CESNA_STATUS_DESTROYED; }

void
CPlane::Load(void)
{
	RemoveTemporaryPlanes();
}

void
CPlane::Save(void)
{
	RemoveTemporaryPlanes();
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
