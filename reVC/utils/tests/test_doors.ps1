$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/doors_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_doors.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_doors.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_doors_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/doors_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_doors.py
exit $LASTEXITCODE
