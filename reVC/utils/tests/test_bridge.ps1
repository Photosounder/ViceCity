param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bridgeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bridgePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $bridgeRoot
try {
    if($Sanitized){
        & $bridgePython build/test_bridge.py --sanitized
    }else{
        & $bridgePython build/test_bridge.py
    }
    if($LASTEXITCODE -ne 0){throw 'Bridge controller comparison failed'}
    if($Native){
        & $bridgePython build/test_bridge_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native bridge system comparison failed'}
        & $bridgePython build/check_bridge_types.py
        if($LASTEXITCODE -ne 0){throw 'Native bridge system type checks failed'}
    }
    if($Consumers){
        & $bridgePython build/check_bridge_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual bridge system C compilation failed'}
    }
    if($BuildProject){
        & $bridgePython build/build_bridge.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $bridgePython build/check_bridge_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
