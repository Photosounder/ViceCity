//+ rouz edit (ChatGPT)
#pragma once

#include "ClumpModelInfo.h"
#include "ColModel.h"
#include "PedType.h"

#include "PedNode.h"

class CPedModelInfo : public CClumpModelInfo
{
public:
	uint32 m_animGroup;
	ePedType m_pedType;
	ePedStats m_pedStatType;
	uint32 m_carsCanDrive;
	CColModel *m_hitColModel;
	int8 radio1, radio2;

	static RwObjectNameIdAssocation m_pPedIds[PED_NODE_MAX];

	CPedModelInfo(void) : CClumpModelInfo(MITYPE_PED) { m_hitColModel = nil; }
//+ rouz edit (ChatGPT)
	~CPedModelInfo(void);
//- rouz edit (ChatGPT)
	void DeleteRwObject(void);
	void SetClump(RpClump *);

	void CreateHitColModelSkinned(RpClump *clump);
	CColModel *GetHitColModel(void) { return m_hitColModel; }
	CColModel *AnimatePedColModelSkinned(RpClump *clump);
	CColModel *AnimatePedColModelSkinnedWorld(RpClump *clump);
};
//- rouz edit (ChatGPT)
