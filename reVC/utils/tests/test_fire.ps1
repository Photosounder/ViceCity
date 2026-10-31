param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$fireRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$firePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $fireRoot
try {
    if($Sanitized){
        & $firePython build/test_fire.py --sanitized
    }else{
        & $firePython build/test_fire.py
    }
    if($LASTEXITCODE -ne 0){throw 'Fire system comparison failed'}
    if($Native){
        & $firePython build/test_fire_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native fire system comparison failed'}
        & $firePython build/check_fire_types.py
        if($LASTEXITCODE -ne 0){throw 'Native fire system type checks failed'}
    }
    if($Consumers){
        & $firePython build/check_fire_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual fire system C compilation failed'}
    }
    if($BuildProject){
        & $firePython build/build_fire.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $firePython build/check_fire_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
