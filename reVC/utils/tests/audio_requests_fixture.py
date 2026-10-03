"""Compare sound submission and reflection expansion against saved owner bodies."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_requests_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-requests-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();kernel=(root/'src/audio/AudioRequests.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
methods=[]
for name,target in [('AddSampleToRequestedQueue','Submit'),('AddReflectionsToRequestedQueue','AddReflections')]:
    original=function(old,'cAudioManager::',name);methods.append(original)
    actual=function(kernel,'AudioRequests_',target)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioManager.cpp',migration.owner),('AudioLogic.cpp',migration.calls),('AudioCollision.cpp',migration.calls),('MusicManager.cpp',migration.calls),('PolRadio.cpp',migration.radio),('AudioPoliceChannel.c',migration.calls),('AudioManager.h',migration.header)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioMath.h'])
template=(root/'utils/tests/audio_requests.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ('before','after'):
    source=template.replace('@METHODS@',methods[0]+'\n#ifdef AUDIO_REFLECTIONS\n'+methods[1]+'\n#endif' if variant=='before' else '')
    source=source.replace('@SUBMIT@','active->AddSampleToRequestedQueue()' if variant=='before' else 'TestRequestsC(active)').replace('@REFLECT@','active->AddReflectionsToRequestedQueue()' if variant=='before' else 'AudioRequests_AddReflections(active)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestRequestsC(cAudioManager *manager)\n{\n    // Submit the prepared owner sound through its actual C declaration\n    AudioRequests_Submit(manager);\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','unscaled':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-external':'#undef EXTERNAL_3D_SOUND\n','no-reflections':'#undef AUDIO_REFLECTIONS\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both complete bodies, partial restoration, recursion, host callbacks and all caller edits audited')
