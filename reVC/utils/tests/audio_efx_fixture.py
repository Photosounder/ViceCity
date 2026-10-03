"""Prepare current C and saved original C++ OpenAL effect trace fixtures."""
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[2]
baseline = '--baseline' in sys.argv
if baseline:
    header = 'build/audio-efx-c-tests/before-oal_utils.h'
    source = (root / 'build/audio-efx-c-tests/before-oal_utils.cpp').read_text()
    source = source.replace('#include "common.h"', '').replace('#include "oal_utils.h"', '')
else:
    header = 'src/audio/oal/oal_utils.h'
    source = '#include "src/audio/oal/oal_utils.c"'
template = (root / 'utils/tests/audio_efx.c.in').read_text()
result = template.replace('@HEADER@', header).replace('@SOURCE@', source)
if baseline:
    result = '#define BASELINE_EFX\n' + result
Path(sys.argv[1]).write_text(result, newline='\n')
