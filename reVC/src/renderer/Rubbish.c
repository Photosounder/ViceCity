//+ rouz edit (ChatGPT)
#include "Rubbish_backend.h"
#include "Timer.h"
#include "General.h"
#include "ZoneCull.h"
#include <stddef.h>
#include <math.h>

#define Min(a,b) ((a) < (b) ? (a) : (b))
#define Max(a,b) ((a) > (b) ? (a) : (b))
#define SQR(x) ((x) * (x))
#ifdef _MSC_VER
float fast_absf(float x);
#define Rubbish_Abs fast_absf
#else
#define Rubbish_Abs fabsf
#endif
static RubbishVector Rubbish_Subtract(RubbishVector left, RubbishVector right)
{
	// Preserve all three original vector subtractions before planar conversion
	RubbishVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}
static RubbishVector2 Rubbish_XY(RubbishVector vector)
{
	// Preserve the original planar constructor component copies
	RubbishVector2 result = {vector.x, vector.y};
	return result;
}
static float Rubbish_MagnitudeSqr2(RubbishVector2 vector)
{
	// Retain the original two float products and addition
	return vector.x*vector.x + vector.y*vector.y;
}
static float Rubbish_Magnitude2(RubbishVector2 vector)
{
	// Retain the native float square-root wrapper
	return sqrtf(Rubbish_MagnitudeSqr2(vector));
}
static float Rubbish_Dot2(RubbishVector2 left, RubbishVector2 right)
{
	// Retain the original planar dot-product operation order
	return left.x*right.x + left.y*right.y;
}

#define RUBBISH_MAX_DIST (23.0f)
#define RUBBISH_FADE_DIST (20.0f)

void *gpRubbishTexture[4];
uint16_t RubbishIndexList[6];
static bool bRubbishInvisible;
static int RubbishVisibility;
static RubbishSheet aSheets[NUM_RUBBISH_SHEETS];
static RubbishSheet StartEmptyList;
static RubbishSheet EndEmptyList;
static RubbishSheet StartStaticsList;
static RubbishSheet EndStaticsList;
static RubbishSheet StartMoversList;
static RubbishSheet EndMoversList;


void
RubbishSheet_AddToList(RubbishSheet *sheet, RubbishSheet *list)
{
	// Insert immediately after the original node without clearing other payloads
	sheet->m_next = list->m_next;
	sheet->m_prev = list;
	list->m_next = sheet;
	sheet->m_next->m_prev = sheet;
}

void
RubbishSheet_RemoveFromList(RubbishSheet *sheet)
{
	// Unlink both neighbors while retaining this node links
	sheet->m_next->m_prev = sheet->m_prev;
	sheet->m_prev->m_next = sheet->m_next;
}


