$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$scriptSoundsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$scriptSoundsBuild=Join-Path $scriptSoundsRoot 'build/audio-script-sounds-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_script_sounds_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'ScriptSounds source audit failed'}
$scriptSoundsSanBuild=Join-Path $scriptSoundsBuild 'sanitized-ps2'
New-Item -ItemType Directory -Path $scriptSoundsSanBuild -Force | Out-Null
$scriptSoundsSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$scriptSoundsRoot/src/audio","-I$scriptSoundsRoot/src/core",'-include',"$scriptSoundsBuild/ps2.h")
foreach($scriptSoundsSanUnit in @('AudioScriptSounds','AudioMath','AudioGeometry','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsRoot/src/audio/$scriptSoundsSanUnit.c" $(if($scriptSoundsSanUnit -eq 'AudioRequests'){'-DAudioRequests_Submit=TestScriptSoundsRealSubmit'}) -o "$scriptSoundsSanBuild/$scriptSoundsSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $scriptSoundsSanUnit"}
}
& clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsBuild/caller.c" -o "$scriptSoundsSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
& clang -std=c11 @scriptSoundsSanArgs -c "$scriptSoundsBuild/fast_hash.c" -o "$scriptSoundsSanBuild/fast_hash.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized observer compile failed'}
$scriptSoundsSanCommon=@("$scriptSoundsSanBuild/fast_hash.o","$scriptSoundsSanBuild/AudioMath.o","$scriptSoundsSanBuild/AudioGeometry.o","$scriptSoundsSanBuild/AudioRequests.o","$scriptSoundsSanBuild/AudioSoundQueue.o")
$scriptSoundsSanResults=@()
foreach($scriptSoundsSanVariant in @('before','after')) {
 $scriptSoundsSanExtra=if($scriptSoundsSanVariant -eq 'after') {@("$scriptSoundsSanBuild/AudioScriptSounds.o","$scriptSoundsSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @scriptSoundsSanArgs "$scriptSoundsBuild/$scriptSoundsSanVariant.cpp" @scriptSoundsSanCommon @scriptSoundsSanExtra -o "$scriptSoundsSanBuild/$scriptSoundsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $scriptSoundsSanResults += & "$scriptSoundsSanBuild/$scriptSoundsSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $scriptSoundsSanVariant"}
}
if($scriptSoundsSanResults.Count -ne 2 -or $scriptSoundsSanResults[0] -ne $scriptSoundsSanResults[1]) {throw 'Sanitized comparison failed'}
$scriptSoundsSanResults | ConvertTo-Json | Set-Content "$scriptSoundsBuild/sanitizer-ps2-runs.json"
Write-Output "Sanitized comparison passed: $($scriptSoundsSanResults[1])"
