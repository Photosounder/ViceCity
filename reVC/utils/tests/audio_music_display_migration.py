"""Move the complete station-name display control flow and shared state into C11."""
import re
import timer_migration as timer
import audio_music_cleanup_migration as cleanup
from audio_controls_migration import function

READERS={
    'Paused':('uint8_t','void','CTimer::GetIsPaused()'),
    'Widescreen':('uint8_t','void','TheCamera.m_WideScreenOn'),
    'Text':('uint16_t *','const char *key','TheText.Get(key)'),
    'ErrorText':('uint16_t *','void','WideErrorString'),
    'ScaleX':('float','float value','SCREEN_SCALE_X(value)'),
    'ScaleY':('float','float value','SCREEN_SCALE_Y(value)'),
    'Width':('float','void','SCREEN_WIDTH'),
    'CentreSize':('float','void','SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH)'),
}
FONTS={
    'JustifyOff':('void','CFont::SetJustifyOff()'),
    'BackgroundOff':('void','CFont::SetBackgroundOff()'),
    'Scale':('float x, float y','CFont::SetScale(x, y)'),
    'PropOn':('void','CFont::SetPropOn()'),
    'Standard':('void','CFont::SetFontStyle(FONT_STANDARD)'),
    'CentreOn':('void','CFont::SetCentreOn()'),
    'CentreSize':('float size','CFont::SetCentreSize(size)'),
    'Color':('uint8_t r, uint8_t g, uint8_t b, uint8_t a','CFont::SetColor(CRGBA(r, g, b, a))'),
    'Print':('float x, float y, uint16_t *text','CFont::PrintString(x, y, text)'),
    'Draw':('void','CFont::DrawFonts()'),
}
PROTOTYPES='void MusicDisplay_StationName(cMusicManager *manager);\n'
HOST='//+ rouz edit (ChatGPT)\nextern wchar WideErrorString[];\n'
for name,(result,args,expression) in READERS.items():
    PROTOTYPES+=result+' MusicDisplayHost_'+name+'('+args+');\n'
    HOST+='extern "C" '+result+'\nMusicDisplayHost_'+name+'('+args+')\n{\n    // Read the original display input at its existing access point\n'
    HOST+='    return '+expression+';\n}\n'
for name,(args,expression) in FONTS.items():
    PROTOTYPES+='void MusicDisplayFont_'+name+'('+args+');\n'
    HOST+='extern "C" void\nMusicDisplayFont_'+name+'('+args+')\n{\n    // Submit the original font operation with the captured C display arguments\n    '+expression+';\n}\n'
HOST+='//- rouz edit (ChatGPT)\n'

def calls(source):
    # Replace the sole game display entry without introducing an intermediate C++ wrapper
    return cleanup.calls(source.replace('MusicManager.DisplayRadioStationName()', 'MusicDisplay_StationName(&MusicManager)'))

def logic(source):
    # Remove the complete display member and append typed renderer and text boundaries
    owner='cMusicManager::cMusicManager(' in source
    if 'cMusicManager::DisplayRadioStationName(' in source:
        source=source.replace(function(source,'cMusicManager::','DisplayRadioStationName')+'\n','')
    source=calls(source)
    # Keep the original adapter order after timer call conversion
    host=timer.calls(HOST) if timer.calls(HOST) in source else HOST
    if owner and host not in source:source+='\n'+host
    if host in source:source=source.replace(host,'')+host
    return cleanup.logic(source)

def header(source):
    # Preserve all owner fields while exposing the C display operation and typed boundaries
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bDisplayRadioStationName\(',line))
    return cleanup.header(source.replace('void MusicState_InitState(cMusicManager *manager);',PROTOTYPES+'void MusicState_InitState(cMusicManager *manager);'))

def body(source):
    # Preserve the original shared timer, pointer identity, branch structure and font-call order
    source=source[source.index('{'):]
    source=re.sub(r'(?<![\w.>])\b(m_\w+)\b',r'manager->\1',source).replace('(this','(manager')
    for a,b in [('uint8','uint8_t'),('wchar','uint16_t'),('nil','NULL')]:source=re.sub(r'\b'+a+r'\b',b,source)
    source=source.replace('CTimer::GetIsPaused()','MusicDisplayHost_Paused()').replace('TheCamera.m_WideScreenOn','MusicDisplayHost_Widescreen()').replace('CReplay::IsPlayingBack()','AudioEnvironmentHost_Replay()')
    source=source.replace('vehicle->m_nRadioStation','(*MusicRadioHost_Station(vehicle))').replace('TheText.Get(', 'MusicDisplayHost_Text(')
    source=source.replace('extern uint16_t WideErrorString[];','').replace('string == WideErrorString','string == MusicDisplayHost_ErrorText()')
    source=source.replace('CTimer::GetLogicalFramesPassed()','AudioPoliceHost_LogicalFramesPassed()')
    source=source.replace('SCREEN_SCALE_X(', 'MusicDisplayHost_ScaleX(').replace('SCREEN_SCALE_Y(', 'MusicDisplayHost_ScaleY(').replace('SCREEN_STRETCH_X(DEFAULT_SCREEN_WIDTH)','MusicDisplayHost_CentreSize()').replace('SCREEN_WIDTH','MusicDisplayHost_Width()')
    for original,target in [('SetJustifyOff','JustifyOff'),('SetBackgroundOff','BackgroundOff'),('SetScale','Scale'),('SetPropOn','PropOn'),('SetCentreOn','CentreOn'),('SetCentreSize','CentreSize'),('PrintString','Print'),('DrawFonts','Draw')]:source=source.replace('CFont::'+original+'(', 'MusicDisplayFont_'+target+'(')
    source=source.replace('CFont::SetFontStyle(FONT_STANDARD)','MusicDisplayFont_Standard()')
    source=re.sub(r'CFont::SetColor\(CRGBA\(([^)]*)\)\)',r'MusicDisplayFont_Color(\1)',source)
    return source

def project(source):
    # Register the complete display operation as an actual C11 translation unit
    if 'MusicDisplay.c' in source:return cleanup.clock.project(source)
    a=source.index('    <ClCompile Include="..\\src\\audio\\MusicService.c"');b=source.index('</ClCompile>',a)+len('</ClCompile>')
    return cleanup.clock.project(source[:b]+'\n'+source[a:b].replace('MusicService.c','MusicDisplay.c')+source[b:])
