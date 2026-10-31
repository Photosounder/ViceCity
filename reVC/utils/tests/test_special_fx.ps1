param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$special_fxRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$special_fxPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $special_fxRoot
try {
    $special_fxExtra=@()
    if($Sanitized){$special_fxExtra+='--sanitized'}
    & $special_fxPython build/test_special_fx.py @special_fxExtra
    if($LASTEXITCODE -ne 0){throw 'SpecialFX comparison failed'}
    if($Native){
        & $special_fxPython build/test_special_fx_native.py
        if($LASTEXITCODE -ne 0){throw 'Native special_fx comparison failed'}
    }
    if($Consumers){
        & $special_fxPython build/check_special_fx_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual special_fx consumer compilation failed'}
    }
    if($BuildProject){
        & $special_fxPython build/build_special_fx.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $special_fxPython build/check_special_fx_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
