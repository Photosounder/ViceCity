param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$antennaRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$antennaPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $antennaRoot
try {
    $antennaExtra=@()
    if($Sanitized){$antennaExtra+='--sanitized'}
    & $antennaPython build/test_antennas.py @antennaExtra
    if($LASTEXITCODE -ne 0){throw 'Antenna comparison failed'}
    if($Native){
        & $antennaPython build/test_antennas_native.py
        if($LASTEXITCODE -ne 0){throw 'Native antenna comparison failed'}
    }
    if($Consumers){
        & $antennaPython build/check_antennas_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual antenna consumer compilation failed'}
    }
    if($BuildProject){
        & $antennaPython build/build_antennas.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $antennaPython build/check_antennas_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
