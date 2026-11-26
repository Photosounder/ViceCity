//+ rouz edit (ChatGPT)
#include "common.h"

#include "Cranes.h"
#include "CraneLogic.h"

#include "Camera.h"
#include "DMAudio.h"
#include "Garages.h"
#include "General.h"
#include "Entity.h"
#include "ModelIndices.h"
#include "Replay.h"
#include "Object.h"
#include "WorldLogic.h"
#include "World.h"
#include "SaveBuf.h"
#include "Pools.h" // rouz edit (ChatGPT)
#include "VehicleLogic.h"

#define MAX_DISTANCE_TO_FIND_CRANE (10.0f)
#define CRANE_UPDATE_RADIUS (300.0f)
#define CRANE_MOVEMENT_PROCESSING_RADIUS (150.0f)
#define CRUSHER_Z (-0.951f)
#define MILITARY_Z (10.7862f)
#define DISTANCE_FROM_PLAYER_TO_REMOVE_VEHICLE (5.0f)
#define DISTANCE_FROM_HOOK_TO_VEHICLE_TO_COLLECT (0.5f)
#define CAR_REWARD_MILITARY_CRANE (1500)
#define CAR_MOVING_SPEED_THRESHOLD (0.01f)
#define CRANE_SLOWDOWN_MULTIPLIER (0.3f)

#define OSCILLATION_SPEED (0.002f)
#define CAR_ROTATION_SPEED (0.0035f)
#define CRANE_MOVEMENT_SPEED (0.001f)
#define HOOK_ANGLE_MOVEMENT_SPEED (0.004f)
#define HOOK_OFFSET_MOVEMENT_SPEED (0.1f)
#define HOOK_HEIGHT_MOVEMENT_SPEED (0.06f)

#define MESSAGE_SHOW_DURATION (4000)

#define MAX_DISTANCE (99999.9f)
#define MIN_VALID_POSITION (-10000.0f)
#define DEFAULT_OFFSET (20.0f)

#ifdef COMPATIBLE_SAVES
#define CRANES_SAVE_SIZE 0x3E0
#else
#define CRANES_SAVE_SIZE sizeof(Cranes_aCranes)
#endif

uint32 TimerForCamInterpolation;

uint32 Cranes_CarsCollectedMilitaryCrane;
int32 Cranes_NumCranes;
CCrane Cranes_aCranes[NUM_CRANES];

void Cranes_InitCranes(void)
{
	Cranes_CarsCollectedMilitaryCrane = 0;
	Cranes_NumCranes = 0;
	for (int i = 0; i < NUMSECTORS_X; i++) {
		for (int j = 0; j < NUMSECTORS_Y; j++) {
			for (CPtrNode* pNode = World_GetSector(i, j)->m_lists[ENTITYLIST_BUILDINGS].first; pNode; pNode = pNode->next) {
				CEntity* pEntity = (CEntity*)pNode->item;
				if (MODELID_CRANE_1 == pEntity->GetModelIndex() ||
					MODELID_CRANE_2 == pEntity->GetModelIndex() ||
					MODELID_CRANE_3 == pEntity->GetModelIndex() ||
					MODELID_CRANE_4 == pEntity->GetModelIndex() ||
					MODELID_CRANE_5 == pEntity->GetModelIndex() ||
					MODELID_CRANE_6 == pEntity->GetModelIndex())
					Cranes_AddThisOneCrane(pEntity);
			}
		}
	}
	for (CPtrNode* pNode = World_GetBigBuildingList(LEVEL_MAINLAND).first; pNode; pNode = pNode->next) {
		CEntity* pEntity = (CEntity*)pNode->item;
		if (MODELID_CRANE_1 == pEntity->GetModelIndex() ||
			MODELID_CRANE_2 == pEntity->GetModelIndex() ||
			MODELID_CRANE_3 == pEntity->GetModelIndex() ||
			MODELID_CRANE_4 == pEntity->GetModelIndex() ||
			MODELID_CRANE_5 == pEntity->GetModelIndex() ||
			MODELID_CRANE_6 == pEntity->GetModelIndex())
			Cranes_AddThisOneCrane(pEntity);
	}
}

void Cranes_AddThisOneCrane(CEntity* pEntity)
{
	pEntity->GetMatrix().ResetOrientation();
	if (Cranes_NumCranes >= NUM_CRANES)
		return;
	CCrane* pCrane = &Cranes_aCranes[Cranes_NumCranes];
	pCrane->Init();
	pCrane->m_pCraneEntity = (CBuilding*)pEntity;
	pCrane->m_nCraneStatus = CCrane::NONE;
	pCrane->m_fHookAngle = Cranes_NumCranes; // lol wtf
	// Delegate the angle wrapping to the C implementation
	pCrane->m_fHookAngle = General_ReduceAbove(pCrane->m_fHookAngle, TWOPI, TWOPI);
	pCrane->m_fHookOffset = DEFAULT_OFFSET;
	pCrane->m_fHookHeight = DEFAULT_OFFSET;
	pCrane->m_nTimeForNextCheck = 0;
	pCrane->m_nCraneState = CCrane::IDLE;
	pCrane->m_bWasMilitaryCrane = false;
	pCrane->m_bIsTop = (MODELID_CRANE_1 != pEntity->GetModelIndex());
	pCrane->m_pHook = nil;
	Cranes_NumCranes++;
}

