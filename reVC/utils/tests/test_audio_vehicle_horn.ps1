$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$vehicleHornRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$vehicleHornBuild=Join-Path $vehicleHornRoot 'build/audio-vehicle-horn-c-tests'
& 'C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe' "$PSScriptRoot/audio_vehicle_horn_fixture.py"
if($LASTEXITCODE -ne 0) {throw 'VehicleHorn source audit failed'}
$vehicleHornRuns=@()
foreach($vehicleHornCompiler in @('clang','gcc')) {
 foreach($vehicleHornOptimization in @('O0','O2')) {
  $vehicleHornCpp=if($vehicleHornCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($vehicleHornShape in @('default','vanilla','no-reverb','no-external','ps2')) {
   $vehicleHornArgs=@("-I$vehicleHornRoot/src/audio","-I$vehicleHornRoot/src/core",'-include',"$vehicleHornBuild/$vehicleHornShape.h")
   foreach($vehicleHornUnit in @('AudioVehicleHorn','AudioMath','AudioRequests','AudioSoundQueue')) {
    & $vehicleHornCompiler -std=c11 -pedantic-errors "-$vehicleHornOptimization" @vehicleHornArgs -c "$vehicleHornRoot/src/audio/$vehicleHornUnit.c" -o "$vehicleHornBuild/$vehicleHornUnit.o"
    if($LASTEXITCODE -ne 0) {throw "C compile failed: $vehicleHornUnit"}
   }
   & $vehicleHornCompiler -std=c11 -pedantic-errors "-$vehicleHornOptimization" @vehicleHornArgs -c "$vehicleHornBuild/caller.c" -o "$vehicleHornBuild/caller.o"
   if($LASTEXITCODE -ne 0) {throw 'C caller compile failed'}
   $vehicleHornCommon=@("$vehicleHornBuild/AudioMath.o","$vehicleHornBuild/AudioRequests.o","$vehicleHornBuild/AudioSoundQueue.o")
   & $vehicleHornCpp -std=c++17 "-$vehicleHornOptimization" @vehicleHornArgs "$vehicleHornBuild/before.cpp" @vehicleHornCommon -o "$vehicleHornBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original owner compile failed'}
   & $vehicleHornCpp -std=c++17 "-$vehicleHornOptimization" @vehicleHornArgs "$vehicleHornBuild/after.cpp" @vehicleHornCommon "$vehicleHornBuild/AudioVehicleHorn.o" "$vehicleHornBuild/caller.o" -o "$vehicleHornBuild/after.exe"
   if($LASTEXITCODE -ne 0) {throw 'Mixed C/C++ link failed'}
   $vehicleHornBefore=& "$vehicleHornBuild/before.exe"
   if($LASTEXITCODE -ne 0) {throw 'Original runtime failed'}
   $vehicleHornAfter=& "$vehicleHornBuild/after.exe"
   if($LASTEXITCODE -ne 0 -or $vehicleHornBefore -ne $vehicleHornAfter) {throw "VehicleHorn comparison failed: $vehicleHornBefore / $vehicleHornAfter"}
   $vehicleHornRuns += "$vehicleHornCompiler-$vehicleHornOptimization-$vehicleHornShape : $vehicleHornAfter"
   Write-Output $vehicleHornRuns[-1]
  }
 }
}
$vehicleHornRuns | ConvertTo-Json | Set-Content "$vehicleHornBuild/runtime-runs.json"
$vehicleHornSanBuild=Join-Path $vehicleHornBuild 'sanitized'
New-Item -ItemType Directory -Path $vehicleHornSanBuild -Force | Out-Null
$vehicleHornSanArgs=@('-O1','-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$vehicleHornRoot/src/audio","-I$vehicleHornRoot/src/core",'-include',"$vehicleHornBuild/default.h")
foreach($vehicleHornSanUnit in @('AudioVehicleHorn','AudioMath','AudioRequests','AudioSoundQueue')) {
 & clang -std=c11 @vehicleHornSanArgs -c "$vehicleHornRoot/src/audio/$vehicleHornSanUnit.c" -o "$vehicleHornSanBuild/$vehicleHornSanUnit.o"
 if($LASTEXITCODE -ne 0) {throw "Sanitized C compile failed: $vehicleHornSanUnit"}
}
& clang -std=c11 @vehicleHornSanArgs -c "$vehicleHornBuild/caller.c" -o "$vehicleHornSanBuild/caller.o"
if($LASTEXITCODE -ne 0) {throw 'Sanitized caller compile failed'}
$vehicleHornSanCommon=@("$vehicleHornSanBuild/AudioMath.o","$vehicleHornSanBuild/AudioRequests.o","$vehicleHornSanBuild/AudioSoundQueue.o")
$vehicleHornSanResults=@()
foreach($vehicleHornSanVariant in @('before','after')) {
 $vehicleHornSanExtra=if($vehicleHornSanVariant -eq 'after') {@("$vehicleHornSanBuild/AudioVehicleHorn.o","$vehicleHornSanBuild/caller.o")} else {@()}
 & clang++ -std=c++17 @vehicleHornSanArgs "$vehicleHornBuild/$vehicleHornSanVariant.cpp" @vehicleHornSanCommon @vehicleHornSanExtra -o "$vehicleHornSanBuild/$vehicleHornSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw 'Sanitized link failed'}
 $vehicleHornSanResults += & "$vehicleHornSanBuild/$vehicleHornSanVariant.exe"
 if($LASTEXITCODE -ne 0) {throw "Undefined behavior found: $vehicleHornSanVariant"}
}
if($vehicleHornSanResults.Count -ne 2 -or $vehicleHornSanResults[0] -ne $vehicleHornSanResults[1]) {throw 'Sanitized comparison failed'}
$vehicleHornSanResults | ConvertTo-Json | Set-Content "$vehicleHornBuild/sanitizer-runs.json"
Write-Output "Sanitized comparison passed: $($vehicleHornSanResults[1])"
