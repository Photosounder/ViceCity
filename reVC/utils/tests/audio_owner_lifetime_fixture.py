"""Audit and compare actual C owner startup/exit against the preceding C++ owner."""
from pathlib import Path
import re
from audio_owner_init_migration import fields
from audio_controls_migration import function
import audio_owner_lifetime_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-owner-lifetime-c-tests'
def normalize(source):
    # Compare complete statements without comments or formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioManager.cpp').read_text();oldh=(out/'before/src/audio/AudioManager.h').read_text();current=(root/'src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioOwner.c').read_text()
for name,transform in [('src/audio/AudioManager.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
assert normalize(fields(oldh))==normalize(fields(current))
ctor=function(old,'cAudioManager::','cAudioManager');dtor=function(old,'cAudioManager::','~cAudioManager')
expected=dtor[dtor.index('{'):].replace('this','manager')
expected=re.sub(r'(?<![\w.])\b(m_\w+)\b',r'manager->\1',expected)
actual=function(kernel,'AudioOwner_','Destroy');assert normalize(expected)==normalize(actual[actual.index('{'):])
assert normalize(function((root/'src/audio/AudioManager.cpp').read_text(),'cAudioManager::','cAudioManager'))==normalize(ctor)
assert normalize(function((root/'src/audio/AudioManager.cpp').read_text(),'cAudioManager::','~cAudioManager'))==normalize(migration.DTOR)
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h'])
record=fields(current)
common='''#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <new>
'''+includes+'''
static uint64_t digest=1469598103934665603ULL;
static unsigned draws;
static void bytes(const void *ptr,size_t n) {
    // Hash complete initialized owner storage including retained fields
    const unsigned char *p=(const unsigned char*)ptr;
    for(size_t i=0;i<n;i++)digest=(digest^p[i])*1099511628211ULL;
}
static void event(unsigned n) {
    // Record the order of startup, random draws, termination and exit
    bytes(&n,sizeof(n));
}
struct cAudioManager {
'''+record+'''@METHODS@
};
void ReportLifetime() {
    // Report only after both neighboring destructors and the owner shutdown have completed
    printf("%llu\\n",(unsigned long long)digest);
}
extern "C" cAudioManager AudioManager;
extern "C" void AudioManager_InitState(cAudioManager*);
extern "C" void AudioOwner_Destroy(cAudioManager*);
extern "C" void TestLifetime_Observe(cAudioManager *manager) {
    // Observe the reset while the C++ reference owner is still alive
    bytes(manager,sizeof(*manager));assert(manager->m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal==0);
}
extern "C" void AudioLifecycle_Terminate(cAudioManager *manager) {
    // Observe live state at termination and mutate script data before the final reset
    event(30);bytes(manager,sizeof(*manager));
    manager->m_bIsInitialised=0;
    manager->m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal=123;
}
'''
rng=(root/'src/core/re3.cpp').read_text();rng=rng[rng.index('#ifdef USE_PS2_RAND\nunsigned long long myrand_seed'):rng.index('void\nmysrand')].replace('int\nmyrand(void)','extern "C" int\nmyrand(void)').replace('myrand(void)\n{','myrand(void)\n{\n    // Count the original generator draws at process startup and later initialization\n    ++draws;event(10);')
probe='''
struct Probe {
    unsigned n;
    Probe(unsigned e):n(e) {
        // Bracket owner startup with neighboring dynamic initialization callbacks
        if(draws==0)atexit(ReportLifetime);event(n);event(draws);
    }
    ~Probe() {
        // Bracket owner shutdown with reverse-order exit callbacks
        event(n+100);event(draws);

    }
};
static Probe beforeOwner(1);
@OWNER@
static Probe afterOwner(2);
int main() {
    // Verify pre-main draws and compare repeated explicit owner lifetimes
    assert(draws==5);
    for(unsigned seed=0;seed<256;seed++) {
        alignas(cAudioManager) unsigned char storage[sizeof(cAudioManager)];
        cAudioManager *owner=(cAudioManager*)storage;
        @INIT@
        // Seed retained bytes only after the reference object lifetime has begun
        memset((void*)owner,seed,sizeof(*owner));
        AudioManager_InitState(owner);
        bytes(owner,sizeof(*owner));
        owner->m_bIsInitialised=(uint8_t)(seed%3);
        owner->m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal=seed;
        @DESTROY@

    }
    // Leave initialized global state for the real registered shutdown callback
    AudioManager.m_bIsInitialised=1;
    AudioManager.m_sAudioScriptObjectManager.m_nScriptObjectEntityTotal=77;
    event(99);return 0;
}
'''
for variant in ['before','after']:
    methods='cAudioManager();~cAudioManager();'
    if variant=='before':
        selected=dtor[:-1]+'    // Observe state before the C++ object lifetime ends\n    TestLifetime_Observe(this);\n}'
        owner='cAudioManager AudioManager;'
        body=common.replace('@METHODS@',methods)+rng+ctor+'\n'+selected+probe.replace('@OWNER@',owner).replace('@INIT@','new(owner) cAudioManager;').replace('@DESTROY@','owner->~cAudioManager();')
    else:
        body=common.replace('@METHODS@',methods)+rng+ctor+'\n'+migration.DTOR+probe.replace('@OWNER@','cAudioManager AudioManager;').replace('@INIT@','new(owner) cAudioManager;').replace('@DESTROY@','owner->~cAudioManager();')
    # Bracket the production owner's object with separate neighboring startup objects
    probe_body=probe[probe.index('struct Probe'):probe.index('static Probe beforeOwner')]
    shared=includes+'\n#include <stdint.h>\n#include <stdio.h>\n#include <stdlib.h>\nstruct cAudioManager {\n'+record+methods+'\n};\nextern "C" cAudioManager AudioManager;\nextern unsigned draws;\nextern uint64_t digest;\nvoid bytes(const void*,size_t);\nvoid event(unsigned);\nvoid ReportLifetime();\n'
    body=body.replace(probe_body,'').replace('static Probe beforeOwner(1);','').replace('static Probe afterOwner(2);','').replace('static uint64_t digest','uint64_t digest').replace('static unsigned draws','unsigned draws').replace('static void bytes','void bytes').replace('static void event','void event')
    for n in [1,2]:(out/(variant+'-probe'+str(n)+'.cpp')).write_text(shared+probe_body+'static Probe adjacent('+str(n)+');\n',newline='\n')
    (out/(variant+'.cpp')).write_text(body,newline='\n')

for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#undef AUDIO_REFLECTIONS\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete C cleanup, unchanged fields, retained constructor/exit registration and project edits audited')

# Add the same observer before returning from the actual C destruction operation
observed=kernel.replace('void\nAudioOwner_Destroy', 'extern void TestLifetime_Observe(cAudioManager *manager);\n\nvoid\nAudioOwner_Destroy',1).replace('AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);','AudioScriptObjectManager_Reset(&manager->m_sAudioScriptObjectManager);\n\t// Observe live state at the same point as the original destructor fixture\n\tTestLifetime_Observe(manager);',1)
(out/'observed-owner.c').write_text(observed,newline='\n')
