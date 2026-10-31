"""Audit full weather/front-end conversion and generate original/C comparisons."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_weather_frontend_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-weather-frontend-c-tests'

def normalize(source):
    # Compare every statement independently of comments and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

old=(out/'before/src/audio/AudioLogic.cpp').read_text();header=(out/'before/src/audio/AudioManager.h').read_text();kernel=(root/'src/audio/AudioWeatherFrontend.c').read_text();methods=[]
for name,target in migration.METHODS.items():
    original=function(old,'cAudioManager::',name);actual=function(kernel,'',target);methods.append(original)
    assert normalize(migration.body(original))==normalize(actual[actual.index('{'):]),name
for name,transform in [('src/audio/AudioLogic.cpp',migration.logic),('src/audio/AudioManager.h',migration.header),('src/audio/AudioDispatch.c',migration.calls),('build/reVC.vcxproj',migration.project),('build/reVC.vcxproj.filters',migration.project)]:assert normalize(transform((out/'before'/name).read_text()))==normalize((root/name).read_text()),name
a=header.index('#ifndef GTA_PS2\n#define RESET_LOOP_OFFSETS');b=header.index('#if defined(AUDIO_MSS)',a);macros=header[a:b]
converted=macros.replace('m_sQueueSample','manager->m_sQueueSample')
for name in migration.MACRO_NAMES:converted=re.sub(r'\b'+name+r'\b','AUDIO_WEATHER_FRONTEND_'+name,converted)
assert normalize(converted)==normalize(kernel[kernel.index('#ifndef GTA_PS2'):kernel.index('void\nAudioWeather_Process')])
assert normalize(migration.CONSTANTS) in normalize(kernel)
for name in ['Max','Min']:
    original=re.search(r'(?m)^#define '+name+r'\(a,b\) (.*)$',(root/'src/core/common.h').read_text()).group(1)
    assert '#define AUDIO_WEATHER_FRONTEND_'+name.upper()+'(a,b) '+original in kernel
assert 'inline float Abs(float x) { return fabsf(x); }' in (root/'src/math/Maths.h').read_text()
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','soundlist.h'])
template=(root/'utils/tests/audio_weather_frontend.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields(header)).replace('@MACROS@',macros).replace('@HOST@',migration.HOST).replace('@CONSTANTS@',migration.CONSTANTS)
for variant in ['before','after']:
    # Compile unchanged complete originals or invoke the actual separately compiled C11 operations
    source=template.replace('@METHODS@','\n'.join(methods) if variant=='before' else '').replace('@RUN@','active->ProcessWeather(currentId);active->ProcessFrontEnd()' if variant=='before' else 'TestWeatherFrontendC(active,currentId)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
void TestWeatherFrontendC(cAudioManager *manager,int32_t id)
{
    // Exercise complete weather and front-end audio through their real C declarations
    AudioWeather_Process(manager,id);
    AudioFrontEnd_Process(manager);
}
''',newline='\n')
(out/'fast_hash.c').write_bytes((root/'utils/tests/audio_aircraft_observer.c').read_bytes())
for shape,flags in {'default':'','vanilla':'#undef FIX_BUGS\n','no-reverb':'#undef AUDIO_REVERB\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n','no-timescale':'#undef USE_TIME_SCALE_FOR_AUDIO\n','no-reflections':'#undef AUDIO_REFLECTIONS\n'}.items():(out/(shape+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete weather/front-end generators, five game adapters, arithmetic macros, shared statics and direct C routing audited')
