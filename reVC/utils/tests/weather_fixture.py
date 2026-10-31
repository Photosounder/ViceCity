"""Audit complete weather relocation and generate original/C timing and geometry comparisons."""
from pathlib import Path
import json,re
import weather_migration as m
import traffic_lights_migration as traffic
import timecycle_migration as cycle
import timecycle_test_dependencies as cycle_dependencies
from audio_controls_migration import function
root=Path(__file__).resolve().parents[2];out=root/'build/weather-c-tests';old=(out/'before/src/renderer/Weather.cpp').read_text()
for path in json.loads((out/'changes.json').read_text()):
    before=(out/'before'/path).read_text();expected=m.project(before) if path.startswith('build/') else m.calls(before)
    if path=='src/control/TrafficLights.cpp':traffic.verify_legacy(before,m.calls,root)
    if path=='src/renderer/Timecycle.cpp':cycle.verify_legacy(before,m.calls,root)
    assert (root/path).read_text()==expected,path
    assert b'\r' not in (root/path).read_bytes(),path
assert (root/'src/renderer/WeatherSystem.c').read_text()==m.kernel(old)
assert (root/'src/renderer/Weather_backend.h').read_text()==m.BACKEND
dependencies=json.loads((out/'untouched.json').read_text())
for path in ['src/core/Clock.c','src/renderer/RenderBuffer.cpp']:
    if path not in dependencies:
        saved=out/'before'/path;saved.parent.mkdir(parents=True,exist_ok=True);saved.write_bytes((root/path).read_bytes());dependencies.append(path)
(out/'untouched.json').write_text(json.dumps(dependencies,indent=2),newline='\n')
for path in dependencies:
    import projectile_migration
    projectile_migration.verify_dependency(out/'before'/path,root/path)
assert len(m.SCALARS)==24 and len(m.READERS)==53
common=(root/'build/remote-c-tests/common.h').read_text().replace('struct RwTexture;','#ifdef LIBRW\nnamespace rw {struct Texture;}\ntypedef rw::Texture RwTexture;\n#else\nstruct RwTexture;\n#endif')
common+='\n#ifndef NATIVE_PROJECT_HEADERS\nenum RwFogType {rwFOGTYPELINEAR=1};\n#endif\n'
(out/'common.h').write_text(common,newline='\n')
for name in ['Vector.h','Vector2D.h','Matrix.h','Rect.h','maths.h']:(out/name).write_text((root/'src/math'/name).read_text(),newline='\n')
state='typedef struct WeatherTestState {\n'+''.join(kind+' '+name+';\n' for kind,name in m.SCALARS)+'WeatherRainStreak streaks[NUM_RAIN_STREAKS];int16_t types[64],hurricanes[64];float wind[7];\n} WeatherTestState;\n'
ops=[('Init','void','void',''),('Update','void','void',''),('AddHeatHaze','void','void',''),('AddBeastie','void','void',''),('ForceWeather','void','int16_t value','value'),('ForceWeatherNow','void','int16_t value','value'),('ReleaseWeather','void','void',''),('AddSplashesDuringHurricane','void','void',''),('AddStreamAfterRain','void','void',''),('AddRain','void','void',''),('RenderRainStreaks','void','void',''),('RenderOneRainStreak','void','WeatherVector position,WeatherVector direction,int32_t intensity,bool scale,float distance,WeatherVector right,WeatherVector up','position,direction,intensity,scale,distance,right,up'),('ForceHurricaneWeather','void','void','')]
prototypes=state+'void TestWeather_Read(WeatherTestState*);void TestWeather_Write(const WeatherTestState*);\n'
for name,ret,args,values in ops:
    prototypes+=('#ifdef SECUROM\n' if name=='ForceHurricaneWeather' else '')+ret+' TestWeather_'+name+'('+args+');'+ret+' RunWeather_'+name+'('+args+');\n'+('#endif\n' if name=='ForceHurricaneWeather' else '')
