"""Compare reload and entity cleanup using actual C records and primitive helpers."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_reload_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-reload-c-tests'
old=(out/'before/src/audio/AudioManager.cpp').read_text();kernel=(root/'src/audio/AudioReload.c').read_text()
def normalize(s):
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
methods=[]
for name,target in [('ResetTimers','ResetTimers'),('DestroyAllGameCreatedEntities','DestroyEntities')]:
    original=function(old,'cAudioManager::',name);methods.append(original)
    actual=function(kernel,'AudioReload_',target);assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('AudioManager.cpp',migration.owner),('AudioManager.h',migration.header),('DMAudio.cpp',migration.facade)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
assert normalize(function(old,'cAudioManager::','SetOutputMode').split('{',1)[1])=='}'
assert normalize(function(kernel,'DMAudio_','SetOutputMode').split('{',1)[1])=='(void)surround;}'
facades=(out/'before/src/audio/DMAudio.cpp').read_text()
for name,target,parameter in [('ResetTimers','ResetTimers','time'),('DestroyAllGameCreatedEntities','DestroyEntities','')]:
    expected=function(facades,'DMAudio_',name).replace('uint32 ','uint32_t ').replace('AudioManager.'+name+'('+parameter+')','AudioReload_'+target+'(&AudioManager'+(', '+parameter if parameter else '')+')')
    assert normalize(expected)==normalize(function(kernel,'DMAudio_',name))
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','AudioScriptObject.h','sampman.h','Pool.h'])
template=(root/'utils/tests/audio_reload.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ('before','after'):
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '')
    source=source.replace('@RESET@','active->ResetTimers(time)' if variant=='before' else 'AudioReload_ResetTimers(active,time)').replace('@DESTROY@','active->DestroyAllGameCreatedEntities()' if variant=='before' else 'AudioReload_DestroyEntities(active)')
    source=source.replace('@FACADE@','AudioManager.ResetTimers(time); AudioManager.DestroyAllGameCreatedEntities();' if variant=='before' else 'TestReloadFacadesC(time);')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "DMAudio.h"\nvoid TestReloadFacadesC(uint32_t time)\n{\n    // Exercise all three public C facades against the live global owner\n    DMAudio_ResetTimers(time);\n    DMAudio_DestroyAllGameCreatedEntities();\n    DMAudio_SetOutputMode(255);\n}\n',newline='\n')
reset=function((root/'src/audio/AudioScriptObject.c').read_text(),'AudioScriptObject_','Reset')
(out/'script-reset.c').write_text('#include "AudioScriptObject.h"\n#include "soundlist.h"\n#include "sampman.h"\n'+reset+'\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','no-oal':'#undef AUDIO_OAL\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Reload/cleanup bodies, facades, no-op semantics and complete owner/header edits audited')
