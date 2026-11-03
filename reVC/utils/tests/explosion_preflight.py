"""Inventory the complete explosion owner and its callers before conversion."""
from pathlib import Path
import hashlib
import json
import re
from rect_migration import mask

root = Path(__file__).resolve().parents[2]
source = root / 'src/weapons/Explosion.cpp'
header = root / 'src/weapons/Explosion.h'
uses = []
for path in (root / 'src').rglob('*'):
    # Inspect actual project sources without including generated or hidden build trees
    if path.suffix not in {'.c', '.cpp', '.h'} or any(p.startswith('.') for p in path.relative_to(root / 'src').parts):
        continue
    code = mask(path.read_text())
    if re.search(r'\b(?:CExplosion|gaExplosion)\b', code):
        uses.append({'source': path.relative_to(root).as_posix(), 'sha256': hashlib.sha256(path.read_bytes()).hexdigest()})
operations = sorted(set(re.findall(r'\bCExplosion::([A-Za-z_]\w*)\s*\(', mask(source.read_text()))))
report = {
    'status': 'diagnosed',
    'production_changed': False,
    'owner': source.relative_to(root).as_posix(),
    'header': header.relative_to(root).as_posix(),
    'operations': operations,
    'consumers': uses,
    'preserve': [
        'Conditional AddExplosion signature, default arguments, and all callers',
        'Actual global explosion array and current initialization field order',
        'Mutable position returned by GetExplosionPosition',
        'Audio creation failure and shutdown behavior',
        'Existing random, timer, particle, fire, world, and entity call ordering',
        'Configuration-dependent branches and any preexisting partial initialization',
    ],
}
assert len(operations) == 12, operations
target = root / 'build/explosion-preflight.json'
target.parent.mkdir(exist_ok=True)
target.write_text(json.dumps(report, indent=2), newline='\n')
print('Inventoried', len(operations), 'operations and', len(uses), 'complete source files')
