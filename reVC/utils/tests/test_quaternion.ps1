$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/quaternion_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_quaternion.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_quaternion.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_quaternion_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/quaternion_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/quaternion_history.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_quaternion.py
exit $LASTEXITCODE
