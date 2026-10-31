//+ rouz edit (ChatGPT)
#include "config.h"
#include "Lights.h"
#include "Lights_backend.h"
#include "Timer.h"
#include "Timecycle.h"
#include "Weather.h"
#include "ZoneCull.h"
#include <stddef.h>
#include <math.h>
#define Min(a,b) ((a)<(b)?(a):(b))
#define Max(a,b) ((a)>(b)?(a):(b))
static TimeCycleVector Lights_CrossProduct(TimeCycleVector a,TimeCycleVector b)
{
    // Preserve the original vector cross product evaluation order
    const TimeCycleVector value={a.y*b.z-a.z*b.y,a.z*b.x-a.x*b.z,a.x*b.y-a.y*b.x};return value;
}
static void Lights_Normalise(TimeCycleVector *value)
{
    // Preserve the original zero-vector fallback and reciprocal square root arithmetic
    float sq=value->x*value->x+value->y*value->y+value->z*value->z;
    if(sq>0.0f){
        // Scale all three components with the original reciprocal square root
        float invsqrt=1.0f/sqrtf(sq);value->x*=invsqrt;value->y*=invsqrt;value->z*=invsqrt;
    }else value->x=1.0f;
}
//+ rouz edit (ChatGPT)


void *Lights_Ambient;
void *Lights_Direct;
void *Lights_ExtraDirectionals[4] = { NULL };
int LightStrengths[4];
int NumExtraDirLightsInWorld;

LightsColour AmbientLightColourForFrame;
LightsColour AmbientLightColourForFrame_PedsCarsAndObjects;
LightsColour DirectionalLightColourForFrame;

LightsColour AmbientLightColour;
LightsColour DirectionalLightColour;


