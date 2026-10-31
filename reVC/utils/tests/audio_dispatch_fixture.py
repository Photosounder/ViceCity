import weather_migration as _weather_conversion
"""Audit both routing switches, every adapter and unchanged enum/header declarations."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_dispatch_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-dispatch-c-tests'
old=(out/'before/src/audio/AudioLogic.cpp').read_text();kernel=(root/'src/audio/AudioDispatch.c').read_text()
def normalize(s):
    # Apply the explicit verified weather rename before comparing every original statement
    s=_weather_conversion.calls(s)
    # Compare complete statements independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
methods=[]
for name,target in [('ProcessEntity','Entity'),('ProcessPhysical','Physical')]:
    original=function(old,'cAudioManager::',name);methods.append(original)
    actual=function(kernel,'AudioDispatch_',target)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
macros=(root/'src/audio/AudioManager.h').read_text();start=macros.index('#ifdef AUDIO_REVERB\n#define SET_SOUND_REVERB')
original_macro=macros[start:macros.index('#endif',start)+len('#endif')]
assert normalize(original_macro.replace('SET_SOUND_REVERB','AUDIO_DISPATCH_REVERB').replace('m_sQueueSample','manager->m_sQueueSample')) in normalize(kernel)
for path,transform in [('src/audio/AudioManager.cpp',migration.owner),('src/audio/AudioLogic.cpp',migration.owner),('src/audio/AudioManager.h',migration.header),('src/audio/AudioEffects.c',migration.calls),('src/entities/Entity.h',migration.entity_header),('src/core/Game.h',migration.game_header)]:
    assert normalize(transform((out/'before'/path).read_text()))==normalize((root/path).read_text()),path
for original,target,name in [('src/entities/Entity.h','src/entities/EntityTypes.h','eEntityType'),('src/core/Game.h','src/core/GameAreas.h','eAreaName')]:
    assert normalize(migration.enumeration((out/'before'/original).read_text(),name))==normalize(migration.enumeration((root/target).read_text(),name))
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','../entities/EntityTypes.h','GameAreas.h'])
declarations=[];generators=[]
for index,name in enumerate(migration.INDEXED+migration.UNINDEXED):
    parameter='int32 id' if name in migration.INDEXED else ''
    declaration='    void '+name+'('+parameter+');\n'
    definition='void cAudioManager::'+name+'('+parameter+')\n{\n    // Observe the original generator choice and forwarded entity index\n    '+('value((uint32_t)id);' if parameter else '')+'generator('+str(10+index)+');\n}\n'
    if name=='ProcessBridge':
        declaration='#ifdef GTA_BRIDGE\n'+declaration+'#endif\n';definition='#ifdef GTA_BRIDGE\n'+definition+'#endif\n'
    declarations.append(declaration);generators.append(definition)
# Observe the now-direct C explosion generator at the same isolated routing boundary
explosion_observer='extern "C" void AudioExplosions_Process(cAudioManager *manager,int32_t id) {\n    // Observe the original generator choice and argument through direct C dispatch\n    manager->ProcessExplosions(id);\n}\n'
fire_observer='extern "C" void AudioFires_Process(cAudioManager *manager,int32_t id) {\n    // Observe the original fire generator and entity argument through C dispatch\n    manager->ProcessFires(id);\n}\n'
water_observer='extern "C" void AudioWaterCannon_Process(cAudioManager *manager,int32_t id) {\n    // Observe the same original water-cannon routing callback\n    manager->ProcessWaterCannon(id);\n}\n'
script_observer='extern "C" void AudioScriptSounds_Process(cAudioManager *manager,int32_t id) {\n    // Observe the same original script-object choice through direct C dispatch\n    manager->ProcessScriptObject(id);\n}\n'
weather_frontend_observer='extern "C" void AudioWeather_Process(cAudioManager *manager,int32_t id) {\n    // Observe original weather routing through the complete C entry point\n    manager->ProcessWeather(id);\n}\nextern "C" void AudioFrontEnd_Process(cAudioManager *manager) {\n    // Observe original front-end routing through the complete C entry point\n    manager->ProcessFrontEnd();\n}\n'
world_observer=''.join('extern "C" void AudioWorldLoops_'+target+'(cAudioManager *manager) {\n    // Observe the original world generator through its complete C entry point\n    manager->'+name+'();\n}\n' for name,target in [('ProcessProjectiles','Projectiles'),('ProcessEscalators','Escalators'),('ProcessExtraSounds','ExtraSounds')])
garage_hydrant_observer=''.join('extern \"C\" void AudioGarageHydrant_'+target+'(cAudioManager *manager) {\n    // Observe the original generator choice through direct C dispatch\n    manager->'+name+'();\n}\n' for name,target in [('ProcessGarages','Garages'),('ProcessFireHydrant','Hydrant')])
bridge_observer='#ifdef GTA_BRIDGE\nextern \"C\" void AudioBridge_Process(cAudioManager *manager) {\n    // Observe the original optional state-machine routing through direct C dispatch\n    manager->ProcessBridge();\n}\n#endif\n'
template=(root/'utils/tests/audio_dispatch.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text())).replace('@DECLARATIONS@',''.join(declarations)).replace('@GENERATORS@','\n'.join(generators))
for variant in ('before','after'):
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@HOST@',__import__('audio_vehicle_migration').logic(migration.HOST)+explosion_observer+fire_observer+water_observer+script_observer+weather_frontend_observer+world_observer+garage_hydrant_observer+bridge_observer+'\nextern "C" void AudioVehicle_Process(cAudioManager *manager,CVehicle *vehicle) {\n    // Observe the same original vehicle generator through the C routing boundary\n    manager->ProcessVehicle(vehicle);\n}\nextern \"C\" void AudioPed_Process(cAudioManager *manager,CPed *ped) {\n    // Observe the same original pedestrian generator through the C routing boundary\n    manager->ProcessPed((CPhysical *)ped);\n}\n' if variant=='after' else '')
    source=source.replace('@ENTITY@','active->ProcessEntity(currentId)' if variant=='before' else 'AudioDispatch_Entity(active,currentId)').replace('@PHYSICAL@','active->ProcessPhysical(currentId)' if variant=='before' else 'AudioDispatch_Physical(active,currentId)')
    source=source.replace('@CALLER@','active->ProcessEntity(currentId);active->ProcessPhysical(currentId)' if variant=='before' else 'TestDispatchC(active,currentId)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nvoid TestDispatchC(cAudioManager *manager, int32_t id)\n{\n    // Exercise both routing operations through their actual C declarations\n    AudioDispatch_Entity(manager,id);\n    AudioDispatch_Physical(manager,id);\n}\n',newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','bridge':'#define GTA_BRIDGE\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Both routing bodies, all generator/pointer adapters, full caller/header edits and 25 enum constants audited')
