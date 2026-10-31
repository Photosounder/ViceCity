//+ rouz edit (ChatGPT)
#pragma once
#include "config.h"
#ifdef __cplusplus
class CEntity;
#else
typedef struct CEntity CEntity;
#endif

typedef struct CReference {
	struct CReference *next;
	CEntity **pentity;
} CReference;
#ifdef __cplusplus
extern "C" {
#endif
extern CReference References_aRefs[NUMREFERENCES];
extern CReference *References_pEmptyList;
void References_Init(void);
void References_RegisterReference(CEntity *entity, CEntity **pent);
void References_CleanUpOldReference(CEntity *entity, CEntity **pent);
void References_ResolveReferences(CEntity *entity);
void References_PruneReferences(CEntity *entity);
void References_RemoveReferencesToPlayer(void);
void References_PruneAllReferencesInWorld(void);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
