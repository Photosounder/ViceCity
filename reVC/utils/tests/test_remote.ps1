param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$remoteRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$remotePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $remoteRoot
try {
    if($Sanitized){
        & $remotePython build/test_remote.py --sanitized
    }else{
        & $remotePython build/test_remote.py
    }
    if($LASTEXITCODE -ne 0){throw 'Remote-car comparison failed'}
    if($Native){
        & $remotePython build/test_remote_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native remote-car comparison failed'}
        & $remotePython build/check_remote_types.py
        if($LASTEXITCODE -ne 0){throw 'Native remote-car type checks failed'}
    }
    if($Consumers){
        & $remotePython build/check_remote_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual remote-car C compilation failed'}
    }
    if($BuildProject){
        & $remotePython build/build_remote.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $remotePython build/check_remote_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
