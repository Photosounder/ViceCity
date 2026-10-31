param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$traffic_lightsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$traffic_lightsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $traffic_lightsRoot
try {
    if($Sanitized){
        & $traffic_lightsPython build/test_traffic_lights.py --sanitized
    }else{
        & $traffic_lightsPython build/test_traffic_lights.py
    }
    if($LASTEXITCODE -ne 0){throw 'Traffic-light controller comparison failed'}
    if($Native){
        & $traffic_lightsPython build/test_traffic_lights_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native traffic_lights system comparison failed'}
        & $traffic_lightsPython build/check_traffic_lights_types.py
        if($LASTEXITCODE -ne 0){throw 'Native traffic_lights system type checks failed'}
    }
    if($Consumers){
        & $traffic_lightsPython build/check_traffic_lights_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual traffic_lights system C compilation failed'}
    }
    if($BuildProject){
        & $traffic_lightsPython build/build_traffic_lights.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $traffic_lightsPython build/check_traffic_lights_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
