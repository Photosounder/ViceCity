//+ rouz edit (ChatGPT)
#pragma once
#include "WantedShared.h"
#include "CrimeTypes.h"
typedef struct CCrimeBeingQd {
    enum eCrimeType m_nType;
    int32_t m_nId;
    uint32_t m_nTime;
    WantedVector m_vecPosn;
    bool m_bReported;
    bool m_bPoliceDoesntCare;
} CCrimeBeingQd;
//- rouz edit (ChatGPT)
