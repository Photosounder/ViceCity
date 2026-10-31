"""Audit complete zone conversion and generate exact original/C runtime models."""
from pathlib import Path
import json,re
import zones_migration as m
import restart_migration as restarts
import user_display_migration as user_display
root=Path(__file__).resolve().parents[2];out=root/'build/zones-c-tests'
old=(out/'before/src/core/Zones.cpp').read_text();oldh=(out/'before/src/core/Zones.h').read_text()

def normalize(source):
    # Require identical complete statements apart from comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for name in json.loads((out/'changes.json').read_text()):
    # Audit every complete caller and project transformation against its immutable original
    before=(out/'before'/name).read_text();expected=m.project(before) if name.startswith('build/') else m.calls(before)
    if name=='src/control/Restart.cpp':restarts.verify_legacy(before,m.calls,root)
    if name=='src/core/User.cpp':user_display.verify_legacy(before,m.calls,root)
    assert normalize(expected)==normalize((root/name).read_text()),name
assert normalize(m.kernel(old))==normalize((root/'src/core/Zones.c').read_text())
for path,generate,saved in [('src/core/ZoneRecords.h',m.records,'src/core/Zones.h'),('src/core/GameLevels.h',m.level_enum,'src/core/Game.h'),('src/peds/GangTypes.h',m.gang_enum,'src/peds/Gangs.h'),('src/control/CarClasses.h',m.car_enum,'src/control/CarCtrl.h')]:
    # Audit every field, enum value and relative constant expression independently of compilation
    assert normalize(generate((out/'before'/saved).read_text()))==normalize((root/path).read_text()),path
assert 'typedef uint16_t wchar;' in (root/'src/core/common.h').read_text()
assert 'typedef ptrdiff_t ssize_t;' in (root/'src/core/common.h').read_text()
assert (root/'src/core/Zones.h').read_text().count('static inline')==7
assert not any(name in (root/'src/core/Zones.h').read_text() for name in ['class CTheZones','class CZone','class CZoneInfo'])
(out/'Vector.h').write_text('#pragma once\n',newline='\n')

def function(source,name):
    # Extract the complete already-audited clock range operation for both model boundaries
    a=source.index(name+'(');b=source.index('{',a);start=source.rfind('\n',0,a-1)+1
    end=b+1;depth=1
    while depth:depth+=(source[end]=='{')-(source[end]=='}');end+=1
    return source[start:end]

