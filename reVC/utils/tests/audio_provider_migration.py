"""Known provider and ordered-volume substitutions for older source audits."""
import re
from audio_controls_migration import function
ARGS='AudioManager.m_bIsInitialised, {which}, &AudioManager.m_nActiveSamples, &AudioManager.m_nActiveQueue, AudioManager.m_aRequestedOrderList, AudioManager.m_nRequestedCount, AudioManager.m_asActiveSamples'
def calls(source):
    # Preserve the position of each migrated ordered-volume pass in the service routine
    return source.replace('AdjustSamplesVolume();','AudioSoundQueue_AdjustVolume(m_aRequestedQueue[m_nActiveQueue], m_aRequestedOrderList[m_nActiveQueue], m_nRequestedCount[m_nActiveQueue]);')
def owner(source):
    # Remove only these two methods whose C bodies are independently audited
    for name in ['SetCurrent3DProvider','AdjustSamplesVolume']:
        source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return calls(source)
def header(source):
    # Retain every stored field and other declaration unchanged
    return '\n'.join(line for line in source.split('\n') if not re.search(r'\b(?:SetCurrent3DProvider|AdjustSamplesVolume)\(',line))
def facade(source, host=False):
    # Substitute the explicit queue-state call at both original game boundary sites
    which='index' if host else 'which'
    include='AudioManager.h' if host else 'AudioControls.h'
    source=source.replace('#include "'+include+'"','#include "'+include+'"\n#include "AudioProvider.h"')
    return source.replace('AudioManager.SetCurrent3DProvider('+which+')','AudioProvider_SetCurrent('+ARGS.format(which=which)+')')