void Cranes_ActivateCrane(float fInfX, float fSupX, float fInfY, float fSupY, float fDropOffX, float fDropOffY, float fDropOffZ, float fHeading, bool bIsCrusher, bool bIsMilitary, float fPosX, float fPosY)
{
	float fMinDistance = MAX_DISTANCE;
	float X = fPosX, Y = fPosY;
	if (X <= MIN_VALID_POSITION || Y <= MIN_VALID_POSITION) {
		X = fDropOffX;
		Y = fDropOffY;
	}
	int index = 0;
	for (int i = 0; i < Cranes_NumCranes; i++) {
		float distance = (CVector2D(X, Y) - Cranes_aCranes[i].m_pCraneEntity->GetPosition()).Magnitude();
		if (distance < fMinDistance && distance < MAX_DISTANCE_TO_FIND_CRANE) {
			fMinDistance = distance;
			index = i;
		}
	}
#ifdef FIX_BUGS // classic
	if (fMinDistance == MAX_DISTANCE)
		return;
#endif
	CCrane* pCrane = &Cranes_aCranes[index];
	pCrane->m_fPickupX1 = fInfX;
	pCrane->m_fPickupX2 = fSupX;
	pCrane->m_fPickupY1 = fInfY;
	pCrane->m_fPickupY2 = fSupY;
	pCrane->m_vecDropoffTarget.x = fDropOffX;
	pCrane->m_vecDropoffTarget.y = fDropOffY;
	pCrane->m_vecDropoffTarget.z = fDropOffZ;
	pCrane->m_nCraneStatus = CCrane::ACTIVATED;
	pCrane->m_pVehiclePickedUp = nil;
	pCrane->m_nVehiclesCollected = 0;
	pCrane->m_fDropoffHeading = fHeading;
	pCrane->m_bIsCrusher = bIsCrusher;
	pCrane->m_bIsMilitaryCrane = bIsMilitary;
	bool military = true;
	if (!bIsMilitary && !pCrane->m_bWasMilitaryCrane)
		military = false;
	pCrane->m_bWasMilitaryCrane = military;
	pCrane->m_nTimeForNextCheck = 0;
	pCrane->m_nCraneState = CCrane::IDLE;
	float Z;
	if (bIsCrusher)
		Z = CRUSHER_Z;
	else if (bIsMilitary)
		Z = MILITARY_Z;
	else
		Z = World_FindGroundZForCoord((fInfX + fSupX) / 2, (fInfY + fSupY) / 2);
	pCrane->FindParametersForTarget((fInfX + fSupX) / 2, (fInfY + fSupY) / 2, Z, &pCrane->m_fPickupAngle, &pCrane->m_fPickupDistance, &pCrane->m_fPickupHeight);
	pCrane->FindParametersForTarget(fDropOffX, fDropOffY, fDropOffZ, &pCrane->m_fDropoffAngle, &pCrane->m_fDropoffDistance, &pCrane->m_fDropoffHeight);
}

void Cranes_DeActivateCrane(float X, float Y)
{
	float fMinDistance = MAX_DISTANCE;
	int index = 0;
	for (int i = 0; i < Cranes_NumCranes; i++) {
		float distance = (CVector2D(X, Y) - Cranes_aCranes[i].m_pCraneEntity->GetPosition()).Magnitude();
		if (distance < fMinDistance && distance < MAX_DISTANCE_TO_FIND_CRANE) {
			fMinDistance = distance;
			index = i;
		}
	}
#ifdef FIX_BUGS // classic
	if (fMinDistance == MAX_DISTANCE)
		return;
#endif
	Cranes_aCranes[index].m_nCraneStatus = CCrane::DEACTIVATED;
	Cranes_aCranes[index].m_nCraneState = CCrane::IDLE;
}

bool Cranes_IsThisCarPickedUp(float X, float Y, CVehicle* pVehicle)
{
	int index = 0;
	bool result = false;
	for (int i = 0; i < Cranes_NumCranes; i++) {
		float distance = (CVector2D(X, Y) - Cranes_aCranes[i].m_pCraneEntity->GetPosition()).Magnitude();
		if (distance < MAX_DISTANCE_TO_FIND_CRANE && Cranes_aCranes[i].m_pVehiclePickedUp == pVehicle) {
			if (Cranes_aCranes[i].m_nCraneState == CCrane::LIFTING_TARGET || Cranes_aCranes[i].m_nCraneState == CCrane::ROTATING_TARGET)
				result = true;
		}
	}
	return result;
}

void Cranes_UpdateCranes(void)
{
	for (int i = 0; i < Cranes_NumCranes; i++) {
		if (Cranes_aCranes[i].m_bIsTop || Cranes_aCranes[i].m_bIsCrusher ||
			(TheCamera.GetPosition().x + CRANE_UPDATE_RADIUS > Cranes_aCranes[i].m_pCraneEntity->GetPosition().x &&
				TheCamera.GetPosition().x - CRANE_UPDATE_RADIUS < Cranes_aCranes[i].m_pCraneEntity->GetPosition().x &&
				TheCamera.GetPosition().y + CRANE_UPDATE_RADIUS > Cranes_aCranes[i].m_pCraneEntity->GetPosition().y &&
				TheCamera.GetPosition().y + CRANE_UPDATE_RADIUS < Cranes_aCranes[i].m_pCraneEntity->GetPosition().y))
			Cranes_aCranes[i].Update();
	}
}

