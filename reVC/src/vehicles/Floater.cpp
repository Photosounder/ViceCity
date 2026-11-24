//+ rouz edit (Claude)
#include "common.h"
#include "CollisionVector_native.h"

#include "Timer.h"
#include "WaterLevel.h"
#include "ModelIndices.h"
#include "Physical.h"
#include "Vehicle.h"
#include "Floater.h"

cBuoyancy mod_Buoyancy;

float fVolMultiplier = 1.0f;
// amount of boat volume in bounding box
// 1.0-volume is the empty space in the bbox
float fBoatVolumeDistribution[9] = {
	// rear
	0.75f, 0.9f, 0.75f,
	0.95f, 1.0f, 0.95f,
	0.4f, 0.7f, 0.4f
	// bow
};
float fBoatVolumeDistributionCat[9] = {
	0.9f, 0.3f, 0.9f,
	1.0f, 0.5f, 1.0f,
	0.95f, 0.4f, 0.95f
};
float fBoatVolumeDistributionSail[9] = {
	0.55f, 0.95f, 0.55f,
	0.75f, 1.1f, 0.75f,
	0.3f, 0.8f, 0.3f
};
float fBoatVolumeDistributionDinghy[9] = {
	0.65f, 0.85f, 0.65f,
	0.85f, 1.1f, 0.85f,
	0.65f, 0.95f, 0.65f
};
float fBoatVolumeDistributionSpeed[9] = {
	0.7f, 0.9f, 0.7f,
	0.95f, 1.0f, 0.95f,
	0.6f, 0.7f, 0.6f
};

static VectorMath3 ToC(const CVector &v)
{
	// Copy the components into the plain C record
	VectorMath3 r = {v.x, v.y, v.z};
	return r;
}

static CVector FromC(VectorMath3 v)
{
	// Copy the components back into the native vector
	return CVector(v.x, v.y, v.z);
}

static FloaterBoatKind BoatKindFromModel(int model)
{
	// Map the boat model to the C boat kind used for box tuning
	switch(model){
	case MI_SPEEDER: return FLOATER_BOAT_SPEEDER;
	case MI_REEFER: return FLOATER_BOAT_REEFER;
	case MI_RIO: return FLOATER_BOAT_RIO;
	case MI_SQUALO: return FLOATER_BOAT_SQUALO;
	case MI_TROPIC: return FLOATER_BOAT_TROPIC;
	case MI_SKIMMER: return FLOATER_BOAT_SKIMMER;
	case MI_COASTG: return FLOATER_BOAT_COASTG;
	case MI_DINGHY: return FLOATER_BOAT_DINGHY;
	case MI_MARQUIS: return FLOATER_BOAT_MARQUIS;
	case MI_JETMAX: return FLOATER_BOAT_JETMAX;
	case MI_PREDATOR:
	default: return FLOATER_BOAT_PREDATOR;
	}
}

bool
cBuoyancy::ProcessBuoyancy(CPhysical *phys, float buoyancy, CVector *point, CVector *impulse)
{
	m_numSteps = 2.0f;

	if(!CWaterLevel::GetWaterLevel(phys->GetPosition(), &m_waterlevel, phys->bTouchingWater))
		return false;
	memcpy(m_matrix, phys->GetMatrix().f, sizeof(m_matrix));

	PreCalcSetup(phys, buoyancy);
	SimpleCalcBuoyancy();
	float f = CalcBuoyancyForce(phys, point, impulse);
	if(m_isBoat)
		return true;
	return f != 0.0f;
}

bool
cBuoyancy::ProcessBuoyancyBoat(CVehicle *veh, float buoyancy, CVector *point, CVector *impulse, bool bNoTurnForce)
{
	m_numSteps = 2.0f;

	if(!CWaterLevel::GetWaterLevel(veh->GetPosition(), &m_waterlevel, veh->bTouchingWater))
		return false;
	memcpy(m_matrix, veh->GetMatrix().f, sizeof(m_matrix));
	PreCalcSetup(veh, buoyancy);


	float x, y;
	int ix, i;
	tWaterLevel waterPosition;
	CVector waterNormal;

	// Floater is divided into 3x3 parts. Process and sum each of them
	float volDiv = 1.0f/((m_dimMax.z - m_dimMin.z)*sq(m_numSteps+1.0f));
	ix = 0;
	for(x = m_dimMin.x; x <= m_dimMax.x; x += m_step.x){
		i = ix;
		for(y = m_dimMin.y; y <= m_dimMax.y; y += m_step.y){
			CVector waterLevel(x, y, 0.0f);
			FindWaterLevelNorm(FromC(m_positionZ), &waterLevel, &waterPosition, &waterNormal);
			switch(veh->GetModelIndex()){
			case MI_RIO:
				fVolMultiplier = fBoatVolumeDistributionCat[i];
				break;
			case MI_SQUALO:
			case MI_SPEEDER:
			case MI_JETMAX:
				fVolMultiplier = fBoatVolumeDistributionSpeed[i];
				break;
			case MI_COASTG:
			case MI_DINGHY:
				fVolMultiplier = fBoatVolumeDistributionDinghy[i];
				break;
			case MI_MARQUIS:
				fVolMultiplier = fBoatVolumeDistributionSail[i];
				break;
			case MI_PREDATOR:
			case MI_SKIMMER:
			case MI_REEFER:
			case MI_TROPIC:
			default:
				fVolMultiplier = fBoatVolumeDistribution[i];
				break;
			}
			if(waterPosition != FLOATER_ABOVE_WATER){
				float volume = SimpleSumBuoyancyData(waterLevel, waterPosition);
				float upImpulse = volume * volDiv * buoyancy * (*Timer_GetTimeStep());
				CVector speed = veh->GetSpeed(Multiply3x3(veh->GetMatrix(), CVector(x, y, 0.0f)));
				// Delegate the damping of the impulse to the C implementation
				float finalImpulse = Floater_DampedImpulse(upImpulse, DotProduct(speed, waterNormal), veh->pHandling->fSuspensionDampingLevel);
				impulse->z += finalImpulse;
				if(!bNoTurnForce)
					veh->ApplyTurnForce(finalImpulse*waterNormal, FromC(Floater_RotateOffset(this, ToC(waterLevel))));
			}
			i += 3;
		}
		ix++;
	}

	m_volumeUnderWater *= volDiv;

	*point = FromC(Floater_RotateOffset(this, m_impulsePoint));
	return m_isBoat || m_haveVolume;

}

