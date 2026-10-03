"""Compare compiler-emitted owner layout constants without linking game libraries."""
from pathlib import Path
import json
import struct
import sys

def constants(path):
    # Read the native COFF symbol table and locate the exported ABI constants
    data = Path(path).read_bytes()
    machine, sections, _, symbols, symbol_count, optional_size, _ = struct.unpack_from('<HHIIIHH', data)
    width = {0x14c: 4, 0x8664: 8}[machine]
    strings = symbols + symbol_count * 18
    section_table = 20 + optional_size
    found = {}
    index = 0
    while index < symbol_count:
        # Skip auxiliary symbol records while preserving the real symbol's section offset
        offset = symbols + index * 18
        name_bytes, value, section, _, _, auxiliary = struct.unpack_from('<8sIhHBB', data, offset)
        if name_bytes[:4] == b'\0' * 4:
            name_offset = strings + struct.unpack_from('<I', name_bytes, 4)[0]
            name = data[name_offset:data.index(b'\0', name_offset)].decode()
        else:
            name = name_bytes.rstrip(b'\0').decode()
        if name.lstrip('_') in ('AudioManagerAbi', 'AudioManagerAbiCount'):
            # Resolve each constant into the file-backed data for its COFF section
            assert 0 < section <= sections
            section_offset = section_table + (section - 1) * 40
            raw_size, raw_offset = struct.unpack_from('<II', data, section_offset + 16)
            assert value < raw_size
            found[name.lstrip('_')] = data[raw_offset + value:raw_offset + raw_size]
        index += 1 + auxiliary
    count = int.from_bytes(found['AudioManagerAbiCount'][:width], 'little')
    values = [int.from_bytes(found['AudioManagerAbi'][i * width:(i + 1) * width], 'little') for i in range(count)]
    assert len(found['AudioManagerAbi']) >= count * width
    return values

before, after, report = sys.argv[1:]
expected, actual = constants(before), constants(after)
assert expected == actual, 'Native owner layout changed'
assert (len(actual) - 2) % 3 == 0
result = {'owner_size': actual[0], 'owner_alignment': actual[1], 'field_count': (len(actual) - 2) // 3, 'field_offset_size_alignment': [actual[i:i + 3] for i in range(2, len(actual), 3)]}
Path(report).write_text(json.dumps(result, indent=2) + '\n', newline='\n')
print(f"Native owner layout matched: size {result['owner_size']}, alignment {result['owner_alignment']}, fields {result['field_count']}")