void CCrane::Update(void)
{
	if (Replay_IsPlayingBack()) // rouz edit (ChatGPT)
		return;
	if (((m_nCraneStatus == ACTIVATED || m_nCraneStatus == DEACTIVATED) &&
		VehicleLogic_WithinSquare(TheCamera.GetGameCamPosition().x, TheCamera.GetGameCamPosition().y, m_pCraneEntity->GetPosition().x, m_pCraneEntity->GetPosition().y, CRANE_MOVEMENT_PROCESSING_RADIUS)) ||
		m_nCraneState != IDLE) {
		switch (m_nCraneState) {
		case IDLE:
			if (GoTowardsTarget(m_fPickupAngle, m_fPickupDistance, GetHeightToPickup()) &&
				(*Timer_GetTimeInMilliseconds()) > m_nTimeForNextCheck) {
				World_AdvanceCurrentScanCode();
#ifdef FIX_BUGS
				// Delegate the range of sectors to the C implementation
				int32_t xstart, ystart, xend, yend;
				World_SectorRange(m_fPickupX1, m_fPickupY1, m_fPickupX2, m_fPickupY2, &xstart, &ystart, &xend, &yend);
#else
				int xstart = World_GetSectorIndexX(m_fPickupX1);
				int xend = World_GetSectorIndexX(m_fPickupX2);
				int ystart = World_GetSectorIndexY(m_fPickupY1);
				int yend = World_GetSectorIndexY(m_fPickupY1);
#endif
				assert(xstart <= xend);
				assert(ystart <= yend);
				for (int i = xstart; i <= xend; i++) {
					for (int j = ystart; j <= yend; j++) {
						FindCarInSectorList(&World_GetSector(i, j)->m_lists[ENTITYLIST_VEHICLES]);
						FindCarInSectorList(&World_GetSector(i, j)->m_lists[ENTITYLIST_VEHICLES_OVERLAP]);
					}
				}
			}
			break;
		case GOING_TOWARDS_TARGET:
			if (m_pVehiclePickedUp){
				if (m_pVehiclePickedUp->GetPosition().x < m_fPickupX1 ||
					m_pVehiclePickedUp->GetPosition().x > m_fPickupX2 ||
					m_pVehiclePickedUp->GetPosition().y < m_fPickupY1 ||
					m_pVehiclePickedUp->GetPosition().y > m_fPickupY2 ||
					m_pVehiclePickedUp->pDriver ||
					VehicleLogic_AnySpeedAbove(m_pVehiclePickedUp->GetMoveSpeed().x, m_pVehiclePickedUp->GetMoveSpeed().y, m_pVehiclePickedUp->GetMoveSpeed().z, CAR_MOVING_SPEED_THRESHOLD) ||
					(FindPlayerPed()->GetPedState() == PED_ENTER_CAR
#ifdef FIX_BUGS
					|| FindPlayerPed()->GetPedState() == PED_CARJACK
#endif
					) && FindPlayerPed()->m_pSeekTarget == m_pVehiclePickedUp) {
					m_pVehiclePickedUp = nil;
					m_nCraneState = IDLE;
				}
				else {
					float fAngle, fOffset, fHeight;
					FindParametersForTarget(
						m_pVehiclePickedUp->GetPosition().x,
						m_pVehiclePickedUp->GetPosition().y,
						m_pVehiclePickedUp->GetPosition().z + m_pVehiclePickedUp->GetColModel()->boundingBox.max.z,
						&fAngle, &fOffset, &fHeight);
					if (GoTowardsTarget(fAngle, fOffset, fHeight)) {
						CVector distance = m_pVehiclePickedUp->GetPosition() - m_vecHookCurPos;
						distance.z += m_pVehiclePickedUp->GetColModel()->boundingBox.max.z;
						if (distance.MagnitudeSqr() < SQR(DISTANCE_FROM_HOOK_TO_VEHICLE_TO_COLLECT)) {
							m_nCraneState = GOING_TOWARDS_TARGET_ONLY_HEIGHT;
							m_vecHookVelocity *= 0.4f;
							m_pVehiclePickedUp->bLightsOn = false;
							m_pVehiclePickedUp->bUsesCollision = false;
							if (m_bIsCrusher)
								m_pVehiclePickedUp->bCollisionProof = true;
						}
					}
				}
			}
			else
				m_nCraneState = IDLE;
			break;
		case LIFTING_TARGET:
			RotateCarriedCarProperly();
			if (GoTowardsTarget(m_fDropoffAngle, m_fDropoffDistance, GetHeightToDropoff(), CRANE_SLOWDOWN_MULTIPLIER))
				m_nCraneState = ROTATING_TARGET;
			if (!m_pVehiclePickedUp || m_pVehiclePickedUp->pDriver) {
				m_pVehiclePickedUp = nil;
				m_nCraneState = IDLE;
			}
			break;
		case GOING_TOWARDS_TARGET_ONLY_HEIGHT:
			RotateCarriedCarProperly();
			if (GoTowardsHeightTarget(GetHeightToPickupHeight(), CRANE_SLOWDOWN_MULTIPLIER))
				m_nCraneState = LIFTING_TARGET;
			TimerForCamInterpolation = (*Timer_GetTimeInMilliseconds());
			if (!m_pVehiclePickedUp || m_pVehiclePickedUp->pDriver) {
				m_pVehiclePickedUp = nil;
				m_nCraneState = IDLE;
			}
			break;
		case ROTATING_TARGET:
		{
			bool bRotateFinished = RotateCarriedCarProperly();
			bool bMovementFinished = GoTowardsTarget(m_fDropoffAngle, m_fDropoffDistance, m_fDropoffHeight, 0.3f);
			if (bMovementFinished && bRotateFinished) {
				float fDistanceFromPlayer = m_pVehiclePickedUp ? ((CVector2D)FindPlayerCoors() - (CVector2D)m_pVehiclePickedUp->GetPosition()).Magnitude() : 0.0f;
				if (fDistanceFromPlayer > DISTANCE_FROM_PLAYER_TO_REMOVE_VEHICLE || !m_bWasMilitaryCrane) {
					m_nCraneState = DROPPING_TARGET;
					if (m_pVehiclePickedUp) {
						m_pVehiclePickedUp->bUsesCollision = true;
						m_pVehiclePickedUp->m_nStaticFrames = 0;
						++m_nVehiclesCollected;
						if (m_bIsMilitaryCrane) {
							Cranes_RegisterCarForMilitaryCrane(m_pVehiclePickedUp->GetModelIndex());
							if (!Cranes_HaveAllCarsBeenCollectedByMilitaryCrane()) {
								World_Players[World_PlayerInFocus].m_nMoney += CAR_REWARD_MILITARY_CRANE;
								Garages_TriggerMessage("GA_10", CAR_REWARD_MILITARY_CRANE, MESSAGE_SHOW_DURATION, -1);
							}
							World_Remove(m_pVehiclePickedUp);
//+ rouz edit (ChatGPT)
							// Destroy and release the crane vehicle without invoking C++ delete.
							m_pVehiclePickedUp->~CVehicle();
							//+ rouz edit (ChatGPT)
							// Access raw storage through the C store or pool API
							CPool_Delete(Pools_GetVehiclePool(), m_pVehiclePickedUp);
							//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
						}
					}
					m_pVehiclePickedUp = nil;
				}
			}
			break;
		}
		case DROPPING_TARGET:
			if (GoTowardsTarget(m_fDropoffAngle, m_fDropoffDistance, GetHeightToDropoffHeight(), CRANE_SLOWDOWN_MULTIPLIER)) {
				m_nCraneState = IDLE;
				m_nTimeForNextCheck = (*Timer_GetTimeInMilliseconds()) + 10000;
			}
			break;
		default:
			break;
		}
		CVector vecHook;
		CalcHookCoordinates(&vecHook.x, &vecHook.y, &vecHook.z);
		m_vecHookVelocity += ((CVector2D)vecHook - (CVector2D)m_vecHookCurPos) * (*Timer_GetTimeStep()) * CRANE_MOVEMENT_SPEED;
		m_vecHookVelocity *= Pow(0.98f, (*Timer_GetTimeStep()));
		m_vecHookCurPos.x += m_vecHookVelocity.x * (*Timer_GetTimeStep());
		m_vecHookCurPos.y += m_vecHookVelocity.y * (*Timer_GetTimeStep());
		m_vecHookCurPos.z = vecHook.z;
		switch (m_nCraneState) {
		case LIFTING_TARGET:
		case GOING_TOWARDS_TARGET_ONLY_HEIGHT:
		case ROTATING_TARGET:
			if (m_pVehiclePickedUp) {
				m_pVehiclePickedUp->SetPosition(m_vecHookCurPos.x, m_vecHookCurPos.y, m_vecHookCurPos.z - m_pVehiclePickedUp->GetColModel()->boundingBox.max.z);
				m_pVehiclePickedUp->SetMoveSpeed(0.0f, 0.0f, 0.0f);
				CVector up(vecHook.x - m_vecHookCurPos.x, vecHook.y - m_vecHookCurPos.y, 20.0f);
				up.Normalise();
				m_pVehiclePickedUp->GetRight() = CrossProduct(m_pVehiclePickedUp->GetForward(), up);
				m_pVehiclePickedUp->GetForward() = CrossProduct(up, m_pVehiclePickedUp->GetRight());
				m_pVehiclePickedUp->GetUp() = up;
			}
			break;
		default:
			break;
		}
	}
	else {
		int16 rnd = (m_pCraneEntity->m_randomSeed + ((*Timer_GetTimeInMilliseconds()) >> 11)) & 0xF;
		// 16 options, lasting 2048 ms each
		// a bit awkward: why there are 4 periods for -= and 6 for +=? is it a bug?
		if (rnd < 4) {
			m_fHookAngle -= OSCILLATION_SPEED * (*Timer_GetTimeStep());
			if (m_fHookAngle < 0.0f)
				m_fHookAngle += TWOPI;
		}
		else if (rnd > 5 && rnd < 12) {
			m_fHookAngle += OSCILLATION_SPEED * (*Timer_GetTimeStep());
			if (m_fHookAngle > TWOPI)
				m_fHookAngle -= TWOPI;
		}
		CalcHookCoordinates(&m_vecHookCurPos.x, &m_vecHookCurPos.y, &m_vecHookCurPos.z);
		m_vecHookVelocity.x = m_vecHookVelocity.y = 0.0f;
	}
	float fCos = Cos(m_fHookAngle);
	float fSin = Sin(m_fHookAngle);
	m_pCraneEntity->GetRight().x = fCos;
	m_pCraneEntity->GetForward().y = fCos;
	m_pCraneEntity->GetRight().y = fSin;
	m_pCraneEntity->GetForward().x = -fSin;
	m_pCraneEntity->GetMatrix().UpdateRW();
	m_pCraneEntity->UpdateRwFrame();
	SetHookMatrix();
}

