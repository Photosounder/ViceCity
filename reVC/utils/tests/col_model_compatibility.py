"""Recheck preceding collision owners against the complete applied C model successor."""
from pathlib import Path
import runpy,sys
root=Path(__file__).resolve().parents[2]
for name in ['finalize_compressed_vector.py','finalize_col_triangle.py','finalize_col_box.py']:
    # Share only byte-keyed historical proofs while running each complete prior owner audit
    sys.argv=[name];runpy.run_path(str(root/'utils/tests'/name),run_name='__main__')
sys.argv=['node_name_stage_audit.py','--production','--integration']
runpy.run_path(str(root/'utils/tests/node_name_stage_audit.py'),run_name='__main__')
