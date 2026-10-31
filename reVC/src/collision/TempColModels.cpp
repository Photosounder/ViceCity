//+ rouz edit (ChatGPT)
//+ rouz edit (ChatGPT)
#include "common.h"
#include "ColSphere_native.h"

#include "TempColModels.h"
#include "Game.h"

static void DestroyStatic_ms_colModelPed1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelPed1);
}
CColModel CTempColModels::ms_colModelPed1=ColModel_InitStatic(&CTempColModels::ms_colModelPed1,DestroyStatic_ms_colModelPed1);
static void DestroyStatic_ms_colModelPed2(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelPed2);
}
CColModel CTempColModels::ms_colModelPed2=ColModel_InitStatic(&CTempColModels::ms_colModelPed2,DestroyStatic_ms_colModelPed2);
static void DestroyStatic_ms_colModelBBox(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelBBox);
}
CColModel CTempColModels::ms_colModelBBox=ColModel_InitStatic(&CTempColModels::ms_colModelBBox,DestroyStatic_ms_colModelBBox);
static void DestroyStatic_ms_colModelBumper1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelBumper1);
}
CColModel CTempColModels::ms_colModelBumper1=ColModel_InitStatic(&CTempColModels::ms_colModelBumper1,DestroyStatic_ms_colModelBumper1);
static void DestroyStatic_ms_colModelWheel1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelWheel1);
}
CColModel CTempColModels::ms_colModelWheel1=ColModel_InitStatic(&CTempColModels::ms_colModelWheel1,DestroyStatic_ms_colModelWheel1);
static void DestroyStatic_ms_colModelPanel1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelPanel1);
}
CColModel CTempColModels::ms_colModelPanel1=ColModel_InitStatic(&CTempColModels::ms_colModelPanel1,DestroyStatic_ms_colModelPanel1);
static void DestroyStatic_ms_colModelBodyPart2(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelBodyPart2);
}
CColModel CTempColModels::ms_colModelBodyPart2=ColModel_InitStatic(&CTempColModels::ms_colModelBodyPart2,DestroyStatic_ms_colModelBodyPart2);
static void DestroyStatic_ms_colModelBodyPart1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelBodyPart1);
}
CColModel CTempColModels::ms_colModelBodyPart1=ColModel_InitStatic(&CTempColModels::ms_colModelBodyPart1,DestroyStatic_ms_colModelBodyPart1);
static void DestroyStatic_ms_colModelCutObj(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_DestroyArray(CTempColModels::ms_colModelCutObj,5);
}
CColModel CTempColModels::ms_colModelCutObj[5]={
    ColModel_InitStatic(&CTempColModels::ms_colModelCutObj[0],nil),
    ColModel_InitStatic(&CTempColModels::ms_colModelCutObj[1],nil),
    ColModel_InitStatic(&CTempColModels::ms_colModelCutObj[2],nil),
    ColModel_InitStatic(&CTempColModels::ms_colModelCutObj[3],nil),
    ColModel_InitStatic(&CTempColModels::ms_colModelCutObj[4],DestroyStatic_ms_colModelCutObj)
};
static void DestroyStatic_ms_colModelPedGroundHit(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelPedGroundHit);
}
CColModel CTempColModels::ms_colModelPedGroundHit=ColModel_InitStatic(&CTempColModels::ms_colModelPedGroundHit,DestroyStatic_ms_colModelPedGroundHit);
static void DestroyStatic_ms_colModelBoot1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelBoot1);
}
CColModel CTempColModels::ms_colModelBoot1=ColModel_InitStatic(&CTempColModels::ms_colModelBoot1,DestroyStatic_ms_colModelBoot1);
static void DestroyStatic_ms_colModelDoor1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelDoor1);
}
CColModel CTempColModels::ms_colModelDoor1=ColModel_InitStatic(&CTempColModels::ms_colModelDoor1,DestroyStatic_ms_colModelDoor1);
static void DestroyStatic_ms_colModelBonnet1(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelBonnet1);
}
CColModel CTempColModels::ms_colModelBonnet1=ColModel_InitStatic(&CTempColModels::ms_colModelBonnet1,DestroyStatic_ms_colModelBonnet1);
static void DestroyStatic_ms_colModelWeapon(void)
{
    // Release the same static collision storage at its original process-exit lifetime boundary
    // Invoke the complete C collision owner at the original native lifetime or operation boundary
    ColModel_Destroy(&CTempColModels::ms_colModelWeapon);
}
CColModel CTempColModels::ms_colModelWeapon=ColModel_InitStatic(&CTempColModels::ms_colModelWeapon,DestroyStatic_ms_colModelWeapon);


