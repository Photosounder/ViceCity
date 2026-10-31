param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$point_lightsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$point_lightsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $point_lightsRoot
try {
    $point_lightsExtra=@()
    if($Sanitized){$point_lightsExtra+='--sanitized'}
    & $point_lightsPython build/test_point_lights.py @point_lightsExtra
    if($LASTEXITCODE -ne 0){throw 'Point lights comparison failed'}
    if($Native){
        & $point_lightsPython build/test_point_lights_native.py
        if($LASTEXITCODE -ne 0){throw 'Native point_lights comparison failed'}
    }
    if($Consumers){
        & $point_lightsPython build/check_point_lights_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual point_lights consumer compilation failed'}
    }
    if($BuildProject){
        & $point_lightsPython build/build_point_lights.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $point_lightsPython build/check_point_lights_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
