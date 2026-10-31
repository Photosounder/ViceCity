//+ rouz edit (ChatGPT)
#include "Antennas_backend.h"
#include <math.h>

static AntennasVector Antennas_Add(AntennasVector left, AntennasVector right)
{
	// Add each original vector component without regrouping expressions
	AntennasVector result = {left.x+right.x, left.y+right.y, left.z+right.z};
	return result;
}
static AntennasVector Antennas_Subtract(AntennasVector left, AntennasVector right)
{
	// Subtract each original vector component
	AntennasVector result = {left.x-right.x, left.y-right.y, left.z-right.z};
	return result;
}
static AntennasVector Antennas_Scale(AntennasVector vector, float scalar)
{
	// Preserve the original componentwise scalar multiplication
	AntennasVector result = {vector.x*scalar, vector.y*scalar, vector.z*scalar};
	return result;
}
static AntennasVector Antennas_Divide(AntennasVector vector, float scalar)
{
	// Preserve the original componentwise division before adding the result
	AntennasVector result = {vector.x/scalar, vector.y/scalar, vector.z/scalar};
	return result;
}
static void Antennas_Normalise(AntennasVector *vector)
{
	// Retain the original float squared length and reciprocal square root
	float sq = vector->x*vector->x + vector->y*vector->y + vector->z*vector->z;
	if (sq > 0.0f) {
		// Scale all three components in the original order
		float invsqrt = 1.0f/sqrtf(sq);
		vector->x *= invsqrt;
		vector->y *= invsqrt;
		vector->z *= invsqrt;
	} else
		vector->x = 1.0f;
}

static Antenna aAntennas[NUMANTENNAS];
#ifdef REVC_SOFTWARE_POLYGONS
static Antenna EnvMapAntennas[NUMANTENNAS];
static int NumEnvMapAntennas;
#endif

void
Antennas_Init(void)
{
	// Clear only the original active and updated flags and auxiliary count
	int i;
	for(i = 0; i < NUMANTENNAS; i++){
		// Retain the original antenna traversal and update order
		aAntennas[i].active = false;
		aAntennas[i].updatedLastFrame = false;
	}
	// Clear the auxiliary-camera-only antenna queue when starting a new scene
	#ifdef REVC_SOFTWARE_POLYGONS
	NumEnvMapAntennas = 0;
	#endif
}

// Free antennas that aren't used anymore
void
Antennas_Update(void)
{
	// Expire unregistered antennas before clearing every updated flag
	int i;

	for(i = 0; i < NUMANTENNAS; i++){
		// Retain the original antenna traversal and update order
		if(aAntennas[i].active && !aAntennas[i].updatedLastFrame)
			aAntennas[i].active = false;
		aAntennas[i].updatedLastFrame = false;
	}
}

// Add a new one or update an old one
void
Antennas_RegisterOne(uint32_t id, AntennasVector dir, AntennasVector position, float length)
{
	// Find the original matching ID or first available record
	int i, j;

	for(i = 0; i < NUMANTENNAS; i++)
		if(aAntennas[i].active && aAntennas[i].id == id)
			break;

	if(i >= NUMANTENNAS){
		// Retain the original antenna traversal and update order
		// not found, register new one

		// find empty slot
		for(i = 0; i < NUMANTENNAS; i++)
			if(!aAntennas[i].active)
				break;

		// there is space
		if(i < NUMANTENNAS){
			// Retain the original antenna traversal and update order
			aAntennas[i].active = true;
			aAntennas[i].updatedLastFrame = true;
			aAntennas[i].id = id;
			aAntennas[i].segmentLength = length/6.0f;
			for(j = 0; j < 6; j++){
				// Retain the original antenna traversal and update order
				aAntennas[i].pos[j] = Antennas_Add(position, Antennas_Scale(Antennas_Scale(dir, (float)j), aAntennas[i].segmentLength));
				aAntennas[i].speed[j] = (AntennasVector){0.0f, 0.0f, 0.0f};
			}
		}
	}else{
		// Retain the original antenna traversal and update order
		// found, update
		Antenna_Update(&aAntennas[i], dir, position);
		aAntennas[i].updatedLastFrame = true;
	}
}

#ifdef REVC_SOFTWARE_POLYGONS
void
Antennas_RegisterOneForEnvMap(uint32_t id, AntennasVector dir, AntennasVector position, float length)
{
	// Retain main-pool and auxiliary-queue duplicate checks before allocating
	// Use the main spring-simulated antenna whenever its normal pre-render hook registered it
	for(int i = 0; i < NUMANTENNAS; i++)
		if(aAntennas[i].active && aAntennas[i].id == id)
			return;
	// Avoid duplicate reflection entries for the same vehicle
	for(int i = 0; i < NumEnvMapAntennas; i++)
		if(EnvMapAntennas[i].id == id)
			return;
	// Build a straight auxiliary antenna for vehicles seen only by the reflection camera
	if(NumEnvMapAntennas >= NUMANTENNAS)
		return;
	Antenna *antenna = &EnvMapAntennas[NumEnvMapAntennas++];
	antenna->active = true;
	antenna->id = id;
	antenna->segmentLength = length/6.0f;
	for(int i = 0; i < 6; i++)
		antenna->pos[i] = Antennas_Add(position, Antennas_Scale(Antennas_Scale(dir, (float)i), antenna->segmentLength));
}
#endif



