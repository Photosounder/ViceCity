$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/frame_update_prepare.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_fixture.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_runtime.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_runtime.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_runtime.py --matrix --mutations --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_runtime.py --sanitized --mutations --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_native_runtime.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_native_runtime.py --mutations
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_native_sources.py --layouts
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/frame_update_history.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_frame_update.py
exit $LASTEXITCODE
