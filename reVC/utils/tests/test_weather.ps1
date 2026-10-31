param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$weatherRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$weatherPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $weatherRoot
try {
    if($Sanitized){
        & $weatherPython build/test_weather.py --sanitized
    }else{
        & $weatherPython build/test_weather.py
    }
    if($LASTEXITCODE -ne 0){throw 'Weather comparison failed'}
    if($Native){
        & $weatherPython build/test_weather_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native weather comparison failed'}
        & $weatherPython build/check_weather_types.py
        if($LASTEXITCODE -ne 0){throw 'Native weather type checks failed'}
    }
    if($Consumers){
        & $weatherPython build/check_weather_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual weather C compilation failed'}
    }
    if($BuildProject){
        & $weatherPython build/build_weather.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $weatherPython build/check_weather_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
