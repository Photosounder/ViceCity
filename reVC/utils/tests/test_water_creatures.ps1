param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$waterCreaturesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$waterCreaturesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $waterCreaturesRoot
try {
    $waterCreaturesExtra=@()
    if($Sanitized){$waterCreaturesExtra+='--sanitized'}
    & $waterCreaturesPython build/test_water_creatures.py @waterCreaturesExtra
    if($LASTEXITCODE -ne 0){throw 'Sea-life comparison failed'}
    if($Native){
        & $waterCreaturesPython build/test_water_creatures_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native sea-life comparison failed'}
        & $waterCreaturesPython build/check_water_creatures_types.py
        if($LASTEXITCODE -ne 0){throw 'Native sea-life type checks failed'}
    }
    if($Consumers){
        & $waterCreaturesPython build/check_water_creatures_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual sea-life consumer compilation failed'}
    }
    if($BuildProject){
        & $waterCreaturesPython build/build_water_creatures.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $waterCreaturesPython build/check_water_creatures_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
