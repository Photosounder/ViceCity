"""Keep historical renderer dependency observations across the new C getter boundary."""
from pathlib import Path
import re
import timecycle_migration as cycle
root=Path(__file__).resolve().parents[2]
original=(root/'build/timecycle-c-tests/before/src/renderer/Timecycle.h').read_text()
kinds={name:kind for kind,name,expression in cycle.getters(original)}

def used(host):
    # Retain only the original getter dependencies actually exercised by this historical renderer fixture
    return list(dict.fromkeys(re.findall(r'CTimeCycle::(Get\w+)\(',host)))

def declarations(host):
    # Declare the same C symbol boundary while keeping each existing mock provider's actual vector object type
    text='\n'
    for name in used(host):text+='extern "C" '+('CVector *' if kinds[name]=='CVector &' else cycle.scalar(kinds[name])+' ')+'TimeCycle_'+name+'(void);\n'
    if 'GetSunDirection' in used(host):text+='CVector TimeCycle_ToNativeVector(CVector value);\n'
    return text

def definitions(host):
    # Preserve the unchanged provider observations while the complete new time-cycle owner is tested separately
    text='\n'
    for name in used(host):text+='extern "C" '+('CVector *' if kinds[name]=='CVector &' else cycle.scalar(kinds[name])+' ')+'TimeCycle_'+name+'(void)\n{\n    // Forward the exact historical observed dependency through the updated production C symbol\n    return '+('&' if kinds[name]=='CVector &' else '')+'CTimeCycle::'+name+'();\n}\n'
    if 'GetSunDirection' in used(host):text+='CVector TimeCycle_ToNativeVector(CVector value)\n{\n    // Preserve the existing native mock vector value without aliasing it as a different record type\n    return value;\n}\n'
    return text
