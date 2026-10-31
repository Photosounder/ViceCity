//+ rouz edit (ChatGPT)
#include "common.h"
#include "Stats_backend.h"
#include "World.h"
#include "PlayerInfo.h"
#include "Pad.h"
#include "Game.h"
#include "main.h"
#include "Font.h"
#include "Frontend.h"
int32_t Stats_BackendMoney(void)
{
    // Access the original typed native primitive at its original C policy access point
    return CWorld::Players[CWorld::PlayerInFocus].m_nMoney;
}

int32_t Stats_BackendVisibleMoney(void)
{
    // Access the original typed native primitive at its original C policy access point
    return CWorld::Players[CWorld::PlayerInFocus].m_nVisibleMoney;
}

bool Stats_BackendHasCheated(void)
{
    // Access the original typed native primitive at its original C policy access point
    return CPad::bHasPlayerCheated;
}

bool Stats_BackendNastyGame(void)
{
    // Access the original typed native primitive at its original C policy access point
    return CGame::nastyGame;
}

CPlayerInfo * Stats_BackendPlayerInfo(void)
{
    // Access the original typed native primitive at its original C policy access point
    return &CWorld::Players[CWorld::PlayerInFocus];
}

int32_t Stats_BackendCollectedPackages(CPlayerInfo *player)
{
    // Access the original typed native primitive at its original C policy access point
    return player->m_nCollectedPackages;
}

int32_t Stats_BackendTotalPackages(CPlayerInfo *player)
{
    // Access the original typed native primitive at its original C policy access point
    return player->m_nTotalPackages;
}

int32_t Stats_BackendLanguage(void)
{
    // Access the original typed native primitive at its original C policy access point
    return FrontEndMenuManager.m_PrefsLanguage;
}

bool Stats_BackendJapanese(void)
{
    // Access the original typed native primitive at its original C policy access point
    #ifdef MORE_LANGUAGES
    return CFont::IsJapanese();
#else
    return false;
#endif
}

char * Stats_BackendString2(void)
{
    // Access the original typed native primitive at its original C policy access point
    return gString2;
}

uint16_t * Stats_BackendUnicode(void)
{
    // Access the original typed native primitive at its original C policy access point
    return gUString;
}

uint16_t * Stats_BackendUnicode2(void)
{
    // Access the original typed native primitive at its original C policy access point
    return gUString2;
}

void Stats_BackendUnicodeStrcpy(uint16_t *target,const uint16_t *source)
{
    // Access the original typed native primitive at its original C policy access point
    UnicodeStrcpy(target,source);
}

void Stats_BackendAsciiToUnicode(const char *source,uint16_t *target)
{
    // Access the original typed native primitive at its original C policy access point
    AsciiToUnicode(source,target);
}

void Stats_BackendFilter(uint16_t *text)
{
    // Access the original typed native primitive at its original C policy access point
    CFont::FilterOutTokensFromString(text);
}

void Stats_BackendAssertFailed(const char *expression,const char *file,unsigned line,const char *function)
{
    // Access the original typed native primitive at its original C policy access point
    re3_assert(expression,file,line,function);
}
//- rouz edit (ChatGPT)