void
Lights_SetLightsWithTimeOfDayColour(void *world)
{
    // Preserve the complete original lighting operation and its mutation order
	TimeCycleVector vec1, vec2, vecsun;
	LightsMatrix mat;

	if(Lights_Ambient){
		if(Lights_HostBlurColours()){
			AmbientLightColourForFrame.red = TimeCycle_GetAmbientRed_Bl() * Lights_HostMultiplier();
			AmbientLightColourForFrame.green = TimeCycle_GetAmbientGreen_Bl() * Lights_HostMultiplier();
			AmbientLightColourForFrame.blue = TimeCycle_GetAmbientBlue_Bl() * Lights_HostMultiplier();
		}else{
			AmbientLightColourForFrame.red = TimeCycle_GetAmbientRed() * Lights_HostMultiplier();
			AmbientLightColourForFrame.green = TimeCycle_GetAmbientGreen() * Lights_HostMultiplier();
			AmbientLightColourForFrame.blue = TimeCycle_GetAmbientBlue() * Lights_HostMultiplier();
		}

		if(Lights_HostBlurColours()){
			AmbientLightColourForFrame_PedsCarsAndObjects.red = TimeCycle_GetAmbientRed_Obj_Bl() * Lights_HostMultiplier();
			AmbientLightColourForFrame_PedsCarsAndObjects.green = TimeCycle_GetAmbientGreen_Obj_Bl() * Lights_HostMultiplier();
			AmbientLightColourForFrame_PedsCarsAndObjects.blue = TimeCycle_GetAmbientBlue_Obj_Bl() * Lights_HostMultiplier();
		}else{
			AmbientLightColourForFrame_PedsCarsAndObjects.red = TimeCycle_GetAmbientRed_Obj() * Lights_HostMultiplier();
			AmbientLightColourForFrame_PedsCarsAndObjects.green = TimeCycle_GetAmbientGreen_Obj() * Lights_HostMultiplier();
			AmbientLightColourForFrame_PedsCarsAndObjects.blue = TimeCycle_GetAmbientBlue_Obj() * Lights_HostMultiplier();
		}

		if(Weather_LightningFlash && !CullZones_CamNoRain()){
			AmbientLightColourForFrame.red = 1.0f;
			AmbientLightColourForFrame.green = 1.0f;
			AmbientLightColourForFrame.blue = 1.0f;

			AmbientLightColourForFrame_PedsCarsAndObjects.red = 1.0f;
			AmbientLightColourForFrame_PedsCarsAndObjects.green = 1.0f;
			AmbientLightColourForFrame_PedsCarsAndObjects.blue = 1.0f;
		}
		Lights_HostColour(Lights_Ambient, &AmbientLightColourForFrame);
	}

	if(Lights_Direct){
		DirectionalLightColourForFrame.red = TimeCycle_GetDirectionalRed() * Lights_HostMultiplier();
		DirectionalLightColourForFrame.green = TimeCycle_GetDirectionalGreen() * Lights_HostMultiplier();
		DirectionalLightColourForFrame.blue = TimeCycle_GetDirectionalBlue() * Lights_HostMultiplier();
		Lights_HostColour(Lights_Direct, &DirectionalLightColourForFrame);

		vecsun = TimeCycle_m_VectorToSun[TimeCycle_m_CurrentStoredValue];
		vec1 = (TimeCycleVector){0.0f, 0.0f, 1.0f};
		vec2 = Lights_CrossProduct(vec1, vecsun);
		Lights_Normalise(&vec2);
		vec1 = Lights_CrossProduct(vec2, vecsun);
		mat.at.x = -vecsun.x;
		mat.at.y = -vecsun.y;
		mat.at.z = -vecsun.z;
		mat.right.x = vec1.x;
		mat.right.y = vec1.y;
		mat.right.z = vec1.z;
		mat.up.x = vec2.x;
		mat.up.y = vec2.y;
		mat.up.z = vec2.z;
		Lights_HostTransform(Lights_HostFrame(Lights_Direct), &mat);
	}

	if(Lights_HostBrightness() > 256){
		float f1 = 2.0f * (Lights_HostBrightness()/256.0f - 1.0f) * 0.6f + 1.0f;
		float f2 = 3.0f * (Lights_HostBrightness()/256.0f - 1.0f) * 0.6f + 1.0f;

		AmbientLightColourForFrame.red = Min(1.0f, AmbientLightColourForFrame.red * f2);
		AmbientLightColourForFrame.green = Min(1.0f, AmbientLightColourForFrame.green * f2);
		AmbientLightColourForFrame.blue = Min(1.0f, AmbientLightColourForFrame.blue * f2);
		AmbientLightColourForFrame_PedsCarsAndObjects.red = Min(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.red * f1);
		AmbientLightColourForFrame_PedsCarsAndObjects.green = Min(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.green * f1);
		AmbientLightColourForFrame_PedsCarsAndObjects.blue = Min(1.0f, AmbientLightColourForFrame_PedsCarsAndObjects.blue * f1);
#ifdef FIX_BUGS
		DirectionalLightColourForFrame.red = Min(1.0f, DirectionalLightColourForFrame.red * f1);
		DirectionalLightColourForFrame.green = Min(1.0f, DirectionalLightColourForFrame.green * f1);
		DirectionalLightColourForFrame.blue = Min(1.0f, DirectionalLightColourForFrame.blue * f1);
#else
		DirectionalLightColourForFrame.red = Min(1.0f, AmbientLightColourForFrame.red * f1);
		DirectionalLightColourForFrame.green = Min(1.0f, AmbientLightColourForFrame.green * f1);
		DirectionalLightColourForFrame.blue = Min(1.0f, AmbientLightColourForFrame.blue * f1);
#endif
	}
}

void*
Lights_Create(void *world)
{
    // Preserve the complete original lighting operation and its mutation order
	int i;
	LightsColour color;
	void *frame;

	if(world == NULL)
		return NULL;

	Lights_Ambient = Lights_HostCreate(LIGHTS_AMBIENT);
	Lights_HostFlags(Lights_Ambient, 1);
	color.red = 0.25f;
	color.green = 0.25f;
	color.blue = 0.2f;
	Lights_HostColour(Lights_Ambient, &color);

	Lights_Direct = Lights_HostCreate(LIGHTS_DIRECTIONAL);
	Lights_HostFlags(Lights_Direct, 1);
	color.red = 1.0f;
	color.green = 0.85f;
	color.blue = 0.45f;
	Lights_HostColour(Lights_Direct, &color);
	Lights_HostRadius(Lights_Direct, 2.0f);
	frame = Lights_HostCreateFrame();
	Lights_HostSetFrame(Lights_Direct, frame);
	LightsVector axis = { 1.0f, 1.0f, 0.0f };
	Lights_HostRotate(frame, &axis, 160.0f);

	Lights_HostAdd(world, Lights_Ambient);
	Lights_HostAdd(world, Lights_Direct);

	for(i = 0; i < NUMEXTRADIRECTIONALS; i++){
		Lights_ExtraDirectionals[i] = Lights_HostCreate(LIGHTS_DIRECTIONAL);
		Lights_HostFlags(Lights_ExtraDirectionals[i], 0);
		color.red = 1.0f;
		color.green = 0.5f;
		color.blue = 0.0f;
		Lights_HostColour(Lights_ExtraDirectionals[i], &color);
		Lights_HostRadius(Lights_ExtraDirectionals[i], 2.0f);
		frame = Lights_HostCreateFrame();
		Lights_HostSetFrame(Lights_ExtraDirectionals[i], frame);
		Lights_HostAdd(world, Lights_ExtraDirectionals[i]);
	}

	return world;
}

