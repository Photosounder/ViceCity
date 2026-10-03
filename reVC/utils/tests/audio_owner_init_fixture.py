"""Compare original owner setup, RNG draws, lifecycle ordering, and native record ABI."""
from pathlib import Path
import re
import audio_mission_data_migration as data
import audio_mission_state_migration as mission
import audio_owner_entities_migration as entities
from audio_controls_migration import function
from audio_owner_init_migration import owner, header, fields
root=Path(__file__).resolve().parents[2]
out=root/'build/audio-owner-init-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text()
old_h=(out/'before/src/audio/AudioManager.h').read_text()
new_h=(root/'src/audio/AudioManager.h').read_text()
kernel=(root/'src/audio/AudioManagerInit.c').read_text()
def normalize(source):
    # Compare statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
ctor=function(old,'cAudioManager::','cAudioManager')
gen=function(old,'cAudioManager::','GenerateIntegerRandomNumberTable')
expected=re.sub(r'(?<![.\w])\b(m_[A-Za-z_0-9]+)\b',r'manager->\1',ctor[ctor.index('{'):])
expected=expected.replace('FALSE','0').replace('TRUE','1').replace('uint32','uint32_t').replace('GenerateIntegerRandomNumberTable();','AudioManager_GenerateRandomTable(manager->m_anRandomTable);').replace('SPEED_OF_SOUND / TIME_SPENT','343.f / 40').replace('TIME_SPENT','40')
assert '#define SPEED_OF_SOUND 343.f' in old and '#define TIME_SPENT 40' in old
actual=function(kernel,'AudioManager_','InitState');assert normalize(expected)==normalize(actual[actual.index('{'):])
expected=gen[gen.index('{'):].replace('uint32','uint32_t').replace('ARRAY_SIZE(m_anRandomTable)','5').replace('m_anRandomTable','randomTable')
actual=function(kernel,'AudioManager_','GenerateRandomTable');assert normalize(expected)==normalize(actual[actual.index('{'):])
assert re.search(r'int32_t m_anRandomTable\[5\];',new_h)
assert normalize(mission.calls(entities.owner(owner(old))))==normalize((root/'src/audio/AudioManager.cpp').read_text())
assert normalize(data.header(mission.header(entities.header(header(old_h)))))==normalize(new_h)
old_common=(out/'before/src/core/common.h').read_text()
assert old_common.replace('int myrand(void);\nvoid mysrand(unsigned int seed);','#include "Random.h" // rouz edit (ChatGPT)')==(root/'src/core/common.h').read_text()
assert (out/'before/src/core/re3.cpp').read_bytes()==(root/'src/core/re3.cpp').read_bytes()
assert normalize(fields(old_h))==normalize(fields(new_h))
headers='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h'])
record=fields(new_h)
dtor=function(old,'cAudioManager::','~cAudioManager')
rng=(root/'src/core/re3.cpp').read_text();rng=rng[rng.index('#ifdef USE_PS2_RAND\nunsigned long long myrand_seed'):rng.index('void\nmysrand')]
rng=rng.replace('int\nmyrand(void)', 'extern "C" int\nmyrand(void)')
common=headers+'''
#include <stdio.h>
#include <stdlib.h>
#include <new>
#include <string.h>
#include <assert.h>
using uint32=uint32_t;
#define FALSE 0
#define TRUE 1
#define SPEED_OF_SOUND 343.f
#define TIME_SPENT 40
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))
static uint64_t digest=1469598103934665603ULL;
static unsigned rngCalls;
static void bytes(const void *ptr,size_t size) {
    // Compare initialized owner bytes and retained partial-state storage
    const unsigned char *data=(const unsigned char*)ptr;
    for(size_t i=0;i<size;++i) digest=(digest^data[i])*1099511628211ULL;
}
static void value(uint32_t v) {
    // Record ordered lifetime events around owner startup and shutdown
    bytes(&v,sizeof(v));
}
struct cAudioManager {
'''+record+'''
    cAudioManager();~cAudioManager();void GenerateIntegerRandomNumberTable();void Terminate();
};
extern "C" void AudioManager_InitState(cAudioManager *manager);
extern "C" void AudioManager_GenerateRandomTable(int32_t randomTable[5]);
'''
rng=rng.replace('myrand(void)\n{','myrand(void)\n{\n    // Observe the actual game generator without altering its arithmetic\n    rngCalls++;value(10);')
# Use the actual destructor body with a quiet game-shutdown observer
terminate='''
void cAudioManager::Terminate() {
    // Observe the original destructor gate without invoking game subsystems
    value(20);value(m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal);m_bIsInitialised=0;
}
struct Probe {
    unsigned event;
    Probe(unsigned value):event(value) { ::value(event);::value(rngCalls); }
    ~Probe() {
        // Observe registered reverse destruction order and final generator state
        ::value(event+100);::value(rngCalls);
        if(event==1) printf("%llu\\n",(unsigned long long)digest);
    }
};
static Probe beforeOwner(1);
cAudioManager AudioManager;
static Probe afterOwner(2);
'''
trace=(root/'utils/tests/audio_owner_init.cpp.in').read_text()
for variant in ['before','after']:
    methods=ctor+'\n'+gen if variant=='before' else function((root/'src/audio/AudioManager.cpp').read_text(),'cAudioManager::','cAudioManager')
    reference=ctor[ctor.index('{'): ]
    reference=re.sub(r'(?<![.\w])\b(m_[A-Za-z_0-9]+)\b',r'object->\1',reference).replace('GenerateIntegerRandomNumberTable();','object->GenerateIntegerRandomNumberTable();')
    reference='static void InitOriginalFields(cAudioManager *object)\n'+reference if variant=='before' else ''
    observed_dtor=dtor[:-1]+'    // Observe the final reset while the record remains alive\n    value(m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal);\n}'
    source=common+rng+methods+'\n'+reference+'\n'+observed_dtor+'\n'+terminate+trace
    source=source.replace('@C_CHECK@','TestAudioOwnerC();' if variant=='after' else '')
    if variant=='after': source='struct cAudioManager;\nextern \"C\" cAudioManager AudioManager;\nextern \"C\" void TestAudioOwnerC(void);\n'+source
    source=source.replace('@INIT@','InitOriginalFields(object)' if variant=='before' else 'AudioManager_InitState(object)')
    source=source.replace('@GENERATE@','object->GenerateIntegerRandomNumberTable()' if variant=='before' else 'AudioManager_GenerateRandomTable(object->m_anRandomTable)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
entries_cpp=[];entries_c=[]
for line in record.splitlines():
    # Emit field metadata from actual declarations with their original conditional branches
    if line.lstrip().startswith('#'): entries_cpp.append(line);entries_c.append(line)
    field=re.match(r'\s*(\w+)\s+(m_\w+|field_\w+)(?:\[[^;]+)?;',line)
    if field:
        type_name,name=field.groups()
        entries_cpp.append('offsetof(cAudioManager,'+name+'),sizeof(((cAudioManager*)0)->'+name+'),alignof(decltype(((cAudioManager*)0)->'+name+')),')
        entries_c.append('offsetof(cAudioManager,'+name+'),sizeof(((cAudioManager*)0)->'+name+'),_Alignof('+type_name+'),')
for variant in ['before','after','c']:
    if variant=='c':
        source='#include "'+(root/'src/audio/AudioManager.h').as_posix()+'"\n#include <stddef.h>\nconst size_t AudioManagerAbi[]={sizeof(cAudioManager),_Alignof(cAudioManager),\n'+'\n'.join(entries_c)+'\n};\nconst size_t AudioManagerAbiCount=sizeof(AudioManagerAbi)/sizeof(AudioManagerAbi[0]);\n'
    else:
        selected=out/'before/src/audio/AudioManager.h' if variant=='before' else root/'src/audio/AudioManager.h'
        source='#include "'+(root/'src/core/common.h').as_posix()+'"\n#include "'+selected.as_posix()+'"\n#include <stddef.h>\nextern "C" const size_t AudioManagerAbi[]={sizeof(cAudioManager),alignof(cAudioManager),\n'+'\n'.join(entries_cpp)+'\n};\nextern "C" const size_t AudioManagerAbiCount=sizeof(AudioManagerAbi)/sizeof(AudioManagerAbi[0]);\n'
    (out/(variant+'-abi.'+('c' if variant=='c' else 'cpp'))).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#define AUDIO_OAL\n#undef AUDIO_REFLECTIONS\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete initializer, RNG loop, record fields, callback, common RNG declaration, and unchanged RNG implementation audited')

(out/'caller.c').write_text('''#include "AudioManager.h"
#include <assert.h>
void TestAudioOwnerC(void) {
    // Read the live C-linkage global through the actual C owner record definition
    assert(AudioManager.m_bIsInitialised==0);
    assert(AudioManager.m_nActiveQueue==0);
    assert(AudioManager.m_nActiveSamples==NUM_CHANNELS_GENERIC);
    assert(AudioManager.m_nTimeSpent==40);
    assert(AudioManager.m_fSpeedOfSound==343.f/40);
    assert(AudioManager.m_nRequestedCount[0]==0 && AudioManager.m_nRequestedCount[1]==0);
    assert(AudioManager.m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal==0);
}''',newline='\n')
