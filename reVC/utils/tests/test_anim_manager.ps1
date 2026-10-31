$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/anim_manager_prepare.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_strict_c.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_native_sources.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_native_sources.py --consumers
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_layouts.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_policy.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_loader.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_loader.py --sanitized
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_loader_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_full.py --matrix
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_full.py --sanitized
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_loader_native.py --full
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_native_sources.py --production --consumers
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_history.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/anim_manager_stage_audit.py --production --integration
exit $LASTEXITCODE
