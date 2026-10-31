# Audit and compare complete speech routing and all original character handlers
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_ped_talk_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-ped-talk-c-tests'
def normalize(source):
    # Ignore only comments and whitespace when comparing complete bodies
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))
old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioPedTalk.c').read_text();handlers=migration.names(old);methods=[]
for name in ['GetPedCommentSfx','GetPhrase']+handlers:
    # Compare every statement in the router, reference helper and all character handlers
    original=function(old,'cAudioManager::',name);methods.append(original);target='Select' if name=='GetPedCommentSfx' else 'Phrase' if name=='GetPhrase' else migration.target(name);actual=function(kernel,'AudioPedTalk_',target)
    assert normalize(migration.body(original,handlers))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioPedSpeech.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
a=old.index('#define cooldown_phrase');b=old.index('\n\n',a);macro=old[a:b]
actual_macro=kernel[kernel.index('#define AUDIO_PED_TALK_COOLDOWN'):kernel.index('\n\n',kernel.index('#define AUDIO_PED_TALK_COOLDOWN'))]
assert normalize(macro.replace('cooldown_phrase','AUDIO_PED_TALK_COOLDOWN').replace('uint8','uint8_t'))==normalize(actual_macro)
assert old.count('cooldown_phrase(')-1==kernel.count('AUDIO_PED_TALK_COOLDOWN(')-1==10
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','AudioPoliceGame.h','audio_enums.h'])
declarations='\n'.join(method.split('{',1)[0].strip().replace('cAudioManager::','')+';' for method in methods)
events=list(dict.fromkeys(re.findall(r'case (SOUND_\w+):','\n'.join(methods))));events.append('SOUND_BOAT_SLOWDOWN')
models=list(dict.fromkeys(re.findall(r'case (MI_\w+):',methods[0])));models.append('MI_LANDSTAL')
original_calls=['case 0:return active->GetPedCommentSfx(ped,sound);','case 1:active->GetPhrase(phraseValues[0],phraseValues[(seed%3==0)?0:1],sample,offset);return phraseValues[0];']
c_calls=['case 0:return AudioPedTalk_Select(manager,ped,sound);','case 1:AudioPedTalk_Phrase(manager,&values[0],&values[(seed%3==0)?0:1],sample,offset);return values[0];']
for i,name in enumerate(handlers,2):
    # Exercise every public handler directly as well as through model routing
    argument='ped, '+('model, ' if name=='GetSpecialCharacterTalkSfx' else '')+'sound'
    original_calls.append('case '+str(i)+':return active->'+name+'('+argument+');');c_calls.append('case '+str(i)+':return AudioPedTalk_'+migration.target(name)+'(manager, '+argument+');')
template=(root/'utils/tests/audio_ped_talk.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@DECLARATIONS@',declarations).replace('@MACRO@',macro).replace('@ORIGINAL_CALLS@','\n'.join(original_calls)).replace('@KINDS@',str(len(handlers)+2)).replace('@EVENTS@','static const uint16_t events[]={'+','.join(events)+'};').replace('@MODELS@','static const int16_t models[]={'+','.join(models)+'};')
for marker,path in [('MODEL_IDS','src/modelinfo/ModelIds.h'),('PED_STATES','src/peds/PedStates.h')]:template=template.replace('@'+marker+'@',str(root/path).replace('\\','/'))
for variant in ['before','after']:
    # Compile untouched original bodies or call the separately compiled C implementations
    source=('#define TEST_ORIGINAL\n' if variant=='before' else '')+template.replace('@METHODS@','\n'.join(methods)).replace('@RUN@','runOriginal(kind,seed,ped,sound,model,sample,offset)' if variant=='before' else 'TestPedTalkC(active,kind,seed,ped,sound,model,phraseValues,sample,offset)')
    if variant=='after':source=source.replace('\n'.join(methods),'')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\nuint32_t TestPedTalkC(cAudioManager *manager,unsigned kind,unsigned seed,CPed *ped,uint16_t sound,int32_t model,uint32_t *values,uint32_t sample,uint32_t offset)\n{\n    // Invoke every complete C operation through its actual declaration\n    switch(kind){\n'+'\n'.join(c_calls)+'\n    default:return UINT32_MAX;\n    }\n}\n',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
print('Router, phrase aliasing, 96 character bodies, ten cooldown sites, adapters and direct C speech caller audited')
