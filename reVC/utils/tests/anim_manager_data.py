"""Project original manager tables and private static storage directly into C without regenerating data."""
import re

def data(original):
    # Preserve every original array initializer and conditional profile in its original order
    start=original.index('CAnimBlock CAnimManager::ms_aAnimBlocks[');end=original.index('void\nCAnimManager::Initialise(void)',start)
    source=original[start:end]
    source=re.sub(r'(?m)^([^\n]*?)CAnimManager::(ms_\w+)',lambda match:'static '+match[1]+match[2],source)
    source=re.sub(r'\bint32\b','int32_t',source)
    assert 'CAnimManager::' not in source
    return '''//+ rouz edit (ChatGPT)
#include "AnimManager.h"
#include "ModelIndices.h"
#include "AnimBlendAssociation_flags.h"
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
'''+source+'//- rouz edit (ChatGPT)\n'
