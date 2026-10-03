"""Extract production stream methods into a quiet mock-OpenAL integration test."""
import pathlib
import re
import sys

root = pathlib.Path(__file__).resolve().parents[2]
source = (root / 'src/audio/oal/stream.c').read_text()
names = ['CStream_Init', 'CStream_Destroy', 'CStream_HasSource',
         'CStream_IsOpened', 'CStream_BuffersShouldBeFilled',
         'CStream_BufferShouldBeFilledAndQueued', 'CStream_FlagAsToBeProcessed',
         'AudioReleaseCloseJob', 'audioFileOpsThread', 'CStream_Initialise', 'CStream_Terminate',
         'CStream_QueueBuffers', 'CStream_FillBuffer', 'CStream_FillBuffers',
         'CStream_ClearBuffers', 'CStream_Setup', 'CStream_SetPlay',
         'CStream_Start', 'CStream_Stop', 'CStream_SetPosMS',
         'CStream_ProviderTerm', 'CStream_ProviderInit', 'CStream_IsPlaying',
         'CStream_Close', 'CStream_Update']
methods = []
for name in names:
    match = re.search(r'^(?:static[ \t]+)?(?:(?:void|bool|int32)[ \t]+)?' + re.escape(name) + r'\s*\(', source, re.M)
    if not match:
        raise RuntimeError('Missing production method: ' + name)
    start = match.start()
    if source[start:start + len(name)] == name:
        previous = source[:start].rstrip().splitlines()[-1]
        if previous in ('void', 'bool', 'int32'):
            start = source.rfind(previous, 0, start)
    opening = source.index('{', match.end())
    depth = 0
    end = None
    tokens = re.compile(r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|[{}]')
    for token in tokens.finditer(source, opening):
        if token.group() == '{':
            depth += 1
        elif token.group() == '}':
            depth -= 1
            if depth == 0:
                end = token.end()
                break
    if end is None:
        raise RuntimeError('Unbalanced production method: ' + name)
    method = source[start:end]
    methods.append(method)
template = (root / 'utils/tests/audio_stream.cpp.in').read_text()
pathlib.Path(sys.argv[1]).write_text(template.replace('@METHODS@', '\n\n'.join(methods)), newline='\n')