clock=function((root/'src/core/Clock.c').read_text(),'Clock_GetIsTimeInRange')
clock=clock.replace('Clock_GetIsTimeInRange(', 'TestZonesClockRange(')
clock+='\n'+function((root/'src/core/Clock.c').read_text(),'Clock_GetHours')
clock+='''
extern "C" bool Clock_GetIsTimeInRange(uint8_t start,uint8_t end)
{
    // Observe the original range result without mutating the pure clock operation
    bool result=TestZonesClockRange(start,end);++callbacks;word(3);word(start);word(end);word(result);return result;
}
'''
template=(root/'utils/tests/zones.cpp.in').read_text()
for variant in ['before','after']:
    # Compile the exact original class, accessors and complete method definitions as the baseline
    before=variant=='before';prefix='CTheZones::' if before else 'Zones_'
    if before:
        levels=m.enum_body((out/'before/src/core/Game.h').read_text(),'enum eLevelName {')
        gangs=m.enum_body((out/'before/src/peds/Gangs.h').read_text(),'enum {\n\tGANG_CUBAN')
        car='struct CCarCtrl { '+m.enum_body((out/'before/src/control/CarCtrl.h').read_text(),'enum eCarClass {')+' };\n'
        headers=levels+'\n'+gangs+'\n'+car+re.sub(r'(?m)^#(?:include[^\n]*|pragma once)\n','',oldh).replace('class CTheZones\n{','class CTheZones\n{\npublic:')
        implementation=re.sub(r'(?m)^#include[^\n]*\n','',old)
    else:
        headers='#include "Zones.h"\n';host=m.calls(m.HOST);implementation=host[host.index('extern "C"'):]
    headers+='\n#include "Clock.h"\n#include "Timer.h"\n'
    fields={name:'CTheZones::'+name if before else target for name,target in m.FIELDS.items()}
    nav=fields['NavigationZoneArray'];info=fields['InfoZoneArray'];data=fields['ZoneInfoArray'];audioCount=fields['NumberOfAudioZones']
    coordinate='&pos' if before else 'pos';kind='CVector' if before else 'ZonesPosition'
    dispatch='''static void dispatch(unsigned op)
{
    // Exercise all original signatures and the corresponding C value arguments
    @KIND@ pos={float(int(seed%401)-200),0,0};
    CZoneInfo result={};CZone local={};uint8_t *cursor;uint32_t length,size;
    int16_t cars[9]={1,2,3,4,5,6,7,8,9},boats[2]={11,22},gangs[9]={4,5,6,7,8,9,10,11,12};
    char label[]="DENSITY",create[]="testzone";
    switch(op){
'''.replace('@KIND@',kind)
    calls={0:prefix+'Init();',1:prefix+'Update();',
      2:'if('+fields['TotalNumberOfInfoZones']+'<NUMINFOZONES)'+prefix+'CreateZone(create,ZONE_INFO,50,-40,-10,-50,40,10,LEVEL_GENERIC);for(unsigned i=0;i<9;i++)word((uint8_t)create[i]);',
      3:'TestZonesClearLinks();'+prefix+'PostZoneCreation();',4:prefix+'CheckZonesForOverlap();',
      5:'TestZonesClearLinks();'+prefix+'InsertZoneIntoZoneHierarchy(&'+nav+'[1]);',
      6:'TestZonesClearLinks();word('+prefix+'InsertZoneIntoZoneHierRecursive(&'+nav+'[1],&'+nav+'[0]));word('+prefix+'InsertZoneIntoZoneHierRecursive(&local,NULL));',
      7:'word('+prefix+'ZoneIsEntirelyContainedWithinOtherZone(&'+nav+'[1],&'+nav+'[0]));',
      8:'word('+prefix+'PointLiesWithinZone('+coordinate+',&'+nav+'[0]));',
      9:'word('+prefix+'GetLevelFromPosition('+coordinate+'));',
      10:'TestZonesWord(TestZonesZoneId('+prefix+'FindInformationZoneForPosition('+coordinate+')));',
      11:'TestZonesWord(TestZonesZoneId('+prefix+'FindSmallestNavigationZoneForPosition('+coordinate+', (seed&1)!=0, (seed&2)!=0)));',
      12:'word((uint16_t)'+prefix+'FindZoneByLabelAndReturnIndex(label,ZONE_INFO));',
      13:'word((uint16_t)'+prefix+'FindNextZoneByLabelAndReturnIndex(label,ZONE_INFO));',
      14:'TestZonesInfo('+prefix+'GetZoneInfo('+coordinate+', (uint8_t)(seed%3)));',
      15:'{CZoneInfo *destination=seed%3==0?'+prefix+'GetZoneInfo('+coordinate+',1):seed%3==1?'+prefix+'GetZoneInfo('+coordinate+',0):&result;'+prefix+'GetZoneInfoForTimeOfDay('+coordinate+',destination);TestZonesInfo(destination);}',
      16:prefix+'SetZoneCarInfo(0,(uint8_t)(seed%3),(int16_t)(seed*17),31,gangs);',
      17:prefix+'SetZoneCivilianCarInfo(0,(uint8_t)(seed%3),cars,boats);',
      18:prefix+'SetZonePedInfo(0,(uint8_t)(seed%3),(int16_t)(seed*17),1,2,3,4,5,6,7,8,9,23);',
      19:prefix+'SetCarDensity(0,(uint8_t)(seed%3),(uint16_t)(seed*200));',
      20:prefix+'SetPedDensity(0,(uint8_t)(seed%3),(uint16_t)(seed*200));',
      21:prefix+'SetPedGroup(0,(uint8_t)(seed%3),(uint16_t)(seed*257));',
      22:'word((uint16_t)'+prefix+'FindAudioZone('+coordinate+'));',
      23:audioCount+'=0;'+prefix+'AddZoneToAudioZoneArray(&'+nav+'[0]);',
      24:audioCount+'=0;'+prefix+'InitialiseAudioZoneArray();',
      25:prefix+'SaveAllZones(saveBuffer,&size);word(size);TestZonesBytes(saveBuffer,size);',
      26:'cursor=saveBuffer;length=0;'+prefix+'SaveOneZone(&'+nav+'[0],&cursor,&length,ZONE_NAVIG);word(length);word((uint32_t)(cursor-saveBuffer));TestZonesBytes(saveBuffer,length);',
      27:prefix+'SaveAllZones(saveBuffer,&size);'+fields['m_CurrLevel']+'=LEVEL_MAINLAND;'+prefix+'LoadAllZones(saveBuffer,size);',
      28:'cursor=saveBuffer;length=0;'+prefix+'SaveOneZone(&'+nav+'[0],&cursor,&length,ZONE_NAVIG);cursor=saveBuffer;length=0;'+prefix+'LoadOneZone(&local,&cursor,&length,ZONE_NAVIG);word(length);TestZonesZone(&local);',
      29:'word(TestZonesZoneId('+prefix+'GetInfoZone(0)));',30:'word(TestZonesZoneId('+prefix+'GetNavigationZone(0)));',31:'word(TestZonesZoneId('+prefix+'GetMapZone(0)));',
      32:'if('+audioCount+')word(TestZonesZoneId('+prefix+'GetAudioZone(0)));',
      33:'word(TestZonesZoneId('+prefix+'GetPointerForNavigationZoneIndex((seed&1)?-1:0)));',
      34:'word((uint32_t)'+prefix+'GetIndexForNavigationZonePointer((seed&1)?NULL:&'+nav+'[0]));',
      35:'word('+('('+nav+'[0].GetTranslatedName()' if before else '(Zone_GetTranslatedName(&'+nav+'[0])')+'==textBuffer));word(textBuffer[0]);'}
    for i,body in calls.items():dispatch+='    case '+str(i)+':'+body+'break;\n'
    dispatch+='    }\n}\n'
    if not before:
        # Drive all 36 operations through an actual C11 caller rather than C++ wrappers
        caller=dispatch.replace('static void dispatch(unsigned op)','void TestZonesDispatch(unsigned op, unsigned seed)')
        caller=caller.replace('float(int(seed%401)-200)','(float)((int)(seed%401)-200)').replace('saveBuffer','TestZonesSaveBuffer').replace('={};','={0};')
        pre='''#include "Zones.h"
extern uint8_t TestZonesSaveBuffer[65536];
void TestZonesWord(uint32_t value);
void TestZonesClearLinks(void);
void TestZonesBytes(const uint8_t *bytes,uint32_t size);
void TestZonesInfo(const CZoneInfo *info);
void TestZonesZone(const CZone *zone);
uint32_t TestZonesZoneId(const CZone *zone);
extern uint16_t TestZonesTextBuffer[8];
#define word TestZonesWord
#define textBuffer TestZonesTextBuffer
'''
        (out/'caller.c').write_text(pre+caller,newline='\n')
        dispatch='extern "C" void TestZonesDispatch(unsigned op, unsigned seed);\nstatic void dispatch(unsigned op)\n{\n    // Enter every migrated operation through the actual C11 caller\n    TestZonesDispatch(op,seed);\n}\n'
    checks='''
    // Verify record layouts, fixed save size, hierarchy selection and duplicate-label continuation
    static_assert(sizeof(CZoneInfo)==68 && alignof(CZoneInfo)==2,"Zone info ABI");
    static_assert(sizeof(CZone)==(sizeof(void*)==8?72:56),"Zone ABI");
    static_assert(sizeof(eZoneType)==4 && sizeof(eLevelName)==4,"Saved enum widths");
    uint32_t size;@P@SaveAllZones(saveBuffer,&size);assert(size==35808);assert(saveBuffer[0]=='Z' && saveBuffer[1]=='N' && saveBuffer[2]=='S');
    assert(@P@GetPointerForNavigationZoneIndex(-1)==NULL);assert(@P@GetIndexForNavigationZonePointer(NULL)==-1);
    char label[]="DENSITY";assert(@P@FindZoneByLabelAndReturnIndex(label,ZONE_INFO)==1);assert(@P@FindNextZoneByLabelAndReturnIndex(label,ZONE_INFO)==2);assert(@P@FindNextZoneByLabelAndReturnIndex(label,ZONE_INFO)==-1);
    @KIND@ edge={100,100,10};assert(@P@PointLiesWithinZone(@COORD@,&NAV[1]));edge.x=100.01f;assert(!@P@PointLiesWithinZone(@COORD@,&NAV[1]));
    edge={0,0,0};assert(@P@FindSmallestNavigationZoneForPosition(@COORD@,false,true)==&NAV[1]);assert(@P@FindSmallestNavigationZoneForPosition(@COORD@,true,false)==&NAV[2]);
    uint8_t original[65536];memcpy(original,saveBuffer,size);@P@LoadAllZones(saveBuffer,size);uint32_t afterSize;@P@SaveAllZones(saveBuffer,&afterSize);assert(afterSize==size && !memcmp(original,saveBuffer,size));
    '''.replace('@P@',prefix).replace('@KIND@',kind).replace('@COORD@','&edge' if before else 'edge')
    source=template.replace('@HEADERS@',headers).replace('@CLOCK@',clock).replace('@IMPLEMENTATION@',implementation).replace('@DISPATCH@',dispatch).replace('@INDEPENDENT@',checks).replace('@P@',prefix)
    for tag,field in [('NAV','NavigationZoneArray'),('INFO','InfoZoneArray'),('MAP','MapZoneArray'),('DATA','ZoneInfoArray'),('NAV_COUNT','TotalNumberOfNavigationZones'),('INFO_COUNT','TotalNumberOfInfoZones'),('MAP_COUNT','TotalNumberOfMapZones'),('DATA_COUNT','TotalNumberOfZoneInfos'),('AUDIO_COUNT','NumberOfAudioZones'),('AUDIO','AudioZoneArray'),('LEVEL','m_CurrLevel'),('FIND','FindIndex')]:source=source.replace('@'+tag+'@',fields[field])
    source=source.replace('static uint8_t saveBuffer[65536];','extern "C" uint8_t TestZonesSaveBuffer[65536];\nuint8_t TestZonesSaveBuffer[65536];\n#define saveBuffer TestZonesSaveBuffer')
    source=source.replace('static uint16_t textBuffer[8];','extern "C" uint16_t TestZonesTextBuffer[8];\nuint16_t TestZonesTextBuffer[8];\n#define textBuffer TestZonesTextBuffer')
    source=source.replace('static Text TheText;','''static Text TheText;
extern "C" uint16_t *Text_Get(Text *self,const char *name)
{
    // Retain the same observed translation dependency through the actual zone adapter's C call
    return self->Get(name);
}
''')
    source=source.replace('static uint32_t zoneId(const CZone *zone)','extern "C" uint32_t TestZonesZoneId(const CZone *zone)').replace('zoneId(','TestZonesZoneId(')
    helper='''extern "C" void TestZonesBytes(const uint8_t *bytes,uint32_t size)
{
    // Observe the exact save bytes independently of buffer addresses
    for(uint32_t i=0;i<size;i++)word(bytes[i]);
}
'''
    source=source.replace('static void setup()',helper+'\nstatic void setup()',1)
    (out/(variant+'.cpp')).write_text(source,newline='\n')
print('All 29 complete zone operations, six pointer/index accessors, both C records, shared enums, text lookup and 31 source/project changes audited')
