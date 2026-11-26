//+ rouz edit (Claude)
#pragma once
#include "VectorFwd.h" // rouz edit (Claude)
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
// Smoothed fraction of a transition of the camera between 0 and 1, from the part of the transition that is done (numerator) and the length of the transition (denominator), 0 when the denominator is 0
float Camera_SmoothInterpolation(float numerator, float denominator);
// Direction from the player in which a fixed camera of the cinematic camera is placed: the speed of the player (flattened and normalised first when flatten is set) turned by an angle and normalised, dir receives x, y and z
void Camera_ObbeOffsetDirection(float speedX, float speedY, float speedZ, float angleOffset, bool flatten, CVector *dir);
// How much of a shake is felt at a distance from the camera, from the vector between the camera and the source of the shake: 1 on the camera down to 0 at 100 units
float Camera_ShakeDistanceMultiplier(float dx, float dy, float dz);
// Whether a new shake replaces the current one, from the multiplier of the distance (1 when there is none), the force of the current shake, the time in milliseconds since it started and the strength of the new one
bool Camera_ShakeReplaces(float mult, float shakeForce, uint32_t elapsed, float strength);
// Normals of the right, left, bottom and top planes of the view frustum, from half the horizontal field of view in radians and the aspect ratio of the screen
void Camera_FrustumNormals(float halfFov, float aspectRatio, float normals[4][3]);
// Direction the camera looks in on the ground, normalised, from the x and y of its forward vector, x is 1 when it is null
void Camera_FrontNormalised(float frontX, float frontY, float *normX, float *normY);
// Direction from the camera to its target and the distance between them, for the avoidance of the geometry: from the vector between the camera and the target and the forward vector of the camera, which is used when the vector points straight up or down
void Camera_AvoidFront(float distX, float distY, float distZ, float forwardX, float forwardY, CVector *front, float *dist);
// Near clip plane the camera needs to keep a geometry that is in front of it out of the view, from the distance to that geometry along the view, the closest the plane may get, the default plane and the current plane: it is only moved closer
float Camera_AvoidNearClip(float frontDist, float closeLimit, float defaultNear, float currentNear);
// Width of the view plane the camera keeps free of geometry, from the field of view in degrees, the aspect ratio and the tweak of the field of view
float Camera_AvoidViewPlaneWidth(float fov, float aspectRatio, float tweakFOV);
// Timer of the problems with the geometry behind a ped after a frame, from the timer, how much the target faces the geometry (dot product of the normal and its forward vector) and the time step: it counts up when the ped faces the geometry and down when it faces away
float Camera_GeomProbsTimer(float timer, float targetNormalDir, float timeStep);
// Offset on the ground of a camera that orbits its target at a distance and an angle (x with the cosine, y with the sine)
void Camera_OrbitOffset(float distance, float angle, float *x, float *y);
// Direction of the camera from its vertical angle (alpha) and its horizontal angle (beta): (cos a cos b, cos a sin b, sin a)
void Camera_SphericalDirection(float alpha, float beta, CVector *dir);
// Up vector of a camera that is rolled by an angle: (cos(roll + half a turn), 0, sin(roll + half a turn)) with the roll in radians
void Camera_UpFromRoll(float roll, CVector *up);
// Roll the camera of a car tilts to, from whether the pad rolls it (dPad), whether it rolls to the left, the speed of the car along its forward vector, the x of the left stick, the overshoot of the tilt of the vehicle type, the largest roll angle and the dot product of the forward vector of the car with the direction of the camera
float Camera_CarTargetRoll(bool dPad, bool dPadLeft, float moveSpeedAlongForward, float leftStickX, float tiltOvershoot, float maxRollAngle, float forwardDotFront);
// Change of the vertical angle of the ped camera that brings it back to its default angle by at most 0.05 a frame, from the angle and the default angle
float Camera_AlphaRecentre(float alpha, float defaultAlpha);
// Change of the angles of the ped camera from the look around of a pad stick: betaOffset and alphaOffset, from the stick values, the sensitivity, the field of view and the time step
void Camera_StickLookOffsets(float lookLeftRight, float lookUpDown, float stickSens, float fov, float timeStep, float *betaOffset, float *alphaOffset);
// Distance of the mouse camera to its target after a frame, from the distance to the target and the current distance: it shrinks at once and grows back at a rate that depends on the time step; returns whether it grows back
bool Camera_MouseAvoidDistance(float targetDist, float returnRate, float timeStep, float *distance);
// Vertical angle of the camera behind a helicopter: it follows the pitch of the helicopter more the faster it flies, from the x and y part of its forward vector (forwardXY), its z part and its speed along it in game units
float Camera_HeliAlpha(float forwardXY, float forwardZ, float fwdSpeed);
// Vertical angle of the camera behind a vehicle limited to between 0 and 89 degrees (with a wrap of the angle first)
float Camera_ClampCarAlpha(float carAlpha);
// How far the camera moves sideways to follow the turret of a firetruck that fires, from the angle of the turret (targetBeta), the angle of the camera, the distance to the target and the tracking multiplier
float Camera_FiretruckTracking(float targetBeta, float beta, float distance, float trackingMult);
// Average speed of the ped followed by the top down camera after five frames, from the previous average and the sum of the speeds of the five frames
float Camera_PedAverageSpeed(float average, float speedSum);
// Height the top down camera zooms out to for a ped that moves at an average speed: 0 when it stands still, higher the faster it moves
float Camera_TopDownHeightTarget(float pedAverageSpeed);
// Point 3 units away from the camera in the direction given by a vertical angle (alpha) and a horizontal angle (beta), from the position of the camera
void Camera_LookPoint(float alpha, float beta, float sourceX, float sourceY, float sourceZ, CVector *point);
// Change of an angle of a first person camera from the movement of the mouse, from the acceleration of the mouse, the movement and the field of view
float Camera_MouseLookDelta(float mouseAccel, float look, float fov);
// Change of an angle of a first person camera from a pad stick, which grows with the square of the stick, from the stick value, its divisor, the sign of the stick, the scale of the axis, the field of view and the time step
float Camera_PadLookDelta(float look, float divisor, float direction, float scale, float fov, float timeStep);
// Vertical angle of a camera limited to between minus 89.5 degrees and the given maximum in degrees
float Camera_ClampAlpha(float alpha, float maxDegrees);
// Speed of the debug camera along an axis after a frame: it speeds up or slows down while a key is held (by 0.1 a frame), stops when none is and is limited to 70
float Camera_DebugSpeed(float speed, bool speedUp, bool slowDown);
// Near clip plane that keeps a collision out of the view, from the current plane and the distance of the collision from the middle of the view in units of the view plane: between 0.1 and the current plane
float Camera_NearClipFromCollision(float currentNear, float dist);
// Field of view of the chase camera of a car after a frame: it widens when the car goes faster than 0.4 along its direction, falls back to the default field of view at a rate that depends on the time step and stays within 30 degrees of it
float Camera_CarFOV(float fov, float defaultFOV, bool widens, float forwardSpeed, float timeStep);
// Value limited to between a lower and an upper limit, the upper limit is tested first
float Camera_ClampBetween(float value, float low, float high);
// Vertical angle of a camera limited to between a lower and an upper limit, the speed of the angle is set to 0 when it hits a limit; the upper limit is tested first
float Camera_ClampAlphaStop(float alpha, float low, float high, float *alphaSpeed);
// Position behind a target the camera of a car would be at, from the target, the horizontal angle (beta), the vertical angle (alpha) of the camera and its distance from the target
void Camera_TargetHistoryPos(const CVector *target, float beta, float alpha, float distance, CVector *pos);
// Angle a turret turns to in a frame to face where the camera looks: the angle it should face is wrapped to within half a turn of the current angle of the turret (gunLR) and the turn is limited to turnPerFrame
float Camera_TurretFace(float angleToFace, float gunLR, float turnPerFrame);
// Vertical angle of a firetruck turret after a frame, from the angle it should face, the current angle and the step per frame: it moves by at most the step and stays within 20 degrees
float Camera_TurretAlpha(float alphaToFace, float gunUD, float alphaTurnPerFrame);
// Movement of the LCS-style car camera along an axis from a pad stick, which grows with the square of the stick, from the stick, the field of view and the scale of the axis (5 horizontally, 3 vertically)
float Camera_StickMovement(float stick, float fov, float scale);
// Horizontal angular speed of the LCS-style car camera after a frame: it blends from the current speed to the target speed by the step and snaps to 0 below 0.0001
float Camera_BlendBetaSpeed(float targetBlendAmount, float angleChangeStep, float betaSpeed);
// Speed variable of the camera that follows a vehicle after a frame: from the x and y of the forward vector of the vehicle, the x and y of its speed and the current speed variable; targetSpeedVar receives the target the speed variable blends to (positive going forward, negative when reversing) and the new speed variable is returned
float Camera_VehicleSpeedVar(float forwardX, float forwardY, float moveSpeedX, float moveSpeedY, float speedVar, float *targetSpeedVar);
// Horizontal angle the syphon camera has to turn by to see past something that is between it and the target: from how far the line to the collision is from the line to the target, the distance of the collision from the target, the distance of the player from the target and the distance of the camera on the ground
float Camera_SyphonBetaOffset(float lineDist, float colDist, float playerToTargetDist, float groundDist);
// Direction a camera looks in from its vertical (alpha) and horizontal (beta) angles, with the horizontal angle pointing from the target to the camera: (-(cos b cos a), -(sin b cos a), sin a)
void Camera_FrontFromAngles(float alpha, float beta, CVector *front);
// Point the debug and editor cameras look at, 7 units ahead on the ground and zScale units up, from the vertical (alpha) and horizontal (beta) angles, with beta measured from the y axis, and the position of the camera
void Camera_DebugTarget(float alpha, float beta, float sourceX, float sourceY, float sourceZ, float zScale, CVector *target);
// Horizontal angle of a vector, 0 for a vector that has no horizontal part
float Camera_BetaOfVector(float x, float y);
// Step of a buffered value that follows a target: the speed accelerates towards the target speed (proportionally to the distance) without overshooting it and the value moves by the speed over the time step (limited to 10)
void Camera_BufferStep(float *value, float *speed, float targetSpeed, float acceleration, float timeStep);
// Step of the horizontal angle of a follow camera: the speed accelerates towards the requested speed by the step times the absolute difference, without overshooting, and the angle moves by the speed over the time step (limited to 10)
void Camera_FollowBetaStep(float *beta, float *betaSpeed, float reqSpeed, float speedStep, float timeStep);
// Distance of the third person camera from a ped, from the base distance, the angle and the falloff and angle distance of the angle (when above the threshold the angle is scaled by the falloff and limited to a quarter turn)
float Camera_PedCamDistance(float baseDist, float alpha, float falloff, float angleDist, bool useFalloff);
// Height of the test point behind a helicopter in the close-in car camera, from the target height, the length, the angle and its offset and the height offset
float Camera_HeliTestHeight(float targetZ, float length, float alpha, float alphaOffset, float heightOffset);
// Angle of the close-in car camera that puts it just above the ground hit by the test line, from the height of the ground, the target height, the height offset, the length and the angle offset
float Camera_AngleAboveGround(float groundZ, float targetZ, float heightOffset, float length, float alphaOffset);
// Height of the source of the close-in car camera, from the target height, the length, the angle and its offset and the height offset
float Camera_CarSourceHeight(float targetZ, float length, float alpha, float alphaOffset, float heightOffset);
// Whether a camera that is behind a vehicle that moves forward must be turned: the angle to the wanted direction is larger than half a turn minus the maximum difference
bool Camera_NeedsBetaFix(float deltaBeta, float maxDiffBeta);
// Whether a camera that is turning behind a vehicle has arrived: the angle to the wanted direction is below 2 degrees
bool Camera_BetaFixDone(float deltaBeta);
// Change of an angle of the helicopter cannon camera from the stick (already divided by 128), the sensitivity (0.56 horizontal, 0.48 vertical), the field of view and the time step: it grows with the square of the stick
float Camera_HeliCannonLookDelta(float look, float sensitivity, float fov, float timeStep);
// Height the third person aiming camera rises by, from the distance to the ground, the angle and its offset
float Camera_GroundHeightGain(float groundDist, float alpha, float alphaOffset);
// Field of view of the sniper and camera modes limited to the default field of view and to a minimum of 3 degrees (camera mode) or 15 degrees (the others)
float Camera_LimitSniperFOV(float fov, float defaultFOV, bool isCameraMode);
// Factor the draw distance and the near clip plane of the camera are scaled by when it is high above the ground in a plane or helicopter: 1 plus the height above 55 units over 60
float Camera_HighAltitudeScale(float cameraZ);
// Zoom value of the camera after a frame of smoothing towards a target at a speed of 0.12 per time step, without overshooting it
float Camera_ZoomStep(float zoom, float target, float timeStep);
// Strength of the shake of the camera, from the force and the start and current times in milliseconds: it decays over time and is limited to 0..2
float Camera_ShakeStrength(float force, uint32_t startMs, uint32_t nowMs);
// Offsets of the camera position by the shake, from its strength and a random number (4 bits for each axis)
void Camera_ShakeOffsets(float strength, int rand, CVector *offsets);
// Multiplier of the draw distances from the field of view of the camera: 70 over the field of view, scaled by the altitude when the camera is high up in a plane or helicopter, limited to 2.2
float Camera_LODDistMultiplier(float fov, bool highAltitude, float cameraZ);
// Smoothing factor of a frame for a smoothing constant per time unit: one minus the constant raised to the power of the time step
float Camera_SmoothingFactor(float constantInv, float timeStep);
// Whether a ped that moves relative to the camera will come between the camera and the player soon: it approaches at more than 0.01 and arrives within 75 time units (7.5 when the camera is already lowered)
bool Camera_PedWillBlockView(float dist, float speedDiff, float heightOffset);
// Value limited to 0..1
float Camera_Clamp01(float a);
// Value of a cubic Bezier spline between the first and last of four control values at the fraction a (0 to 1)
float Camera_BezierBlend(float a, float p0, float p1, float p2, float p3);
// Position in a camera spline at a time: advances the marker past frames that are too close in time, then returns the fraction between the marker frame and the one before it limited to 0..1 (the spline has numFrames frames of stride floats each, the first float is the number of frames); markerOutOfRange is set when the marker is past the end of the spline
float Camera_SplineFraction(const float *spline, uint32_t time, uint32_t *marker, int stride, bool *markerOutOfRange);
// Up vector components (x and z) of a fly-by camera from its roll in degrees: the roll turned by a quarter turn, as (cos, sin); returns the turned angle in radians
float Camera_FlyByUp(float rollDegrees, float *upX, float *upZ);
// Cosine and sine of the roll of the wheel camera, from the time in milliseconds: the roll swings with the cosine of a turn that takes 131071 milliseconds, scaled by 0.4
void Camera_WheelCamRoll(uint32_t timeMs, float *cosine, float *sine);
// Brightness of the colour of the water seen from underwater, from the red, green and blue of the water
float Camera_WaterLuminance(float red, float green, float blue);
// Up vector of a camera with a fixed position looking along a front vector: the world up plus an offset, made perpendicular to the front vector (the result is up = cross(cross(front, up), front))
void Camera_FixedModeUp(const CVector *front, const CVector *upOffset, CVector *up);
// Up and front vectors of the syphon camera: the up vector plus an offset is normalised, then the front vector is made perpendicular to it and normalised (the up vector is updated in place)
void Camera_SyphonBasis(CVector *up, const CVector *upOffset, CVector *front);
// Up vector of a camera made perpendicular to its front vector: left = cross(up, front) (normalised when normaliseLeft is set), up = cross(front, left), normalised
void Camera_PerpendicularUp(const CVector *front, const CVector *up, bool normaliseLeft, CVector *outUp);
// Right and up vectors of a camera from its front and up vectors: right = normalised cross(front, up), up = cross(right, front) (normalised when normaliseUp is set)
void Camera_RightAndUp(const CVector *front, const CVector *up, bool normaliseUp, CVector *right, CVector *outUp);
// Direction the fixed-pitch camera of the end-of-game or dead scene looks in from its horizontal angle, with a fixed downward pitch of 0.3: (cos 0.3 sin a, cos 0.3 cos a, -sin 0.3)
void Camera_FixedPitchFront(float angle, CVector *front);
// Right vector on the ground of the dead camera from the forward vector of its target: cross(forward, up) with z cleared, normalised
void Camera_DeadCamRight(const CVector *forward, CVector *right);
// Front vector of the dead camera as it wafts, from the time since the change and the right vector: looking down, tilted by the cosine of the time, normalised
void Camera_DeadCamFront(float time, float waftRate, float tiltAmp, const CVector *right, CVector *front);
// Offset of the dead camera target as it wafts along the right vector, from the time since the change
void Camera_DeadCamWaftOffset(float time, float waftAmplitude, float waftRate, const CVector *right, CVector *offset);
// Position of the arrest camera that looks over the shoulder of a cop at the target, from the positions of the cop and the target, the distance to the right of the cop, the distance behind the cop and the minimum distance from the target
void Camera_OverShoulderPos(const CVector *cop, const CVector *target, float rightOfCop, float behindCop, float minFromTarget, CVector *out);
// Position before the ground height test of the arrest camera that looks along the ground, from the target and the cop: beside the target and 5 units above it
void Camera_AlongGroundPos(const CVector *target, const CVector *cop, float alongGround, float sideGround, CVector *out);
// Variant of the arrest camera that looks along the ground, from a random number between 0 and 5: 0 straight, 1 right, 2 right and up, 3 left, 4 left and up
int Camera_ArrestVariantIndex(float rnd);
// Direction on the ground the arrest camera moves in when it circles around the target from a lamp post: the direction to the target turned to the right by an amount and made flat and normalised; also returns the (flat, normalised) direction to the target in up
void Camera_LampPostFront(const CVector *target, const CVector *camSource, float rotateDist, CVector *front, CVector *up);
// Vector from the target to a camera dragged along at a distance between a minimum and a maximum: scaled to the limit when too close or too far (or a default direction when it is at the target); returns the resulting distance
float Camera_DragDistance(CVector *dist, float minDist, float maxDist);
// Horizontal angle of a camera during a transition that points from the target to the camera: the angle of the direction (-cos b, -sin b)
float Camera_TransitionBetaAngle(float transitionBeta);
// Near clip plane when a ped is very close to the camera: the distance to the ped minus 0.3 but at least 0.05
float Camera_NearClipNearPed(float pedColDist);
// Near clip plane when a ped is in the way but not clipping through: the room left minus 0.35, at most the default near clip plane
float Camera_NearClipPedInWay(float colCamDist, float defaultNear);
// Angle between the direction of the target and the front of the camera, from their dot product: the arc cosine of its absolute value limited to 1
float Camera_DirectionAngle(float dot);
// The angle of the two with the smaller absolute value (the right one when equal)
float Camera_SmallerDelta(float deltaLeft, float deltaRight);
// Distances of the car camera of the improved camera, from the row of the camera set of the car (radius factor, distance, radius offset, minimum factor, minimum for the vehicle type), the smoothed zoom value, the approximate length of the car, the height of the car, the extra distance of an RC helicopter (1) or plane (2) in rcKind, whether the camera stays outside a radius (otherwise it is raised above a helicopter), whether the car is the RC Goblin and whether the nearest zoom of a small car is used: returns the distance (newDistance), the height the target is raised by (targetZAdd), the minimum distance for this car, the minimum distance for the type of vehicle and the next distance; the alpha offset is updated in place
void Camera_CarCamDistances(const float row[5], float zoomSmooth, float approxCarLength, float colMaxZ, int rcKind, float rcHeliExtra, float rcPlaneExtra, bool staysOutside, bool isGoblin, bool nearZoomSmallCar, float *alphaOffset, float *targetZAdd, float *newDistance, float *minDistForThisCar, float *nextDistance);
// Target horizontal angle of the car camera of the improved camera: the camera is brought towards the direction the car moves in, from the heading of the front vector, the length of the horizontal speed, the heading of the speed, the current angle, the time step, the two factors of the camera set and the side speed
float Camera_FollowCarTargetBeta(float frontHeading, float speed2D, float speedHeading, float beta, float timeStep, float betaChangeFactor, float betaChangeLimitFactor, float sideSpeed);
// Target vertical angle of the car camera of the improved camera: the angle the front vector points at minus the offset of the zoom mode, limited between the lower and upper limits
float Camera_FollowCarTargetAlpha(float frontZ, float alphaOffset, float lowerLimit, float upperLimit);
// Amount the vertical angle of the car camera moves towards the target in a frame, from the target, the current angle, the blend factor (raised to the power of the time step), the speed factor and the time step
float Camera_FollowCarAlphaBlend(float targetAlpha, float alpha, float blendBase, float speedFactor, float timeStep);
// Height of the camera source when the car or the ped under it would block the view: the highest of the target and the corner of the car plus 0.1, limited by the original source height, never below the current source height
float Camera_ClearedHeight(float targetZ, float cornerZ, float origZ, float sourceZ);
// Height the camera target is raised by from the buffered height: half of it, at most 1
float Camera_BufferedHeightOffset(float bufferedHeight);
// Minimum distance of the car camera from the base distance: 0.6 times it, at most 3.5
float Camera_MinCarDistance(float baseDist);
// Near clip plane of the sniper-like camera mode from the field of view (up to 15), the near clip multiplier and the default near clip
float Camera_SniperNearClip(float fov, float nearClipMult, float defaultNear);
// Ratio the ground distance of the camera is scaled by, at least 0.5
float Camera_ClampedGroundRatio(float ratio);
// Steps left to change the beta by the mouse after a time step, never negative
float Camera_StepsLeftAfterTimeStep(float stepsLeft, float timeStep);
// Divisor of the beta change blended with the stick: the time step, at least 1
float Camera_BetaStickBlendDivisor(float timeStep);
// Near clip plane of a script camera from the magnitude of the vector to the target: half of it, at least 0.05
float Camera_ScriptNearClip(float magnitude);
// Whether the target of a script camera is farther than a limit and the player moves along the direction to it (the dot product of the player speed and that direction is positive)
bool Camera_ScriptTargetPullingAway(float distance, float speedTowardsDir, float limit);
// Whether a script camera ends from the distance to the player: the player is beyond the far limit and moves away from it, or is closer than the near limit
bool Camera_ScriptDistanceEnds(float distance, float speedTowardsDir, float farLimit, float nearLimit);
// Whether the distance to the player is beyond the far limit or closer than the near limit
bool Camera_ScriptDistanceOutOfRange(float distance, float farLimit, float nearLimit);
// Position ahead of the given coordinates along the horizontal direction of a speed, by a distance
void Camera_AheadPosition(const CVector *coors, const CVector *speed, float ahead, CVector *out);
// Same as Camera_AheadPosition and then moved sideways, to the right of the direction of the speed, by a distance
void Camera_AheadAndSidePosition(const CVector *coors, const CVector *speed, float ahead, float side, CVector *out);
// What to do when starting a fixed script camera from the distance to the player: 0 give up (the player is beyond the far limit and moves away), 1 stop without taking control (closer than the near limit), 2 go on
int Camera_ScriptStartVerdict(float distance, float speedTowardsDir, float farLimit, float nearLimit);
// Whether a police car is behind the player (the offset to it points against the player forward direction) and heads nearly the same way (dot product of the forward directions above 0.8)
bool Camera_CopCarFollowsPlayer(float dx, float dy, float playerFwdX, float playerFwdY, float carFwdX, float carFwdY);
// What to do when starting a fixed script camera from the distance only: 0 give up (beyond the far limit), 1 stop without taking control (closer than the near limit), 2 go on
int Camera_ScriptStartRangeVerdict(float distance, float farLimit, float nearLimit);
// Height of a camera kept above the water: at least 1 above the water level, or 2 below it when the player is in a boat
float Camera_RaisedAboveWater(float camZ, float waterZ, bool isBoat);
// Height of a camera kept above the ground: moved to half a unit above the ground when it is under it
float Camera_RaisedAboveGround(float camZ, float groundZ);
// Angle of the horizontal part of a forward direction that is normalised in 3D first
float Camera_TargetForwardOrientation(const CVector *forward);
// Up and side directions of a target when looking to a side: the up and forward directions are normalised, the side direction is their cross product (negated when asked) normalised
void Camera_SideFront(const CVector *upIn, const CVector *forwardIn, bool negate, CVector *upOut, CVector *frontOut);
// Normalised direction of the difference a - b of two positions
void Camera_NormalisedDifference(const CVector *a, const CVector *b, CVector *out);
// Orthonormal camera basis from a front and an up direction: both are normalised, right is their cross product normalised, up is the cross product of right and front normalised
void Camera_OrthonormalBasis(const CVector *frontIn, const CVector *upIn, CVector *frontOut, CVector *rightOut, CVector *upOut);
// Camera basis for a top-down view: front normalised, right fixed at (-1, 0, 0), up the cross product of front and right normalised
void Camera_TopDownBasis(const CVector *frontIn, CVector *frontOut, CVector *rightOut, CVector *upOut);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
