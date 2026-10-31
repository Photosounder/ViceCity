$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$autopilotRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$autopilotPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $autopilotRoot
try {
    # Prepare complete production policies and stable dependencies before any compiler starts
    & $autopilotPython utils/tests/autopilot_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Autopilot source audit failed'}
    foreach($autopilotCheck in @(@('utils/tests/run_autopilot.py','--prepared','--matrix'),@('utils/tests/run_autopilot.py','--prepared','--sanitized'),@('utils/tests/run_autopilot_native.py'),@('utils/tests/autopilot_native_sources.py','--production'))) {
        # Keep failures tied to their real commands while running quiet console and compiler checks
        & $autopilotPython @autopilotCheck
        if($LASTEXITCODE -ne 0){throw "Autopilot check failed: $autopilotCheck"}
    }
} finally {
    # Restore the original shell location after verification completes
    Pop-Location
}
