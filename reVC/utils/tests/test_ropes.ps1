param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$ropeRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$ropePython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $ropeRoot
try {
    $ropeExtra=@()
    if($Sanitized){$ropeExtra+='--sanitized'}
    & $ropePython build/test_ropes.py @ropeExtra
    if($LASTEXITCODE -ne 0){throw 'Rope comparison failed'}
    if($Native){
        & $ropePython build/test_ropes_native.py
        if($LASTEXITCODE -ne 0){throw 'Native rope comparison failed'}
    }
    if($Consumers){
        & $ropePython build/check_ropes_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual rope consumer compilation failed'}
    }
    if($BuildProject){
        & $ropePython build/build_ropes.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $ropePython build/check_ropes_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
