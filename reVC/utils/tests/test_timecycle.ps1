$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$timeCycleRoot=(Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$timeCyclePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $timeCycleRoot
try {
    # Prepare actual production sources once before any compiler or runtime reads the fixture
    & $timeCyclePython utils/tests/timecycle_fixture.py --production
    if ($LASTEXITCODE -ne 0) { throw 'Time-cycle source audit failed' }
    foreach ($timeCycleCheck in @(@('build/test_timecycle.py','--prepared'),@('build/test_timecycle.py','--prepared','--sanitized'),@('build/test_timecycle_native.py','--prepared'),@('build/check_timecycle_types.py'),@('build/check_timecycle_consumers.py'),@('build/build_timecycle.py'),@('utils/tests/object_names_fixture.py'),@('build/check_timecycle_audits.py'),@('build/check_timecycle_compat.py'),@('build/finalize_timecycle.py'))) {
        # Preserve the precise failed command while keeping successful quiet checks concise
        & $timeCyclePython @timeCycleCheck
        if ($LASTEXITCODE -ne 0) { throw "Time-cycle check failed: $timeCycleCheck" }
    }
} finally {
    # Restore the calling directory after the complete verification sequence
    Pop-Location
}
