"""Extract production game-state callbacks for a mixed C/C++ ABI check."""
from pathlib import Path
import sys
root = Path(__file__).resolve().parents[2]
source = (root / 'src/audio/oal/AudioSampleHost.cpp').read_text()
start = source.index('extern "C" bool AudioStream_IsCutscene')
end = source.index('#ifndef _WIN32', start)
fixture = (root / 'utils/tests/audio_sample_host.cpp.in').read_text()
Path(sys.argv[1]).write_text(fixture.replace('@HOST_CALLBACKS@',source[start:end]),newline='\n')
