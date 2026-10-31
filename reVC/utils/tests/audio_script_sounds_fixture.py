"""Audit complete script sound conversion and generate original/C comparisons."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_script_sounds_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-script-sounds-c-tests'

def normalize(source):
    # Compare all statements independently of formatting and comments
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioScriptSounds.c').read_text()
methods=[]
for name,target in migration.METHODS.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'AudioScriptSounds_',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b]
converted=macros.replace('m_sQueueSample','manager->m_sQueueSample')
for name in migration.MACRO_NAMES:converted=re.sub(r'\b'+name+r'\b','AUDIO_SCRIPT_SOUNDS_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioScriptSounds_Process')])
assert normalize(migration.constants(old)) in normalize(kernel)
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioScriptObject.h','soundlist.h','../core/SurfaceTypes.h'])
template=(root/'utils/tests/audio_script_sounds.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@HOST@',migration.HOST).replace('@CONSTANTS@',migration.constants(old))
for variant in ['before','after']:
    # Compile unchanged original generators or invoke their real separate C11 implementation
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','active->ProcessScriptObject(currentId);active->ProcessOneShotScriptObject(sound);active->ProcessLoopingScriptObject(sound)' if variant=='before' else 'TestScriptSoundsC(active,currentId,sound)')
    if variant=='after':
        # Observe direct C music entries using the real global record and the original scripted callback model
        source=source.replace('static MusicState MusicManager;','static MusicState fakeMusic;')
        source=source.replace('MusicManager.m_n','fakeMusic.m_n').replace('&MusicManager,sizeof(MusicManager)','&fakeMusic,sizeof(fakeMusic)').replace('memset(&MusicManager,0,sizeof(MusicManager))','memset(&fakeMusic,0,sizeof(fakeMusic))')
        source=source.replace(migration.HOST.replace('MusicManager.m_n','fakeMusic.m_n'),'''extern "C" uint8_t MusicState_GetMusicMode(const cMusicManager *manager) {
    // Check the global owner before observing the original mode read
    assert(manager==&MusicManager);return fakeMusic.m_nMusicMode;
}
extern "C" uint32_t MusicState_GetCurrentTrack(const cMusicManager *manager) {
    // Check the global owner before observing the original playing-track read
    assert(manager==&MusicManager);return fakeMusic.m_nPlayingTrack;
}
extern "C" void MusicClubs_SetMalibuClubTrackPos(cMusicManager *manager,uint8_t sound) {
    // Check the global owner at the original Malibu sample setup boundary
    assert(manager==&MusicManager);fakeMusic.SetMalibuClubTrackPos(sound);
}
extern "C" void MusicClubs_SetStripClubTrackPos(cMusicManager *manager,uint8_t sound) {
    // Check the global owner at the original strip-club sample setup boundary
    assert(manager==&MusicManager);fakeMusic.SetStripClubTrackPos(sound);
}
''')
        source='#include "Validate.h"\n#include "MusicManager.h"\n'+source
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'music-owner.c').write_text('#include "MusicManager.h"\n/* Supply the real C music record for isolated direct-entry observers */\ncMusicManager MusicManager;\n',newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestScriptSoundsC(cAudioManager *manager,int32_t id,uint8_t sound)
{
    // Exercise complete routing and both generators through their real C declarations
    AudioScriptSounds_Process(manager,id);
    AudioScriptSounds_OneShot(manager,sound);
    AudioScriptSounds_Looping(manager,sound);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#undef AUDIO_REFLECTIONS\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete script routing, one-shot/looping generators, direct C music reads/positioning, constants, macros and project registration audited')
