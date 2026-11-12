//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

// Enums keep their engine type in C++ and int in C
#ifdef __cplusplus
#define WEAPONDATA_ENUM(type) type
#else
#define WEAPONDATA_ENUM(type) int
#endif

// The plain data of a weapon slot, usable from C
typedef struct CWeaponData {
	WEAPONDATA_ENUM(eWeaponType) m_eWeaponType;
	WEAPONDATA_ENUM(eWeaponState) m_eWeaponState;
	int32_t m_nAmmoInClip;
	int32_t m_nAmmoTotal;
	uint32_t m_nTimer;
	bool m_bAddRotOffset;
} CWeaponData;
//- rouz edit (Claude)