bool CCrane::RotateCarriedCarProperly()
{
	if (m_fDropoffHeading <= 0.0f)
		return true;
	if (!m_pVehiclePickedUp)
		return true;
	float fAngleDelta = m_fDropoffHeading - General_GetATanOfXY(m_pVehiclePickedUp->GetForward().x, m_pVehiclePickedUp->GetForward().y);
	// Delegate the angle wrapping to the C implementation
	fAngleDelta = General_RaiseBelow(fAngleDelta, -HALFPI, PI);
	fAngleDelta = General_ReduceAbove(fAngleDelta, HALFPI, PI);
	// Delegate the step of the rotation to the C implementation
	float fRotation;
	if (Crane_RotationStep(fAngleDelta, CAR_ROTATION_SPEED, (*Timer_GetTimeStep()), &fRotation)) // no rotation is actually applied?
		return true;
	m_pVehiclePickedUp->GetMatrix().RotateZ(fRotation);
	return false;
}

void CCrane::FindCarInSectorList(CPtrList* pList)
{
	CPtrNode* node;
	for (node = pList->first; node; node = node->next) {
		CVehicle* pVehicle = (CVehicle*)node->item;
		if (pVehicle->m_scanCode == World_GetCurrentScanCode())
			continue;
		pVehicle->m_scanCode = World_GetCurrentScanCode();
		if (pVehicle->GetPosition().x < m_fPickupX1 || pVehicle->GetPosition().x > m_fPickupX2 ||
			pVehicle->GetPosition().y < m_fPickupY1 || pVehicle->GetPosition().y > m_fPickupY2)
			continue;
		if (pVehicle->pDriver)
			continue;
		// Delegate the moving check to the C implementation
		if (VehicleLogic_AnySpeedAtLeast(pVehicle->GetMoveSpeed().x, pVehicle->GetMoveSpeed().y, pVehicle->GetMoveSpeed().z, CAR_MOVING_SPEED_THRESHOLD))
			continue;
		if (!pVehicle->IsCar() || pVehicle->GetStatus() == STATUS_WRECKED || pVehicle->m_fHealth < 250.0f)
			continue;
		if (!DoesCranePickUpThisCarType(pVehicle->GetModelIndex()) ||
			m_bIsMilitaryCrane && Cranes_DoesMilitaryCraneHaveThisOneAlready(pVehicle->GetModelIndex())) {
			if (!pVehicle->bCraneMessageDone) {
				pVehicle->bCraneMessageDone = true;
				if (!m_bIsMilitaryCrane)
					Garages_TriggerMessage("CR_1", -1, MESSAGE_SHOW_DURATION, -1); // Crane cannot lift this vehicle.
				else if (DoesCranePickUpThisCarType(pVehicle->GetModelIndex()))
					Garages_TriggerMessage("GA_20", -1, MESSAGE_SHOW_DURATION, -1); // We got more of these than we can shift. Sorry man, no deal.
				else
					Garages_TriggerMessage("GA_19", -1, MESSAGE_SHOW_DURATION, -1); // We're not interested in that model.
			}
		}
		else {
			m_pVehiclePickedUp = pVehicle;
			pVehicle->RegisterReference((CEntity**)&m_pVehiclePickedUp);
			m_nCraneState = GOING_TOWARDS_TARGET;
		}
	}
}

