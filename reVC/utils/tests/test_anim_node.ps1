$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/anim_node_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_node.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_node.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_node_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_node_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_node_history.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_anim_node.py
exit $LASTEXITCODE
