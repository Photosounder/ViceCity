param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$particleMgrRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$particleMgrPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $particleMgrRoot
try {
    $particleMgrExtra=@()
    if($Sanitized){$particleMgrExtra+='--sanitized'}
    & $particleMgrPython build/test_particle_mgr.py @particleMgrExtra
    if($LASTEXITCODE -ne 0){throw 'Particle configuration comparison failed'}
    if($Native){
        & $particleMgrPython build/test_particle_mgr_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native particle configuration comparison failed'}
        & $particleMgrPython build/check_particle_mgr_types.py
        if($LASTEXITCODE -ne 0){throw 'Native particle configuration type checks failed'}
    }
    if($Consumers){
        & $particleMgrPython build/check_particle_mgr_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual particle configuration consumer compilation failed'}
    }
    if($BuildProject){
        & $particleMgrPython build/build_particle_mgr.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $particleMgrPython build/check_particle_mgr_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