void
Rubbish_Render(void)
{
	// Preserve camera selection, texture groups and the complete shared-buffer draw sequence
	int type;
	// Fade debris from the active environment camera while building a reflection
	RubbishVector rubbishCameraPosition = Rubbish_BackendCameraPosition();
#if defined(LIBRW) && defined(EXTENDED_PIPELINES)
	if(Rubbish_BackendEnvMap() && Rubbish_BackendEnvCamera() != NULL){
		// Retain the original sheet traversal and update order
		void *effectCamera = Rubbish_BackendEnvCamera();
		void *effectFrame = Rubbish_BackendCameraFrame(effectCamera);
		if(effectFrame && Rubbish_BackendFrameMatrix(effectFrame))
			rubbishCameraPosition = Rubbish_BackendMatrixPosition(Rubbish_BackendFrameMatrix(effectFrame));
	}
#endif

	if(RubbishVisibility == 0)
		return;

	// Keep live counter accesses attached to the original shared storage
	int32_t *indicesStored = Rubbish_BackendIndexCount();
	int32_t *verticesStored = Rubbish_BackendVertexCount();

	Rubbish_BackendPush("CRubbish::Render");

	Rubbish_BackendZWrite(false);
	Rubbish_BackendVertexAlpha(true);
	Rubbish_BackendFog(true);

	for(type = 0; type < 4; type++){
		// Retain the original sheet traversal and update order
		if(type < 3 || Rubbish_BackendPamphlet())
			Rubbish_BackendTextureRaster(Rubbish_BackendRaster(gpRubbishTexture[type]));

		(*indicesStored) = 0;
		(*verticesStored) = 0;

		RubbishSheet *sheet;
		for(sheet = &aSheets[type*NUM_RUBBISH_SHEETS / 4];
		    sheet < &aSheets[(type+1)*NUM_RUBBISH_SHEETS / 4];
		    sheet++){
			// Retain the original sheet traversal and update order
			if(sheet->m_state == 0)
				continue;

			uint32_t alpha = 100;
			RubbishVector pos;
			if(sheet->m_state == 1){
				// Retain the original sheet traversal and update order
				pos = sheet->m_basePos;
				if(!sheet->m_isVisible)
					alpha = 0;
			}else{
				// Retain the original sheet traversal and update order
				pos = sheet->m_animatedPos;
				// Not fully visible during animation, calculate current alpha
				if(!sheet->m_isVisible || !sheet->m_targetIsVisible){
					// Retain the original sheet traversal and update order
					float t = (float)((*Timer_GetTimeInMilliseconds()) - sheet->m_moveStart)/sheet->m_moveDuration;
					float f1 = sheet->m_isVisible ? 1.0f-t : 0.0f;
					float f2 = sheet->m_targetIsVisible ? t : 0.0f;
					alpha = 100 * (f1+f2);
				}
			}

			// Use the selected render camera for debris distance fading
			float camDist = Rubbish_Magnitude2(Rubbish_XY(Rubbish_Subtract(pos, rubbishCameraPosition)));
			if(camDist < RUBBISH_MAX_DIST){
				// Retain the original sheet traversal and update order
				if(camDist >= RUBBISH_FADE_DIST)
					alpha -= alpha*(camDist-RUBBISH_FADE_DIST)/(RUBBISH_MAX_DIST-RUBBISH_FADE_DIST);
				alpha = (RubbishVisibility*alpha)/256;

				float vx1, vy1, vx2, vy2;
				if(type == 0 || type == 1){
					// Retain the original sheet traversal and update order
					vx1 = 0.9f*sinf(sheet->m_angle);
					vy1 = 0.9f*cosf(sheet->m_angle);
					vx2 = 0.3f*cosf(sheet->m_angle);
					vy2 = -0.3f*sinf(sheet->m_angle);
				}else{
					// Retain the original sheet traversal and update order
					vx1 = 0.3f*sinf(sheet->m_angle);
					vy1 = 0.3f*cosf(sheet->m_angle);
					vx2 = 0.3f*cosf(sheet->m_angle);
					vy2 = -0.3f*sinf(sheet->m_angle);
				}

				int v = (*verticesStored);
				Rubbish_BackendSetPos(v+0, pos.x + vx1 + vx2, pos.y + vy1 + vy2, pos.z);
				Rubbish_BackendSetPos(v+1, pos.x + vx1 - vx2, pos.y + vy1 - vy2, pos.z);
				Rubbish_BackendSetPos(v+2, pos.x - vx1 + vx2, pos.y - vy1 + vy2, pos.z);
				Rubbish_BackendSetPos(v+3, pos.x - vx1 - vx2, pos.y - vy1 - vy2, pos.z);
				Rubbish_BackendSetRGBA(v+0, 255, 255, 255, alpha);
				Rubbish_BackendSetRGBA(v+1, 255, 255, 255, alpha);
				Rubbish_BackendSetRGBA(v+2, 255, 255, 255, alpha);
				Rubbish_BackendSetRGBA(v+3, 255, 255, 255, alpha);
				Rubbish_BackendSetU(v+0, 0.0f);
				Rubbish_BackendSetV(v+0, 0.0f);
				Rubbish_BackendSetU(v+1, 1.0f);
				Rubbish_BackendSetV(v+1, 0.0f);
				Rubbish_BackendSetU(v+2, 0.0f);
				Rubbish_BackendSetV(v+2, 1.0f);
				Rubbish_BackendSetU(v+3, 1.0f);
				Rubbish_BackendSetV(v+3, 1.0f);

				int i = (*indicesStored);
				Rubbish_BackendSetIndex(i+0, RubbishIndexList[0] + (*verticesStored));
				Rubbish_BackendSetIndex(i+1, RubbishIndexList[1] + (*verticesStored));
				Rubbish_BackendSetIndex(i+2, RubbishIndexList[2] + (*verticesStored));
				Rubbish_BackendSetIndex(i+3, RubbishIndexList[3] + (*verticesStored));
				Rubbish_BackendSetIndex(i+4, RubbishIndexList[4] + (*verticesStored));
				Rubbish_BackendSetIndex(i+5, RubbishIndexList[5] + (*verticesStored));
				(*verticesStored) += 4;
				(*indicesStored) += 6;
			}
		}

		if((*indicesStored) != 0){
			// Retain the original sheet traversal and update order
			Rubbish_BackendLittleTest();
			if(Rubbish_BackendTransform((*verticesStored))){
				// Retain the original sheet traversal and update order
				Rubbish_BackendRenderIndexed((*indicesStored));
				Rubbish_BackendEnd();
			}
		}
	}

	Rubbish_BackendFog(false);
	Rubbish_BackendZWrite(false);
	Rubbish_BackendVertexAlpha(true);

	Rubbish_BackendPop();
}

