param([switch]$Sanitized,[switch]$Native,[switch]$Consumers,[switch]$BuildProject,[switch]$AuditPrevious)
$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$setPiecesRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$setPiecesPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
Push-Location $setPiecesRoot
try {
    if($Sanitized){
        & $setPiecesPython build/test_set_pieces.py --sanitized
    }else{
        & $setPiecesPython build/test_set_pieces.py
    }
    if($LASTEXITCODE -ne 0){throw 'Set-piece comparison failed'}
    if($Native){
        & $setPiecesPython build/test_set_pieces_native.py --prepared
        if($LASTEXITCODE -ne 0){throw 'Native set-piece comparison failed'}
        & $setPiecesPython build/check_set_pieces_types.py
        if($LASTEXITCODE -ne 0){throw 'Native set-piece type checks failed'}
    }
    if($Consumers){
        & $setPiecesPython build/check_set_pieces_consumers.py
        if($LASTEXITCODE -ne 0){throw 'Actual set-piece C compilation failed'}
    }
    if($BuildProject){
        & $setPiecesPython build/build_set_pieces.py
        if($LASTEXITCODE -ne 0){throw 'Actual D3D9 project build failed'}
    }
    if($AuditPrevious){
        & $setPiecesPython build/check_set_pieces_audits.py
        if($LASTEXITCODE -ne 0){throw 'Preceding whole-source audit failed'}
    }
} finally { Pop-Location }
