"""Recheck complete earlier animation audits against the applied C plugin successor."""
from pathlib import Path
import runpy,sys
root=Path(__file__).resolve().parents[2]
for name in ['finalize_anim_clump.py','finalize_anim_sequence.py','finalize_anim_association.py']:
    # Reuse only digest-keyed complete successor proofs across the independent historical audits
    sys.argv=[name];runpy.run_path(str(root/'utils/tests'/name),run_name='__main__')
sys.argv=['frame_update_stage_audit.py','--production','--integration']
runpy.run_path(str(root/'utils/tests/frame_update_stage_audit.py'),run_name='__main__')
