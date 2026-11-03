//+ rouz edit (ChatGPT)
#include "config.h"
#ifndef MASTER
extern void re3_assert(const char *,const char *,unsigned int,const char *);
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
#include "2dEffect.h"
#include "ModelRegistry.h"
#include "Store.h"
#include "TxdStore.h"
#include "RwHelper.h"
#include <stdio.h>
#include <stddef.h>

void
Effect2d_Load(const char *line)
{
	// Parse the shared effect header before switching texture dictionaries
	int id, r, g, b, a, type, ptype;
	float x, y, z;
	char corona[32], shadow[32];
	int shadowIntens, lightType, roadReflection, flare, flags, probability;
	void *mi;
	C2dEffect *effect;
	char *p;

	sscanf(line, "%d %f %f %f %d %d %d %d %d", &id, &x, &y, &z, &r, &g, &b, &a, &type);

	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PushCurrentTxd();
	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_SetCurrentTxd(TxdStore_FindTxdSlot("particle"));

	mi = ModelRegistry_Get(id); // rouz edit (ChatGPT)
	//+ rouz edit (ChatGPT)
	// Access raw storage through the C store or pool API
	effect = (C2dEffect*)CStore_Alloc(ModelRegistry_EffectsStore());
	//- rouz edit (ChatGPT)
	ModelRegistry_AddEffect(mi, effect); // rouz edit (ChatGPT)
	effect->pos = (CollisionVector3){x, y, z}; // rouz edit (ChatGPT)
	effect->col = (Effect2dColor){(uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a}; // rouz edit (ChatGPT)
	effect->type = type;

	// Parse the selected effect payload using the original format
	switch(effect->type){
	case EFFECT_LIGHT:
		// Read both texture names and the light parameters
		while(*line++ != '"');
		p = corona;
		while(*line != '"') *p++ = *line++;
		*p = '\0';
		line++;

		while(*line++ != '"');
		p = shadow;
		while(*line != '"') *p++ = *line++;
		*p = '\0';
		line++;

		sscanf(line, "%f %f %f %f %d %d %d %d %d",
			&effect->light.dist,
			&effect->light.range,
			&effect->light.size,
			&effect->light.shadowSize,
			&shadowIntens, &lightType, &roadReflection, &flare, &flags);
		effect->light.corona = RwHelper_ReadTexture(corona, NULL);
		effect->light.shadow = RwHelper_ReadTexture(shadow, NULL);
		effect->light.shadowIntensity = shadowIntens;
		effect->light.lightType = lightType;
		effect->light.roadReflection = roadReflection;
		effect->light.flareType = flare;

		if(flags & LIGHTFLAG_FOG_ALWAYS)
			flags &= ~LIGHTFLAG_FOG_NORMAL;
		effect->light.flags = flags;
		break;

	case EFFECT_PARTICLE:
		// Read the particle type, direction, and scale
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %f",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&effect->particle.particleType,
			&effect->particle.dir.x,
			&effect->particle.dir.y,
			&effect->particle.dir.z,
			&effect->particle.scale);
		break;

	case EFFECT_ATTRACTOR:
		// Read the attractor direction and probability
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %d",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&flags,
			&effect->attractor.dir.x,
			&effect->attractor.dir.y,
			&effect->attractor.dir.z,
			&probability);
		effect->attractor.type = flags;
#ifdef FIX_BUGS
		effect->attractor.probability = (probability < 0 ? 0 : probability > 255 ? 255 : probability);
#else
		effect->attractor.probability = probability;
#endif
		break;
	case EFFECT_PED_ATTRACTOR:
		// Read the pedestrian attractor directions and type
		sscanf(line, "%d %f %f %f %d %d %d %d %d %d %f %f %f %f %f %f",
			&id, &x, &y, &z, &r, &g, &b, &a, &type,
			&ptype,
			&effect->pedattr.queueDir.x,
			&effect->pedattr.queueDir.y,
			&effect->pedattr.queueDir.z,
			&effect->pedattr.useDir.x,
			&effect->pedattr.useDir.y,
			&effect->pedattr.useDir.z);
		effect->pedattr.type = ptype;
		break;
	}

	// Use the complete C texture dictionary owner with the original live arguments
	TxdStore_PopCurrentTxd();
}

//- rouz edit (ChatGPT)
