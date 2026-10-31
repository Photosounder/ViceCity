param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$rangesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$rangesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $rangesRoot
try {
    $rangesExtra=@()
    if($Sanitized){$rangesExtra+='--sanitized'}
    & $rangesPython build/test_ranges.py @rangesExtra
    if($LASTEXITCODE -ne 0){throw 'Ranges comparison failed'}
    if($Native){
        & $rangesPython build/test_ranges_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native ranges comparison failed'}
        & $rangesPython build/check_ranges_types.py
        if($LASTEXITCODE -ne 0){throw 'Native ranges type checks failed'}
    }
    if($Consumers){
        & $rangesPython build/check_ranges_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual ranges consumer compilation failed'}
    }
    if($BuildProject){
        & $rangesPython build/build_ranges.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $rangesPython build/check_ranges_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
