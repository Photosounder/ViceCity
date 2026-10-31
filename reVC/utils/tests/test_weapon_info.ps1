param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$weapon_infoRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$weapon_infoPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $weapon_infoRoot
try {
    if($Sanitized){
        & $weapon_infoPython build/test_weapon_info.py --sanitized
    }else{
        & $weapon_infoPython build/test_weapon_info.py
    }
    if($LASTEXITCODE -ne 0){throw 'Weapon metadata comparison failed'}
    if($Native){
        & $weapon_infoPython build/test_weapon_info_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native weapon_info system comparison failed'}
        & $weapon_infoPython build/check_weapon_info_types.py
        if($LASTEXITCODE -ne 0){throw 'Native weapon_info system type checks failed'}
    }
    if($Consumers){
        & $weapon_infoPython build/check_weapon_info_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual weapon_info system C compilation failed'}
    }
    if($BuildProject){
        & $weapon_infoPython build/build_weapon_info.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $weapon_infoPython build/check_weapon_info_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
