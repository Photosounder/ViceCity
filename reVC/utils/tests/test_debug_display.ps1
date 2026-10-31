$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$debugDisplayRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$debugDisplayPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $debugDisplayRoot
try {
    # Prepare exact production policies and stable dependencies before any compiler starts
    & $debugDisplayPython utils/tests/debug_display_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Debug display source audit failed'}
    foreach($debugDisplayCheck in @(@('utils/tests/run_debug_display.py','--prepared','--matrix'),@('utils/tests/run_debug_display.py','--prepared','--sanitized'),@('utils/tests/run_debug_display_native.py'),@('utils/tests/debug_display_native_sources.py','--production'))) {
        # Keep each failure tied to its real command while running quiet console and compiler checks
        & $debugDisplayPython @debugDisplayCheck
        if($LASTEXITCODE -ne 0){throw "Debug display check failed: $debugDisplayCheck"}
    }
} finally {
    # Restore the original shell location after verification completes
    Pop-Location
}