void
Rubbish_StirUp(CVehicle *veh)
{
	// Preserve lazy vehicle eligibility and saved-next list traversal
	if(((*Timer_GetFrameCounter()) ^ (Rubbish_BackendVehicleSeed(veh)&3)) == 0)
		return;

	if(Rubbish_Abs(Rubbish_BackendVehicleX(veh) - Rubbish_BackendCameraX()) < 20.0f &&
	   Rubbish_Abs(Rubbish_BackendVehicleY(veh) - Rubbish_BackendCameraY()) < 20.0f)
		if(Rubbish_Abs(Rubbish_BackendSpeedX(veh)) > 0.05f || Rubbish_Abs(Rubbish_BackendSpeedY(veh)) > 0.05f){
			// Retain the original sheet traversal and update order
			float speed = Rubbish_Magnitude2(Rubbish_BackendSpeed(veh));
			if(speed > 0.05f){
				// Retain the original sheet traversal and update order
				bool movingForward = Rubbish_Dot2(Rubbish_BackendSpeed(veh), Rubbish_BackendForward(veh)) > 0.0f;
				RubbishSheet *sheet = StartStaticsList.m_next;
				RubbishVector2 size = Rubbish_BackendBounds(veh);

				// Check all static sheets
				while(sheet != &EndStaticsList){
					// Retain the original sheet traversal and update order
					RubbishSheet *next = sheet->m_next;
					RubbishVector2 carToSheet = Rubbish_XY(Rubbish_Subtract(sheet->m_basePos, Rubbish_BackendVehiclePosition(veh)));
					float distFwd = Rubbish_Dot2(carToSheet, Rubbish_BackendForward(veh));

					// sheet has to be a bit behind car
					if(movingForward && distFwd < -0.5f*size.y && distFwd > -1.5f*size.y ||
					   !movingForward && distFwd > 0.5f*size.y && distFwd < 1.5f*size.y){
						// Retain the original sheet traversal and update order
						float distSide = Rubbish_Abs(Rubbish_Dot2(carToSheet, Rubbish_BackendRight(veh)));
						if(distSide < 1.5*size.x){
							// Retain the original sheet traversal and update order
							// Check with higher speed for sheet directly behind car
							float speedToCheck = distSide < size.x ? speed : speed*0.5f;
							if(speedToCheck > 0.05f){
								// Retain the original sheet traversal and update order
								sheet->m_state = 2;
								if(speedToCheck > 0.15f)
									sheet->m_animationType = 2;
								else
									sheet->m_animationType = 1;
								sheet->m_moveDuration = 2000;
								sheet->m_xDist = Rubbish_BackendSpeedX(veh);
								sheet->m_yDist = Rubbish_BackendSpeedY(veh);
								float dist = sqrtf(SQR(sheet->m_xDist)+SQR(sheet->m_yDist));
								sheet->m_xDist *= 25.0f*speed/dist;
								sheet->m_yDist *= 25.0f*speed/dist;
								sheet->m_animHeight = 3.0f*speed;
								sheet->m_moveStart = (*Timer_GetTimeInMilliseconds());
								float tx = sheet->m_basePos.x + sheet->m_xDist;
								float ty = sheet->m_basePos.y + sheet->m_yDist;
								float tz = sheet->m_basePos.z + 3.0f;
								sheet->m_targetZ = Rubbish_BackendGround(tx, ty, tz, NULL) + 0.1f;
								RubbishSheet_RemoveFromList(sheet);
								RubbishSheet_AddToList(sheet, &StartMoversList);
							}
						}
					}

					sheet = next;
				}
			}
		}
}