void
Lights_Destroy(void *world)
{
    // Preserve the complete original lighting operation and its mutation order
	int i;

	if(world == NULL)
		return;

	if(Lights_Ambient){
		Lights_HostRemove(world, Lights_Ambient);
		Lights_HostDestroy(Lights_Ambient);
		Lights_Ambient = NULL;
	}

	if(Lights_Direct){
		Lights_HostRemove(world, Lights_Direct);
		Lights_HostDestroyFrame(Lights_HostFrame(Lights_Direct));
		Lights_HostDestroy(Lights_Direct);
		Lights_Direct = NULL;
	}

	for(i = 0; i < NUMEXTRADIRECTIONALS; i++)
		if(Lights_ExtraDirectionals[i]){
			Lights_HostRemove(world, Lights_ExtraDirectionals[i]);
			Lights_HostDestroyFrame(Lights_HostFrame(Lights_ExtraDirectionals[i]));
			Lights_HostDestroy(Lights_ExtraDirectionals[i]);
			Lights_ExtraDirectionals[i] = NULL;
		}
}

void
Lights_WorldReplaceNormalLightsWithScorched(void *world, float l)
{
    // Preserve the complete original lighting operation and its mutation order
	LightsColour color;
	color.red = l;
	color.green = l;
	color.blue = l;
	Lights_HostColour(Lights_Ambient, &color);
	Lights_HostFlags(Lights_Direct, 0);
}

void
Lights_WorldReplaceScorchedLightsWithNormal(void *world)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostColour(Lights_Ambient, &AmbientLightColourForFrame);
	Lights_HostFlags(Lights_Direct, 1);
}

void
Lights_AddAnExtraDirectionalLight(void *world, float dirx, float diry, float dirz, float red, float green, float blue)
{
    // Preserve the complete original lighting operation and its mutation order
	float strength;
	int weakest;
	int i, n;
	LightsColour color;
	LightsVectorOut dir;

	strength = Max(Max(red, green), blue);
	n = -1;
	if(NumExtraDirLightsInWorld < NUMEXTRADIRECTIONALS)
		n = NumExtraDirLightsInWorld;
	else{
		weakest = strength;
		for(i = 0; i < NUMEXTRADIRECTIONALS; i++)
			if(LightStrengths[i] < weakest){
				weakest = LightStrengths[i];
				n = i;
			}
	}

	if(n < 0)
		return;

	color.red = red;
	color.green = green;
	color.blue = blue;
	Lights_HostColour(Lights_ExtraDirectionals[n], &color);
	dir = Lights_HostAt(Lights_HostMatrix(Lights_HostFrame(Lights_ExtraDirectionals[n])));
	*dir.x = -dirx;
	*dir.y = -diry;
	*dir.z = -dirz;
	Lights_HostUpdateMatrix(Lights_HostMatrix(Lights_HostFrame(Lights_ExtraDirectionals[n])));
	Lights_HostUpdateObjects(Lights_HostFrame(Lights_ExtraDirectionals[n]));
	Lights_HostFlags(Lights_ExtraDirectionals[n], 1);
	LightStrengths[n] = strength;
	NumExtraDirLightsInWorld = Min(NumExtraDirLightsInWorld+1, NUMEXTRADIRECTIONALS);
}

void
Lights_RemoveExtraDirectionalLights(void *world)
{
    // Preserve the complete original lighting operation and its mutation order
	int i;
	for(i = 0; i < NumExtraDirLightsInWorld; i++)
		Lights_HostFlags(Lights_ExtraDirectionals[i], 0);
	NumExtraDirLightsInWorld = 0;
}

void
Lights_SetAmbientAndDirectionalColours(float f)
{
    // Preserve the complete original lighting operation and its mutation order
	AmbientLightColour.red = AmbientLightColourForFrame.red * f;
	AmbientLightColour.green = AmbientLightColourForFrame.green * f;
	AmbientLightColour.blue = AmbientLightColourForFrame.blue * f;

	DirectionalLightColour.red = DirectionalLightColourForFrame.red * f;
	DirectionalLightColour.green = DirectionalLightColourForFrame.green * f;
	DirectionalLightColour.blue = DirectionalLightColourForFrame.blue * f;

	Lights_HostColour(Lights_Ambient, &AmbientLightColour);
	Lights_HostColour(Lights_Direct, &DirectionalLightColour);
}

