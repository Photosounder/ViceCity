$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$darkelRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$darkelPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $darkelRoot
try {
    # Prepare exact production policies and stable controlled dependencies before any compiler starts
    & $darkelPython utils/tests/darkel_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Rampage source audit failed'}
    foreach($darkelCheck in @(@('utils/tests/run_darkel.py','--prepared','--matrix'),@('utils/tests/run_darkel.py','--prepared','--sanitized'),@('utils/tests/run_darkel_native.py'),@('utils/tests/darkel_native_sources.py','--production'))) {
        # Keep each failure tied to its real command while running quiet console and compiler checks
        & $darkelPython @darkelCheck
        if($LASTEXITCODE -ne 0){throw "Rampage check failed: $darkelCheck"}
    }
} finally {
    # Restore the original shell location after verification completes
    Pop-Location
}
