#include "common.h"

#include "main.h"
#include "Antennas.h"

CAntenna CAntennas::aAntennas[NUMANTENNAS];
//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
static CAntenna EnvMapAntennas[NUMANTENNAS];
static int NumEnvMapAntennas;
#endif
//- rouz edit (ChatGPT)

void
CAntennas::Init(void)
{
	int i;
	for(i = 0; i < NUMANTENNAS; i++){
		aAntennas[i].active = false;
		aAntennas[i].updatedLastFrame = false;
	}
	// Clear the auxiliary-camera-only antenna queue when starting a new scene
	//+ rouz edit (ChatGPT)
	#ifdef REVC_SOFTWARE_POLYGONS
	NumEnvMapAntennas = 0;
	#endif
	//- rouz edit (ChatGPT)
}

// Free antennas that aren't used anymore
void
CAntennas::Update(void)
{
	int i;

	for(i = 0; i < NUMANTENNAS; i++){
		if(aAntennas[i].active && !aAntennas[i].updatedLastFrame)
			aAntennas[i].active = false;
		aAntennas[i].updatedLastFrame = false;
	}
}

// Add a new one or update an old one
void
CAntennas::RegisterOne(uint32 id, CVector dir, CVector position, float length)
{
	int i, j;

	for(i = 0; i < NUMANTENNAS; i++)
		if(aAntennas[i].active && aAntennas[i].id == id)
			break;

	if(i >= NUMANTENNAS){
		// not found, register new one

		// find empty slot
		for(i = 0; i < NUMANTENNAS; i++)
			if(!aAntennas[i].active)
				break;

		// there is space
		if(i < NUMANTENNAS){
			aAntennas[i].active = true;
			aAntennas[i].updatedLastFrame = true;
			aAntennas[i].id = id;
			aAntennas[i].segmentLength = length/6.0f;
			for(j = 0; j < 6; j++){
				aAntennas[i].pos[j] = position + dir*j*aAntennas[i].segmentLength;
				aAntennas[i].speed[j] = CVector(0.0f, 0.0f, 0.0f);
			}
		}
	}else{
		// found, update
		aAntennas[i].Update(dir, position);
		aAntennas[i].updatedLastFrame = true;
	}
}

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
void
CAntennas::RegisterOneForEnvMap(uint32 id, CVector dir, CVector position, float length)
{
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
	CAntenna *antenna = &EnvMapAntennas[NumEnvMapAntennas++];
	antenna->active = true;
	antenna->id = id;
	antenna->segmentLength = length/6.0f;
	for(int i = 0; i < 6; i++)
		antenna->pos[i] = position + dir*i*antenna->segmentLength;
}
#endif
//- rouz edit (ChatGPT)

static RwIm3DVertex vertexbufferA[2];

void
CAntennas::Render(void)
{
	int i, j;

	PUSH_RENDERGROUP("CAntennas::Render");
	for(i = 0; i < NUMANTENNAS; i++){
		if(!aAntennas[i].active)
			continue;

		RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
		RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
		RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
		RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);

		for(j = 0; j < 5; j++){
			RwIm3DVertexSetRGBA(&vertexbufferA[0], 200, 200, 200, 100);
			RwIm3DVertexSetPos(&vertexbufferA[0],
				aAntennas[i].pos[j].x,
				aAntennas[i].pos[j].y,
				aAntennas[i].pos[j].z);
			RwIm3DVertexSetRGBA(&vertexbufferA[1], 200, 200, 200, 100);
			RwIm3DVertexSetPos(&vertexbufferA[1],
				aAntennas[i].pos[j+1].x,
				aAntennas[i].pos[j+1].y,
				aAntennas[i].pos[j+1].z);

			// LittleTest();
			if(RwIm3DTransform(vertexbufferA, 2, nil, 0)){
				RwIm3DRenderLine(0, 1);
				RwIm3DEnd();
			}
		}
	}

	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);

	POP_RENDERGROUP();
}

//+ rouz edit (ChatGPT)
#ifdef REVC_SOFTWARE_POLYGONS
void
CAntennas::RenderForEnvMap(void)
{
	// Render and consume straight antenna entries for reflection-only vehicles
	if(NumEnvMapAntennas == 0)
		return;
	PUSH_RENDERGROUP("CAntennas::RenderForEnvMap");
	RwRenderStateSet(rwRENDERSTATEZWRITEENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)TRUE);
	RwRenderStateSet(rwRENDERSTATESRCBLEND, (void*)rwBLENDSRCALPHA);
	RwRenderStateSet(rwRENDERSTATEDESTBLEND, (void*)rwBLENDINVSRCALPHA);
	RwRenderStateSet(rwRENDERSTATETEXTURERASTER, nil);
	// Draw each auxiliary antenna as the same five semi-transparent line segments
	for(int i = 0; i < NumEnvMapAntennas; i++)
		for(int j = 0; j < 5; j++){
			RwIm3DVertexSetRGBA(&vertexbufferA[0], 200, 200, 200, 100);
			RwIm3DVertexSetPos(&vertexbufferA[0],
				EnvMapAntennas[i].pos[j].x, EnvMapAntennas[i].pos[j].y, EnvMapAntennas[i].pos[j].z);
			RwIm3DVertexSetRGBA(&vertexbufferA[1], 200, 200, 200, 100);
			RwIm3DVertexSetPos(&vertexbufferA[1],
				EnvMapAntennas[i].pos[j+1].x, EnvMapAntennas[i].pos[j+1].y, EnvMapAntennas[i].pos[j+1].z);
			if(RwIm3DTransform(vertexbufferA, 2, nil, 0)){
				RwIm3DRenderLine(0, 1);
				RwIm3DEnd();
			}
		}
	RwRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void*)FALSE);
	NumEnvMapAntennas = 0;
	POP_RENDERGROUP();
}
#endif
//- rouz edit (ChatGPT)

void
CAntenna::Update(CVector dir, CVector basepos)
{
	int i;

	pos[0] = basepos;
	pos[1] = basepos + dir*segmentLength;

	for(i = 2; i < 6; i++){
		CVector basedir = pos[i-1] - pos[i-2];
		CVector newdir = pos[i] - pos[i-1] +	// drag along
			dir*0.1f +	// also drag up a bit for stiffness
			speed[i];	// and keep moving
		newdir.Normalise();
		newdir *= segmentLength;
		CVector newpos = pos[i-1] + (basedir + newdir)/2.0f;
		speed[i] = (newpos - pos[i])*0.9f;
		pos[i] = newpos;
	}
}