void
Antennas_Render(void)
{
	// Preserve main-pool traversal and every render-state and vertex operation
	int i, j;

	Antennas_BackendPush("CAntennas::Render");
	for(i = 0; i < NUMANTENNAS; i++){
		// Retain the original antenna traversal and update order
		if(!aAntennas[i].active)
			continue;

		Antennas_BackendZWrite();
		Antennas_BackendVertexAlpha(true);
		Antennas_BackendSrcBlend();
		Antennas_BackendDestBlend();
		Antennas_BackendTextureRaster();

		for(j = 0; j < 5; j++){
			// Retain the original antenna traversal and update order
			Antennas_BackendSetRGBA(0, 200, 200, 200, 100);
			Antennas_BackendSetPos(0,
				aAntennas[i].pos[j].x,
				aAntennas[i].pos[j].y,
				aAntennas[i].pos[j].z);
			Antennas_BackendSetRGBA(1, 200, 200, 200, 100);
			Antennas_BackendSetPos(1,
				aAntennas[i].pos[j+1].x,
				aAntennas[i].pos[j+1].y,
				aAntennas[i].pos[j+1].z);

			// LittleTest();
			if(Antennas_BackendTransform()){
				// Retain the original antenna traversal and update order
				Antennas_BackendRenderLine();
				Antennas_BackendEnd();
			}
		}
	}

	Antennas_BackendVertexAlpha(false);

	Antennas_BackendPop();
}

#ifdef REVC_SOFTWARE_POLYGONS
void
Antennas_RenderForEnvMap(void)
{
	// Render and consume the original auxiliary queue
	// Render and consume straight antenna entries for reflection-only vehicles
	if(NumEnvMapAntennas == 0)
		return;
	Antennas_BackendPush("CAntennas::RenderForEnvMap");
	Antennas_BackendZWrite();
	Antennas_BackendVertexAlpha(true);
	Antennas_BackendSrcBlend();
	Antennas_BackendDestBlend();
	Antennas_BackendTextureRaster();
	// Draw each auxiliary antenna as the same five semi-transparent line segments
	for(int i = 0; i < NumEnvMapAntennas; i++)
		for(int j = 0; j < 5; j++){
			// Retain the original antenna traversal and update order
			Antennas_BackendSetRGBA(0, 200, 200, 200, 100);
			Antennas_BackendSetPos(0,
				EnvMapAntennas[i].pos[j].x, EnvMapAntennas[i].pos[j].y, EnvMapAntennas[i].pos[j].z);
			Antennas_BackendSetRGBA(1, 200, 200, 200, 100);
			Antennas_BackendSetPos(1,
				EnvMapAntennas[i].pos[j+1].x, EnvMapAntennas[i].pos[j+1].y, EnvMapAntennas[i].pos[j+1].z);
			if(Antennas_BackendTransform()){
				// Retain the original antenna traversal and update order
				Antennas_BackendRenderLine();
				Antennas_BackendEnd();
			}
		}
	Antennas_BackendVertexAlpha(false);
	NumEnvMapAntennas = 0;
	Antennas_BackendPop();
}
#endif

void
Antenna_Update(Antenna *antenna, AntennasVector dir, AntennasVector basepos)
{
	// Update the original spring chain sequentially with explicit vector operations
	int i;

	antenna->pos[0] = basepos;
	antenna->pos[1] = Antennas_Add(basepos, Antennas_Scale(dir, antenna->segmentLength));

	for(i = 2; i < 6; i++){
		// Retain the original antenna traversal and update order
		AntennasVector basedir = Antennas_Subtract(antenna->pos[i-1], antenna->pos[i-2]);
		AntennasVector newdir = Antennas_Add(Antennas_Add(Antennas_Subtract(antenna->pos[i], antenna->pos[i-1]), Antennas_Scale(dir, 0.1f)), antenna->speed[i]);
		Antennas_Normalise(&newdir);
		newdir = Antennas_Scale(newdir, antenna->segmentLength);
		AntennasVector newpos = Antennas_Add(antenna->pos[i-1], Antennas_Divide(Antennas_Add(basedir, newdir), 2.0f));
		antenna->speed[i] = Antennas_Scale(Antennas_Subtract(newpos, antenna->pos[i]), 0.9f);
		antenna->pos[i] = newpos;
	}
}
//- rouz edit (ChatGPT)
