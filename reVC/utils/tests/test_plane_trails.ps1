param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$planeTrailsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$planeTrailsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $planeTrailsRoot
try {
    $planeTrailsExtra=@()
    if($Sanitized){$planeTrailsExtra+='--sanitized'}
    & $planeTrailsPython build/test_plane_trails.py @planeTrailsExtra
    if($LASTEXITCODE -ne 0){throw 'Plane-trail comparison failed'}
    if($Native){
        & $planeTrailsPython build/test_plane_trails_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native plane-trail comparison failed'}
        & $planeTrailsPython build/check_plane_trails_types.py
        if($LASTEXITCODE -ne 0){throw 'Native plane-trail type checks failed'}
    }
    if($Consumers){
        & $planeTrailsPython build/check_plane_trails_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual plane-trail consumer compilation failed'}
    }
    if($BuildProject){
        & $planeTrailsPython build/build_plane_trails.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $planeTrailsPython build/check_plane_trails_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
