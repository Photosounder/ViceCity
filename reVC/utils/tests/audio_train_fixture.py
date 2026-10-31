# Compare the guarded train generator with its complete original C++ body
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_train_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-train-c-tests'
def normalize(source):
    # Ignore only comments and whitespace when checking complete statements
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioTrain.c').read_text()
original=function(old,'cAudioManager::','ProcessTrainNoise');actual=function(kernel,'AudioTrain_','Process')
assert normalize(migration.body(original))==normalize(actual[actual.index('{'):])
assert normalize(migration.logic(old))==normalize((root/'src/audio/AudioLogic.cpp').read_text())
assert normalize(migration.header(header))==normalize((root/'src/audio/AudioManager.h').read_text())
assert normalize(migration.CONSTANTS) in normalize(kernel)
assert normalize(migration.HOST) in normalize((root/'src/audio/AudioLogic.cpp').read_text())
macros=header[header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS'):header.index('#if defined(AUDIO_MSS)')]
converted=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',macros)
for name in ['SET_EMITTING_VOLUME','RESET_LOOP_OFFSETS','SET_LOOP_OFFSETS','SET_SOUND_REVERB','SET_SOUND_REFLECTION']:converted=re.sub(r'\b'+name+r'\b','AUDIO_TRAIN_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('static uint8_t AudioTrain_Less')])
for name in ['reVC.vcxproj','reVC.vcxproj.filters']:assert migration.project((out/'before/build'/name).read_text())==(root/'build'/name).read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h'])
template=(root/'utils/tests/audio_train.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@CONSTANTS@',migration.CONSTANTS)
for variant in ['before','after']:
    # Insert the unmodified original method or the separately compiled C entry point
    source=template.replace('@METHODS@',original if variant=='before' else '').replace('@RUN@','active->ProcessTrainNoise(params)' if variant=='before' else 'TestTrainC(active,&params)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nuint8_t TestTrainC(cAudioManager *manager,cVehicleParams *params)\n{\n    // Invoke the guarded entry point through its actual C declaration\n    return AudioTrain_Process(manager,params);\n}\n',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Complete train body, macros, constants, declarations, adapters and project registration audited')
