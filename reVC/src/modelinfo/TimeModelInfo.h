//+ rouz edit (ChatGPT)
#pragma once

#include "SimpleModelInfo.h"
#include "TimeModelData.h"

class CTimeModelInfo : public CSimpleModelInfo
{
public:
	TimeModelData m_timeData;
public:
	CTimeModelInfo(void) : CSimpleModelInfo(MITYPE_TIME) {
		// Initialize time-specific state once when the native object is constructed
		TimeModelData_Init(&m_timeData);
	}

};
//static_assert(sizeof(CTimeModelInfo) == 0x58, "CTimeModelInfo: error");
//- rouz edit (ChatGPT)
