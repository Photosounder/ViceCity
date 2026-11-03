//+ rouz edit (ChatGPT)
#pragma once

#include "2dEffectTypes.h"
#include "Effect2dData.h"


enum {
	LIGHT_ON,
	LIGHT_ON_NIGHT,
	LIGHT_FLICKER,
	LIGHT_FLICKER_NIGHT,
	LIGHT_FLASH1,
	LIGHT_FLASH1_NIGHT,
	LIGHT_FLASH2,
	LIGHT_FLASH2_NIGHT,
	LIGHT_FLASH3,
	LIGHT_FLASH3_NIGHT,
	LIGHT_RANDOM_FLICKER,
	LIGHT_RANDOM_FLICKER_NIGHT,
	LIGHT_SPECIAL,
	LIGHT_BRIDGE_FLASH1,
	LIGHT_BRIDGE_FLASH2,
};

enum {
	ATTRACTORTYPE_ICECREAM,
	ATTRACTORTYPE_STARE
};

enum {
	LIGHTFLAG_LOSCHECK = 1,
	// same order as CPointLights flags, must start at 2
	LIGHTFLAG_FOG_NORMAL = 2,	// can have light and fog
	LIGHTFLAG_FOG_ALWAYS = 4,	// fog only
	LIGHTFLAG_HIDE_OBJECT = 8,	// hide the object instead of rendering light (???)
	LIGHTFLAG_LONG_DIST = 16,
	LIGHTFLAG_FOG = (LIGHTFLAG_FOG_NORMAL|LIGHTFLAG_FOG_ALWAYS)
};

typedef struct C2dEffect {
    CollisionVector3 pos;
    Effect2dColor col;
    uint8_t type;
    union {
        Effect2dLight light;
        Effect2dParticle particle;
        Effect2dAttractor attractor;
        Effect2dPedAttractor pedattr;
    };
} C2dEffect;

#ifdef __cplusplus
extern "C" {
#endif
void Effect2d_Load(const char *line);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
VALIDATE_SIZE(C2dEffect, 0x34);
#endif
//- rouz edit (ChatGPT)
