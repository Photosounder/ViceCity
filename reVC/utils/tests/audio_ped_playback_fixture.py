"""Audit full playback bodies and generate observable original/C comparisons."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_ped_playback_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-ped-playback-c-tests'

def normalize(source):
    # Ignore only comments and whitespace in complete statement comparisons
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPedPlayback.c').read_text()
debug=function(old,'cAudioManager::','DebugPlayPedComment');play=migration.extract(old)
actual=function(kernel,'AudioPedPlayback_','Debug')
assert normalize(migration.debug_body(debug))==normalize(actual[actual.index('{'):])
actual=migration.extract(kernel)
assert normalize(migration.playback_body(play))==normalize(actual[actual.index('{'):])
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioPoliceGame.h','audio_enums.h'])
template=(root/'utils/tests/audio_ped_playback.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@HOST@',migration.HOST)
for variant in ['before','after']:
    # Compare unchanged original bodies against a separately compiled C caller
    source=template.replace('@METHODS@',debug+'\n'+play if variant=='before' else '').replace('@DEBUG@','active->DebugPlayPedComment(sound)' if variant=='before' else 'TestPedPlaybackC(active,NULL,sound)').replace('@PROCESS@','PedComments_Process(queue)' if variant=='before' else 'TestPedPlaybackC(NULL,queue,0)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestPedPlaybackC(cAudioManager *manager,cPedComments *comments,int32_t sound)
{
    // Invoke both complete operations through the real C declarations
    if(manager)AudioPedPlayback_Debug(manager,sound);
    else PedComments_Process(comments);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-attach':'#undef ATTACH_RELEASING_SOUNDS_TO_ENTITIES\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Full playback, shared history, debug producer, adapters, declarations and project settings audited')
