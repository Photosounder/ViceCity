#pragma once
#include "ModelRegistry.h" // rouz edit (ChatGPT)

struct CColModel;

#define MAX_MODEL_NAME (21)

#include "ModelInfoType.h" // rouz edit (ChatGPT)

struct C2dEffect; // rouz edit (ChatGPT)

class CBaseModelInfo
{
    //+ rouz edit (ChatGPT)
    friend int16_t *ModelRegistry_EffectId(void *model);
    friend uint8_t *ModelRegistry_EffectCount(void *model);
    //- rouz edit (ChatGPT)
protected:
	char         m_name[MAX_MODEL_NAME];
	uint8        m_type;
	uint8        m_num2dEffects;
	bool         m_bOwnsColModel;
	CColModel   *m_colModel;
	int16        m_2dEffectsID;
	int16        m_objectId;
	uint16       m_refCount;
	int16        m_txdSlot;

public:
	CBaseModelInfo(ModelInfoType type);
	virtual ~CBaseModelInfo() {}
	virtual void Shutdown(void);
	virtual void DeleteRwObject(void) = 0;
	virtual RwObject *CreateInstance(void) = 0;
	virtual RwObject *CreateInstance(RwMatrix *) = 0;
	virtual RwObject *GetRwObject(void) = 0;
	virtual void SetAnimFile(const char *file) {}
	virtual void ConvertAnimFileIndex(void) {}
	virtual int GetAnimFileIndex(void) { return -1; }

	// one day it becomes virtual
	uint8 GetModelType() const { return m_type; }
	bool IsBuilding(void) { return m_type == MITYPE_SIMPLE || m_type == MITYPE_TIME; }
	bool IsSimple(void) { return m_type == MITYPE_SIMPLE || m_type == MITYPE_TIME || m_type == MITYPE_WEAPON; }
	bool IsClump(void) { return m_type == MITYPE_CLUMP || m_type == MITYPE_PED || m_type == MITYPE_VEHICLE;	}
	char *GetModelName(void) { return m_name; }
	void SetModelName(const char *name) { strncpy(m_name, name, MAX_MODEL_NAME); }
	void SetColModel(CColModel *col, bool owns = false){
		m_colModel = col; m_bOwnsColModel = owns; }
	CColModel *GetColModel(void) { return m_colModel; }
	bool DoesOwnColModel(void) { return m_bOwnsColModel; }
	void DeleteCollisionModel(void);
	void ClearTexDictionary(void) { m_txdSlot = -1; }
	int16 GetObjectID(void) { return m_objectId; }
	void SetObjectID(int16 id) { m_objectId = id; }
	int16 GetTxdSlot(void) { return m_txdSlot; }
	void AddRef(void);
	void RemoveRef(void);
	void SetTexDictionary(const char *name);
	void AddTexDictionaryRef(void);
	void RemoveTexDictionaryRef(void);
	uint16 GetNumRefs() const { return m_refCount; }
};
