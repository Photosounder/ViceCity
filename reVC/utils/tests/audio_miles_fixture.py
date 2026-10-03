"""Extract actual Miles state/metadata/volume methods for a saved C++ comparison."""
from pathlib import Path
import re,sys
root=Path(__file__).resolve().parents[2]
baseline='--baseline' in sys.argv
source=(root / ('build/audio-miles-native-c-tests/before/src/audio/sampman_miles.cpp' if baseline else 'src/audio/sampman_miles.c')).read_text()
names=['UpdateEffectsVolume','SetEffectsMasterVolume','SetMusicMasterVolume','SetMP3BoostVolume','SetEffectsFadeVolume','SetMusicFadeVolume','SetMonoMode','GetBankContainingSound','GetSampleBaseFrequency','GetSampleLoopStartOffset','GetSampleLoopEndOffset','GetSampleLength','GetChannelUsedFlag','SetStreamedVolumeAndPan','GetNum3DProvidersAvailable','SetNum3DProvidersAvailable','Get3DProviderName','Set3DProviderName']
functions=[]
for name in names:
    match=re.search(r'(?:void|bool8|uint32|uint8|int32|int8|char\s*\*)\s*SampleManager_'+name+r'\([^)]*\)[^\n{]*\s*\{',source)
    if not match:raise RuntimeError(name)
    end=source.index('{',match.start())+1;depth=1
    while depth:
        depth+=(source[end]=='{')-(source[end]=='}');end+=1
    functions.append(source[match.start():end])
fixture=(root/'utils/tests/audio_miles.c.in').read_text()
Path(sys.argv[1]).write_text(fixture.replace('@METHODS@','\n'.join(functions)),newline='\n')
