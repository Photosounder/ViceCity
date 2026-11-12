//+ rouz edit (Claude)
#pragma once
#include <stdint.h>
#include <stdbool.h>

enum ePickupType
{
	PICKUP_NONE = 0,
	PICKUP_IN_SHOP,
	PICKUP_ON_STREET,
	PICKUP_ONCE,
	PICKUP_ONCE_TIMEOUT,
	PICKUP_ONCE_TIMEOUT_SLOW,
	PICKUP_COLLECTABLE1,
	PICKUP_IN_SHOP_OUT_OF_STOCK,
	PICKUP_MONEY,
	PICKUP_MINE_INACTIVE,
	PICKUP_MINE_ARMED,
	PICKUP_NAUTICAL_MINE_INACTIVE,
	PICKUP_NAUTICAL_MINE_ARMED,
	PICKUP_FLOATINGPACKAGE,
	PICKUP_FLOATINGPACKAGE_FLOATING,
	PICKUP_ON_STREET_SLOW,
	PICKUP_ASSET_REVENUE,
	PICKUP_PROPERTY_LOCKED,
	PICKUP_PROPERTY_FORSALE,
	PICKUP_NUMOFTYPES
};

// The vector keeps the engine type in C++ and plain floats in C
#ifdef __cplusplus
typedef CVector PickupVec;
class CObject;
#else
typedef struct PickupVec {float x,y,z;} PickupVec;
typedef struct CObject CObject;
#endif

// The plain data of one pickup, usable from C
typedef struct CPickupData {
	PickupVec m_vecPos;
	float m_fRevenue;
	CObject *m_pObject;
	CObject *m_pExtraObject;
	uint32_t m_nQuantity;
	uint32_t m_nTimer;
	uint16_t m_nMoneySpeed;
	int16_t m_eModelIndex;
	uint16_t m_nIndex;
	char m_sTextKey[8];
	uint8_t m_eType;
	bool m_bRemoved;
	uint8_t m_bWasAmmoCollected:1;
	uint8_t m_bWasControlMessageShown:1;
} CPickupData;
//- rouz edit (Claude)