static float aAnimations[3][34] = {
	{ 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
	  0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },

	// Normal move
	{ 0.0f, 0.05f, 0.12f, 0.25f, 0.42f, 0.57f, 0.68f, 0.8f, 0.86f, 0.9f, 0.93f, 0.95f, 0.96f, 0.97f, 0.98f, 0.99f, 1.0f,	// XY movemnt
	  0.15f, 0.35f, 0.6f, 0.9f, 1.2f, 1.25f, 1.3f, 1.2f, 1.1f, 0.95f, 0.8f, 0.6f, 0.45f, 0.3f, 0.2f, 0.1f, 0 },	// Z movement

	// Stirred up by fast vehicle
	{ 0.0f, 0.05f, 0.12f, 0.25f, 0.42f, 0.57f, 0.68f, 0.8f, 0.95f, 1.1f, 1.15f, 1.18f, 1.15f, 1.1f, 1.05f, 1.03f, 1.0f,
	  0.15f, 0.35f, 0.6f, 0.9f, 1.2f, 1.25f, 1.3f, 1.2f, 1.1f, 0.95f, 0.8f, 0.6f, 0.45f, 0.3f, 0.2f, 0.1f, 0 }
};

void
Rubbish_Update(void)
{
	// Preserve random draws, live inputs, animation arithmetic and list transitions
	bool foundGround;

	// FRAMETIME
	if(bRubbishInvisible)
		RubbishVisibility = Max(RubbishVisibility-5, 0);
	else
		RubbishVisibility = Min(RubbishVisibility+5, 255);

	// Spawn a new sheet
	RubbishSheet *sheet = StartEmptyList.m_next;
	if(sheet != &EndEmptyList){
		// Retain the original sheet traversal and update order
		float spawnDist;
		float spawnAngle;

		spawnDist = (General_GetRandomNumber()&0xFF)/256.0f + RUBBISH_MAX_DIST;
		uint8_t r = General_GetRandomNumber();
		if(r&1)
			spawnAngle = (General_GetRandomNumber()&0xFF)/256.0f * 6.28f;
		else
			spawnAngle = (r-128)/160.0f + Rubbish_BackendOrientation();
		sheet->m_basePos.x = Rubbish_BackendCameraX() + spawnDist*sinf(spawnAngle);
		sheet->m_basePos.y = Rubbish_BackendCameraY() + spawnDist*cosf(spawnAngle);
		sheet->m_basePos.z = Rubbish_BackendGround(sheet->m_basePos.x, sheet->m_basePos.y, Rubbish_BackendCameraZ(), &foundGround) + 0.1f;
		if(foundGround){
			// Retain the original sheet traversal and update order
			// Found ground, so add to statics list
			sheet->m_angle = (General_GetRandomNumber()&0xFF)/256.0f * 6.28f;
			sheet->m_state = 1;
			if(CullZones_FindAttributesForCoors((CullZonesPosition){sheet->m_basePos.x, sheet->m_basePos.y, sheet->m_basePos.z}, NULL) & ATTRZONE_NORAIN)
				sheet->m_isVisible = false;
			else
				sheet->m_isVisible = true;
			RubbishSheet_RemoveFromList(sheet);
			RubbishSheet_AddToList(sheet, &StartStaticsList);
		}
	}

	// Process animation
	sheet = StartMoversList.m_next;
	while(sheet != &EndMoversList){
		// Retain the original sheet traversal and update order
		uint32_t currentTime = (*Timer_GetTimeInMilliseconds()) - sheet->m_moveStart;
		if(currentTime < sheet->m_moveDuration){
			// Retain the original sheet traversal and update order
			// Animation
			int step = 16 * currentTime / sheet->m_moveDuration;	// 16 steps in animation
			int stepTime = sheet->m_moveDuration/16;	// time in each step
			float s = (float)(currentTime - stepTime*step) / stepTime;	// position on step
			float t = (float)currentTime / sheet->m_moveDuration;	// position on total animation
			// factors for xy and z-movment
			float fxy = aAnimations[sheet->m_animationType][step]*(1.0f-s) + aAnimations[sheet->m_animationType][step+1]*s;
			float fz = aAnimations[sheet->m_animationType][step+17]*(1.0f-s) + aAnimations[sheet->m_animationType][step+1+17]*s;
			sheet->m_animatedPos.x = sheet->m_basePos.x + fxy*sheet->m_xDist;
			sheet->m_animatedPos.y = sheet->m_basePos.y + fxy*sheet->m_yDist;
			sheet->m_animatedPos.z = (1.0f-t)*sheet->m_basePos.z + t*sheet->m_targetZ + fz*sheet->m_animHeight;
			sheet->m_angle += (*Timer_GetTimeStep())*0.04f;
			if(sheet->m_angle > 6.28f)
				sheet->m_angle -= 6.28f;
			sheet = sheet->m_next;
		}else{
			// Retain the original sheet traversal and update order
			// End of animation, back into statics list
			sheet->m_basePos.x += sheet->m_xDist;
			sheet->m_basePos.y += sheet->m_yDist;
			sheet->m_basePos.z = sheet->m_targetZ;
			sheet->m_state = 1;
			sheet->m_isVisible = sheet->m_targetIsVisible;

			RubbishSheet *next = sheet->m_next;
			RubbishSheet_RemoveFromList(sheet);
			RubbishSheet_AddToList(sheet, &StartStaticsList);
			sheet = next;
		}
	}

	// Stir up a sheet by wind
	// FRAMETIME
	int freq;
	if(Rubbish_BackendWind() < 0.1f)
		freq = 31;
	else if(Rubbish_BackendWind() < 0.4f)
		freq = 7;
	else if(Rubbish_BackendWind() < 0.7f)
		freq = 1;
	else
		freq = 0;
	if(((*Timer_GetFrameCounter()) & freq) == 0){
		// Retain the original sheet traversal and update order
		// Pick a random sheet and set animation state if static
		int i = General_GetRandomNumber() % NUM_RUBBISH_SHEETS;
		if(aSheets[i].m_state == 1){
			// Retain the original sheet traversal and update order
			aSheets[i].m_moveStart = (*Timer_GetTimeInMilliseconds());
			aSheets[i].m_moveDuration = Rubbish_BackendWind()*1500.0f + 1000.0f;
			aSheets[i].m_animHeight = 0.2f;
			aSheets[i].m_xDist = 3.0f*Rubbish_BackendWind();
			aSheets[i].m_yDist = 3.0f*Rubbish_BackendWind();
			// Check if target position is ok
			float tx = aSheets[i].m_basePos.x + aSheets[i].m_xDist;
			float ty = aSheets[i].m_basePos.y + aSheets[i].m_yDist;
			float tz = aSheets[i].m_basePos.z + 3.0f;
			aSheets[i].m_targetZ = Rubbish_BackendGround(tx, ty, tz, &foundGround) + 0.1f;
			if(CullZones_FindAttributesForCoors((CullZonesPosition){tx, ty, aSheets[i].m_targetZ}, NULL) & ATTRZONE_NORAIN)
				aSheets[i].m_targetIsVisible = false;
			else
				aSheets[i].m_targetIsVisible = true;
			if(foundGround){
				// Retain the original sheet traversal and update order
				// start animation
				aSheets[i].m_state = 2;
				aSheets[i].m_animationType = 1;
				RubbishSheet_RemoveFromList(&aSheets[i]);
				RubbishSheet_AddToList(&aSheets[i], &StartMoversList);
			}
		}
	}

	// Remove sheets that are too far away
	int i = ((*Timer_GetFrameCounter())%(NUM_RUBBISH_SHEETS/4))*4;
	int last = (((*Timer_GetFrameCounter())%(NUM_RUBBISH_SHEETS/4)) + 1)*4;
	for(; i < last; i++){
		// Retain the original sheet traversal and update order
		if(aSheets[i].m_state == 1 &&
		   Rubbish_MagnitudeSqr2(Rubbish_XY(Rubbish_Subtract(aSheets[i].m_basePos, Rubbish_BackendCameraPosition()))) > SQR(RUBBISH_MAX_DIST+1.0f)){
			// Retain the original sheet traversal and update order
			aSheets[i].m_state = 0;
			RubbishSheet_RemoveFromList(&aSheets[i]);
			RubbishSheet_AddToList(&aSheets[i], &StartEmptyList);
		}
	}
}