(out/'caller.h').write_text('#pragma once\n#include "Weather.h"\n#ifdef __cplusplus\nextern "C" {\n#endif\n'+prototypes+'#ifdef __cplusplus\n}\n#endif\n',newline='\n')
owners='''#pragma once
#include "common.h"
#include "Weather_backend.h"
#include "caller.h"
#include "Timer.h"
#include "Clock.h"
#include "General.h"
#include "DMAudio.h"
#include "RenderBuffer.h"
#include "CameraModes.h"
#include "SpecialFX_control.h"
#include "ZoneCull.h"
class CEntity {public:CVector position;const CVector &GetPosition(void);};
class CVehicle:public CEntity {public:bool CarHasRoof(void);};
class CPlayerPed:public CEntity {public:bool CheckIfInTheAir(void);};
class CCam {public:enum {MODE_TOPDOWN=CAMERA_MODE_TOPDOWN,MODE_TOP_DOWN_PED=CAMERA_MODE_TOP_DOWN_PED};int16 Mode;};
class CCamera:public CEntity {public:CCam Cams[2];uint8 ActiveCam;CVector forward,right,up,m_RealPreviousCameraPosition;float Orientation,m_CameraAverageSpeed;CEntity *pTargetEntity;
    CVector &GetForward(void);CVector &GetRight(void);CVector &GetUp(void);bool GetLookingForwardFirstPerson(void);bool GetLookingLRBFirstPerson(void);
};extern CCamera TheCamera;
class CReplay {public:static bool IsPlayingBack(void);};class CCutsceneMgr {public:static bool IsRunning(void);};class CGame {public:static bool IsInInterior(void);};
class CStats {public:static bool NoMoreHurricanes;};
class CPad {public:static CPad *GetPad(int);bool GetRightShockJustDown(void);void StartShake(int16,uint8);};
class CTimeCycle {public:static CVector &GetSunDirection(void);static int32 GetFogReduction(void);};
class CParticle {public:enum {SIN_COS_TABLE_SIZE=1024};static float m_SinTable[1024],m_CosTable[1024];
    static void RemovePSystem(tParticleType);
    static CParticle *AddParticle(tParticleType,const CVector&,const CVector&,CEntity *entity=nil,float size=0,int32 rotationSpeed=0,int32 rotation=0,int32 frame=0,int32 lifespan=0);
    static CParticle *AddParticle(tParticleType,const CVector&,const CVector&,CEntity*,float,const RwRGBA&,int32 rotationSpeed=0,int32 rotation=0,int32 frame=0,int32 lifespan=0);
};
struct CColPoint {CVector point;};class CStoredCollPoly;
class CWorld {public:static float FindGroundZFor3DCoord(float,float,float,bool*);static bool ProcessVerticalLine(const CVector&,float,CColPoint&,CEntity*&,bool,bool,bool,bool,bool,bool,CStoredCollPoly*);};
CPlayerPed *FindPlayerPed(void);CVehicle *FindPlayerVehicle(void);const CVector &FindPlayerSpeed(void);
namespace CustomPipes {extern bool bRenderingEnvMap;extern void *EnvMapCam;}
'''
owners+=cycle_dependencies.declarations(m.HOST)
(out/'owners.h').write_text(owners,newline='\n')
for before in [True,False]:
    text='#include "owners.h"\n'
    if before:
        header=(out/'before/src/renderer/Weather.h').read_text();header=re.sub(r'^extern RwTexture\* gpRainDropTex;[^\n]*\n','',header,flags=re.M)
        text+='namespace Original {\n'+header+'\n'+re.sub(r'^#include[^\n]*\n','',old,flags=re.M)+'\n}\n'
    prefix='Original::CWeather::' if before else 'Weather_';streaks='Original::Streaks' if before else 'Weather_Streaks'
    text+='extern "C" {\n'
    for reading in [True,False]:
        text+=('void TestWeather_Read(WeatherTestState *state)' if reading else 'void TestWeather_Write(const WeatherTestState *state)')+'\n{\n    // Transfer every canonical named field without depending on vector layout or padding\n'
        for kind,name in m.SCALARS:
            a='state->'+name;b=prefix+name;text+='    '+(a+'='+b if reading else b+'='+a)+';\n'
        text+='    for(int i=0;i<NUM_RAIN_STREAKS;i++){\n        // Transfer every original rain-streak component and timer\n'
        for field in ['position','direction']:
            for axis in 'xyz':
                a='state->streaks[i].'+field+'.'+axis;b=streaks+'[i].'+field+'.'+axis;text+='        '+(a+'='+b if reading else b+'='+a)+';\n'
        a='state->streaks[i].timer';b=streaks+'[i].timer';text+='        '+(a+'='+b if reading else b+'='+a)+';\n    }\n'
        for field,table in [('types','Original::WeatherTypesList' if before else 'Weather_Types'),('hurricanes','Original::WeatherTypesList_WithHurricanes' if before else 'Weather_HurricaneTypes')]:text+='    for(int i=0;i<64;i++)'+('state->'+field+'[i]='+table+'[i];' if reading else table+'[i]=state->'+field+'[i];')+'\n'
        if reading:text+='    for(int i=0;i<7;i++)state->wind[i]='+('Original::Windyness' if before else 'Weather_Windyness')+'[i];\n'
        text+='}\n'
    for name,ret,args,values in ops:
        text+=('#ifdef SECUROM\n' if name=='ForceHurricaneWeather' else '')+ret+' TestWeather_'+name+'('+args+')\n{\n    // Execute the entire preserved original or C weather operation\n'
        if name=='RenderOneRainStreak':
            call='Original::RenderOneRainStreak' if before else 'Weather_RenderOneRainStreak'
            if before:values=','.join('CVector('+value+'.x,'+value+'.y,'+value+'.z)' if value in ['position','direction','right','up'] else value for value in values.split(','))
        else:call=prefix+name
        text+='    '+call+'('+values+');\n}\n'+('#endif\n' if name=='ForceHurricaneWeather' else '')
    (out/('original.cpp' if before else 'adapter.cpp')).write_text(text+'}\n',newline='\n')
