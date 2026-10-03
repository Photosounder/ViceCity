"""Known cached distance conversion for complete audio source audits."""
import re
import audio_geometry_migration as geometry
from audio_controls_migration import function

def logic(source):
    # Remove the member and pass its original flag and cached field explicitly
    if 'cAudioManager::CalculateDistance(' in source:
        source=source.replace(function(source,'cAudioManager::','CalculateDistance')+'\n','')
    return geometry.logic(re.sub(r'(?<![\w.:>])CalculateDistance\(([^,\n]+), ([^\n]+)\);',r'AudioMath_CalculateDistance(&\1, &m_sQueueSample.m_fDistance, \2);',source))

def header(source):
    # Remove only the obsolete method declaration while preserving owner fields
    return geometry.header('\n'.join(line for line in source.split('\n') if not re.search(r'\bCalculateDistance\(',line)))
