param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$projectileRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$projectilePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $projectileRoot
try {
    if($Sanitized){
        & $projectilePython build/test_projectile.py --sanitized
    }else{
        & $projectilePython build/test_projectile.py
    }
    if($LASTEXITCODE -ne 0){throw 'Projectile comparison failed'}
    if($Native){
        & $projectilePython build/test_projectile_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native projectile comparison failed'}
        & $projectilePython build/check_projectile_types.py
        if($LASTEXITCODE -ne 0){throw 'Native projectile type checks failed'}
    }
    if($Consumers){
        & $projectilePython build/check_projectile_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual projectile C compilation failed'}
    }
    if($BuildProject){
        & $projectilePython build/build_projectile.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $projectilePython build/check_projectile_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
