"""Known owner-control removals for earlier conversion source audits."""
import re
VOLUMES = ['SetMP3BoostVolume','SetEffectsMasterVolume','SetMusicMasterVolume','SetEffectsFadeVol','SetMusicFadeVol']
CONTROLS = ['GetNum3DProvidersAvailable','Get3DProviderName','GetCurrent3DProviderIndex','AutoDetect3DProviders','SetSpeakerConfig','IsMP3RadioChannelAvailable','ReleaseDigitalHandle','ReacquireDigitalHandle','CheckForAnAudioFileOnCD','GetCDAudioDriveLetter']
OTHER = ['SetDynamicAcousticModelingStatus','IsAudioInitialised']
def function(source, prefix, name):
    # Extract the actual original body without reconstructing backend decisions
    m=re.search(r'(?m)^(?:[\w *]+\n)?[\w *]*'+prefix+name+r'\([^\n]*\)\n\{',source)
    assert m,name
    a=source.index('{',m.start());b=a+1;depth=1
    while depth:
        depth+=(source[b]=='{')-(source[b]=='}');b+=1
    return source[m.start():b]
def owner(source):
    # Permit only these separately verified member removals in older full-source audits
    for name in VOLUMES+CONTROLS+OTHER:
        source=source.replace(function(source,'cAudioManager::',name)+'\n','')
    return source.replace('#ifdef AUDIO_REFLECTIONS\n\n#endif\n','')
def header(source):
    # Permit only these removed method declarations while retaining every stored field
    return '\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+name+r'\(',line) for name in VOLUMES+CONTROLS+OTHER))
