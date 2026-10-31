"""Persist full preceding-owner audits against the applied C lighting successor."""
from pathlib import Path
import contextlib,io,json,runpy,sys

root=Path(__file__).resolve().parents[2]
out=root/'.diagnostics/lights-c-tests'
rows=[]
for name,arguments in [('clump_read_history.py',['--production']),('clump_read_stage_audit.py',['--production','--integration']),('clump_read_compatibility.py',[])]:
    # Preserve complete predecessor checks and their output without replacing them with summary status reads
    output=io.StringIO()
    sys.argv=[name]+arguments
    with contextlib.redirect_stdout(output):runpy.run_path(str(root/'utils/tests'/name),run_name='__main__')
    log=out/(name.removesuffix('.py')+'-compatibility.log')
    log.write_text(output.getvalue(),newline='\n')
    rows.append({'audit':name,'arguments':arguments,'log':str(log),'exit':0})
    print(name,'passed',flush=True)
(out/'preceding-owner-audits.json').write_text(json.dumps(rows,indent=2),newline='\n')
