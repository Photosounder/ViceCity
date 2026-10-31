"""Recheck complete preceding owner audits against the applied C memory manager."""
from pathlib import Path
import runpy,sys
root=Path(__file__).resolve().parents[2]
for name in ['anim_plugin_stage_audit.py','col_model_stage_audit.py']:
    # Require full original ownership and archived integration proofs through explicit complete successors
    sys.argv=[name,'--production','--integration'];runpy.run_path(str(root/'utils/tests'/name),run_name='__main__')
sys.argv=['anim_plugin_compatibility.py']
runpy.run_path(str(root/'utils/tests/anim_plugin_compatibility.py'),run_name='__main__')
