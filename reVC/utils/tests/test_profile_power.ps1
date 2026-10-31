$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$profilePowerRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$profilePowerPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $profilePowerRoot
try {
    # Prepare exact production policies and stable dependencies before any compiler starts
    & $profilePowerPython utils/tests/profile_power_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Profiler/power-point source audit failed'}
    foreach($profilePowerCheck in @(@('utils/tests/run_profile_power.py','--prepared','--matrix'),@('utils/tests/run_profile_power.py','--prepared','--sanitized'),@('utils/tests/run_profile_power_native.py'),@('utils/tests/profile_power_native_sources.py','--production'))) {
        # Keep failures tied to their real commands while running quiet console and compiler checks
        & $profilePowerPython @profilePowerCheck
        if($LASTEXITCODE -ne 0){throw "Profiler/power-point check failed: $profilePowerCheck"}
    }
} finally {
    # Restore the original shell location after verification completes
    Pop-Location
}
