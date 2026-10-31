"""Keep original observed statistics providers in independent historical subsystem fixtures."""
import re

def aliases(host):
    # Resolve each projected native statistics access to its unchanged original fixture provider
    names=sorted(set(re.findall(r'\bStats_(\w+)\b',host)))
    return ''.join('#define Stats_'+name+' CStats::'+name+'\n' for name in names)
