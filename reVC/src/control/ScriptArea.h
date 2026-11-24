//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

// An angled area: a rectangle whose first side is the line from the lowest to the highest corner and whose second side is turned a quarter, the numbers are worked out by ScriptArea_AngledGeometry
typedef struct ScriptAngledArea {
	float infX, infY;
	float rotatedSupX, rotatedSupY, rotatedInfX, rotatedInfY;
	float side1X, side1Y, side1Length;
	float side2X, side2Y, side2Length;
} ScriptAngledArea;

#ifdef __cplusplus
extern "C" {
#endif
// Whether a point is in a box given by its center and the distance to its sides, edges included
bool ScriptArea_InCenteredBox2D(float X, float Y, float dX, float dY, float posX, float posY);
bool ScriptArea_InCenteredBox3D(float X, float Y, float Z, float dX, float dY, float dZ, float posX, float posY, float posZ);
// Whether a point is in a box given by its lowest and highest corner, edges included
bool ScriptArea_InMinMaxBox2D(float infX, float infY, float supX, float supY, float posX, float posY);
bool ScriptArea_InMinMaxBox3D(float infX, float infY, float infZ, float supX, float supY, float supZ, float posX, float posY, float posZ);
// Works out the rotated corners and sides of an angled area from two corners and the length of the second side
void ScriptArea_AngledGeometry(float infX, float infY, float supX, float supY, float side2length, ScriptAngledArea *area);
// Whether a point is in an angled area, for 3D areas its height has to be between infZ and supZ
bool ScriptArea_InAngledArea(const ScriptAngledArea *area, float posX, float posY, float posZ, float infZ, float supZ, bool is3D);
// Puts the lowest of two coordinates in inf and the highest in sup
void ScriptArea_SortPair(float *inf, float *sup);
// Heading in degrees an object that a script rotates has after a step, from its heading, the heading it is rotated to and the amount it turns by: it takes the shorter way and stops on the target
float ScriptArea_RotateObjectHeading(float heading, float headingTarget, float rotateBy);
// Position along one axis of an object that a script slides towards a target, after a step: it moves by slideBy and stops on the target when it is closer than that
float ScriptArea_SlideAxis(float pos, float posTarget, float slideBy);
// Heading in degrees between 0 and 360 that a script gets for a heading in radians
float ScriptArea_HeadingDegrees(float radians);
// Smallest of four values taken as Min(a, Min(b, Min(c, d)))
float ScriptArea_Min4(float a, float b, float c, float d);
// Largest of four values taken as Max(a, Max(b, Max(c, d)))
float ScriptArea_Max4(float a, float b, float c, float d);
// Radius of the area of a script, the larger half size of the area on x and y, from the centre and the lower corner
float ScriptArea_Radius(float centerX, float centerY, float infX, float infY);
// Bounds of an angled area highlighted by a script, from the middles of its four sides (given by its four corners)
void ScriptArea_AngledBounds(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4, float *infX, float *supX, float *infY, float *supY);
// Lower and upper bounds on both axes of an area given by two opposite corners in any order
void ScriptArea_OrderedBounds(float x1, float y1, float x2, float y2, float *infX, float *supX, float *infY, float *supY);
// Sine of an angle in degrees, for the script command that computes it
float ScriptMath_SinDegrees(float degrees);
// Cosine of an angle in degrees, for the script command that computes it
float ScriptMath_CosDegrees(float degrees);
// Square root of a value, for the script command that computes it
float ScriptMath_Sqrt(float value);
// Component of a forward vector on the ground divided by the length of its ground part, for the script commands that read the heading of a vehicle
float ScriptMath_GroundDirection(float component, float forwardX, float forwardY);
// Armour of a ped after the script adds some, limited between 0 and a maximum
float ScriptMath_ArmourAfterAdd(float armour, int32_t add, float maxArmour);
// Distance between two points on the ground, for the script command that measures it
float ScriptMath_Distance2D(float ax, float ay, float bx, float by);
// Distance between two points in 3D, for the script command that measures it
float ScriptMath_Distance3D(float ax, float ay, float az, float bx, float by, float bz);
// Whether a car is upside down and still, from the z of its up vector, its speeds and the three thresholds
bool ScriptMath_IsUpsideDown(float upZ, float moveX, float moveY, float moveZ, float turnX, float turnY, float turnZ, float upThreshold, float moveThreshold, float turnThreshold);
#ifdef __cplusplus
}
#endif
//- rouz edit (Claude)
