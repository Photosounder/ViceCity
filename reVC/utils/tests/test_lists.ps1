$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/lists_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_lists.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_lists.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_lists_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/lists_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_lists.py
exit $LASTEXITCODE
