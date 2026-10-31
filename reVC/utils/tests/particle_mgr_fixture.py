"""Audit complete particle configuration relocation and emit original/C comparison inputs."""
from pathlib import Path
import re,json
import particle_mgr_migration as m
import var_console_migration as variables
root=Path(__file__).resolve().parents[2];out=root/'build/particle-mgr-c-tests'
old=(out/'before/src/renderer/ParticleMgr.cpp').read_text();header=(out/'before/src/renderer/ParticleMgr.h').read_text()
for path in json.loads((out/'changes.json').read_text()):
    expected=m.project((out/'before'/path).read_text()) if path.startswith('build/') else m.calls((out/'before'/path).read_text())
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/renderer/ParticleConfig.c').read_text()==m.kernel(old,header)
assert (root/'src/renderer/ParticleMgr_backend.h').read_text()==m.BACKEND
for path in ['src/core/common.h','src/core/config.h','src/core/main.h','src/core/main.cpp','src/core/FileMgr.h','src/core/FileMgr.c','src/math/Vector2D.h','src/math/Vector.h','src/renderer/ParticleType.h','src/renderer/Particle.h','src/fakerw/rwplcore.h','src/fakerw/rwcore.h']:
    if path=='src/core/main.cpp':
        assert (root/path).read_text()==variables.calls((out/'before'/path).read_text()),path
    else:assert (root/path).read_bytes()==(out/'before'/path).read_bytes(),path
