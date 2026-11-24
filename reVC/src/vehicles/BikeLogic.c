//+ rouz edit (Claude)
#include <stddef.h>
#include "BikeLogic.h"
#include "General.h"
#include <math.h>
#include <string.h>
#include "WheelState.h"
#include "SurfaceTypes.h"
#include "MatrixMath.h"
#include "VectorMath.h"

bool Bike_IsComponentPresent(const CBikeData *self, int component)
{
	// The component exists when its frame does
	return self->m_aBikeNodes[component] != NULL;
}

void Bike_ResetSuspension(CBikeData *self)
{
	int i;

	// Stopped wheels
	for (i = 0; i < 2; i++) {
		self->m_aWheelRotation[i] = 0.0f;
		self->m_aWheelState[i] = WHEEL_STATE_NORMAL;
	}

	// Fully extended springs
	for (i = 0; i < 4; i++) {
		self->m_aSuspensionSpringRatio[i] = 1.0f;
		self->m_aWheelTimer[i] = 0.0f;
	}
}

void Bike_ClearNodes(CBikeData *self)
{
	// Forget all the frames
	int i;
	for (i = 0; i < BIKE_NUM_NODES; i++)
		self->m_aBikeNodes[i] = NULL;
}

void Bike_RemoveRefsToVehicle(CBikeData *self, const CEntity *ent)
{
	// Forget the ground entity if it is the removed one
	int i;
	for (i = 0; i < 4; i++)
		if ((const void*)self->m_aGroundPhysical[i] == (const void*)ent)
			self->m_aGroundPhysical[i] = NULL;
}
void Bike_CalculateLeanMatrix(CMatrixData *leanMatrix, float leanAngle, const float *up, float minZ)
{
	float mat[4][4];
	float result[4][4];

	// Roll forwards a little with the lean and lean sideways
	MatrixMath_SetRotateXOnly(mat, -0.05f*fabsf(leanAngle));
	mat[3][0] = 0.0f;
	mat[3][1] = 0.0f;
	mat[3][2] = 0.0f;
	MatrixMath_RotateY(mat, leanAngle);

	// Apply it to the matrix of the bike
	MatrixMath_Multiply(result, (const float (*)[4])leanMatrix->f, (const float (*)[4])mat);
	memcpy(leanMatrix->f, result, sizeof(result));

	// Place the wheel back on the ground
	leanMatrix->f[3][0] += up[0]*(1.0f-cosf(leanAngle))*minZ;
	leanMatrix->f[3][1] += up[1]*(1.0f-cosf(leanAngle))*minZ;
	leanMatrix->f[3][2] += up[2]*(1.0f-cosf(leanAngle))*minZ;
}
void Bike_GetCorrectedWorldDoorPosition(const float *forward, const float *up, const float *position, float maxX, float maxZ, const float *p1, const float *p2, float *out)
{
	VectorMath3 fwd = {forward[0], forward[1], forward[2]};
	VectorMath3 worldUp = {0.0f, 0.0f, 1.0f};
	VectorMath3 rightWorld = VectorMath_Cross(fwd, worldUp);
	VectorMath3 upWorld = VectorMath_Cross(rightWorld, fwd);
	float onSide = up[0]*rightWorld.x + up[1]*rightWorld.y + up[2]*rightWorld.z;
	float diff = maxZ - maxX;
	float x, y, z;

	// A leaning bike moves the point sideways by the difference of the box sizes
	diff = diff > 0.0f ? diff : 0.0f;
	y = p2[1] - p1[1];
	x = onSide*diff + p2[0] + p1[0];
	z = p2[2] - p1[2];
	out[0] = x*rightWorld.x + y*fwd.x + z*upWorld.x + position[0];
	out[1] = x*rightWorld.y + y*fwd.y + z*upWorld.y + position[1];
	out[2] = x*rightWorld.z + y*fwd.z + z*upWorld.z + position[2];
}
void BikeLogic_Pedals(float speed, int16_t accelerate, int16_t brake, bool isDodo, float *gasPedal, float *brakePedal, bool *burnout)
{
	*burnout = false;

	float acceleration = (accelerate - brake)/255.0f;
	if(isDodo && acceleration < 0.0f)
		acceleration *= 0.3f;
	if(fabsf(speed) < 0.01f){
		// standing still, go into direction we want
		if(accelerate > 150.0f && brake > 150.0f){
			*gasPedal = accelerate/255.0f;
			*brakePedal = brake/255.0f;
			*burnout = true;
		}else{
			*gasPedal = acceleration;
			*brakePedal = 0.0f;
		}
	}else{
		if(speed * acceleration < 0.0f){
			// if opposite directions, have to brake first
			*gasPedal = 0.0f;
			*brakePedal = fabsf(acceleration);
		}else{
			// accelerating in same direction we were already going
			*gasPedal = acceleration;
			*brakePedal = 0.0f;
		}
	}
}

