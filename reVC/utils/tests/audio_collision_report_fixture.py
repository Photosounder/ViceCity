"""Compare original collision reporting against its actual C unit and public facade."""
from pathlib import Path
import re
from audio_controls_migration import function
from audio_owner_init_migration import fields
import audio_collision_report_migration as migration
root=Path(__file__).resolve().parents[2];out=root/'build/audio-collision-report-c-tests'
def normalize(s):
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
old=(out/'before/src/audio/AudioCollision.cpp').read_text();method=migration.extract(old)
kernel=(root/'src/audio/AudioCollisionReport.c').read_text()
actual=function(kernel,'AudioCollisionReport_','Report')
assert normalize(migration.body(method))==normalize(actual[actual.index('{'):])
facade=function((out/'before/src/audio/DMAudio.cpp').read_text(),'DMAudio_','ReportCollision').replace('uint8 ', 'uint8_t ').replace('AudioManager.ReportCollision(', 'AudioCollisionReport_Report(&AudioManager, ')
assert normalize(facade)==normalize(function(kernel,'DMAudio_','ReportCollision'))
for name,transform in [('AudioCollision.cpp',migration.owner),('AudioManager.h',migration.header),('DMAudio.cpp',migration.facade),('AudioLogic.cpp',migration.calls)]:
    assert normalize(transform((out/'before/src/audio'/name).read_text()))==normalize((root/'src/audio'/name).read_text()),name
includes='\n'.join('#include "'+name+'"' for name in ['AudioSound.h','AudioCollision.h','AudioEntities.h','AudioPedComments.h','AudioMissionPosition.h','AudioState.h','AudioReflectionTypes.h','AudioCrimes.h','AudioPoliceQueue.h','sampman.h','SurfaceTypes.h'])
template=(root/'utils/tests/audio_collision_report.cpp.in').read_text().replace('@INCLUDES@',includes).replace('@FIELDS@',fields((root/'src/audio/AudioManager.h').read_text()))
for variant in ('before','after'):
    call='active->ReportCollision(a,b,(uint8_t)seed,(uint8_t)(seed>>4),power,speed);' if variant=='before' else 'TestCollisionReportC(active,a,b,(uint8_t)seed,(uint8_t)(seed>>4),power,speed,active==&AudioManager);'
    source=template.replace('@METHOD@',method if variant=='before' else '').replace('@HOST@',migration.HOST if variant=='after' else '').replace('@CALL@',call)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('#include "AudioManager.h"\n#include "DMAudio.h"\nvoid TestCollisionReportC(cAudioManager *m,CEntity *a,CEntity *b,uint8_t s1,uint8_t s2,float power,float speed,int facade)\n{\n    // Exercise the public facade and direct operation from an actual C caller\n    if(facade) DMAudio_ReportCollision(a,b,s1,s2,power,speed);\n    else AudioCollisionReport_Report(m,a,b,s1,s2,power,speed);\n}\n',newline='\n')
for name in ['default','vanilla','no-reverb','no-external','ps2','single','single-c','wav']:
    (out/(name+'.h')).write_bytes((root/'build/audio-collision-service-c-tests'/(name+'.h')).read_bytes())
print('Complete collision report, owner declarations and public facade audited')
