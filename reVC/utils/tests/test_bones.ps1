$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$python='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $python utils/tests/bones_prepare.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/bones_runtime.py
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/bones_runtime.py --sanitized
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/bones_native.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/bones_history.py --production
if ($LASTEXITCODE) { exit $LASTEXITCODE }
& $python utils/tests/finalize_bones.py --production
exit $LASTEXITCODE