bool CCrane::DoesCranePickUpThisCarType(uint32 mi)
{
	// Delegate to the C implementation
	return Crane_DoesPickUpThisCarType(this, mi);
}

bool Cranes_DoesMilitaryCraneHaveThisOneAlready(uint32 mi)
{
	// Delegate to the C implementation
	return CraneLogic_DoesMilitaryCraneHaveThisOneAlready(Cranes_CarsCollectedMilitaryCrane, mi);
}

void Cranes_RegisterCarForMilitaryCrane(uint32 mi)
{
	// Delegate to the C implementation
	CraneLogic_RegisterCarForMilitaryCrane(&Cranes_CarsCollectedMilitaryCrane, mi);
}

bool Cranes_HaveAllCarsBeenCollectedByMilitaryCrane()
{
	// Delegate to the C implementation
	return CraneLogic_HaveAllCarsBeenCollectedByMilitaryCrane(Cranes_CarsCollectedMilitaryCrane);
}

bool CCrane::GoTowardsTarget(float fAngleToTarget, float fDistanceToTarget, float fTargetHeight, float fSpeedMultiplier)
{
	// Delegate to the C implementation
	return Crane_GoTowardsTarget(this, fAngleToTarget, fDistanceToTarget, fTargetHeight, fSpeedMultiplier, (*Timer_GetTimeStep()));
}