void
Rubbish_SetVisibility(bool visible)
{
	// Set the original inverted visibility target
	bRubbishInvisible = !visible;
}

void
Rubbish_Init(void)
{
	// Link every original record and sentinel before acquiring textures
	int i;
	for(i = 0; i < NUM_RUBBISH_SHEETS; i++){
		// Retain the original sheet traversal and update order
		aSheets[i].m_state = 0;
		if(i < NUM_RUBBISH_SHEETS-1)
			aSheets[i].m_next = &aSheets[i+1];
		else
			aSheets[i].m_next = &EndEmptyList;
		if(i > 0)
			aSheets[i].m_prev = &aSheets[i-1];
		else
			aSheets[i].m_prev = &StartEmptyList;
	}

	StartEmptyList.m_next = &aSheets[0];
	StartEmptyList.m_prev = NULL;
	EndEmptyList.m_next = NULL;
	EndEmptyList.m_prev = &aSheets[NUM_RUBBISH_SHEETS-1];

	StartStaticsList.m_next = &EndStaticsList;
	StartStaticsList.m_prev = NULL;
	EndStaticsList.m_next = NULL;
	EndStaticsList.m_prev = &StartStaticsList;

	StartMoversList.m_next = &EndMoversList;
	StartMoversList.m_prev = NULL;
	EndMoversList.m_next = NULL;
	EndMoversList.m_prev = &StartMoversList;

	RubbishIndexList[0] = 0;
	RubbishIndexList[1] = 1;
	RubbishIndexList[2] = 2;
	RubbishIndexList[3] = 1;
	RubbishIndexList[4] = 3;
	RubbishIndexList[5] = 2;

	Rubbish_BackendTxdPush();
	int slot = Rubbish_BackendTxdFind("particle");
	Rubbish_BackendTxdSet(slot);
	gpRubbishTexture[0] = Rubbish_BackendTextureRead("gameleaf01_64", NULL);
	gpRubbishTexture[1] = Rubbish_BackendTextureRead("gameleaf02_64", NULL);
	gpRubbishTexture[2] = Rubbish_BackendTextureRead("newspaper01_64", NULL);
	gpRubbishTexture[3] = Rubbish_BackendTextureRead("newspaper02_64", NULL);
	Rubbish_BackendTxdPop();
	RubbishVisibility = 255;
	bRubbishInvisible = false;
}

void
Rubbish_Shutdown(void)
{
	// Destroy and clear each original texture in order
	Rubbish_BackendTextureDestroy(gpRubbishTexture[0]);
	gpRubbishTexture[0] = NULL;
	Rubbish_BackendTextureDestroy(gpRubbishTexture[1]);
	gpRubbishTexture[1] = NULL;
	Rubbish_BackendTextureDestroy(gpRubbishTexture[2]);
	gpRubbishTexture[2] = NULL;
	Rubbish_BackendTextureDestroy(gpRubbishTexture[3]);
	gpRubbishTexture[3] = NULL;
}
//- rouz edit (ChatGPT)
