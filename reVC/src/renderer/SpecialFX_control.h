//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
extern bool SpecialFX_bVideoCam,SpecialFX_bLiftCam,SpecialFX_bSnapShotActive;
extern int32_t SpecialFX_SnapShotFrames;
void SpecialFX_Init(void);
void SpecialFX_Update(void);
void SpecialFX_Shutdown(void);
void SpecialFX_Render(void);
void SpecialFX_AddWeaponStreak(int32_t type);
void SpecialFX_Render2DFXs(void);
void *SpecialFX_LookForBat(void *object,void *data);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
