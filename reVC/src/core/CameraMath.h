//+ rouz edit (Claude)
#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "VectorMath.h"

#ifdef __cplusplus
extern "C" {
#endif
// Frustum visibility tests, the four frustum normals are the camera ones, matrices use the float[4][4] layout of CMatrix
bool CameraMath_IsPointVisible(const VectorMath3 normals[4], VectorMath3 center, const float mat[4][4]);
bool CameraMath_IsSphereVisible(const VectorMath3 normals[4], VectorMath3 center, float radius, const float mat[4][4]);
// The eight box corners are transformed in place
bool CameraMath_IsBoxVisible(const VectorMath3 normals[4], VectorMath3 box[8], const float mat[4][4]);
// Make the camera front vector unit length and build an up vector from it, never looking exactly straight up or down
void CameraMath_GetVectorsReadyForRW(VectorMath3 *front, VectorMath3 *up);
// Index of a vehicle appearance in the per-type camera arrays, index is left alone for unknown appearances
void CameraMath_GetArrPosForVehicleType(int appearance, int *index);
// State of the screen fade from its current value: 0 faded in, 1 in between, 2 faded out
int CameraMath_GetScreenFadeStatus(float fade);
// Fade direction as reported to scripts: FADE_IN, FADE_OUT or FADE_NONE when not fading
int CameraMath_GetFadingDirection(bool fading, int direction);
// Pitch the aiming camera needs so the crosshair lines up, from the z of the camera front vector, the crosshair height and the field of view in degrees
float CameraMath_Find3rdPersonQuickAimPitch(float frontZ, float crosshairMultY, float fov);
// Whether a player weapon camera mode is a first person one
bool CameraMath_Using1stPersonWeaponMode(int mode);
// Average of the last vectors fed to a camera, prev holds two vectors, pos and counter are the running state which reset restarts
VectorMath3 CameraMath_DoAverageOnVector(VectorMath3 prev[2], int *pos, int *counter, bool reset, VectorMath3 vec);
// Whether a camera target is under water, drowning or in water with its height below the water level
bool CameraMath_IsTargetUnderWater(bool drowning, bool inWater, float z, float waterZ);
// Wraps an angle in radians to the range from -PI up to but not including PI
void CameraMath_MakeAngleLessThan180(float *angle);
// Values of the previous frame a camera compares to work out how fast it moves
typedef struct CamSpeedState {
	VectorMath3 source, target, up;
	float beta, alpha, fov;
} CamSpeedState;
// Speeds of the camera over one frame from the values of this frame and the previous ones, which are then updated, the previous vectors are replaced first after a camera switch
void CameraMath_KeepTrackOfTheSpeed(CamSpeedState *st, bool justSwitched, VectorMath3 source, VectorMath3 target, VectorMath3 up, float alpha, float beta, float fov,
	VectorMath3 *sourceSpeed, VectorMath3 *targetSpeed, VectorMath3 *upSpeed, float *fovSpeed, float *betaSpeed, float *alphaSpeed);
// Near clip plane to use when a ped stands in front of the camera too close, returns false when the plane is left alone, the angles are the camera heading and the one towards the ped
bool CameraMath_NearClipForPedInFront(float fwdAngle, float pedAngle, float distX, float distY, float defaultNear, float *nearClip);
// Speed and acceleration limits and the target of the wall avoidance when a wall was hit, dy is the distance to the wall along y
void CameraMath_AvoidWallsHit(float dy, float yDistLimit, float *target, float *maxSpeed, float *acceleration);
// Camera mode of the lighthouse camera: 0 below the top, 1 looking at the player from outside, 2 looking out from the inside, timer is the running state
int CameraMath_LightHouseMode(float *timer, float targetZ, float dist2D, float timeStep);
// Marker of the spline segment a time falls in, the marker moves forward by step entries while the time is past the segment end, times in the path data are in seconds from its start time
int CameraMath_AdvanceSplineMarker(const float *pathData, int marker, uint32_t time, int step);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
