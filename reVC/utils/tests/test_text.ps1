$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$textRoot=(Resolve-Path (Join-Path $PSScriptRoot '../..')).Path
$textPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $textRoot
try {
    # Prepare the actual production kernel and immutable original fixture once before running comparisons
    & $textPython utils/tests/text_fixture.py --production
    if ($LASTEXITCODE -ne 0) { throw 'Text source audit failed' }
    foreach ($textCheck in @(@('build/test_text.py','--prepared'),@('build/test_text.py','--prepared','--sanitized'),@('build/test_text_native.py','--prepared'),@('build/check_text_types.py'),@('build/check_text_consumers.py'),@('build/build_text.py'),@('utils/tests/object_names_fixture.py'),@('build/check_text_audits.py'),@('build/test_user_display.py','--smoke','--prepared'),@('build/check_text_compat.py'),@('build/finalize_text.py'))) {
        # Keep each bounded compiler or console verification quiet and stop at its exact failed command
        & $textPython @textCheck
        if ($LASTEXITCODE -ne 0) { throw "Text check failed: $textCheck" }
    }
} finally {
    # Restore the caller's directory after the complete console-only test sequence
    Pop-Location
}