CColSphere s_aPedSpheres[3];
CColSphere s_aPed2Spheres[3];
CColSphere s_aPedGSpheres[4];
#ifdef FIX_BUGS
CColSphere s_aDoorSpheres[3];
#else
CColSphere s_aDoorSpheres[4];
#endif
CColSphere s_aBumperSpheres[4];
CColSphere s_aPanelSpheres[4];
CColSphere s_aBonnetSpheres[4];
CColSphere s_aBootSpheres[4];
CColSphere s_aWheelSpheres[2];
CColSphere s_aBodyPartSpheres1[2];
CColSphere s_aBodyPartSpheres2[2];

void
CTempColModels::Initialise(void)
{
#define SET_COLMODEL_SPHERES(colmodel, sphrs)\
	colmodel.numSpheres = ARRAY_SIZE(sphrs);\
	colmodel.spheres = sphrs;\
	colmodel.level = LEVEL_GENERIC;\
	colmodel.ownsCollisionVolumes = false;

	int i;

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter0=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelBBox.boundingSphere,2.0f,COLLISION_POINT_REF(sphereCenter0));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum0=CVector(-2.0f, -2.0f, -2.0f);
	    const CVector &boxMaximum0=CVector(2.0f, 2.0f, 2.0f);
	    Box_Set(&ms_colModelBBox.boundingBox,COLLISION_POINT_REF(boxMinimum0),COLLISION_POINT_REF(boxMaximum0));
	}
	ms_colModelBBox.level = LEVEL_GENERIC;

	for (i = 0; i < ARRAY_SIZE(ms_colModelCutObj); i++) {
		{
		    // Keep the native center expression alive at the former member-call boundary
		    const CVector &sphereCenter1=CVector(0.0f, 0.0f, 0.0f);
		    Sphere_Set(&ms_colModelCutObj[i].boundingSphere,2.0f,COLLISION_POINT_REF(sphereCenter1));
		}
		{
		    // Keep native bound expressions alive through the ordered C update
		    const CVector &boxMinimum1=CVector(-2.0f, -2.0f, -2.0f);
		    const CVector &boxMaximum1=CVector(2.0f, 2.0f, 2.0f);
		    Box_Set(&ms_colModelCutObj[i].boundingBox,COLLISION_POINT_REF(boxMinimum1),COLLISION_POINT_REF(boxMaximum1));
		}
		ms_colModelCutObj[i].level = LEVEL_GENERIC;
	}

	// Ped Spheres

	for (i = 0; i < ARRAY_SIZE(s_aPedSpheres); i++)
		s_aPedSpheres[i].bounds.radius = 0.35f;

	s_aPedSpheres[0].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, -0.25f));
	s_aPedSpheres[1].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.15f));
	s_aPedSpheres[2].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.55f));

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aPedSpheres); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aPedGSpheres); i++) {
#endif
		s_aPedSpheres[i].surface = SURFACE_PED;
		s_aPedSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter2=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelPed1.boundingSphere,1.25f,COLLISION_POINT_REF(sphereCenter2));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum2=CVector(-0.35f, -0.35f, -1.0f);
	    const CVector &boxMaximum2=CVector(0.35f, 0.35f, 0.9f);
	    Box_Set(&ms_colModelPed1.boundingBox,COLLISION_POINT_REF(boxMinimum2),COLLISION_POINT_REF(boxMaximum2));
	}
	SET_COLMODEL_SPHERES(ms_colModelPed1, s_aPedSpheres);

	// Ped 2 Spheres

	s_aPed2Spheres[0].bounds.radius = 0.3f;
	s_aPed2Spheres[1].bounds.radius = 0.4f;
	s_aPed2Spheres[2].bounds.radius = 0.3f;

	s_aPed2Spheres[0].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.35f, -0.9f));
	s_aPed2Spheres[1].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, -0.9f));
	s_aPed2Spheres[2].bounds.center = CollisionVector_FromNative(CVector(0.0f, -0.35f, -0.9f));

	for (i = 0; i < ARRAY_SIZE(s_aPed2Spheres); i++) {
		s_aPed2Spheres[i].surface = SURFACE_PED;
		s_aPed2Spheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter3=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelPed2.boundingSphere,2.0f,COLLISION_POINT_REF(sphereCenter3));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum3=CVector(-0.7f, -0.7f, -1.2f);
	    const CVector &boxMaximum3=CVector(0.7f, 0.7f, 0.0f);
	    Box_Set(&ms_colModelPed2.boundingBox,COLLISION_POINT_REF(boxMinimum3),COLLISION_POINT_REF(boxMaximum3));
	}

	SET_COLMODEL_SPHERES(ms_colModelPed2, s_aPed2Spheres);

	// Ped ground collision
	
	s_aPedGSpheres[0].bounds.radius = 0.35f;
	s_aPedGSpheres[1].bounds.radius = 0.35f;
	s_aPedGSpheres[2].bounds.radius = 0.35f;
	s_aPedGSpheres[3].bounds.radius = 0.3f;

	s_aPedGSpheres[0].bounds.center = CollisionVector_FromNative(CVector(0.0f, -0.4f, -0.9f));
	s_aPedGSpheres[1].bounds.center = CollisionVector_FromNative(CVector(0.0f, -0.1f, -0.9f));
	s_aPedGSpheres[2].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.25f, -0.9f));
	s_aPedGSpheres[3].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.65f, -0.9f));

	s_aPedGSpheres[0].surface = SURFACE_PED;
	s_aPedGSpheres[1].surface = SURFACE_PED;
	s_aPedGSpheres[2].surface = SURFACE_PED;
	s_aPedGSpheres[3].surface = SURFACE_PED;
	s_aPedGSpheres[0].piece = 4;
	s_aPedGSpheres[1].piece = 1;
	s_aPedGSpheres[2].piece = 0;
	s_aPedGSpheres[3].piece = 6;

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter4=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelPedGroundHit.boundingSphere,2.0f,COLLISION_POINT_REF(sphereCenter4));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum4=CVector(-0.4f, -1.0f, -1.25f);
	    const CVector &boxMaximum4=CVector(0.4f, 1.2f, -0.5f);
	    Box_Set(&ms_colModelPedGroundHit.boundingBox,COLLISION_POINT_REF(boxMinimum4),COLLISION_POINT_REF(boxMaximum4));
	}

	SET_COLMODEL_SPHERES(ms_colModelPedGroundHit, s_aPedGSpheres);

	// Door Spheres

	s_aDoorSpheres[0].bounds.radius = 0.15f;
	s_aDoorSpheres[1].bounds.radius = 0.15f;
	s_aDoorSpheres[2].bounds.radius = 0.25f;

	s_aDoorSpheres[0].bounds.center = CollisionVector_FromNative(CVector(0.0f, -0.25f, -0.35f));
	s_aDoorSpheres[1].bounds.center = CollisionVector_FromNative(CVector(0.0f, -0.95f, -0.35f));
	s_aDoorSpheres[2].bounds.center = CollisionVector_FromNative(CVector(0.0f, -0.6f, 0.25f));

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aDoorSpheres); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aPed2Spheres); i++) {
#endif
		s_aDoorSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aDoorSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter5=CVector(0.0f, -0.6f, 0.0f);
	    Sphere_Set(&ms_colModelDoor1.boundingSphere,1.5f,COLLISION_POINT_REF(sphereCenter5));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum5=CVector(-0.3f, 0.0f, -0.6f);
	    const CVector &boxMaximum5=CVector(0.3f, -1.2f, 0.6f);
	    Box_Set(&ms_colModelDoor1.boundingBox,COLLISION_POINT_REF(boxMinimum5),COLLISION_POINT_REF(boxMaximum5));
	}

	SET_COLMODEL_SPHERES(ms_colModelDoor1, s_aDoorSpheres);

	// Bumper Spheres

	for (i = 0; i < ARRAY_SIZE(s_aBumperSpheres); i++)
		s_aBumperSpheres[i].bounds.radius = 0.15f;

	s_aBumperSpheres[0].bounds.center = CollisionVector_FromNative(CVector(0.85f, -0.05f, 0.0f));
	s_aBumperSpheres[1].bounds.center = CollisionVector_FromNative(CVector(0.4f, 0.05f, 0.0f));
	s_aBumperSpheres[2].bounds.center = CollisionVector_FromNative(CVector(-0.4f, 0.05f, 0.0f));
	s_aBumperSpheres[3].bounds.center = CollisionVector_FromNative(CVector(-0.85f, -0.05f, 0.0f));

	for (i = 0; i < ARRAY_SIZE(s_aBumperSpheres); i++) {
		s_aBumperSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aBumperSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter6=CVector(0.0f, -0.6f, 0.0f);
	    Sphere_Set(&ms_colModelBumper1.boundingSphere,2.2f,COLLISION_POINT_REF(sphereCenter6));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum6=CVector(-1.2f, -0.3f, -0.2f);
	    const CVector &boxMaximum6=CVector(1.2f, 0.3f, 0.2f);
	    Box_Set(&ms_colModelBumper1.boundingBox,COLLISION_POINT_REF(boxMinimum6),COLLISION_POINT_REF(boxMaximum6));
	}

	SET_COLMODEL_SPHERES(ms_colModelBumper1, s_aBumperSpheres);

	// Panel Spheres

	for (i = 0; i < ARRAY_SIZE(s_aPanelSpheres); i++)
		s_aPanelSpheres[i].bounds.radius = 0.15f;

	s_aPanelSpheres[0].bounds.center = CollisionVector_FromNative(CVector(0.15f, 0.45f, 0.0f));
	s_aPanelSpheres[1].bounds.center = CollisionVector_FromNative(CVector(0.15f, -0.45f, 0.0f));
	s_aPanelSpheres[2].bounds.center = CollisionVector_FromNative(CVector(-0.15f, -0.45f, 0.0f));
	s_aPanelSpheres[3].bounds.center = CollisionVector_FromNative(CVector(-0.15f, 0.45f, 0.0f));

	for (i = 0; i < ARRAY_SIZE(s_aPanelSpheres); i++) {
		s_aPanelSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aPanelSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter7=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelPanel1.boundingSphere,1.4f,COLLISION_POINT_REF(sphereCenter7));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum7=CVector(-0.3f, -0.6f, -0.15f);
	    const CVector &boxMaximum7=CVector(0.3f, 0.6f, 0.15f);
	    Box_Set(&ms_colModelPanel1.boundingBox,COLLISION_POINT_REF(boxMinimum7),COLLISION_POINT_REF(boxMaximum7));
	}

	SET_COLMODEL_SPHERES(ms_colModelPanel1, s_aPanelSpheres);

	// Bonnet Spheres

	for (i = 0; i < ARRAY_SIZE(s_aBonnetSpheres); i++)
		s_aBonnetSpheres[i].bounds.radius = 0.2f;

	s_aBonnetSpheres[0].bounds.center = CollisionVector_FromNative(CVector(-0.4f, 0.1f, 0.0f));
	s_aBonnetSpheres[1].bounds.center = CollisionVector_FromNative(CVector(-0.4f, 0.9f, 0.0f));
	s_aBonnetSpheres[2].bounds.center = CollisionVector_FromNative(CVector(0.4f, 0.1f, 0.0f));
	s_aBonnetSpheres[3].bounds.center = CollisionVector_FromNative(CVector(0.4f, 0.9f, 0.0f));

	for (i = 0; i < ARRAY_SIZE(s_aBonnetSpheres); i++) {
		s_aBonnetSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aBonnetSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter8=CVector(0.0f, 0.5f, 0.0f);
	    Sphere_Set(&ms_colModelBonnet1.boundingSphere,1.7f,COLLISION_POINT_REF(sphereCenter8));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum8=CVector(-0.7f, -0.2f, -0.3f);
	    const CVector &boxMaximum8=CVector(0.7f, 1.2f, 0.3f);
	    Box_Set(&ms_colModelBonnet1.boundingBox,COLLISION_POINT_REF(boxMinimum8),COLLISION_POINT_REF(boxMaximum8));
	}

	SET_COLMODEL_SPHERES(ms_colModelBonnet1, s_aBonnetSpheres);

	// Boot Spheres

	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++)
		s_aBootSpheres[i].bounds.radius = 0.2f;

	s_aBootSpheres[0].bounds.center = CollisionVector_FromNative(CVector(-0.4f, -0.1f, 0.0f));
	s_aBootSpheres[1].bounds.center = CollisionVector_FromNative(CVector(-0.4f, -0.6f, 0.0f));
	s_aBootSpheres[2].bounds.center = CollisionVector_FromNative(CVector(0.4f, -0.1f, 0.0f));
	s_aBootSpheres[3].bounds.center = CollisionVector_FromNative(CVector(0.4f, -0.6f, 0.0f));

	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
		s_aBootSpheres[i].surface = SURFACE_CAR_PANEL;
		s_aBootSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter9=CVector(0.0f, -0.4f, 0.0f);
	    Sphere_Set(&ms_colModelBoot1.boundingSphere,1.4f,COLLISION_POINT_REF(sphereCenter9));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum9=CVector(-0.7f, -0.9f, -0.3f);
	    const CVector &boxMaximum9=CVector(0.7f, 0.2f, 0.3f);
	    Box_Set(&ms_colModelBoot1.boundingBox,COLLISION_POINT_REF(boxMinimum9),COLLISION_POINT_REF(boxMaximum9));
	}

	SET_COLMODEL_SPHERES(ms_colModelBoot1, s_aBootSpheres);

	// Wheel Spheres

	s_aWheelSpheres[0].bounds.radius = 0.35f;
	s_aWheelSpheres[1].bounds.radius = 0.35f;

	s_aWheelSpheres[0].bounds.center = CollisionVector_FromNative(CVector(-0.3f, 0.0f, 0.0f));
	s_aWheelSpheres[1].bounds.center = CollisionVector_FromNative(CVector(0.3f, 0.0f, 0.0f));

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aWheelSpheres); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
#endif
		s_aWheelSpheres[i].surface = SURFACE_WHEELBASE;
		s_aWheelSpheres[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter10=CVector(0.0f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelWheel1.boundingSphere,1.4f,COLLISION_POINT_REF(sphereCenter10));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum10=CVector(-0.7f, -0.4f, -0.4f);
	    const CVector &boxMaximum10=CVector(0.7f, 0.4f, 0.4f);
	    Box_Set(&ms_colModelWheel1.boundingBox,COLLISION_POINT_REF(boxMinimum10),COLLISION_POINT_REF(boxMaximum10));
	}

	SET_COLMODEL_SPHERES(ms_colModelWheel1, s_aWheelSpheres);

	// Body Part Spheres 1

	s_aBodyPartSpheres1[0].bounds.radius = 0.2f;
	s_aBodyPartSpheres1[1].bounds.radius = 0.2f;

	s_aBodyPartSpheres1[0].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.0f));
	s_aBodyPartSpheres1[1].bounds.center = CollisionVector_FromNative(CVector(0.8f, 0.0f, 0.0f));

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aBodyPartSpheres1); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
#endif
		s_aBodyPartSpheres1[i].surface = SURFACE_PED;
		s_aBodyPartSpheres1[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter11=CVector(0.4f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelBodyPart1.boundingSphere,0.7f,COLLISION_POINT_REF(sphereCenter11));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum11=CVector(-0.3f, -0.3f, -0.3f);
	    const CVector &boxMaximum11=CVector(1.1f, 0.3f, 0.3f);
	    Box_Set(&ms_colModelBodyPart1.boundingBox,COLLISION_POINT_REF(boxMinimum11),COLLISION_POINT_REF(boxMaximum11));
	}

	SET_COLMODEL_SPHERES(ms_colModelBodyPart1, s_aBodyPartSpheres1);

	// Body Part Spheres 2

	s_aBodyPartSpheres2[0].bounds.radius = 0.15f;
	s_aBodyPartSpheres2[1].bounds.radius = 0.15f;

	s_aBodyPartSpheres2[0].bounds.center = CollisionVector_FromNative(CVector(0.0f, 0.0f, 0.0f));
	s_aBodyPartSpheres2[1].bounds.center = CollisionVector_FromNative(CVector(0.5f, 0.0f, 0.0f));

