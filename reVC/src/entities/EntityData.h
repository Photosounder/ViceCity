//+ rouz edit (Claude)
#pragma once
#include <stdint.h>

// The entity state that follows CPlaceable, as plain data usable from C
#ifdef __cplusplus
#define ENTITYDATA_RWOBJECT_TYPE RwObject
#else
#define ENTITYDATA_RWOBJECT_TYPE void
#endif

typedef struct CEntityData {
	ENTITYDATA_RWOBJECT_TYPE *m_rwObject;
	uint32_t m_type : 3;
	uint32_t m_status : 5;
	// flagsA
	uint32_t bUsesCollision : 1;			// does entity use collision
	uint32_t bCollisionProcessed : 1;		// has object been processed by a ProcessEntityCollision function
	uint32_t bIsStatic : 1;				// is entity static
	uint32_t bHasContacted : 1;			// has entity processed some contact forces
	uint32_t bPedPhysics : 1;
	uint32_t bIsStuck : 1;				// is entity stuck
	uint32_t bIsInSafePosition : 1;		// is entity in a collision free safe position
	uint32_t bUseCollisionRecords : 1;

	// flagsB
	uint32_t bWasPostponed : 1;			// was entity control processing postponed
	uint32_t bExplosionProof : 1;
	uint32_t bIsVisible : 1;				//is the entity visible
	uint32_t bHasCollided : 1;
	uint32_t bRenderScorched : 1;
	uint32_t bHasBlip : 1;
	uint32_t bIsBIGBuilding : 1;			// Set if this entity is a big building
	uint32_t bStreamBIGBuilding : 1;	// set when draw dist <= 2000

	// flagsC
	uint32_t bRenderDamaged : 1;			// use damaged LOD models for objects with applicable damage
	uint32_t bBulletProof : 1;
	uint32_t bFireProof : 1;
	uint32_t bCollisionProof : 1;
	uint32_t bMeleeProof : 1;
	uint32_t bOnlyDamagedByPlayer : 1;
	uint32_t bStreamingDontDelete : 1;	// Dont let the streaming remove this 
	uint32_t bRemoveFromWorld : 1;		// remove this entity next time it should be processed

	// flagsD
	uint32_t bHasHitWall : 1;				// has collided with a building (changes subsequent collisions)
	uint32_t bImBeingRendered : 1;		// don't delete me because I'm being rendered
	uint32_t bTouchingWater : 1;	// used by cBuoyancy::ProcessBuoyancy
	uint32_t bIsSubway : 1;	// set when subway, but maybe different meaning?
	uint32_t bDrawLast : 1;				// draw object last
	uint32_t bNoBrightHeadLights : 1;
	uint32_t bDoNotRender : 1;	//-- only applies to CObjects apparently
	uint32_t bDistanceFade : 1;			// Fade entity because it is far away

	// flagsE
	uint32_t m_flagE1 : 1;
	uint32_t bDontCastShadowsOn : 1;       // Dont cast shadows on this object
	uint32_t bOffscreen : 1;               // offscreen flag. This can only be trusted when it is set to true
	uint32_t bIsStaticWaitingForCollision : 1; // this is used by script created entities - they are static until the collision is loaded below them
	uint32_t bDontStream : 1;              // tell the streaming not to stream me
	uint32_t bUnderwater : 1;              // this object is underwater change drawing order
	uint32_t bHasPreRenderEffects : 1; // Object has a prerender effects attached to it
	uint32_t bDrawFarAway : 1;             // render this dynamic entity in the far-distance pass // rouz edit (ChatGPT)

	uint16_t m_scanCode;
	uint16_t m_randomSeed;
	int16_t m_modelIndex;
	int8_t m_level;
	int8_t m_area;
	struct CReference *m_pFirstReference;
} CEntityData;
//- rouz edit (Claude)