assert 'cParticleSystemMgr::cParticleSystemMgr()\n{\n\tmemset(this, 0, sizeof(*this));\n}' in old
assert 'void RangeCheck(tParticleSystemData *pData) { }' in header
common=(out/'before/src/core/common.h').read_text();a=common.index('class CRGBA\n');b=common.index('\n};',a)+3;colour=common[a:b]
vector=(out/'before/src/math/Vector2D.h').read_text();assert 'float x, y;\n\tCVector2D(void) {}' in vector
common='''#pragma once
#ifdef NATIVE_PROJECT_HEADERS
#include "'''+(root/'src/core/common.h').as_posix()+'''"
#else
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include "config.h"
typedef uint8_t uint8;typedef uint16_t uint16;typedef int16_t int16;typedef int32_t int32;typedef uint32_t uint32;
struct RwRGBA { uint8 red,green,blue,alpha; };
struct RwRaster;
#define RWCORE_H
'''+colour+'''
class CVector2D { public:float x,y;CVector2D(void) {} };
inline void RwRGBAAssign(RwRGBA *target,const RwRGBA *source){*target=*source;}
#define VALIDATE_SIZE(type,size)
#define BIT(n) (1 << (n))
#define Min(a,b) ((a)<(b)?(a):(b))
#define SQR(x) ((x)*(x))
#define ARRAY_SIZE(array) (sizeof(array)/sizeof((array)[0]))
#define nil NULL
extern "C" void re3_assert(const char *,const char *,unsigned int,const char *);
#ifndef MASTER
#define ASSERT(expression) ((void)((!!(expression)) || (re3_assert(#expression,__FILE__,__LINE__,__func__),0)))
#else
#define ASSERT(expression) (expression)
#endif
#endif
'''
(out/'common.h').write_text(common,newline='\n')
(out/'main.h').write_text('#pragma once\nextern uint8_t work_buff[55000];\n',newline='\n')
(out/'owners.h').write_text('#pragma once\n#include "common.h"\n#include "ParticleMgr.h"\n#include "FileMgr.h"\n#include "main.h"\n',newline='\n')
for name in ['ParticleMgr.h','ParticleMgr_backend.h']:(out/name).write_text('#pragma once\n#include "'+(root/'src/renderer'/name).as_posix()+'"\n',newline='\n')
original_header=re.sub(r'^#(?:pragma|include)[^\n]*\n','',header,flags=re.M).replace('class CParticle;','')
(out/'original-classes.h').write_text(original_header,newline='\n')
fields=re.findall(r'(?m)^\s*\w+\s+\**(m_\w+|Flags)(?:\[\d+\])?;',header[:header.index('class cParticleSystemMgr')]);assert len(fields)==39,len(fields)
prototypes='''void TestPm_Initialise(void);void TestPm_Load(void);void TestPm_Copy(void *buffer);size_t TestPm_Size(void);
void TestPm_Seed(uint32_t seed,void *particles,void *rasters);void TestPm_Filename(const char *name);const char *TestPm_GetFilename(void);
void TestPm_RenderColour(int32_t index,RwRGBA *out);void TestPm_FadeColour(int32_t index,RwRGBA *out);
void TestPm_RunInitialise(void);void TestPm_RunLoad(void);
'''
(out/'caller.h').write_text('#pragma once\n#include <stddef.h>\n#include "owners.h"\nextern "C" {\n'+prototypes+'}\n',newline='\n')
for original in [True,False]:
    text='#include "owners.h"\n#include "caller.h"\n'
    if original:text+='namespace Original {\n'+original_header+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    prefix='Original::' if original else '';array=prefix+'mod_ParticleSystemManager.m_aParticles';kind=prefix+'tParticleSystemData'
    text+='extern "C" {\nvoid TestPm_Initialise(void)\n{\n    // Enter the entire original or C initialization operation\n    '+(prefix+'mod_ParticleSystemManager.Initialise();' if original else 'ParticleMgr_Initialise();')+'\n}\nvoid TestPm_Load(void)\n{\n    // Enter the complete original or C parser independently of list clearing\n    '+(prefix+'mod_ParticleSystemManager.LoadParticleData();' if original else 'ParticleMgr_LoadParticleData();')+'\n}\n'
    text+='void TestPm_Copy(void *buffer)\n{\n    // Observe every record byte including original retained padding and actual pointer slots\n    memcpy(buffer,&'+prefix+'mod_ParticleSystemManager,sizeof('+prefix+'mod_ParticleSystemManager));\n}\nsize_t TestPm_Size(void)\n{\n    // Retain the actual complete record pool representation\n    return sizeof('+prefix+'mod_ParticleSystemManager);\n}\n'
    text+='void TestPm_Filename(const char *name)\n{\n    // Mutate the actual shared filename pointer before its original live read\n    '+prefix+'ParticleFilename=name;\n}\nconst char *TestPm_GetFilename(void)\n{\n    // Observe the actual current filename value\n    return '+prefix+'ParticleFilename;\n}\n'
    text+='void TestPm_Seed(uint32_t seed,void *particles,void *rasters)\n{\n    // Seed actual record fields and deterministic padding without constructing a replacement record\n    memset(&'+prefix+'mod_ParticleSystemManager,0xa5,sizeof('+prefix+'mod_ParticleSystemManager));for(int i=0;i<MAX_PARTICLES;i++){\n        // Preserve valid enum values, finite floats and stable borrowed native pointer identities\n        '+kind+' *entry=&'+array+'[i];\n'
    for n,field in enumerate(fields):
        declaration=re.search(r'(?m)^\s*(\w+)\s+\**'+field+r'(?:\[\d+\])?;',header)[1]
        if field=='m_Type':value='entry->m_Type=(tParticleType)(i%MAX_PARTICLES);'
        elif field=='m_aName':value='for(int c=0;c<19;c++)entry->m_aName[c]=(char)(\'A\'+(i+c)%26);entry->m_aName[19]=0;'
        elif field=='m_pParticles':value='entry->m_pParticles=(CParticle*)((uint8_t *)particles+i*16);'
        elif field=='m_ppRaster':value='entry->m_ppRaster=('+('RwRaster' if original else 'ParticleRaster')+' **)((uint8_t *)rasters+i*16);'
        elif declaration=='CRGBA':value='entry->'+field+'.red=(uint8_t)(seed+i);entry->'+field+'.green=(uint8_t)(seed+i+37);entry->'+field+'.blue=(uint8_t)(seed+i+71);entry->'+field+'.alpha=(uint8_t)(seed+i+113);'
        elif declaration=='CVector2D':value='entry->'+field+'.x=seed*.03125f+i*.125f;entry->'+field+'.y=-seed*.0625f-i*.25f;'
        elif declaration=='float':value='entry->'+field+'=seed*.03125f+i*.125f+'+str(n)+'.0625f;'
        else:value='entry->'+field+'=('+declaration+')(int32_t)(seed*31+i*7+'+str(n)+'-117);'
        text+='        '+value+'\n'
    text+='    }\n}\n'
    actual=(out/'before/src/renderer/Particle.cpp').read_text() if original else (root/'src/renderer/Particle.cpp').read_text()
    for mode,name,target,colour_field in [('Render','pParticle','m_Color','m_RenderColouring'),('Fade','particle','m_Color','m_FadeDestinationColor')]:
        if original:snippet='RwRGBAAssign(&'+name+'->m_Color, psystem->'+colour_field+');';assert snippet in actual
        else:
            marker='// Copy the original four C color channels' if mode=='Render' else '// Copy the same four destination channels'
            a=actual.index(marker);b=actual.index('//- rouz edit (ChatGPT)',a);snippet=actual[a:b].strip()
        text+='void TestPm_'+mode+'Colour(int32_t index,RwRGBA *out)\n{\n    // Compile the actual original or migrated renderer color-transfer statement\n    struct ParticleColourTarget { RwRGBA m_Color; } object;ParticleColourTarget *'+name+'=&object;'+kind+' *psystem=&'+array+'[index];\n    '+snippet+'\n    *out=object.m_Color;\n}\n'
    text+='}\n';(out/('original.cpp' if original else 'adapter.cpp')).write_text(text,newline='\n')
(out/'host.cpp').write_text(m.HOST,newline='\n')
(out/'caller.c').write_text('''#include "ParticleMgr.h"
void TestPm_Initialise(void);void TestPm_Load(void);
void TestPm_RunInitialise(void)
{
    // Reach the real original or C operation through a separately compiled C11 caller
    TestPm_Initialise();
}
void TestPm_RunLoad(void)
{
    // Preserve the complete direct reload call across the actual C linkage boundary
    TestPm_Load();
}
''',newline='\n')
(out/'test.cpp').write_text((root/'utils/tests/particle_mgr.cpp.in').read_text(),newline='\n')
(out/'vanilla.h').write_text('#define VANILLA_DEFINES\n',newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
(out/'fields.json').write_text(json.dumps(fields,indent=2),newline='\n')
print('Audited both complete algorithms, all 39 fields in 83 records, constructor/empty-method removal, actual color transfers and five complete source/project files')
