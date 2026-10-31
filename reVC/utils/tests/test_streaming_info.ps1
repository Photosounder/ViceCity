$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/streaming_info_fixture.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_streaming_info.py --matrix --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_streaming_info.py --sanitized --prepared
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/run_streaming_info_native.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/streaming_info_native_sources.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_streaming_info.py
exit $LASTEXITCODE
