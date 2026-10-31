import weather_migration as _weather_conversion
"""Audit complete zone migration and generate original/C behavior comparisons."""
from pathlib import Path
import json,re
import cull_zones_migration as m
import weather_migration as weather
root=Path(__file__).resolve().parents[2];out=root/'build/cull-zones-c-tests'
old=(out/'before/src/core/ZoneCull.cpp').read_text();oldh=(out/'before/src/core/ZoneCull.h').read_text()

def normalize(source):
    # Apply the explicit verified weather rename before comparing every original statement
    source=_weather_conversion.calls(source)
    # Compare every statement independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

for name in json.loads((out/'changes.json').read_text()):
    # Require the complete explicit transformation for all affected source and project files
    before=(out/'before'/name).read_text();expected=m.project(before) if name.startswith('build/') else m.calls(before)
    if name=='src/renderer/Weather.cpp':weather.verify_legacy(before,m.calls,root)
    assert normalize(expected)==normalize((root/name).read_text()),name
assert normalize(m.kernel(old))==normalize((root/'src/core/ZoneCull.c').read_text())
assert 'TRACK_ELTRAIN,' in (root/'src/vehicles/Train.h').read_text() and (root/'src/vehicles/Train.h').read_text().index('TRACK_ELTRAIN,')<(root/'src/vehicles/Train.h').read_text().index('TRACK_SUBWAY')
assert 'class CCullZones' not in (root/'src/core/ZoneCull.h').read_text()
assert (root/'src/core/ZoneCull.h').read_text().count('static inline')==12
(out/'Vector.h').write_text('#pragma once\n',newline='\n')
getters=re.findall(r'static (bool|int32) (\w+)\(void\) \{([^}]+)\}',oldh)
assert len(getters)==11
template=(root/'utils/tests/cull_zones.cpp.in').read_text()
for variant in ['before','after']:
    # Compile the complete original class and definitions rather than a reconstructed algorithm
    before=variant=='before';prefix='CCullZones::' if before else 'CullZones_'
    headers=oldh if before else '#include "ZoneCull.h"\n'
    implementation=re.sub(r'(?m)^#include[^\n]*\n','',old) if before else m.HOST[m.HOST.index('extern "C"'):]
    if before:
        headers+='\n#include "Timer.h"\n#define nil NULL\n#define Max(a,b) ((a) > (b) ? (a) : (b))\n'
    position='CVector' if before else 'CullZonesPosition'
    fields={name:('CCullZones::'+name if before else target) for name,(_,target) in m.FIELDS.items()}
    zone=fields['aAttributeZones'];count=fields['NumAttributeZones'];wanted=fields['CurrentWantedLevelDrop_Player']
    dispatch='static void dispatch(unsigned op)\n{\n    // Exercise original signatures and C value arguments with identical captured inputs\n    '+position+' coors={float(int(seed%25)-12),0,0};\n    int32_t wanted=-777;\n    switch(op){\n'
    calls={0:prefix+'Init();',1:'if('+count+'<NUMATTRIBZONES)'+prefix+'AddCullZone('+('player' if before else '&player')+',-9.75f,10.75f,-12.5f,15.5f,-8.25f,8.25f,(uint16_t)(seed*257),(int16_t)(seed%201));',
      2:'word('+prefix+'FindAttributesForCoors(coors,&wanted));word(wanted);',3:'word('+prefix+'FindAttributesForCoors(coors,NULL));',4:'word('+prefix+'FindAttributesForCoors(coors,&'+wanted+'));',
      5:'{CAttributeZone *result='+prefix+'FindZoneWithStairsAttributeForPlayer();word(result?(uint32_t)(result-'+zone+')+1:0);if(result)result->wantedLevel=(int16_t)(seed%101);}',
      6:prefix+'MarkSubwayAsInvisible(true);',7:prefix+'MarkSubwayAsInvisible(false);',8:prefix+'UpdateAtBeachForAudio();',9:prefix+'Update();',10:prefix+'ForceCullZoneCoors(coors);',
      22:'word(IsPointWithinArbitraryArea(coors.x,coors.y,-2,-3,4,-3,4,5,-2,5));',23:prefix+'Update();'}
    calls.update({11+i:'word('+prefix+name+'());' for i,(_,name,_) in enumerate(getters)})
    for i,body in calls.items():dispatch+='    case '+str(i)+':'+body+'break;\n'
    dispatch+='    }\n}\n'
    if not before:
        # Compile all migrated operations and accessors through an actual C11 caller
        caller=dispatch.replace('static void dispatch(unsigned op)','void TestCullDispatch(unsigned op, unsigned seed)')
        caller=caller.replace('float(int(seed%25)-12)','(float)((int)(seed%25)-12)').replace('&player','NULL')
        (out/'caller.c').write_text('#include "ZoneCull.h"\nvoid TestCullWord(uint32_t value);\n#define word TestCullWord\n'+caller,newline='\n')
        dispatch='extern "C" void TestCullDispatch(unsigned op, unsigned seed);\nstatic void dispatch(unsigned op)\n{\n    // Exercise the migrated operations through the actual C11 caller\n    TestCullDispatch(op, seed);\n}\n'
    checks='''mutate=0;setup();
    // Check partial initialization, record widths, exact edges and a writable stairs alias
    static_assert(sizeof(CAttributeZone)==16 && alignof(CAttributeZone)==2,"Zone record ABI");
    @P@Init();assert(COUNT==0 && WANTED==0 && CAMERA_FLAGS==0 && PLAYER_FLAGS==0 && !SUBWAY);assert(BEACH==false);assert(ZONES[0].minx==-10);
    BEACH=true;@P@Init();assert(BEACH);
    @P@AddCullZone(@ARG@,-1.75f,1.75f,-2.75f,2.75f,-3.75f,3.75f,ATTRZONE_STAIRS|ATTRZONE_NORAIN,7);
    @P@AddCullZone(@ARG@,-1,1,-2,2,-3,3,ATTRZONE_SUBWAYVISIBLE,11);
    int32_t level=-1;@POS@ edge={1,2,3};assert(@P@FindAttributesForCoors(edge,&level)==(ATTRZONE_STAIRS|ATTRZONE_NORAIN|ATTRZONE_SUBWAYVISIBLE) && level==11);
    edge.x=1.01f;assert(@P@FindAttributesForCoors(edge,&level)==0 && level==0);
    player=CVector(0,0,0);assert(@P@FindZoneWithStairsAttributeForPlayer()==&ZONES[0]);
    // Retain the original unordered comparisons for NaN coordinates and degenerate polygons
    uint32_t nanBits=0x7fc01234;float nan;memcpy(&nan,&nanBits,sizeof nan);edge.x=nan;
    assert(@P@FindAttributesForCoors(edge,&level)==0 && level==0);
    assert(IsPointWithinArbitraryArea(nan,0,0,0,0,0,0,0,0,0));
    assert(IsPointWithinArbitraryArea(1,2,0,0,0,0,0,0,0,0));

    for(unsigned mask=0;mask<2048;mask++){
        // Check every flag combination against the original bit masks
        CAMERA_FLAGS=PLAYER_FLAGS=(int32_t)mask;
        @FLAG_CHECKS@
    }
    setup();@P@MarkSubwayAsInvisible(false);
    for(unsigned i=0;i<5;i++){
        // Check eligible entities while retaining holes and elevated trains
        if(!(buildingFlags[i]&POOLFLAG_ISFREE) && buildings[i].bIsSubway)assert(!buildings[i].bIsVisible);
        if(!(treadableFlags[i]&POOLFLAG_ISFREE) && treadables[i].bIsSubway)assert(!treadables[i].bIsVisible);
        if(!(vehicleFlags[i]&POOLFLAG_ISFREE) && vehicles[i].train && vehicles[i].m_nTrackId!=TRACK_ELTRAIN)assert(!vehicles[i].bIsVisible);
    }
    '''
    flagchecks=''
    for kind,name,body in getters:
        expression=body.strip().removeprefix('return ').removesuffix(';')
        for field,target in fields.items():expression=re.sub(r'\b'+field+r'\b',target,expression)
        flagchecks+='assert('+prefix+name+'()==('+expression+'));\n'
    checks=checks.replace('@FLAG_CHECKS@',flagchecks).replace('@P@',prefix).replace('@POS@',position).replace('@ARG@','player' if before else '&player')
    source=template.replace('@HEADERS@',headers).replace('@IMPLEMENTATION@',implementation).replace('@DISPATCH@',dispatch).replace('@INDEPENDENT@',checks)
    for tag,field in [('COUNT','NumAttributeZones'),('ZONES','aAttributeZones'),('WANTED','CurrentWantedLevelDrop_Player'),('CAMERA_FLAGS','CurrentFlags_Camera'),('PLAYER_FLAGS','CurrentFlags_Player'),('SUBWAY','bCurrentSubwayIsInvisible'),('BEACH','bAtBeachForAudio')]:
        source=source.replace('@'+tag+'@',fields[field])
    (out/(variant+'.cpp')).write_text(source,newline='\n')
print('Complete C kernel, 11 flag accessors, 28 source/project edits and 16-byte zone record audited')
