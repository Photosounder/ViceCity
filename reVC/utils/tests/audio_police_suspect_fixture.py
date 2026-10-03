"""Compare the complete color table, model IDs and original/C vehicle report."""
from pathlib import Path
import re
import audio_game_entries_migration as entries
from audio_controls_migration import function
import audio_police_suspect_migration as suspect
import audio_police_state_fixture as state
root=Path(__file__).resolve().parents[2];out=root/'build/audio-police-suspect-c-tests'
old=(out/'before/src/audio/PolRadio.cpp').read_text();kernel=(root/'src/audio/AudioPoliceSuspect.c').read_text()
def normalize(s):
    # Compare actual statements and declarations independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))
actual=function(kernel,'AudioPolice_','SuspectReport')
assert normalize(suspect.body(function(old,'cAudioManager::','SetupSuspectLastSeenReport')))==normalize(actual[actual.index('{'):])
a=kernel.index('static const uint32_t gCarColourTable');b=kernel.index('\nvoid\n',a)
assert normalize(suspect.TABLE.replace('Const uint32','static const uint32_t'))==normalize(kernel[a:b])
for file,transform in [('PolRadio.cpp',suspect.radio),('AudioManager.h',suspect.header),('AudioPoliceGame.h',suspect.game_header),('AudioPoliceReports.c',suspect.calls)]:
    assert normalize(transform((out/'before/src/audio'/file).read_text()))==normalize((root/'src/audio'/file).read_text()),file
model=(out/'before/src/modelinfo/ModelIndices.h').read_text()
expected=model.replace(suspect.ENUMS,'#include "ModelIds.h"')
assert normalize(expected)==normalize((root/'src/modelinfo/ModelIndices.h').read_text())
assert normalize('#pragma once\n'+suspect.ENUMS)==normalize((root/'src/modelinfo/ModelIds.h').read_text())
names=re.findall(r'(?m)^\s*([A-Z][A-Z_0-9]*)\s*(?:=|,)',suspect.ENUMS)
assert names and len(names)==len(set(names))
common=state.common.replace('void InitialisePoliceRadioZones();','void SetupSuspectLastSeenReport();\nCVehicle *FindVehicleOfPlayer();\nvoid InitialisePoliceRadioZones();')
common=common.replace('value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);','value(m==&AudioManager);value(m->m_bIsInitialised);value(m->m_FrameCounter);\nfor(unsigned i=0;i<5;++i) {value(m->m_anRandomTable[i]);}')
common+='''
#include "AudioPoliceGame.h"
#include <stdarg.h>
using int16=int16_t;
#define Const const
#define nil NULL
#define debug(format,...) re3_debug("[DBG]: " format,__VA_ARGS__)
extern "C" void AudioPolice_SuspectReport(cAudioManager *);
static bool hasVehicle;
class CVehicle {
public:
    uint8 m_currentColour1;
    int16 model;
    int16 GetModelIndex() const {
        // Observe model access only after color validation
        value(21);snapshot(active);return model;
    }
};
static CVehicle vehicle;
CVehicle *cAudioManager::FindVehicleOfPlayer() {
    // Observe the original owner lookup even when the queue lacks report capacity
    value(22);snapshot(this);return hasVehicle?&vehicle:NULL;
}
extern "C" CVehicle *AudioGame_FindVehicle(void) {
    // Supply the same controlled lookup boundary while its actual C implementation is tested separately
    return active->FindVehicleOfPlayer();
}
struct FakeMusic {uint8 m_nMusicMode;};
static FakeMusic MusicManager;
extern "C" uint8_t AudioPoliceHost_MusicMode(void) {
    // Supply the same music byte read at the report gate
    return MusicManager.m_nMusicMode;
}
extern "C" void re3_debug(const char *format,...) {
    // Compare the exact original diagnostic prefix and promoted color argument
    for(const char *p=format;*p;++p) {value((uint8_t)*p);}value(0);
    va_list args;va_start(args,format);value(va_arg(args,int));va_end(args);snapshot(active);
}
'''
values='static const int32_t modelValues[]={'+','.join(names)+'};\n'
trace=(root/'utils/tests/audio_police_suspect.cpp.in').read_text()
method=function(old,'cAudioManager::','SetupSuspectLastSeenReport')
bridge='''
extern "C" void AudioPolice_SuspectReport(cAudioManager *m) {
    // Reach the original member through the same C ABI comparison caller
    m->SetupSuspectLastSeenReport();
}
'''
for variant in ('before','after'):
    ids=suspect.ENUMS if variant=='before' else '#include "ModelIds.h"\n'
    source=common+ids+values+(suspect.TABLE+'\n'+method+bridge if variant=='before' else entries.remove_find_bridge(suspect.HOST))+trace
    source=source.replace('@REPORT@','m->SetupSuspectLastSeenReport()' if variant=='before' else 'AudioPolice_SuspectReport(m)')
    (out/(variant+'.cpp')).write_text(source,newline='\n')
for name,flags in {'default':'#define AUDIO_OAL\n','vanilla':'#define AUDIO_OAL\n#undef FIX_BUGS\n','no-external':'#undef EXTERNAL_3D_SOUND\n','ps2':'#undef GTA_PC\n#undef AUDIO_REVERB\n#undef EXTERNAL_3D_SOUND\n#define GTA_PS2\n'}.items():
    (out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
(out/'caller.c').write_text('''#include "AudioManager.h"
#include "ModelIds.h"
#include <stdint.h>
static const int32_t values[]={'''+','.join(names)+'''};
int32_t TestModelIdsC(unsigned index) {
    // Return every shared model and count constant from a real C translation unit
    return values[index];
}
void TestSuspectReportC(void) {
    // Reach the actual report operation through the live owner from C
    AudioPolice_SuspectReport(&AudioManager);
}
''',newline='\n')
print('Complete color table/report, three adapters, model enum blocks, all caller/header edits audited; model constants:',len(names))
