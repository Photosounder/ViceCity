$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$releaseRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$releaseBuild=Join-Path $releaseRoot 'build/audio-release-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_release_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'Release source audit failed'}
$releaseRuns=@()
foreach($releaseCompiler in @('clang','gcc')) {
 foreach($releaseOptimization in @('O0','O2')) {
  $releaseCpp=if($releaseCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($releaseShape in @('default','vanilla','no-attachment','no-external','no-reflections','ps2')) {
   $releaseArgs=@("-I$releaseRoot/src/audio","-I$releaseRoot/src/core",'-include',"$releaseBuild/$releaseShape.h")
   foreach($releaseUnit in @('AudioRelease','AudioEntities','AudioRequests','AudioSoundQueue','AudioGeometry','AudioMath')) {
    & $releaseCompiler -std=c11 -pedantic-errors "-$releaseOptimization" @releaseArgs -c "$releaseRoot/src/audio/$releaseUnit.c" -o "$releaseBuild/$releaseUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $releaseUnit"}
   }
   & $releaseCompiler -std=c11 -pedantic-errors "-$releaseOptimization" @releaseArgs -c "$releaseBuild/caller.c" -o "$releaseBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $releaseCommon=@("$releaseBuild/AudioEntities.o","$releaseBuild/AudioRequests.o","$releaseBuild/AudioSoundQueue.o","$releaseBuild/AudioGeometry.o","$releaseBuild/AudioMath.o")
   & $releaseCpp -std=c++17 "-$releaseOptimization" @releaseArgs "$releaseBuild/before.cpp" @releaseCommon -o "$releaseBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $releaseCpp -std=c++17 "-$releaseOptimization" @releaseArgs "$releaseBuild/after.cpp" @releaseCommon "$releaseBuild/AudioRelease.o" "$releaseBuild/caller.o" -o "$releaseBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $releaseBefore=& "$releaseBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $releaseAfter=& "$releaseBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $releaseBefore -ne $releaseAfter) {throw "Release comparison failed: $releaseBefore / $releaseAfter"}
   $releaseRuns += "$releaseCompiler-$releaseOptimization-$releaseShape : $releaseAfter"
   Write-Output $releaseRuns[-1]
  }
 }
}
$releaseRuns | ConvertTo-Json | Set-Content "$releaseBuild/runtime-runs.json"
$releaseSanBuild=Join-Path $releaseBuild 'sanitized'
New-Item -ItemType Directory -Path $releaseSanBuild -Force | Out-Null
$releaseSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$releaseRoot/src/audio","-I$releaseRoot/src/core",'-include',"$releaseBuild/default.h")
foreach($releaseSanUnit in @('AudioRelease','AudioEntities','AudioRequests','AudioSoundQueue','AudioGeometry','AudioMath')) {
 & clang -std=c11 @releaseSanArgs -c "$releaseRoot/src/audio/$releaseSanUnit.c" -o "$releaseSanBuild/$releaseSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $releaseSanUnit"}
}
& clang -std=c11 @releaseSanArgs -c "$releaseBuild/caller.c" -o "$releaseSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$releaseSanCommon=@("$releaseSanBuild/AudioEntities.o","$releaseSanBuild/AudioRequests.o","$releaseSanBuild/AudioSoundQueue.o","$releaseSanBuild/AudioGeometry.o","$releaseSanBuild/AudioMath.o")
$releaseSanResults=@()
foreach($releaseSanVariant in @('before','after')) {
 $releaseSanExtra=if($releaseSanVariant -eq 'after') {@("$releaseSanBuild/AudioRelease.o","$releaseSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @releaseSanArgs "$releaseBuild/$releaseSanVariant.cpp" @releaseSanCommon @releaseSanExtra -o "$releaseSanBuild/$releaseSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $releaseSanResults += & "$releaseSanBuild/$releaseSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $releaseSanVariant"}
}
if($releaseSanResults.Count -ne 2 -or $releaseSanResults[0] -ne $releaseSanResults[1]) {throw 'Sanitized comparison failed'}
$releaseSanResults | ConvertTo-Json | Set-Content "$releaseBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($releaseSanResults[1])"
