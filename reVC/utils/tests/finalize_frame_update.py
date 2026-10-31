"""Finalize complete C frame callback ownership against actual production and full build evidence."""
from pathlib import Path
import subprocess,sys
root=Path(__file__).resolve().parents[2]
subprocess.run([sys.executable,str(root/'utils/tests/frame_update_stage_audit.py'),'--production','--integration'],check=True)
