param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$boat_foamRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$boat_foamPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $boat_foamRoot
try {
    $boat_foamExtra=@()
    if($Sanitized){$boat_foamExtra+='--sanitized'}
    & $boat_foamPython build/test_boat_foam.py @boat_foamExtra
    if($LASTEXITCODE -ne 0){throw 'Boat foam comparison failed'}
    if($Native){
        & $boat_foamPython build/test_boat_foam_native.py
        if($LASTEXITCODE -ne 0){throw 'Native boat_foam comparison failed'}
    }
    if($Consumers){
        & $boat_foamPython build/check_boat_foam_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual boat_foam consumer compilation failed'}
    }
    if($BuildProject){
        & $boat_foamPython build/build_boat_foam.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $boat_foamPython build/check_boat_foam_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
