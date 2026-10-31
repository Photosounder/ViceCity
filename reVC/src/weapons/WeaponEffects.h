//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct WeaponEffectsVector {float x,y,z;} WeaponEffectsVector;
typedef struct WeaponEffects {
    bool m_bActive;
    WeaponEffectsVector m_vecPos;
    uint8_t m_nRed,m_nGreen,m_nBlue,m_nAlpha;
    float m_fSize,m_fRotation;
} WeaponEffects;
#ifdef __cplusplus
extern "C" {
#endif
extern WeaponEffects gCrossHair;
void WeaponEffects_Construct(WeaponEffects *self);
void WeaponEffects_Deinit(WeaponEffects *self);
void WeaponEffects_Init(void);
void WeaponEffects_Shutdown(void);
void WeaponEffects_MarkTarget(WeaponEffectsVector pos,uint8_t red,uint8_t green,uint8_t blue,uint8_t alpha,float size);
void WeaponEffects_ClearCrossHair(void);
void WeaponEffects_Render(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
