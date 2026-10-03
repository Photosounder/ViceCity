"""Build a quiet test fixture from actual production WAV and VB decoders."""
from pathlib import Path
import sys

root = Path(__file__).resolve().parents[2]
baseline = '--baseline' in sys.argv
source = root / ('build/audio-decoder-tests/before-stream.cpp' if baseline else 'src/audio/oal/stream.c')
text = source.read_text()
start = text.index('class CSortStereoBuffer') if baseline else text.index('#define AUDIO_FORMAT_DEBUG(')
end = text.index('// For multi-thread:')
template = (root / 'utils/tests/audio_decoder.cpp.in').read_text()
header = 'build/audio-decoder-tests/before-stream.h' if baseline else 'src/audio/oal/stream.h'
if baseline:
    stream_methods = text[text.index('CStream::CStream('):text.index('CStream::~CStream()')]
    stream_methods += text[text.index('CStream::~CStream()'):text.index('void CStream::Close()')]
    stream_methods += text[text.index('bool CStream::IsOpened()'):text.index('bool CStream::IsPlaying()')]
else:
    stream_methods = text[text.index('void CStream_Init('):text.index('void CStream_Destroy(')]
    stream_methods += text[text.index('void CStream_Destroy('):text.index('void CStream_Close(')]
    stream_methods += text[text.index('bool CStream_IsOpened('):text.index('bool CStream_IsPlaying(')]
result = template.replace('@HEADER@', header).replace('@DECODERS@', text[start:end]).replace('@STREAM_METHODS@', stream_methods)
if baseline:
    result = '#define BASELINE_DECODER\n' + result
Path(sys.argv[1]).write_text(result, newline='\n')
