param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$planeBannersRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$planeBannersPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $planeBannersRoot
try {
    $planeBannersExtra=@()
    if($Sanitized){$planeBannersExtra+='--sanitized'}
    & $planeBannersPython build/test_plane_banners.py @planeBannersExtra
    if($LASTEXITCODE -ne 0){throw 'Plane-banner comparison failed'}
    if($Native){
        & $planeBannersPython build/test_plane_banners_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native plane-banner comparison failed'}
        & $planeBannersPython build/check_plane_banners_types.py
        if($LASTEXITCODE -ne 0){throw 'Native plane-banner type checks failed'}
    }
    if($Consumers){
        & $planeBannersPython build/check_plane_banners_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual plane-banner consumer compilation failed'}
    }
    if($BuildProject){
        & $planeBannersPython build/build_plane_banners.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $planeBannersPython build/check_plane_banners_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
