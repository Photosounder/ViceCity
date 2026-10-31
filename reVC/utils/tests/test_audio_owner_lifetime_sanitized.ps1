$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$controlRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$controlBuild=Join-Path $controlRoot 'build/audio-owner-lifetime-c-tests'
$controlSources=$controlBuild
$controlBuild=Join-Path $controlBuild 'sanitized'
New-Item -ItemType Directory -Force $controlBuild | Out-Null
$controlArgs=@('-fsanitize=undefined,float-cast-overflow','-fsanitize-trap=all',"-I$controlSources", "-I$controlRoot/src/audio", "-I$controlRoot/src/core")
$controlRuns=@()
foreach($controlCompiler in @('clang')) {
 foreach($controlOptimization in @('O1')) {
  foreach($controlShape in @('default','ps2')) {
  $controlShapeArgs=@('-include', "$controlSources/$controlShape.h")
  $controlCpp=if($controlCompiler -eq 'clang') {'clang++'} else {'g++'}
  foreach($providerModule in @('AudioOwner','AudioManagerInit','AudioSoundReset','AudioCollisionQueue','AudioCrimes','AudioPedComments')) {
    $unitSource=if($providerModule -eq 'AudioOwner') {"$controlSources/observed-owner.c"} else {"$controlRoot/src/audio/$providerModule.c"}
    & $controlCompiler -std=c11 -pedantic-errors "-$controlOptimization" @controlArgs @controlShapeArgs -c $unitSource -o "$controlBuild/$providerModule.o"
    if($LASTEXITCODE -ne 0) {throw "C compilation failed: $providerModule"}
  }
  foreach($variant in @('before','after')) {
    foreach($part in @("$variant-probe1",$variant,"$variant-probe2")) {
      & $controlCpp -std=c++17 "-$controlOptimization" @controlArgs @controlShapeArgs -c "$controlSources/$part.cpp" -o "$controlBuild/$part.o"
      if($LASTEXITCODE -ne 0) {throw "Lifetime fixture compilation failed: $part"}
    }
    $objects=@("$controlBuild/$variant-probe1.o","$controlBuild/$variant.o","$controlBuild/$variant-probe2.o","$controlBuild/AudioManagerInit.o","$controlBuild/AudioSoundReset.o","$controlBuild/AudioCollisionQueue.o","$controlBuild/AudioCrimes.o","$controlBuild/AudioPedComments.o")
    if($variant -eq 'after') {$objects+= "$controlBuild/AudioOwner.o"}
    & $controlCpp '-fsanitize=undefined,float-cast-overflow' '-fsanitize-trap=all' @objects -o "$controlBuild/$variant.exe"
    if($LASTEXITCODE -ne 0) {throw 'Lifetime fixture link failed'}
  }
  $controlExpected=& "$controlBuild/before.exe"
  if($LASTEXITCODE -ne 0) {throw 'Baseline control runtime failed'}
  $controlActual=& "$controlBuild/after.exe"
  if($LASTEXITCODE -ne 0 -or $controlExpected -ne $controlActual) {throw "Control trace mismatch: $controlExpected / $controlActual"}
  $controlRuns += "$controlCompiler-$controlOptimization-$controlShape : $controlActual"
  Write-Output "Owner initialization and lifetime trace matched: $($controlRuns[-1])"
  }
 }
}
$controlRuns | ConvertTo-Json | Set-Content "$controlBuild/runtime-runs.json"

