$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/keyframes_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_keyframes.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_keyframes.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_keyframes_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/keyframes_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/keyframes_history.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_keyframes.py
exit $LASTEXITCODE