bool CCrane::GoTowardsHeightTarget(float fTargetHeight, float fSpeedMultiplier)
{
	// Delegate to the C implementation
	return Crane_GoTowardsHeightTarget(this, fTargetHeight, fSpeedMultiplier, (*Timer_GetTimeStep()));
}

void CCrane::FindParametersForTarget(float X, float Y, float Z, float* pAngle, float* pDistance, float* pHeight)
{
	*pAngle = General_GetATanOfXY(X - m_pCraneEntity->GetPosition().x, Y - m_pCraneEntity->GetPosition().y);
	*pDistance = ((CVector2D(X, Y) - (CVector2D)m_pCraneEntity->GetPosition())).Magnitude();
	*pHeight = Z;
}

void CCrane::CalcHookCoordinates(float* pX, float* pY, float* pZ)
{
	// Delegate the offset of the hook to the C implementation
	float hookDX, hookDY;
	Crane_HookOffset(m_fHookAngle, m_fHookOffset, &hookDX, &hookDY);
	*pX = hookDX + m_pCraneEntity->GetPosition().x;
	*pY = hookDY + m_pCraneEntity->GetPosition().y;
	*pZ = m_fHookHeight;
}

void CCrane::SetHookMatrix()
{
	if (m_pHook == nil)
		return;
	m_pHook->SetPosition(m_vecHookCurPos);
	CVector up(m_vecHookInitPos.x - m_vecHookCurPos.x, m_vecHookInitPos.y - m_vecHookCurPos.y, 20.0f);
	up.Normalise();
	m_pHook->GetRight() = CrossProduct(CVector(0.0f, 1.0f, 0.0f), up);
	m_pHook->GetForward() = CrossProduct(up, m_pHook->GetRight());
	m_pHook->GetUp() = up;
	m_pHook->SetOrientation(0.0f, 0.0f, -HALFPI);
	m_pHook->GetMatrix().UpdateRW();
	m_pHook->UpdateRwFrame();
	World_Remove(m_pHook);
	World_Add(m_pHook);
}

bool Cranes_IsThisCarBeingCarriedByAnyCrane(CVehicle* pVehicle)
{
	for (int i = 0; i < Cranes_NumCranes; i++) {
		if (pVehicle == Cranes_aCranes[i].m_pVehiclePickedUp) {
			switch (Cranes_aCranes[i].m_nCraneState) {
			case CCrane::GOING_TOWARDS_TARGET_ONLY_HEIGHT:
			case CCrane::LIFTING_TARGET:
			case CCrane::ROTATING_TARGET:
				return true;
			default:
				break;
			}
		}
	}
	return false;
}

bool Cranes_IsThisCarBeingTargettedByAnyCrane(CVehicle* pVehicle)
{
	for (int i = 0; i < Cranes_NumCranes; i++) {
		if (pVehicle == Cranes_aCranes[i].m_pVehiclePickedUp)
			return true;
	}
	return false;
}

