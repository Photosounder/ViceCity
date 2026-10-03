"""Known reflection probe update conversion and world boundaries."""
import re
import audio_reload_migration as reload
from audio_controls_migration import function
HOST='//+ rouz edit (ChatGPT)\n#ifdef AUDIO_REFLECTIONS\nuint8_t\nAudioReflectionsHost_Line(const AudioSoundPosition *start, const AudioSoundPosition *end, AudioSoundPosition *hit)\n{\n    // Preserve the original line query flags and copy only a successful collision position\n    CColPoint point;\n    CEntity *entity;\n    if (!CWorld::ProcessLineOfSight(AudioSoundPosition_ToVector(start), AudioSoundPosition_ToVector(end), point, entity, true, false, false, true, false, true, true))\n        return 0;\n    *hit = AudioSoundPosition_FromVector(point.point);\n    return 1;\n}\n\nuint8_t\nAudioReflectionsHost_Vertical(const AudioSoundPosition *start, float endZ, float *hitZ)\n{\n    // Preserve the original vertical query flags and read only the successful height\n    CColPoint point;\n    CEntity *entity;\n    if (!CWorld::ProcessVerticalLine(AudioSoundPosition_ToVector(start), endZ, point, entity, true, false, false, false, true, false, nil))\n        return 0;\n    *hitZ = point.point.z;\n    return 1;\n}\n#endif\n//- rouz edit (ChatGPT)\n'
PROTOTYPES='#ifdef AUDIO_REFLECTIONS\nvoid AudioReflections_Update(cAudioManager *manager);\nuint8_t AudioReflectionsHost_Line(const AudioSoundPosition *start, const AudioSoundPosition *end, AudioSoundPosition *hit);\nuint8_t AudioReflectionsHost_Vertical(const AudioSoundPosition *start, float endZ, float *hitZ);\n#endif\n'

def calls(source):
    # Preserve the existing service call point and live owner
    source=source.replace('AudioManager.UpdateReflections()', 'AudioReflections_Update(&AudioManager)')
    return re.sub(r'(?<![\w.:>])UpdateReflections\(\)', 'AudioReflections_Update(this)',source)

def owner(source):
    # Remove only the converted member and retain world access in two narrow callbacks
    if 'cAudioManager::UpdateReflections(' in source:
        source=source.replace(function(source,'cAudioManager::','UpdateReflections')+'\n','')+HOST
    return reload.facade(reload.owner(calls(source)))

def header(source):
    # Expose C operations without changing any owner storage
    source='\n'.join(line for line in source.split('\n') if not re.search(r'\bUpdateReflections\(',line))
    return reload.header(source.replace('void AudioManager_InitState(cAudioManager *manager);',PROTOTYPES+'void AudioManager_InitState(cAudioManager *manager);'))

def body(source):
    # Keep both version schedules, endpoint writes and exact frame wrap arithmetic
    source=source[source.index('{'):]
    source=re.sub(r'(?<![.>\w])\b(m_\w+)\b',r'manager->\1',source)
    source=source.replace('CVector camPos;','AudioSoundPosition camPos;').replace('CColPoint colpoint;','AudioSoundPosition hit;\n\tfloat hitZ;').replace('\n\tCEntity *ent;','')
    source=source.replace('camPos = TheCamera.GetPosition();','AudioGeometryHost_CameraPosition(&camPos);').replace('AudioSoundPosition_FromVector(camPos)','camPos')
    source=re.sub(r'CWorld::ProcessLineOfSight\(camPos, AudioSoundPosition_ToVector\(&([^)]*)\), colpoint, ent, true, false, false, true, false, true, true\)',r'AudioReflectionsHost_Line(&camPos, &\1, &hit)',source)
    source=re.sub(r'CWorld::ProcessVerticalLine\(camPos, ([^,]*), colpoint, ent, true, false, false, false, true, false, nil\)',r'AudioReflectionsHost_Vertical(&camPos, \1, &hitZ)',source)
    source=source.replace('Distance(camPos, colpoint.point)', 'AudioReflections_Distance(&camPos, &hit)').replace('colpoint.point.z', 'hitZ')
    return source
