#pragma once

#include "WeaponType.h"
#include "config.h" // rouz edit (ChatGPT)
#include <stdbool.h>
#include <stdint.h> // rouz edit (ChatGPT)

#ifdef __cplusplus
class CEntity;
extern "C" {
#else
typedef struct CEntity CEntity;
#endif // rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
enum { BULLET_INFO_NUM_BULLETS = 100 };
//- rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)
typedef struct BulletInfoVector { float x, y, z; } BulletInfoVector;
//- rouz edit (ChatGPT)

typedef struct CBulletInfo // rouz edit (ChatGPT)
{
	enum eWeaponType m_eWeaponType; // rouz edit (ChatGPT)
	CEntity* m_pSource;
	float m_fTimer; // big mistake
	bool m_bInUse;
	BulletInfoVector m_vecPosition; // rouz edit (ChatGPT)
	BulletInfoVector m_vecSpeed; // rouz edit (ChatGPT)
	int16_t m_nDamage; // rouz edit (ChatGPT)
} CBulletInfo; // rouz edit (ChatGPT)

//+ rouz edit (ChatGPT)

extern CBulletInfo gaBulletInfo[BULLET_INFO_NUM_BULLETS];
extern bool bPlayerSniperBullet;
#ifdef SQUEEZE_PERFORMANCE
extern uint32_t bulletInfoInUse;
#endif
void BulletInfo_Initialise(void);
void BulletInfo_Shutdown(void);
bool BulletInfo_AddBullet(CEntity* pSource, enum eWeaponType type, BulletInfoVector vecPosition, BulletInfoVector vecSpeed);
void BulletInfo_Update(void);
bool BulletInfo_TestForSniperBullet(float x1, float x2, float y1, float y2, float z1, float z2);
//- rouz edit (ChatGPT)

#ifdef __cplusplus
}
#endif // rouz edit (ChatGPT)
