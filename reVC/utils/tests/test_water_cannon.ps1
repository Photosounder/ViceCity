param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$water_cannonRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$water_cannonPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $water_cannonRoot
try {
    $water_cannonExtra=@()
    if($Sanitized){$water_cannonExtra+='--sanitized'}
    & $water_cannonPython build/test_water_cannon.py @water_cannonExtra
    if($LASTEXITCODE -ne 0){throw 'Water cannon comparison failed'}
    if($Native){
        & $water_cannonPython build/test_water_cannon_native.py
        if($LASTEXITCODE -ne 0){throw 'Native water_cannon comparison failed'}
    }
    if($Consumers){
        & $water_cannonPython build/check_water_cannon_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual water_cannon consumer compilation failed'}
    }
    if($BuildProject){
        & $water_cannonPython build/build_water_cannon.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $water_cannonPython build/check_water_cannon_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