void Cranes_Save(uint8* buf, uint32* size)
{
	INITSAVEBUF

	*size = 2 * sizeof(uint32) + CRANES_SAVE_SIZE;
	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	WriteSaveBuf(&buf, &Cranes_NumCranes, sizeof(Cranes_NumCranes));
	WriteSaveBuf(&buf, &Cranes_CarsCollectedMilitaryCrane, sizeof(Cranes_CarsCollectedMilitaryCrane));
	//- rouz edit (ChatGPT)
	for (int i = 0; i < NUM_CRANES; i++) {
#ifdef COMPATIBLE_SAVES
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		int32 tmp = Cranes_aCranes[i].m_pCraneEntity != nil ? CPool_GetJustIndex_NoFreeAssert(Pools_GetBuildingPool(), Cranes_aCranes[i].m_pCraneEntity) + 1 : 0;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &tmp, sizeof(tmp));
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		tmp = Cranes_aCranes[i].m_pHook != nil ? CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), Cranes_aCranes[i].m_pHook) + 1 : 0;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &tmp, sizeof(tmp));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupX1, sizeof(Cranes_aCranes[i].m_fPickupX1));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupX2, sizeof(Cranes_aCranes[i].m_fPickupX2));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupY1, sizeof(Cranes_aCranes[i].m_fPickupY1));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupY2, sizeof(Cranes_aCranes[i].m_fPickupY2));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_vecDropoffTarget, sizeof(Cranes_aCranes[i].m_vecDropoffTarget));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fDropoffHeading, sizeof(Cranes_aCranes[i].m_fDropoffHeading));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupAngle, sizeof(Cranes_aCranes[i].m_fPickupAngle));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fDropoffAngle, sizeof(Cranes_aCranes[i].m_fDropoffAngle));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupDistance, sizeof(Cranes_aCranes[i].m_fPickupDistance));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fDropoffDistance, sizeof(Cranes_aCranes[i].m_fDropoffDistance));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fPickupHeight, sizeof(Cranes_aCranes[i].m_fPickupHeight));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fDropoffHeight, sizeof(Cranes_aCranes[i].m_fDropoffHeight));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fHookAngle, sizeof(Cranes_aCranes[i].m_fHookAngle));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fHookOffset, sizeof(Cranes_aCranes[i].m_fHookOffset));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_fHookHeight, sizeof(Cranes_aCranes[i].m_fHookHeight));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_vecHookInitPos, sizeof(Cranes_aCranes[i].m_vecHookInitPos));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_vecHookCurPos, sizeof(Cranes_aCranes[i].m_vecHookCurPos));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_vecHookVelocity, sizeof(Cranes_aCranes[i].m_vecHookVelocity));
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		tmp = Cranes_aCranes[i].m_pVehiclePickedUp != nil ? CPool_GetJustIndex_NoFreeAssert(Pools_GetVehiclePool(), Cranes_aCranes[i].m_pVehiclePickedUp) + 1 : 0;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		WriteSaveBuf(&buf, &tmp, sizeof(tmp));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_nTimeForNextCheck, sizeof(Cranes_aCranes[i].m_nTimeForNextCheck));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_nCraneStatus, sizeof(Cranes_aCranes[i].m_nCraneStatus));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_nCraneState, sizeof(Cranes_aCranes[i].m_nCraneState));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_nVehiclesCollected, sizeof(Cranes_aCranes[i].m_nVehiclesCollected));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_bIsCrusher, sizeof(Cranes_aCranes[i].m_bIsCrusher));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_bIsMilitaryCrane, sizeof(Cranes_aCranes[i].m_bIsMilitaryCrane));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_bWasMilitaryCrane, sizeof(Cranes_aCranes[i].m_bWasMilitaryCrane));
		WriteSaveBuf(&buf, &Cranes_aCranes[i].m_bIsTop, sizeof(Cranes_aCranes[i].m_bIsTop));
		ZeroSaveBuf(&buf, 1);
		//- rouz edit (ChatGPT)
#else
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		CCrane *pCrane = (CCrane*)buf;
		*pCrane = Cranes_aCranes[i];
		SkipSaveBuf(&buf, sizeof(Cranes_aCranes[i]));
		//- rouz edit (ChatGPT)
		if (pCrane->m_pCraneEntity != nil)
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			pCrane->m_pCraneEntity = (CBuilding*)(CPool_GetJustIndex_NoFreeAssert(Pools_GetBuildingPool(), pCrane->m_pCraneEntity) + 1);
			//- rouz edit (ChatGPT)
		if (pCrane->m_pHook != nil)
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			pCrane->m_pHook = (CObject*)(CPool_GetJustIndex_NoFreeAssert(Pools_GetObjectPool(), pCrane->m_pHook) + 1);
			//- rouz edit (ChatGPT)
		if (pCrane->m_pVehiclePickedUp != nil)
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			pCrane->m_pVehiclePickedUp = (CVehicle*)(CPool_GetJustIndex_NoFreeAssert(Pools_GetVehiclePool(), pCrane->m_pVehiclePickedUp) + 1);
			//- rouz edit (ChatGPT)
#endif
	}

	VALIDATESAVEBUF(*size);
}

