$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$statsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$statsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $statsRoot
try {
    # Prepare exact production policies and controlled dependencies before any compiler starts
    & $statsPython utils/tests/stats_fixture.py --production
    if($LASTEXITCODE -ne 0){throw 'Statistics source audit failed'}
    foreach($statsCheck in @(@('utils/tests/run_stats.py','--prepared','--matrix'),@('utils/tests/run_stats.py','--prepared','--sanitized'),@('utils/tests/run_stats_native.py'),@('utils/tests/stats_native_sources.py','--production'))) {
        # Keep each failure tied to its real command while running quiet console and compiler checks
        & $statsPython @statsCheck
        if($LASTEXITCODE -ne 0){throw "Statistics check failed: $statsCheck"}
    }
} finally {
    # Restore the original shell location after verification completes
    Pop-Location
}
