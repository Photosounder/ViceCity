param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$motion_blur_streaksRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$motion_blur_streaksPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $motion_blur_streaksRoot
try {
    $motion_blur_streaksExtra=@()
    if($Sanitized){$motion_blur_streaksExtra+='--sanitized'}
    & $motion_blur_streaksPython build/test_motion_blur_streaks.py @motion_blur_streaksExtra
    if($LASTEXITCODE -ne 0){throw 'Motion-blur streaks comparison failed'}
    if($Native){
        & $motion_blur_streaksPython build/test_motion_blur_streaks_native.py
        if($LASTEXITCODE -ne 0){throw 'Native motion_blur_streaks comparison failed'}
    }
    if($Consumers){
        & $motion_blur_streaksPython build/check_motion_blur_streaks_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual motion_blur_streaks consumer compilation failed'}
    }
    if($BuildProject){
        & $motion_blur_streaksPython build/build_motion_blur_streaks.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $motion_blur_streaksPython build/check_motion_blur_streaks_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