float BikeLogic_LeanInput(float leanInput, float updown, float leanMul, float timeStep)
{
	leanInput += (updown - leanInput)*0.2f*timeStep * leanMul;
	float low = -1.0f*leanMul, high = 1.0f*leanMul;
	return leanInput < low ? low : leanInput > high ? high : leanInput;
}

void BikeLogic_ComedyControls(uint32_t timeMs, float *gasPedal, float *brakePedal, float *steerAngle)
{
	if(((timeMs >> 10) & 0xF) < 12)
		*gasPedal = 1.0f;
	if((((timeMs >> 10)+6) & 0xF) < 12)
		*brakePedal = 0.0f;
	if(timeMs & 0x800)
		*steerAngle += 0.08f;
	else
		*steerAngle -= 0.03f;
}

int Bike_KnockOffRiderObjective(bool temperAboveFear, bool pedNotMission, bool bikeNotMission, bool playerGoesForBike, bool playerNotOnMission)
{
	if (temperAboveFear && pedNotMission && bikeNotMission && playerGoesForBike && playerNotOnMission)
		return BIKE_KNOCKOFF_ENTER_AS_DRIVER;
	else if (temperAboveFear && pedNotMission && bikeNotMission && playerNotOnMission)
		return BIKE_KNOCKOFF_ENTER_AS_DRIVER;
	else if (!temperAboveFear && pedNotMission && bikeNotMission && playerNotOnMission)
		return BIKE_KNOCKOFF_WANDER;
	return BIKE_KNOCKOFF_NOTHING;
}

float Bike_KnockOffHeading(float upZ, float bikeHeading)
{
	if (upZ < 0.0f)
		return General_LimitRadianAngle(bikeHeading + (float)3.14159265358979323846);
	return bikeHeading;
}

float Bike_KnockOffForceXY(float damageImpulse, float pedMass, float bikeMass)
{
	return -0.6 * damageImpulse * pedMass / bikeMass;
}
void Bike_WheelBiases(bool neutralHandling, float brakeBias, float tractionBias, float biases[4])
{
	float tractionBiasFront = neutralHandling ? 1.0f : 2.0f * tractionBias;

	biases[0] = neutralHandling ? 1.0f : 2.0f * brakeBias;
	biases[1] = neutralHandling ? 1.0f : 2.0f * (1.0f - brakeBias);
	biases[2] = tractionBiasFront;
	biases[3] = neutralHandling ? 1.0f : 2.0f - tractionBiasFront;
}
float Bike_LeanForce(float leanInput, float turnMass, float leanForce, float moveSpeed)
{
	return leanInput * turnMass * leanForce * (moveSpeed < 0.1f ? moveSpeed : 0.1f);
}

