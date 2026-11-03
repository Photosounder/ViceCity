//+ rouz edit (ChatGPT)
#pragma once
#include "Stats.h"
#ifdef __cplusplus
class CPlayerInfo;
extern "C" {
#else
typedef struct CPlayerInfo CPlayerInfo;
#endif
int32_t Stats_BackendMoney(void);
int32_t Stats_BackendVisibleMoney(void);
bool Stats_BackendNastyGame(void);
CPlayerInfo * Stats_BackendPlayerInfo(void);
int32_t Stats_BackendCollectedPackages(CPlayerInfo *player);
int32_t Stats_BackendTotalPackages(CPlayerInfo *player);
int32_t Stats_BackendLanguage(void);
bool Stats_BackendJapanese(void);
void Stats_BackendFilter(uint16_t *text);
void Stats_BackendAssertFailed(const char *expression,const char *file,unsigned line,const char *function);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
