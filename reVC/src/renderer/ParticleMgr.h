//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include "ParticleType.h"
#ifdef __cplusplus
class CParticle;
#ifdef LIBRW
namespace rw { struct Raster; }
typedef rw::Raster ParticleRaster;
#else
typedef struct RwRaster ParticleRaster;
#endif
#else
typedef struct CParticle CParticle;
typedef struct ParticleRaster ParticleRaster;
#endif
typedef union ParticleSystemColor {
    uint32_t color32;
    struct { uint8_t red,green,blue,alpha; };
} ParticleSystemColor;
typedef struct ParticleSystemStretch { float x,y; } ParticleSystemStretch;
enum
{
	ZCHECK_FIRST      = (1 << 0),
	ZCHECK_STEP       = (1 << 1),
	DRAW_OPAQUE       = (1 << 2),
	SCREEN_TRAIL      = (1 << 3),
	SPEED_TRAIL       = (1 << 4),
	RAND_VERT_V       = (1 << 5),
	CYCLE_ANIM        = (1 << 6),
	DRAW_DARK         = (1 << 7),
	VERT_TRAIL        = (1 << 8),
	_FLAG9            = (1 << 9),		// unused
	DRAWTOP2D         = (1 << 10),
	CLIPOUT2D         = (1 << 11),
	ZCHECK_BUMP       = (1 << 12),
	ZCHECK_BUMP_FIRST = (1 << 13)
};

typedef struct tParticleSystemData
{
	enum tParticleType m_Type;
	char          m_aName[20];
	float         m_fCreateRange;
	float         m_fDefaultInitialRadius;
	float         m_fExpansionRate;
	uint16_t        m_nZRotationInitialAngle;
	int16_t         m_nZRotationAngleChangeAmount;
	uint16_t        m_nZRotationChangeTime;
	uint16_t        m_nZRadiusChangeTime;
	float         m_fInitialZRadius;
	float         m_fZRadiusChangeAmount;
	int16_t         m_nFadeToBlackTime;
	uint8_t         m_nFadeToBlackInitialIntensity;
	int16_t         m_nFadeToBlackAmount;
	uint8_t         m_nFadeAlphaInitialIntensity;
	int16_t         m_nFadeAlphaTime;
	int16_t         m_nFadeAlphaAmount;
	uint8_t         m_nStartAnimationFrame;
	uint8_t         m_nFinalAnimationFrame;
	uint16_t        m_nAnimationSpeed;
	uint16_t        m_nRotationSpeed;
	float         m_fGravitationalAcceleration;
	int32_t         m_nFrictionDecceleration;
	int32_t         m_nLifeSpan;
	float         m_fPositionRandomError;
	float         m_fVelocityRandomError;
	float         m_fExpansionRateError;
	int32_t         m_nRotationRateError;
	uint32_t        m_nLifeSpanErrorShape;
	float         m_fTrailLengthMultiplier;
	uint32_t        Flags;
	ParticleSystemColor         m_RenderColouring;
	uint8_t         m_InitialColorVariation;
	ParticleSystemColor         m_FadeDestinationColor;
	uint32_t        m_ColorFadeTime;
	
	ParticleSystemStretch     m_vecTextureStretch;
	float         m_fWindFactor;

	ParticleRaster      **m_ppRaster;
	CParticle     *m_pParticles;
} tParticleSystemData;
typedef struct ParticleSystemMgr {
    tParticleSystemData m_aParticles[MAX_PARTICLES];
} ParticleSystemMgr;
#ifdef __cplusplus
extern "C" {
#endif
extern ParticleSystemMgr mod_ParticleSystemManager;
extern const char *ParticleFilename;
void ParticleMgr_Initialise(void);
void ParticleMgr_LoadParticleData(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
