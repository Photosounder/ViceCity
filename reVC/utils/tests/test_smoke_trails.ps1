param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$smokeTrailsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$smokeTrailsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $smokeTrailsRoot
try {
    $smokeTrailsExtra=@()
    if($Sanitized){$smokeTrailsExtra+='--sanitized'}
    & $smokeTrailsPython build/test_smoke_trails.py @smokeTrailsExtra
    if($LASTEXITCODE -ne 0){throw 'Smoke-trail comparison failed'}
    if($Native){
        & $smokeTrailsPython build/test_smoke_trails_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native smoke-trail comparison failed'}
        & $smokeTrailsPython build/check_smoke_trails_types.py
        if($LASTEXITCODE -ne 0){throw 'Native smoke-trail type checks failed'}
    }
    if($Consumers){
        & $smokeTrailsPython build/check_smoke_trails_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual smoke-trail consumer compilation failed'}
    }
    if($BuildProject){
        & $smokeTrailsPython build/build_smoke_trails.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $smokeTrailsPython build/check_smoke_trails_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