void
cBuoyancy::PreCalcSetup(CPhysical *phys, float buoyancy)
{
	CColModel *colModel;

	// Delegate the box tuning and accumulator reset to the C implementation
	bool isBoat = phys->IsVehicle() && ((CVehicle*)phys)->IsBoat();
	colModel = phys->GetColModel();
	CVector dimMin = CollisionVector_ToNative(colModel->boundingBox.min);
	CVector dimMax = CollisionVector_ToNative(colModel->boundingBox.max);
	Floater_PreCalcSetup(this, ToC(dimMin), ToC(dimMax), isBoat, BoatKindFromModel(phys->GetModelIndex()), ToC(phys->GetPosition()), buoyancy);
}

void
cBuoyancy::SimpleCalcBuoyancy(void)
{
	float x, y;
	tWaterLevel waterPosition;

	// Floater is divided into 3x3 parts. Process and sum each of them
	for(x = m_dimMin.x; x <= m_dimMax.x; x += m_step.x){
		for(y = m_dimMin.y; y <= m_dimMax.y; y += m_step.y){
			CVector waterLevel(x, y, 0.0f);
			FindWaterLevel(FromC(m_positionZ), &waterLevel, &waterPosition);
			fVolMultiplier = 1.0f;
			if(waterPosition != FLOATER_ABOVE_WATER)
				SimpleSumBuoyancyData(waterLevel, waterPosition);
		}
	}

	// Delegate the normalisation to the C implementation
	Floater_NormaliseVolume(this);
}

float
cBuoyancy::SimpleSumBuoyancyData(CVector &waterLevel, tWaterLevel waterPosition)
{
	// Delegate to the C implementation
	return Floater_SumBuoyancyData(this, ToC(waterLevel), fVolMultiplier);
}

void
cBuoyancy::FindWaterLevel(const CVector &zpos, CVector *waterLevel, tWaterLevel *waterPosition)
{
	// waterLevel is a local x,y point
	// m_position is the global position of our floater
	// zpos is the global z coordinate of our floater
	CVector xWaterLevel = FromC(Floater_RotateOffset(this, ToC(*waterLevel)));
	CWaterLevel::GetWaterLevel(xWaterLevel.x + m_position.x, xWaterLevel.y + m_position.y, m_position.z,
		&waterLevel->z, true);
	waterLevel->z -= xWaterLevel.z + zpos.z;	// make local
	*waterPosition = (tWaterLevel)Floater_ClampWaterLevel(this, &waterLevel->z);
}

// Same as above but also get normal
void
cBuoyancy::FindWaterLevelNorm(const CVector &zpos, CVector *waterLevel, tWaterLevel *waterPosition, CVector *normal)
{
	CVector xWaterLevel = FromC(Floater_RotateOffset(this, ToC(*waterLevel)));
	CWaterLevel::GetWaterLevel(xWaterLevel.x + m_position.x, xWaterLevel.y + m_position.y, m_position.z,
		&waterLevel->z, true);
	waterLevel->z -= xWaterLevel.z + zpos.z;	// make local
	if(waterLevel->z >= m_dimMin.z)
		*normal = CWaterLevel::GetWaterNormal(xWaterLevel.x + m_position.x, xWaterLevel.y + m_position.y);
	*waterPosition = (tWaterLevel)Floater_ClampWaterLevel(this, &waterLevel->z);
}

bool
cBuoyancy::CalcBuoyancyForce(CPhysical *phys, CVector *point, CVector *impulse)
{
	// Delegate to the C implementation
	VectorMath3 p, i;
	if(!Floater_CalcForce(this, *Timer_GetTimeStep(), &p, &i))
		return false;
	*point = FromC(p);
	*impulse = FromC(i);
	return true;
}
//- rouz edit (Claude)
