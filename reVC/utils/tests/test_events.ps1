$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$eventsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$eventsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $eventsRoot
try {
    # Prepare the exact production implementation and controlled dependencies before running compilers
    & $eventsPython utils/tests/events_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Event source audit failed'}
    foreach($eventsCheck in @(@('utils/tests/run_events.py','--prepared','--matrix'),@('utils/tests/run_events.py','--prepared','--sanitized'),@('utils/tests/run_events_native.py'),@('utils/tests/check_events_native_sources.py','--production'))) {
        # Keep each failed compiler or comparison command visible without launching a game window
        & $eventsPython @eventsCheck
        if($LASTEXITCODE -ne 0){throw "Event check failed: $eventsCheck"}
    }
} finally {
    # Restore the original shell location after quiet verification
    Pop-Location
}
