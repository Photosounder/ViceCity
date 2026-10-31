//+ rouz edit (ChatGPT)
#pragma once
typedef struct LightsColour {float red,green,blue,alpha;} LightsColour;
#ifdef __cplusplus
extern "C" {
#endif
extern void *Lights_Ambient,*Lights_Direct,*Lights_ExtraDirectionals[4];
extern int LightStrengths[4],NumExtraDirLightsInWorld;
void Lights_SetLightsWithTimeOfDayColour(void *world);
void *Lights_Create(void *world);
void Lights_Destroy(void *world);
void Lights_WorldReplaceNormalLightsWithScorched(void *world,float l);
void Lights_WorldReplaceScorchedLightsWithNormal(void *world);
void Lights_AddAnExtraDirectionalLight(void *world,float dirx,float diry,float dirz,float red,float green,float blue);
void Lights_RemoveExtraDirectionalLights(void *world);
void Lights_SetAmbientAndDirectionalColours(float f);
void Lights_SetFlashyColours(float f);
void Lights_SetFlashyColours_Mild(float f);
void Lights_SetBrightMarkerColours(float f);
void Lights_ReSetAmbientAndDirectionalColours(void);
void Lights_DeActivateDirectional(void);
void Lights_ActivateDirectional(void);
void Lights_SetAmbientColours(void);
void Lights_SetAmbientColoursForPedsCarsAndObjects(void);
void Lights_SetAmbientColoursToIndicateRoadGroup(int i);
void Lights_SetAmbientColour(const LightsColour *colour);
void Lights_SetFullAmbient(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
