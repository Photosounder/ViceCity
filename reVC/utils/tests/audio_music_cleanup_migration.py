"""Remove unused music members whose exact original bodies have no game behavior."""
import re
import clock_migration as clock
from audio_controls_migration import function
METHODS=['ServiceAmbience','ChangeRadioChannel','Enable','Disable']

def calls(source):
    # Preserve earlier caller audits through the complete static clock conversion
    return clock.calls(source)

def logic(source):
    # Remove only the verified unused empty methods and constant-return placeholder
    for name in METHODS[:2]:
        if 'cMusicManager::'+name+'(' in source:
            body=function(source,'cMusicManager::',name)
            statements=re.sub(r'\s+','',body[body.index('{'):])
            assert statements==('{}' if name=='ServiceAmbience' else '{returnTRUE;}'),name
            source=source.replace(body+'\n','')
    for name in METHODS[2:]:
        source=re.sub(r'(?m)^void cMusicManager::'+name+r'\(\) \{\}\n','',source)
    if 'cMusicManager::cMusicManager(' in source:
        source=source.replace('// these two are empty\n','')
        source=re.sub(r'\n{3,}','\n\n',source)
    return calls(source)

def header(source):
    # Remove unused member declarations without changing fields or the startup constructor
    if 'typedef struct cMusicManager {' not in source:return source
    source='\n'.join(line for line in source.split('\n') if not any(re.search(r'\b'+n+r'\(',line) for n in METHODS))
    return re.sub(r'\n{3,}','\n\n',source)