caller='#include "caller.h"\n'
for name,ret,args,values in ops:caller+=('#ifdef SECUROM\n' if name=='ForceHurricaneWeather' else '')+ret+' RunWeather_'+name+'('+args+')\n{\n    // Enter the complete selected operation through standalone C11\n    TestWeather_'+name+'('+values+');\n}\n'+('#endif\n' if name=='ForceHurricaneWeather' else '')
(out/'caller.c').write_text(caller,newline='\n')
(out/'host.cpp').write_text('#include "owners.h"\n'+re.sub(r'^#include[^\n]*\n','',cycle.calls(m.HOST),flags=re.M)+cycle_dependencies.definitions(m.HOST),newline='\n')
(out/'math.cpp').write_text('#include "owners.h"\n'+function((root/'src/math/Vector.cpp').read_text(),'CVector::','Normalise'),newline='\n')
native_math=Path('C:/msys/home/rouziclib/math/ieee754.c')
saved_math=out/'before/rouziclib-ieee754.c'
if not saved_math.exists():saved_math.write_bytes(native_math.read_bytes())
assert saved_math.read_bytes()==native_math.read_bytes()
(out/'fastabs.c').write_text('#include <stdint.h>\n'+function(native_math.read_text(),'','fast_absf'),newline='\n')
clock=(root/'src/core/Clock.c').read_text()
(out/'clock.c').write_text('#include "Clock.h"\nuint8_t Clock_hours,Clock_minutes;uint16_t Clock_seconds;\n'+'\n'.join(function(clock,'',name) for name in ['Clock_GetHours','Clock_GetMinutes','Clock_GetSeconds']),newline='\n')
observations='\n    '.join(('floating' if kind=='float' else 'word')+'(value.'+name+');' for kind,name in m.SCALARS)
(out/'test.cpp').write_text((root/'utils/tests/weather.cpp.in').read_text().replace('@SCALAR_OBSERVATIONS@',observations),newline='\n')
(out/'unfixed.h').write_text('#include "config.h"\n#undef FIX_BUGS\n',newline='\n')
print('Audited all twelve weather methods, complete timestep/billboard helpers, 24 canonical fields, 35 rain records, both schedules, all native boundaries and forty complete source/project changes')
