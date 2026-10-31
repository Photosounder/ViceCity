//+ rouz edit (ChatGPT)
#pragma once
#include <stdint.h>
#include <stdbool.h>
typedef struct StoredCarVector { float x, y, z; } StoredCarVector;
enum StoredCarFlags {
		STORED_CAR_FLAG_BULLETPROOF = 0x1,
		STORED_CAR_FLAG_FIREPROOF = 0x2,
		STORED_CAR_FLAG_EXPLOSIONPROOF = 0x4,
		STORED_CAR_FLAG_COLLISIONPROOF = 0x8,
		STORED_CAR_FLAG_MELEEPROOF = 0x10,
	};
typedef struct CStoredCar {
	int32_t m_nModelIndex;
	StoredCarVector m_vecPos;
	StoredCarVector m_vecAngle;
	int32_t m_nFlags;
	int8_t m_nPrimaryColor;
	int8_t m_nSecondaryColor;
	int8_t m_nRadioStation;
	int8_t m_nVariationA;
	int8_t m_nVariationB;
	int8_t m_nCarBombType;
} CStoredCar;
#ifdef __cplusplus
class CVehicle;
extern "C" {
#else
typedef struct CVehicle CVehicle;
#endif
void StoredCar_Init(CStoredCar *self);
void StoredCar_Clear(CStoredCar *self);
bool StoredCar_HasCar(CStoredCar *self);
const CStoredCar *StoredCar_Assign(CStoredCar *self, const CStoredCar *other);
void StoredCar_StoreCar(CStoredCar *self, CVehicle *vehicle);
CVehicle *StoredCar_RestoreCar(CStoredCar *self);
#ifdef __cplusplus
}
#endif
//- rouz edit (ChatGPT)
