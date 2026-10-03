"""Known record exposure and initializer migration for earlier source audits."""
import re
from audio_controls_migration import function
CTOR='cAudioManager::cAudioManager()\n{\n\t// Initialize live owner state at the original C++ startup callback\n\tAudioManager_InitState(this);\n}'
def owner(source):
    # Preserve the original lifecycle callback while replacing its verified state setup
    source=source.replace(function(source,'cAudioManager::','cAudioManager'),CTOR)
    source=source.replace(function(source,'cAudioManager::','GenerateIntegerRandomNumberTable')+'\n','')
    return source.replace('GenerateIntegerRandomNumberTable();','AudioManager_GenerateRandomTable(m_anRandomTable);')
def fields(source):
    # Extract only stored members independently of C++ method and access declarations
    start=source.index('class cAudioManager') if 'class cAudioManager' in source else source.index('typedef struct cAudioManager')
    data=source[source.index('{',start)+1:source.index('\tcAudioManager();')]
    data=data.replace('public:','').removesuffix('#ifdef __cplusplus\n')
    for a,b in [('bool8','uint8_t'),('uint8','uint8_t'),('uint32','uint32_t'),('int32','int32_t')]:data=re.sub(r'\b'+a+r'\b',b,data)
    return data

END='\tfloat Sqrt(float v) const { return v <= 0.0f ? 0.0f : ::Sqrt(v); }\n#endif\n} cAudioManager;\n\n#ifdef __cplusplus\nextern "C" {\n#endif\nvoid AudioManager_InitState(cAudioManager *manager);\nvoid AudioManager_GenerateRandomTable(int32_t randomTable[5]);\n#ifdef __cplusplus\n}\n#endif'
GLOBAL='#ifdef __cplusplus\nextern "C" {\n#endif\nextern cAudioManager AudioManager;\n#ifdef __cplusplus\n}\n#endif'
def header(source):
    # Expose the same fixed-width record while keeping remaining C++ methods conditional
    source=source.replace('#pragma once\n','#pragma once\n#include "../core/config.h"\n',1)
    source=source.replace('#include "VehicleModelInfo.h"\n#include "Vehicle.h"','#ifdef __cplusplus\n#include "VehicleModelInfo.h"\n#include "Vehicle.h"\n#endif')
    for block in ['VALIDATE_SIZE(tSound, 96);','class CPhysical;\nclass CAutomobile;','VALIDATE_SIZE(tAudioEntity, 40);','VALIDATE_SIZE(cPedComments, 0x490);','class CEntity;\nclass CPlane;','VALIDATE_SIZE(cVehicleParams, 0x1C);']:
        source=source.replace(block,'#ifdef __cplusplus\n'+block+'\n#endif')
    source=source.replace('class cAudioManager\n{\npublic:','typedef struct cAudioManager {')
    a=source.index('typedef struct cAudioManager');b=source.index('\tcAudioManager();')
    data=source[a:b]
    for old,new in [('bool8','uint8_t'),('uint8','uint8_t'),('uint32','uint32_t'),('int32','int32_t')]:data=re.sub(r'\b'+old+r'\b',new,data)
    source=source[:a]+data+'#ifdef __cplusplus\n'+source[b:]
    source=source.replace('\tvoid GenerateIntegerRandomNumberTable();\n','')
    source=source.replace('\tfloat Sqrt(float v) const { return v <= 0.0f ? 0.0f : ::Sqrt(v); }\n};', END)
    source=source.replace('static_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");','#ifdef __cplusplus\nstatic_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");\n#else\n_Static_assert(sizeof(cAudioManager) == 0x5558, "cAudioManager: error");\n#endif')
    return source.replace('extern cAudioManager AudioManager;',GLOBAL)