#ifdef FIX_BUGS
	for (i = 0; i < ARRAY_SIZE(s_aBodyPartSpheres2); i++) {
#else
	for (i = 0; i < ARRAY_SIZE(s_aBootSpheres); i++) {
#endif
		s_aBodyPartSpheres2[i].surface = SURFACE_PED;
		s_aBodyPartSpheres2[i].piece = 0;
	}

	{
	    // Keep the native center expression alive at the former member-call boundary
	    const CVector &sphereCenter12=CVector(0.25f, 0.0f, 0.0f);
	    Sphere_Set(&ms_colModelBodyPart2.boundingSphere,0.5f,COLLISION_POINT_REF(sphereCenter12));
	}
	{
	    // Keep native bound expressions alive through the ordered C update
	    const CVector &boxMinimum12=CVector(-0.2f, -0.2f, -0.2f);
	    const CVector &boxMaximum12=CVector(0.7f, 0.2f, 0.2f);
	    Box_Set(&ms_colModelBodyPart2.boundingBox,COLLISION_POINT_REF(boxMinimum12),COLLISION_POINT_REF(boxMaximum12));
	}

	SET_COLMODEL_SPHERES(ms_colModelBodyPart2, s_aBodyPartSpheres2);

	ms_colModelWeapon.boundingSphere.radius = 0.25f;
	ms_colModelWeapon.boundingBox.min.x = -0.25f;
	ms_colModelWeapon.boundingBox.min.y = -0.25f;
	ms_colModelWeapon.boundingBox.min.z = -0.25f;
	ms_colModelWeapon.boundingBox.max.x = 0.25f;
	ms_colModelWeapon.boundingBox.max.y = 0.25f;
	ms_colModelWeapon.boundingBox.max.z = 0.25f;

#undef SET_COLMODEL_SPHERES
}
//- rouz edit (ChatGPT)
//- rouz edit (ChatGPT)
