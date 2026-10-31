"""Prepare complete original/C transmission algorithms with reference aliases and real timer access."""
from pathlib import Path
import re,json,sys
import transmission_migration as m
root=Path(__file__).resolve().parents[2];out=root/'.diagnostics/transmission-c-tests';saved=out/'before';tree=root if '--production' in sys.argv else out/'overlay'
if '--production' in sys.argv:
    for name in json.loads((out/'changes.json').read_text()):assert (root/m.RENAMES.get(name,name)).read_text()==m.calls((saved/name).read_text()),name
    assert not (root/'src/vehicles/Transmission.cpp').exists()
for name in ['Transmission.h','Transmission.c','HandlingFlags.h']:(out/name).write_text((tree/'src/vehicles'/name).read_text(),newline='\n')
(out/'Timer.h').write_bytes((saved/'src/core/Timer.h').read_bytes());(out/'config.h').write_text('#pragma once\n',newline='\n')
(out/'original.h').write_text('#pragma once\n#include <stdint.h>\ntypedef int8_t int8;typedef uint8_t uint8;\n'+(saved/'src/vehicles/Transmission.h').read_text(),newline='\n')
owners='''#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "Timer.h"
#include "HandlingFlags.h"
#ifdef ORIGINAL
#include "original.h"
#else
#include "Transmission.h"
#endif
#define nil NULL
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#ifndef MASTER
#define assert(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define assert(expression) (expression)
#endif
static inline float sq(float value){return value*value;}
static inline float Abs(float value){return fabsf(value);}
''';(out/'owners.h').write_text(owners,newline='\n')
entry='''#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif
void Before_Init(void *self);
void Run_Init(void *self);
void Before_Simple(void *self,float speed,uint8_t *gear);
void Run_Simple(void *self,float speed,uint8_t *gear);
float Before_Drive(void *self,const float *gas,uint8_t *gear,float *time,const float *velocity,bool cheat);
float Run_Drive(void *self,const float *gas,uint8_t *gear,float *time,const float *velocity,bool cheat);
#ifdef __cplusplus
}
#endif
'''
original='#include "owners.h"\n#include "entry.h"\n'+re.sub(r'^#include[^\n]*\n','',(saved/'src/vehicles/Transmission.cpp').read_text(),flags=re.M)+'''
extern "C" void Before_Init(void *self)
{
    // Invoke the whole original gear-ratio initializer
    ((cTransmission*)self)->InitGearRatios();
}
extern "C" void Before_Simple(void *self,float speed,uint8_t *gear)
{
    // Keep the original gear reference bound to its caller storage
    ((cTransmission*)self)->CalculateGearForSimpleCar(speed,*gear);
}
extern "C" float Before_Drive(void *self,const float *gas,uint8_t *gear,float *time,const float *velocity,bool cheat)
{
    // Preserve all four original reference bindings through recursive gear changes
    return ((cTransmission*)self)->CalculateDriveAcceleration(*gas,*gear,*time,*velocity,cheat);
}
'''
caller='''#include "entry.h"
#ifndef ORIGINAL
#include "Transmission.h"
#endif
void Run_Init(void *self)
{
    // Exercise the complete initializer through an independent C11 caller
#ifdef ORIGINAL
    Before_Init(self);
#else
    Transmission_InitGearRatios((cTransmission*)self);
#endif
}
void Run_Simple(void *self,float speed,uint8_t *gear)
{
    // Exercise the public simple-car operation with a live gear pointer
#ifdef ORIGINAL
    Before_Simple(self,speed,gear);
#else
    Transmission_CalculateGearForSimpleCar((cTransmission*)self,speed,gear);
#endif
}
float Run_Drive(void *self,const float *gas,uint8_t *gear,float *time,const float *velocity,bool cheat)
{
    // Exercise the C acceleration operation without copying aliased reference inputs
#ifdef ORIGINAL
    return Before_Drive(self,gas,gear,time,velocity,cheat);
#else
    return Transmission_CalculateDriveAcceleration((cTransmission*)self,gas,gear,time,velocity,cheat);
#endif
}
'''
for name,value in [('entry.h',entry),('original.cpp',original),('caller.c',caller),('fixture.cpp',(root/'utils/tests/transmission.cpp.in').read_text())]:(out/name).write_text(value,newline='\n')
print('Prepared all three complete original/C algorithms, reference aliases, recursive shifts, assertion variants and the actual timer accessor')