void Cranes_Load(uint8* buf, uint32 size)
{
	INITSAVEBUF

	//+ rouz edit (ChatGPT)
	// Transfer save data through the C buffer API with explicit sizes
	ReadSaveBuf(&Cranes_NumCranes, &buf, sizeof(Cranes_NumCranes));
	ReadSaveBuf(&Cranes_CarsCollectedMilitaryCrane, &buf, sizeof(Cranes_CarsCollectedMilitaryCrane));
	//- rouz edit (ChatGPT)
	for (int i = 0; i < NUM_CRANES; i++) {
#ifdef COMPATIBLE_SAVES
		int32 tmp;
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&tmp, &buf, sizeof(tmp));
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		Cranes_aCranes[i].m_pCraneEntity = tmp != 0 ? ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), tmp - 1)) : nil;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&tmp, &buf, sizeof(tmp));
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		Cranes_aCranes[i].m_pHook = tmp != 0 ? ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), tmp - 1)) : nil;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupX1, &buf, sizeof(Cranes_aCranes[i].m_fPickupX1));
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupX2, &buf, sizeof(Cranes_aCranes[i].m_fPickupX2));
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupY1, &buf, sizeof(Cranes_aCranes[i].m_fPickupY1));
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupY2, &buf, sizeof(Cranes_aCranes[i].m_fPickupY2));
		ReadSaveBuf(&Cranes_aCranes[i].m_vecDropoffTarget, &buf, sizeof(Cranes_aCranes[i].m_vecDropoffTarget));
		ReadSaveBuf(&Cranes_aCranes[i].m_fDropoffHeading, &buf, sizeof(Cranes_aCranes[i].m_fDropoffHeading));
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupAngle, &buf, sizeof(Cranes_aCranes[i].m_fPickupAngle));
		ReadSaveBuf(&Cranes_aCranes[i].m_fDropoffAngle, &buf, sizeof(Cranes_aCranes[i].m_fDropoffAngle));
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupDistance, &buf, sizeof(Cranes_aCranes[i].m_fPickupDistance));
		ReadSaveBuf(&Cranes_aCranes[i].m_fDropoffDistance, &buf, sizeof(Cranes_aCranes[i].m_fDropoffDistance));
		ReadSaveBuf(&Cranes_aCranes[i].m_fPickupHeight, &buf, sizeof(Cranes_aCranes[i].m_fPickupHeight));
		ReadSaveBuf(&Cranes_aCranes[i].m_fDropoffHeight, &buf, sizeof(Cranes_aCranes[i].m_fDropoffHeight));
		ReadSaveBuf(&Cranes_aCranes[i].m_fHookAngle, &buf, sizeof(Cranes_aCranes[i].m_fHookAngle));
		ReadSaveBuf(&Cranes_aCranes[i].m_fHookOffset, &buf, sizeof(Cranes_aCranes[i].m_fHookOffset));
		ReadSaveBuf(&Cranes_aCranes[i].m_fHookHeight, &buf, sizeof(Cranes_aCranes[i].m_fHookHeight));
		ReadSaveBuf(&Cranes_aCranes[i].m_vecHookInitPos, &buf, sizeof(Cranes_aCranes[i].m_vecHookInitPos));
		ReadSaveBuf(&Cranes_aCranes[i].m_vecHookCurPos, &buf, sizeof(Cranes_aCranes[i].m_vecHookCurPos));
		ReadSaveBuf(&Cranes_aCranes[i].m_vecHookVelocity, &buf, sizeof(Cranes_aCranes[i].m_vecHookVelocity));
		ReadSaveBuf(&tmp, &buf, sizeof(tmp));
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Access raw storage through the C store or pool API
		Cranes_aCranes[i].m_pVehiclePickedUp = tmp != 0 ? ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), tmp - 1)) : nil;
		//- rouz edit (ChatGPT)
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		ReadSaveBuf(&Cranes_aCranes[i].m_nTimeForNextCheck, &buf, sizeof(Cranes_aCranes[i].m_nTimeForNextCheck));
		ReadSaveBuf(&Cranes_aCranes[i].m_nCraneStatus, &buf, sizeof(Cranes_aCranes[i].m_nCraneStatus));
		ReadSaveBuf(&Cranes_aCranes[i].m_nCraneState, &buf, sizeof(Cranes_aCranes[i].m_nCraneState));
		ReadSaveBuf(&Cranes_aCranes[i].m_nVehiclesCollected, &buf, sizeof(Cranes_aCranes[i].m_nVehiclesCollected));
		ReadSaveBuf(&Cranes_aCranes[i].m_bIsCrusher, &buf, sizeof(Cranes_aCranes[i].m_bIsCrusher));
		ReadSaveBuf(&Cranes_aCranes[i].m_bIsMilitaryCrane, &buf, sizeof(Cranes_aCranes[i].m_bIsMilitaryCrane));
		ReadSaveBuf(&Cranes_aCranes[i].m_bWasMilitaryCrane, &buf, sizeof(Cranes_aCranes[i].m_bWasMilitaryCrane));
		ReadSaveBuf(&Cranes_aCranes[i].m_bIsTop, &buf, sizeof(Cranes_aCranes[i].m_bIsTop));
		SkipSaveBuf(&buf, 1);
		//- rouz edit (ChatGPT)
#else
		//+ rouz edit (ChatGPT)
		// Transfer save data through the C buffer API with explicit sizes
		// Preserve record assignment semantics and compiler padding behavior
		Cranes_aCranes[i] = *(CCrane*)buf;
		SkipSaveBuf(&buf, sizeof(Cranes_aCranes[i]));
		//- rouz edit (ChatGPT)
	}
	for (int i = 0; i < NUM_CRANES; i++) {
		CCrane *pCrane = &Cranes_aCranes[i];
		if (pCrane->m_pCraneEntity != nil)
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			pCrane->m_pCraneEntity = ((CBuilding*)CPool_GetSlot(Pools_GetBuildingPool(), (uintptr)pCrane->m_pCraneEntity - 1));
			//- rouz edit (ChatGPT)
		if (pCrane->m_pHook != nil)
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			pCrane->m_pHook = ((CObject*)CPool_GetSlot(Pools_GetObjectPool(), (uintptr)pCrane->m_pHook - 1));
			//- rouz edit (ChatGPT)
		if (pCrane->m_pVehiclePickedUp != nil)
			//+ rouz edit (ChatGPT)
			// Access raw storage through the C store or pool API
			pCrane->m_pVehiclePickedUp = ((CVehicle*)CPool_GetSlot(Pools_GetVehiclePool(), (uintptr)pCrane->m_pVehiclePickedUp - 1));
			//- rouz edit (ChatGPT)
#endif
	}

	VALIDATESAVEBUF(size);
}

//- rouz edit (ChatGPT)
