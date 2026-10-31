param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$markers3dRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$markers3dPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $markers3dRoot
try {
    $markers3dExtra=@()
    if($Sanitized){$markers3dExtra+='--sanitized'}
    & $markers3dPython build/test_markers_3d.py @markers3dExtra
    if($LASTEXITCODE -ne 0){throw '3D-marker comparison failed'}
    if($Native){
        & $markers3dPython build/test_markers_3d_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native 3D-marker comparison failed'}
    }
    if($Consumers){
        & $markers3dPython build/check_markers_3d_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual 3D-marker consumer compilation failed'}
    }
    if($BuildProject){
        & $markers3dPython build/build_markers_3d.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $markers3dPython build/check_markers_3d_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
