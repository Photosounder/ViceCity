"""Prepare current C and saved original C++ channel behavior fixtures."""
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[2]
baseline = '--baseline' in sys.argv
if baseline:
    header = 'build/audio-channel-c-tests/before-channel.h'
    source = (root / 'build/audio-channel-c-tests/before-channel.cpp').read_text()
    source = source.replace('#include "common.h"', '').replace('#include "sampman.h"', '').replace('#include "channel.h"', '')
else:
    header = 'src/audio/oal/channel.h'
    source = '#include "src/audio/oal/channel.c"'
template = (root / 'utils/tests/audio_channel.c.in').read_text()
result = template.replace('@HEADER@', header).replace('@SOURCE@', source)
if baseline:
    result = '#define BASELINE_CHANNEL\n' + result
Path(sys.argv[1]).write_text(result, newline='\n')
