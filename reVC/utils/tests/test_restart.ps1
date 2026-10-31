param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$restartRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$restartPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $restartRoot
try {
    if($Sanitized){
        & $restartPython build/test_restart.py --sanitized
    }else{
        & $restartPython build/test_restart.py
    }
    if($LASTEXITCODE -ne 0){throw 'Restart-point comparison failed'}
    if($Native){
        & $restartPython build/test_restart_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native restart-point comparison failed'}
        & $restartPython build/check_restart_types.py
        if($LASTEXITCODE -ne 0){throw 'Native restart-point type checks failed'}
    }
    if($Consumers){
        & $restartPython build/check_restart_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual restart-point C compilation failed'}
    }
    if($BuildProject){
        & $restartPython build/build_restart.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $restartPython build/check_restart_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
