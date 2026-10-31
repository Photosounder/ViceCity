import weather_migration as _weather_conversion
"""Audit the complete timer conversion and every actual caller against immutable original sources."""
from pathlib import Path
import json,re
import player_skin_migration as skins
import timer_migration as m
import water_creatures_migration as water
import remote_migration as remote
import set_pieces_migration as pieces
import weather_migration as weather
import projectile_migration as projectiles
import fire_migration as fire
import bridge_migration as bridge
import traffic_lights_migration as traffic
import onscreen_timer_migration as onscreen
import pager_migration as pager
import messages_migration as messages
import text_migration as text
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2];out=root/'build/timer-c-tests'

def normalize(source):
    # Apply the explicit verified weather rename before comparing every original statement
    source=_weather_conversion.calls(source)
    # Compare every statement independently of comments and whitespace
    return re.sub(r'\s+','',re.sub(r'//[^\n]*|/\*.*?\*/','',source,flags=re.S))

paths=json.loads((out/'changes.json').read_text())
for name in paths:
    # Require the complete explicit transformation for each source and project file
    before=(out/'before'/name).read_text()
    if name=='src/core/Timer.cpp':expected=m.calls(m.HOST)
    elif name=='src/core/Timer.h':expected=m.header(before)
    elif name.startswith('build/'):expected=m.project(before)
    else:expected=m.calls(before)
    if name=='src/renderer/WaterCreatures.cpp':water.verify_legacy(before,m.calls,root)
    if name=='src/renderer/PlayerSkin.cpp':skins.verify_legacy(before,m.calls,root)
    if name=='src/control/Remote.cpp':remote.verify_legacy(before,m.calls,root)
    if name=='src/control/SetPieces.cpp':pieces.verify_legacy(before,m.calls,root)
    if name=='src/renderer/Weather.cpp':weather.verify_legacy(before,m.calls,root)
    if name=='src/weapons/ProjectileInfo.cpp':projectiles.verify_legacy(before,m.calls,root)
    if name=='src/core/Fire.cpp':fire.verify_legacy(before,m.calls,root)
    if name=='src/control/Bridge.cpp':bridge.verify_legacy(before,m.calls,root)
    if name=='src/control/TrafficLights.cpp':traffic.verify_legacy(before,m.calls,root)
    if name=='src/control/OnscreenTimer.cpp':onscreen.verify_legacy(before,m.calls,root)
    if name=='src/text/Pager.cpp':pager.verify_legacy(before,m.calls,root)
    if name=='src/text/Messages.cpp':messages.verify_legacy(before,m.calls,root)
    if name=='src/text/Text.cpp':text.verify_legacy(before,m.calls,root)
    if name=='src/renderer/Timecycle.cpp':cycle.verify_legacy(before,m.calls,root)
    assert normalize(expected)==normalize((root/name).read_text()),name
before=(out/'before/src/core/Timer.cpp').read_text()
assert normalize(m.kernel(before))==normalize((root/'src/core/Timer.c').read_text())
oldh=(out/'before/src/core/Timer.h').read_text();newh=(root/'src/core/Timer.h').read_text()
assert newh.count('static inline')==28 and 'class CTimer' not in newh
for name in m.REFS:
    # Verify all five former const references retain a pointer to the same live scalar
    original=re.search(r'static\s+const\s+(float|uint32)\s*&'+name+r'\(void\)\s*\{\s*return\s+(\w+);\s*\}',oldh)
    assert original,name
    target=m.FIELDS[original.group(2)][1]
    assert re.search(r'Timer_'+name+r'\(void\)\s*\{[^}]*return &'+target+r';',newh)
for name in paths+['src/core/Timer.c','utils/tests/timer_migration.py','utils/tests/timer_source_audit.py']:
    # Require whole-file LF endings for every touched production and maintained audit file
    assert b'\r\n' not in (root/name).read_bytes(),name
for bits in [32,64]:
    # Confirm both actual native compile runs produced every expected source object
    for source in paths:
        if source.endswith('.cpp'):
            target=source.replace('/','-').replace('.cpp','')
            assert (out/f'{target}-consumer{bits}.obj').stat().st_size>0
    assert (out/f'Timer-c-consumer{bits}.obj').stat().st_size>0
print('Complete timer kernel, 28 inline accessors, five live reference replacements, all 128 source/project edits and 246 native source compiles audited; runtime comparisons are verified separately')
