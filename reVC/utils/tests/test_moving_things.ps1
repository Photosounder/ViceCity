param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$movingThingsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$movingThingsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $movingThingsRoot
try {
    $movingThingsExtra=@()
    if($Sanitized){$movingThingsExtra+='--sanitized'}
    & $movingThingsPython build/test_moving_things.py @movingThingsExtra
    if($LASTEXITCODE -ne 0){throw 'Moving-thing comparison failed'}
    if($Native){
        & $movingThingsPython build/test_moving_things_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native moving-thing comparison failed'}
        & $movingThingsPython build/check_moving_things_types.py
        if($LASTEXITCODE -ne 0){throw 'Native moving-thing type checks failed'}
    }
    if($Consumers){
        & $movingThingsPython build/check_moving_things_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual moving-thing consumer compilation failed'}
    }
    if($BuildProject){
        & $movingThingsPython build/build_moving_things.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $movingThingsPython build/check_moving_things_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
