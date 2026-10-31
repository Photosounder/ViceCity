param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$bullet_tracesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$bullet_tracesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $bullet_tracesRoot
try {
    $bullet_tracesExtra=@()
    if($Sanitized){$bullet_tracesExtra+='--sanitized'}
    & $bullet_tracesPython build/test_bullet_traces.py @bullet_tracesExtra
    if($LASTEXITCODE -ne 0){throw 'Bullet traces comparison failed'}
    if($Native){
        & $bullet_tracesPython build/test_bullet_traces_native.py
        if($LASTEXITCODE -ne 0){throw 'Native bullet_traces comparison failed'}
    }
    if($Consumers){
        & $bullet_tracesPython build/check_bullet_traces_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual bullet_traces consumer compilation failed'}
    }
    if($BuildProject){
        & $bullet_tracesPython build/build_bullet_traces.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $bullet_tracesPython build/check_bullet_traces_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
