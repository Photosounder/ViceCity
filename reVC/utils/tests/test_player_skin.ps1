param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$playerSkinRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$playerSkinPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $playerSkinRoot
try {
    if($Sanitized){
        & $playerSkinPython build/test_player_skin.py --sanitized
    }else{
        & $playerSkinPython build/test_player_skin.py
    }
    if($LASTEXITCODE -ne 0){throw 'Player-skin comparison failed'}
    if($Native){
        & $playerSkinPython build/test_player_skin_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native player-skin comparison failed'}
        & $playerSkinPython build/check_player_skin_types.py
        if($LASTEXITCODE -ne 0){throw 'Native player-skin type checks failed'}
    }
    if($Consumers){
        & $playerSkinPython build/check_player_skin_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual player-skin C compilation failed'}
    }
    if($BuildProject){
        & $playerSkinPython build/build_player_skin.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $playerSkinPython build/check_player_skin_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
