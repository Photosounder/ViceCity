$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$wantedRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$wantedPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $wantedRoot
try {
    # Prepare the exact production policies and stable controlled dependencies before starting compilers
    & $wantedPython utils/tests/wanted_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Wanted source audit failed'}
    foreach($wantedCheck in @(@('utils/tests/run_wanted.py','--prepared','--matrix'),@('utils/tests/run_wanted.py','--prepared','--sanitized'),@('utils/tests/run_wanted_native.py'),@('utils/tests/wanted_native_sources.py','--production'))) {
        # Keep failures tied to the actual command while using only quiet compiler and console checks
        & $wantedPython @wantedCheck
        if($LASTEXITCODE -ne 0){throw "Wanted check failed: $wantedCheck"}
    }
} finally {
    # Restore the calling shell directory after all verification finishes
    Pop-Location
}
