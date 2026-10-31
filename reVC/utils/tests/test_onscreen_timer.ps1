param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$onscreenRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$onscreenPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $onscreenRoot
try {
    if($Sanitized){
        & $onscreenPython build/test_onscreen_timer.py --sanitized
    }else{
        & $onscreenPython build/test_onscreen_timer.py
    }
    if($LASTEXITCODE -ne 0){throw 'On-screen timer comparison failed'}
    if($Native){
        & $onscreenPython build/test_onscreen_timer_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native on-screen timer comparison failed'}
        & $onscreenPython build/check_onscreen_timer_types.py
        if($LASTEXITCODE -ne 0){throw 'Native on-screen timer record checks failed'}
    }
    if($Consumers){
        & $onscreenPython build/check_onscreen_timer_consumers.py
        if($LASTEXITCODE -ne 0){throw 'On-screen timer consumer compilation failed'}
    }
    if($BuildProject){
        & $onscreenPython build/build_onscreen_timer.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $onscreenPython build/check_onscreen_timer_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
