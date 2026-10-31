param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bright_lightsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bright_lightsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $bright_lightsRoot
try {
    $bright_lightsExtra=@()
    if($Sanitized){$bright_lightsExtra+='--sanitized'}
    & $bright_lightsPython build/test_bright_lights.py @bright_lightsExtra
    if($LASTEXITCODE -ne 0){throw 'Bright lights comparison failed'}
    if($Native){
        & $bright_lightsPython build/test_bright_lights_native.py
        if($LASTEXITCODE -ne 0){throw 'Native bright_lights comparison failed'}
    }
    if($Consumers){
        & $bright_lightsPython build/check_bright_lights_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual bright_lights consumer compilation failed'}
    }
    if($BuildProject){
        & $bright_lightsPython build/build_bright_lights.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $bright_lightsPython build/check_bright_lights_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
