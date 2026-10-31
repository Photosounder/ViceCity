"""Compare the complete former static clock class against the real C11 implementation."""
from pathlib import Path
import json,re
import clock_migration as m
import weather_migration as weather
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2];out=root/'build/clock-c-tests'

def normalize(s):
    # Compare every statement independently of edit markers and formatting
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',s,flags=re.S))

paths=json.loads((out/'changes.json').read_text())
for name in paths:
    # Audit every complete caller and project file against its immutable original snapshot
    old=(out/'before'/name).read_text()
    if name=='src/core/Clock.cpp':new=m.timer.calls(m.HOST)
    elif name=='src/core/Clock.h':new=m.header(old)
    elif name.startswith('build/'):new=m.project(old)
    else:new=m.calls(old)
    if name=='src/renderer/Weather.cpp':weather.verify_legacy(old,m.calls,root)
    if name=='src/renderer/Timecycle.cpp':cycle.verify_legacy(old,m.calls,root)
    assert normalize(new)==normalize((root/name).read_text()),name
old=(out/'before/src/core/Clock.cpp').read_text();oldh=(out/'before/src/core/Clock.h').read_text()
assert normalize(m.kernel(old))==normalize((root/'src/core/Clock.c').read_text())
for field,(kind,_) in m.FIELDS.items():
    # Require each original static field width and both reference-backed accessor targets
    before_kind={'uint8_t':'uint8','uint16_t':'uint16','uint32_t':'uint32','bool':'bool'}[kind]
    assert re.search(r'static\s+'+before_kind+r'\s+'+field+r';',oldh)
assert 'uint8 &GetHoursRef(void) { return ms_nGameClockHours; }' in oldh
assert 'uint8 &GetMinutesRef(void) { return ms_nGameClockMinutes; }' in oldh
assert 'int16 GetSeconds(void) { return ms_nGameClockSeconds; }' in oldh
assert 'bool GetRightShoulder1(void) { return !!NewState.RightShoulder1; }' in (root/'src/core/Pad.h').read_text()
assert 'static const uint32 &GetTimeInMilliseconds(void) { return m_snTimeInMilliseconds; }' in (root/'build/timer-c-tests/before/src/core/Timer.h').read_text()
template=(root/'utils/tests/clock.cpp.in').read_text()
saved=(out/'before/src/save/GenericGameStorage.cpp').read_text()
card=(out/'before/src/save/MemoryCard.cpp').read_text()
macros='\n'.join(l for l in saved.splitlines() if l.startswith('#define ReadDataFromBufferPointer(') or l.startswith('#define WriteDataToBufferPointer('))
assert all(l in card for l in macros.splitlines())
alignment=m.function(card,'','align4bytes')
write='\n'.join(l.strip() for l in saved.splitlines() if 'WriteDataToBufferPointer(buf, CClock::' in l)
read='\n'.join(l.strip() for l in saved.splitlines() if 'ReadDataFromBufferPointer(buf, CClock::' in l)
assert len(write.splitlines())==4 and len(read.splitlines())==4
for variant in ['before','after']:
    # Reuse the preserved complete class, definitions and inline accessors in the baseline
    source=template.replace('@HEADER@',oldh if variant=='before' else '#include "Clock.h"')
    source=source.replace('@SAVE_MACROS@',macros+'\n'+alignment).replace('@SAVE_WRITE@',write if variant=='before' else m.calls(write)).replace('@SAVE_READ@',read if variant=='before' else m.calls(read))
    body=old if variant=='before' else m.HOST
    source=source.replace('@BODY@',re.sub(r'(?m)^#include[^\n]*\n','',body))
    for field,(_,target) in m.FIELDS.items():source=source.replace('@'+field+'@','CClock::'+field if variant=='before' else target)
    if variant=='before':
        run='''switch(op){
case 0:CClock::Initialise(scale);return 0;
case 1:CClock::Update();return 0;
case 2:CClock::SetGameClock(h,minute);return 0;
case 3:return CClock::GetGameClockMinutesUntil(h,minute);
case 4:return CClock::GetIsTimeInRange(h,minute);
case 5:CClock::StoreClock();return 0;
case 6:CClock::RestoreClock();return 0;
case 7:return CClock::GetHours();
case 8:return CClock::GetMinutes();
case 9:return CClock::GetSeconds();
case 10:{uint8_t &value=CClock::GetHoursRef();assert(&value==&CClock::ms_nGameClockHours);value=h;return value;}
default:{uint8_t &value=CClock::GetMinutesRef();assert(&value==&CClock::ms_nGameClockMinutes);value=minute;return value;}
}'''
    else:run='return TestClockC(op,h,minute,scale);'
    source=source.replace('@RUN@',run)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
(out/'caller.c').write_text('''#include "Clock.h"
#include <assert.h>
_Static_assert(sizeof(bool)==1,"Original clock boolean storage width");
uint32_t TestClockC(unsigned op,uint8_t h,uint8_t minute,uint32_t scale)
{
    // Exercise complete operations and writable pointer aliases through their actual C declarations
    switch(op){
    case 0:Clock_Initialise(scale);return 0;
    case 1:Clock_Update();return 0;
    case 2:Clock_SetGameClock(h,minute);return 0;
    case 3:return Clock_GetGameClockMinutesUntil(h,minute);
    case 4:return Clock_GetIsTimeInRange(h,minute);
    case 5:Clock_StoreClock();return 0;
    case 6:Clock_RestoreClock();return 0;
    case 7:return Clock_GetHours();
    case 8:return Clock_GetMinutes();
    case 9:return Clock_GetSeconds();
    case 10:{uint8_t *value=Clock_GetHoursRef();assert(value==&Clock_hours);*value=h;return *value;}
    default:{uint8_t *value=Clock_GetMinutesRef();assert(value==&Clock_minutes);*value=minute;return *value;}
    }
}
''',newline='\n')
for name,flags in {'default':'','master':'#define MASTER\n','vanilla':'#undef FIX_BUGS\n','ps2':'#undef GTA_PC\n#define GTA_PS2\n'}.items():(out/(name+'.h')).write_text('#include "config.h"\n'+flags,newline='\n')
print('Complete C clock, seven former methods, five scalar/pointer accessors, nine field widths, live timer/button reads and all 32 source/project edits audited')