float Bike_LeanBackGasScale(bool isSanchez, float gasPedal)
{
	return isSanchez ? 0.7f * gasPedal + 0.3f : 0.5f * gasPedal + 0.5f;
}
void Bike_TurnResistance(bool isPlayer, bool frontOffGround, bool rearOffGround, float forwardZ, float resX, float resY, float dAxisX, float dAxisXExtra, float dAxisY, float inAirXRes, float wheelieAng, float stoppieAng, float localTurnX, float localTurnY, float timeStep, float turn[2])
{
	float dx = dAxisX;

	if (isPlayer) {
		if (frontOffGround) {
			float a;

			dx = dAxisXExtra;
			if (!rearOffGround && forwardZ > 0.0f) {
				a = 0.25f * fabsf(wheelieAng - forwardZ);
				resX -= a < 0.07f ? a : 0.07f;
			} else
				resX = inAirXRes;
		} else if (rearOffGround) {
			dx = dAxisXExtra;
			if (forwardZ < 0.0f) {
				float a = 0.3f * fabsf(stoppieAng - forwardZ);
				resX *= (a < 0.1f ? a : 0.1f) + 0.9f;
			}
		}
	}

	resX *= 1.0f / (dx * (localTurnX * localTurnX) + 1.0f);
	resY *= 1.0f / (dAxisY * (localTurnY * localTurnY) + 1.0f);
	resX = powf(resX, timeStep);
	resY = powf(resY, timeStep);
	turn[0] = localTurnX * (resX - 1.0f);
	turn[1] = localTurnY * (resY - 1.0f);
}
float Bike_WheelAngle(bool followsSteering, float moveSpeedX, float moveSpeedY, float steerAngle, float wheelAngle, float leanAngle, float fwdSpeed, bool isPlayer, bool limitedByGrip, float gripSteer, float steeringLock, float timeStep)
{
	const float pi = (float)3.14159265358979323846;

	if (!followsSteering) {
		if (wheelAngle < 20.0f * pi / 180.0f)
			wheelAngle += 1.5f * pi / 180.0f * timeStep;
		return wheelAngle;
	}
	if (fabsf(moveSpeedX) < 0.01f && fabsf(moveSpeedY) < 0.01f && steerAngle == 0.0f)
		return wheelAngle * powf(0.96f, timeStep);

	float f = 1.0f;
	if (limitedByGrip && isPlayer) {
		float q = gripSteer / (fwdSpeed * fwdSpeed);
		f = asinf(q < 1.0f ? q : 1.0f) / (steeringLock * pi / 180.0f);
		if ((steerAngle < 0.0f && leanAngle < 0.0f) || (steerAngle > 0.0f && leanAngle > 0.0f))
			f *= 2.0f;
		f = f < 1.0f ? f : 1.0f;
	}
	return steerAngle * f;
}
float Bike_WheelAdhesion(float adhesiveLimit, float traction, float brakeDestabilization, int adhesionGroup, bool isPlayer, float wetMultiplier, bool burst, float *destab)
{
	float adhesion = adhesiveLimit * traction;

	*destab = 1.0f;
	if (brakeDestabilization > 0.0f) {
		switch (adhesionGroup) {
		case ADHESIVE_HARD:
		case ADHESIVE_LOOSE:
			*destab = 0.9f;
			break;
		case ADHESIVE_ROAD:
			*destab = 0.7f;
			break;
		}
	}
	if (isPlayer)
		adhesion *= wetMultiplier;
	if (burst)
		adhesion *= 0.4f;
	return adhesion;
}
float Bike_SurfaceShake(int surface, float currentShake)
{
	if (surface == SURFACE_GRAVEL || surface == SURFACE_WATER || surface == SURFACE_HEDGE) {
		if (currentShake < 0.2f)
			currentShake = 0.3f;
	} else if (surface == SURFACE_MUD_DRY || surface == SURFACE_SAND || surface == SURFACE_SAND_BEACH) {
		if (currentShake < 0.1f)
			currentShake = 0.2f;
	} else if (surface == SURFACE_GRASS) {
		if (currentShake < 0.05f)
			currentShake = 0.1f;
	}
	return currentShake;
}
float Bike_WheelieStabilisation(float wheelieAng, float forwardZ, float stabMult, float speed)
{
	float wheelie = wheelieAng - forwardZ;

	if (wheelie > 0.15f) {
		// Below the wheelie angle
		wheelie = 0.3f - wheelie;
		wheelie = wheelie > 0.0f ? wheelie : 0.0f;
	} else if (wheelie < -0.08f) {
		// Above the wheelie angle
		wheelie = -0.15f - wheelie;
		wheelie = wheelie < 0.0f ? wheelie : 0.0f;
	}
	return stabMult * (speed < 0.1f ? speed : 0.1f) * wheelie;
}

