param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$cloudEffectsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$cloudEffectsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $cloudEffectsRoot
try {
    $cloudEffectsExtra=@()
    if($Sanitized){$cloudEffectsExtra+='--sanitized'}
    & $cloudEffectsPython build/test_cloud_effects.py @cloudEffectsExtra
    if($LASTEXITCODE -ne 0){throw 'Cloud effects comparison failed'}
    if($Native){
        & $cloudEffectsPython build/test_cloud_effects_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native cloud effects comparison failed'}
    }
    if($Consumers){
        & $cloudEffectsPython build/check_cloud_effects_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual cloud effects consumer compilation failed'}
    }
    if($BuildProject){
        & $cloudEffectsPython build/build_cloud_effects.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $cloudEffectsPython build/check_cloud_effects_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