// unused
void
Lights_SetFlashyColours(float f)
{
    // Preserve the complete original lighting operation and its mutation order
	if((*Timer_GetTimeInMilliseconds()) & 0x100){
		AmbientLightColour.red = 1.0f;
		AmbientLightColour.green = 1.0f;
		AmbientLightColour.blue = 1.0f;

		DirectionalLightColour.red = DirectionalLightColourForFrame.red;
		DirectionalLightColour.green = DirectionalLightColourForFrame.green;
		DirectionalLightColour.blue = DirectionalLightColourForFrame.blue;

		Lights_HostColour(Lights_Ambient, &AmbientLightColour);
		Lights_HostColour(Lights_Direct, &DirectionalLightColour);
	}else{
		Lights_SetAmbientAndDirectionalColours(f * 0.75f);
	}
}

// unused
void
Lights_SetFlashyColours_Mild(float f)
{
    // Preserve the complete original lighting operation and its mutation order
	if((*Timer_GetTimeInMilliseconds()) & 0x100){
		AmbientLightColour.red = 0.65f;
		AmbientLightColour.green = 0.65f;
		AmbientLightColour.blue = 0.65f;

		DirectionalLightColour.red = DirectionalLightColourForFrame.red;
		DirectionalLightColour.green = DirectionalLightColourForFrame.green;
		DirectionalLightColour.blue = DirectionalLightColourForFrame.blue;

		Lights_HostColour(Lights_Ambient, &AmbientLightColour);
		Lights_HostColour(Lights_Direct, &DirectionalLightColour);
	}else{
		Lights_SetAmbientAndDirectionalColours(f * 0.9f);
	}
}

void
Lights_SetBrightMarkerColours(float f)
{
    // Preserve the complete original lighting operation and its mutation order
	AmbientLightColour.red = 0.6f;
	AmbientLightColour.green = 0.6f;
	AmbientLightColour.blue = 0.6f;

	DirectionalLightColour.red = (1.0f - DirectionalLightColourForFrame.red) * 0.4f + DirectionalLightColourForFrame.red;
	DirectionalLightColour.green = (1.0f - DirectionalLightColourForFrame.green) * 0.4f + DirectionalLightColourForFrame.green;
	DirectionalLightColour.blue = (1.0f - DirectionalLightColourForFrame.blue) * 0.4f + DirectionalLightColourForFrame.blue;

	Lights_HostColour(Lights_Ambient, &AmbientLightColour);
	Lights_HostColour(Lights_Direct, &DirectionalLightColour);
}

void
Lights_ReSetAmbientAndDirectionalColours(void)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostColour(Lights_Ambient, &AmbientLightColourForFrame);
	Lights_HostColour(Lights_Direct, &DirectionalLightColourForFrame);
}

void
Lights_DeActivateDirectional(void)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostFlags(Lights_Direct, 0);
}

void
Lights_ActivateDirectional(void)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostFlags(Lights_Direct, 1);
}

LightsColour FullLight = { 1.0f, 1.0f, 1.0f, 1.0f };

void
Lights_SetFullAmbient(void)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostColour(Lights_Ambient, &FullLight);
}

void
Lights_SetAmbientColours(void)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostColour(Lights_Ambient, &AmbientLightColourForFrame);
}

void
Lights_SetAmbientColoursForPedsCarsAndObjects(void)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostColour(Lights_Ambient, &AmbientLightColourForFrame_PedsCarsAndObjects);
}

uint8_t IndicateR[] = { 0, 255, 0, 0, 255, 255, 0 };
uint8_t IndicateG[] = { 0, 0, 255, 0, 255, 0, 255 };
uint8_t IndicateB[] = { 0, 0, 0, 255, 0, 255, 255 };

void
Lights_SetAmbientColoursToIndicateRoadGroup(int i)
{
    // Preserve the complete original lighting operation and its mutation order
	AmbientLightColour.red   = IndicateR[i%7]/255.0f;
	AmbientLightColour.green = IndicateG[i%7]/255.0f;
	AmbientLightColour.blue  = IndicateB[i%7]/255.0f;
	Lights_HostColour(Lights_Ambient, &AmbientLightColour);
}

void
Lights_SetAmbientColour(const LightsColour *color)
{
    // Preserve the complete original lighting operation and its mutation order
	Lights_HostColour(Lights_Ambient, color);
}

//- rouz edit (ChatGPT)

//- rouz edit (ChatGPT)
