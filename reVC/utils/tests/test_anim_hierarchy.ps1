$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/anim_hierarchy_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_hierarchy.py --master-check --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_hierarchy.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_hierarchy.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_anim_hierarchy_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_hierarchy_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_hierarchy_custom_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_hierarchy_history.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_hierarchy_static_lifetime.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_anim_hierarchy.py
exit $LASTEXITCODE
