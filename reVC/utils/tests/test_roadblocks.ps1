$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$roadblocksRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$roadblocksPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $roadblocksRoot
try {
    # Prepare exact production policies and stable dependencies before any compiler starts
    & $roadblocksPython utils/tests/roadblocks_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Roadblock source audit failed'}
    foreach($roadblocksCheck in @(@('utils/tests/run_roadblocks.py','--prepared','--matrix'),@('utils/tests/run_roadblocks.py','--prepared','--sanitized'),@('utils/tests/run_roadblocks_native.py'),@('utils/tests/run_roadblocks_native.py','--debug'),@('utils/tests/roadblocks_native_sources.py','--production'))) {
        # Keep each failure tied to its real command while running quiet console and compiler checks
        & $roadblocksPython @roadblocksCheck
        if($LASTEXITCODE -ne 0){throw "Roadblock check failed: $roadblocksCheck"}
    }
} finally {
    # Restore the original shell location after verification completes
    Pop-Location
}