float Bike_StoppieStabilisation(float stoppieAng, float forwardZ, float stabMult, float speed)
{
	float stoppie = stoppieAng - forwardZ;

	if (stoppie > 0.15f) {
		// Below the stoppie angle
		stoppie = 0.3f - stoppie;
		stoppie = stoppie > 0.0f ? stoppie : 0.0f;
	} else if (stoppie < -0.15f) {
		// Above the stoppie angle
		stoppie = -0.3f - stoppie;
		stoppie = stoppie < 0.0f ? stoppie : 0.0f;
	}
	return stabMult * (speed < 0.1f ? speed : 0.1f) * stoppie;
}
float Bike_LeanAngle(bool heldByRider, bool isStanding, int wheelsOnGround, float steerAngle, float steeringLock, float sideAccel, bool frontBurst, float maxLean, float desLean, float idleAngle, float rightZ, float leanAngle, float gravity, float timeStep)
{
	const float pi = (float)3.14159265358979323846;
	float f, lean, limit;

	if (heldByRider) {
		if (wheelsOnGround == 0)
			lean = -(steerAngle / (steeringLock * pi / 180.0f)) * 0.5f * gravity * timeStep;
		else
			lean = sideAccel;
		lean /= gravity * (timeStep > 0.01f ? timeStep : 0.01f);
		limit = frontBurst ? 0.4f * maxLean : maxLean;
		lean = lean < -limit ? -limit : lean > limit ? limit : lean;
		f = powf(desLean, timeStep);
		return (asinf(lean) - idleAngle) * (1.0f - f) + leanAngle * f;
	}
	if (isStanding) {
		f = powf(0.97f, timeStep);
		return leanAngle * f - (asinf(rightZ) + 15.0f * pi / 180.0f + idleAngle) * (1.0f - f);
	}
	f = powf(0.95f, timeStep);
	return leanAngle * f;
}
float Bike_BrakeDestabilization(float destabilization, float random, float timeStep)
{
	return destabilization + random * 0.2f * timeStep;
}

float Bike_BrakeDestabForce(float turnMass, float destabilization, float fwdSpeed, bool rearOnGround)
{
	// The original clamp has its limits swapped, so it gives 0.5 below 0.5 and 0.2 otherwise
	float clamped = fwdSpeed < 0.5f ? 0.5f : fwdSpeed > 0.2f ? 0.2f : fwdSpeed;

	return turnMass * sinf(destabilization) * clamped * (rearOnGround ? 0.013f : 0.003f);
}
int Bike_RearWheelGrip(bool handbrakeOn, float handbrakeBrake, bool burnout, float tireTemperature, float gasPedal, float brake, float traction, float *rearBrake, float *rearTraction)
{
	*rearBrake = brake;
	*rearTraction = traction;
	if (handbrakeOn) {
		*rearBrake = handbrakeBrake;
		return BIKE_REARGRIP_HANDBRAKE;
	}
	if (burnout) {
		*rearBrake = 0.0f;
		*rearTraction = 0.0f;
		return BIKE_REARGRIP_BURNOUT;
	}
	if (tireTemperature < 1.0f && gasPedal > 0.75f) {
		*rearTraction *= tireTemperature;
		return BIKE_REARGRIP_COLD_TYRES;
	}
	return BIKE_REARGRIP_NORMAL;
}

float Bike_RearWheelAirSpeed(float wheelSpeed, float acceleration, bool handbrakeOn)
{
	if (handbrakeOn)
		return 0.0f;
	if (acceleration > 0.0f) {
		if (wheelSpeed < 2.0f)
			wheelSpeed -= 0.2f;
	} else {
		if (wheelSpeed > -2.0f)
			wheelSpeed += 0.1f;
	}
	return wheelSpeed;
}
bool Bike_IsDrowning(float impulseRatio, bool anySuspensionFullyExtended)
{
	return impulseRatio > 0.8f || (impulseRatio > 0.4f && anySuspensionFullyExtended);
}
int Bike_BurstWheelSparkCount(float speedSqr, int surface)
{
	int count;

	if (!(speedSqr > (0.1f * 0.1f)) || surface == SURFACE_GRASS || surface == SURFACE_MUD_DRY ||
		surface == SURFACE_SAND || surface == SURFACE_SAND_BEACH)
		return 0;

	count = 1;
	if (speedSqr > 0.04f)
		count++;
	if (speedSqr > 0.16f)
		count += 2;
	return count;
}

