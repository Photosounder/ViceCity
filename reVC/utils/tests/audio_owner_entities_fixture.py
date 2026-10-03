"""Compare real C entity creation/facades with saved owner member calls."""
from pathlib import Path
import re
import audio_police_state_migration as police
import audio_mission_data_migration as data
import audio_mission_state_migration as mission
from audio_controls_migration import function
from audio_owner_init_migration import fields
from audio_owner_entities_migration import METHODS, FACADES, calls, owner, header, facade, creation_body
root=Path(__file__).resolve().parents[2]
out=root/'build/audio-owner-entities-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text()
kernel=(root/'src/audio/AudioManagerEntities.c').read_text()
def normalize(source):
    # Compare real statements independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
create=function(old,'cAudioManager::','CreateEntity')
actual=function(kernel,'AudioManager_','CreateEntity')
assert normalize(creation_body(create))==normalize(actual[actual.index('{'):])
old_facade=(out/'before/src/audio/DMAudio.cpp').read_text()
for name in FACADES:
    expected=calls(function(old_facade,'DMAudio_',name)).replace('(CPhysical *)UID','UID')
    for a,b in [('int32','int32_t'),('eAudioType','enum eAudioType'),('bool8','uint8_t'),('uint16','uint16_t')]:expected=re.sub(r'\b'+a+r'\b',b,expected)
    assert normalize(expected)==normalize(function(kernel,'DMAudio_',name)),name
for name in ['AudioManager.cpp','AudioLogic.cpp','AudioCollision.cpp','PolRadio.cpp','MusicManager.cpp','DMAudio.cpp']:
    saved=(out/'before/src/audio'/name).read_text()
    expected=owner(saved) if name=='AudioManager.cpp' else facade(saved) if name=='DMAudio.cpp' else calls(saved)
    assert normalize(data.logic(mission.logic(expected)) if name=='AudioLogic.cpp' else data.facade(mission.facade(expected)) if name=='DMAudio.cpp' else police.radio(mission.calls(expected)) if name=='PolRadio.cpp' else mission.calls(expected))==normalize((root/'src/audio'/name).read_text()),name
assert normalize(data.header(mission.header(header((out/'before/src/audio/AudioManager.h').read_text()))))==normalize((root/'src/audio/AudioManager.h').read_text())
record=fields((root/'src/audio/AudioManager.h').read_text())
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','DMAudio.h'])
common=includes+'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
using uint8=uint8_t;using uint16=uint16_t;using uint32=uint32_t;using int32=int32_t;using bool8=uint8_t;
struct CPhysical;
struct cAudioManager {
'''+record+'''
'''+'\n'.join(function(old,'cAudioManager::',name).split('{',1)[0].replace('cAudioManager::','').strip()+';' for name in METHODS)+'''
};
extern "C" cAudioManager AudioManager;
cAudioManager AudioManager;
extern "C" int32_t AudioManager_CreateEntity(cAudioManager*,enum eAudioType,void*);
extern "C" void TestOwnerEntityC(void);
'''
methods='\n'.join(function(old,'cAudioManager::',name) for name in METHODS)
public='\n'.join(function(old_facade,'DMAudio_',name) for name in FACADES)
trace=(root/'utils/tests/audio_owner_entities.cpp.in').read_text()
for variant in ['before','after']:
    source=common+(methods+'\n'+public if variant=='before' else '')+trace
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
#include "DMAudio.h"
#include <string.h>
#include <assert.h>
void TestOwnerEntityC(void) {
    // Reach the same live record and public facade exports from an actual C caller
    memset(&AudioManager,0,sizeof(AudioManager));
    assert(DMAudio_CreateEntity(AUDIOTYPE_PHYSICAL,(void*)(uintptr_t)8)==AEHANDLE_ERROR_NOAUDIOSYS);
    AudioManager.m_bIsInitialised=1;
    int32_t id=DMAudio_CreateEntity(AUDIOTYPE_PHYSICAL,(void*)(uintptr_t)8);
    assert(id==0 && AudioManager.m_nAudioEntitiesCount==1);
    DMAudio_SetEntityStatus(id,255);assert(DMAudio_GetEntityStatus(id)==255);
    DMAudio_PlayOneShot(id,SOUND_FRONTEND_RADIO_TURN_OFF,1.25f);
    assert(AudioManager.m_asAudioEntities[id].m_AudioEvents==1);
    DMAudio_DestroyEntity(id);assert(AudioManager.m_nAudioEntitiesCount==0);
}''',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Actual C creation body, all five facade bodies, six caller files, and owner header audited')
