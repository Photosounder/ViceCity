"""Finalize the complete C manager against exact production sources and full integration evidence."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,str(root/'utils/tests/anim_manager_stage_audit.py'),'--production','--integration'],check=True)