void Bike_TailLightGlow(float behindness, bool braking, float *intensity, float *size)
{
	*intensity = 0.4f * behindness + 0.4f;
	*size = (behindness + 1.0f) / 2.0f;
	if (braking) {
		*intensity += 0.4f;
		*size += 0.3f;
	}
}
float Bike_WheelContactZ(float lineTopZ, float springLength, float springRatio, float wheelScale)
{
	return lineTopZ - springLength * springRatio - 0.5f * wheelScale;
}

void Bike_FrontWheelPos(float lineY1, float lineY2, float wheelPos, float wheelBasePos, float forkZ, float forkY, float forkSlope, float *y, float *z)
{
	float lineY = (lineY1 + lineY2) / 2.0f - forkY;

	*z = wheelPos - forkZ;
	*y = lineY - (wheelPos - wheelBasePos) * forkSlope;
}

float Bike_RearForkAngle(float wheelPos, float wheelBasePos, float forkLength)
{
	float sine = (wheelPos - wheelBasePos) / forkLength;

	return -asinf(sine);
}

float Bike_ChassisOffsetZ(float leanAngle, float boundingMinZ)
{
	return (1.0f - cosf(leanAngle)) * (0.9 * boundingMinZ);
}
float Bike_HeightAboveRoad(float springLength, float suspensionForceLevel, float lineTopZ, float wheelScale)
{
	return springLength * (1.0f - 1.0f / (4.0f * suspensionForceLevel)) - lineTopZ + wheelScale * 0.5f;
}

float Bike_RearForkLength(float dy, float dz)
{
	return sqrtf(dy * dy + dz * dz);
}

bool Bike_BurstThrowsRider(float speedSqr, bool isPlayer, float fallSpeed, float fallSpeedPlayer)
{
	return speedSqr > fallSpeed && (!isPlayer || speedSqr > fallSpeedPlayer);
}
float Bike_CollisionSpeed(float impulse, float mass, bool isPlayer, bool isMissionVehicle)
{
	float colSpeed = 800.0f * impulse / mass;

	if (isPlayer)
		colSpeed *= 0.65f;
	else if (isMissionVehicle)
		colSpeed *= 0.4f;
	return colSpeed;
}

float Bike_RiderDamage(float dotForward, float dotRight, float dotUp, float normalZ, bool isSanchez, float colSpeed)
{
	float fwd = 0.6f;
	float up = 0.05f;
	float total;

	if (fabsf(dotForward) > 0.85f) {
		float u = normalZ > 0.0f ? normalZ : 0.0f;
		if (u < 0.85f)
			u = 0.0f;
		fwd += 7.0f * (u * u);
	}

	if (isSanchez) {
		fwd *= 0.65f;
		up *= 0.75f;
	}

	total = fwd * fabsf(dotForward) +
		0.45f * fabsf(dotRight) +
		up * (dotUp > 0.0f ? dotUp : 0.0f);
	return (total - 1.5f * (dotUp < 0.0f ? dotUp : 0.0f)) * colSpeed;
}
bool Bike_SteamSkipsFrame(int counter)
{
	return counter < 10 ||
		(counter < 70 && counter > 25) ||
		(counter < 160 && counter > 100) ||
		(counter < 200 && counter > 175) ||
		counter > 235;
}

bool Bike_SteamFrame(int counter)
{
	return (counter & 3) == 0 || (counter & 3) == 2;
}
float Bike_LeanSmokeOffset(float leanAngle, float boundingMinZ)
{
	return sinf(leanAngle) * 0.8f * boundingMinZ;
}

float Bike_ViewAngle(float behindness)
{
	return fabsf(acosf(fabsf(behindness)));
}
float Bike_BurstWheelWobble(float rotation, float amplitude)
{
	return amplitude * sinf(rotation);
}

float Bike_ChassisRoll(float leanAngle)
{
	return -0.05f * fabsf(leanAngle);
}

void Bike_ForkAxis(float steerAngleDegrees, float *y, float *z)
{
	const float pi = (float)3.14159265358979323846;
	float a = steerAngleDegrees * pi / 180.0f;

	*y = sinf(a);
	*z = -cosf(a);
}
//- rouz edit (Claude)
