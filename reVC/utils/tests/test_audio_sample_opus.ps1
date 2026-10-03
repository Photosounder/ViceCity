$ErrorActionPreference='Stop'
$env:Path='C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$opusRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$opusBuild=Join-Path $opusRoot 'build/audio-sample-native-c-tests'
New-Item -ItemType Directory -Force $opusBuild | Out-Null
$opusPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
& $opusPython (Join-Path $PSScriptRoot 'audio_sample_opus_fixture.py') "$opusBuild/opus-fixture.c"
if($LASTEXITCODE -ne 0) {throw 'Opus fixture generation failed'}
$opusArgs=@('-DAUDIO_OAL','-DAUDIO_OPUS','-DOPUS_SFX',"-I$opusRoot/src/core","-I$opusRoot/src/audio","-I$opusRoot/vendor/opusfile/include","-I$opusRoot/vendor/opus/include","-I$opusRoot/vendor/ogg/include")
$opusRuns=@()
foreach($opusCompiler in @('clang','gcc')) {
 foreach($opusOpt in @('O0','O2')) {
  & $opusCompiler -std=c11 "-$opusOpt" @opusArgs "$opusBuild/opus-fixture.c" -o "$opusBuild/opus-fixture.exe"
  if($LASTEXITCODE -ne 0) {throw 'Opus fixture compilation failed'}
  & "$opusBuild/opus-fixture.exe"
  if($LASTEXITCODE -ne 0) {throw 'Opus fixture runtime failed'}
  $opusRuns += "$opusCompiler-$opusOpt"
  Write-Output "Production mission/ped Opus loaders passed: $($opusRuns[-1])"
 }
}
$opusRuns | ConvertTo-Json | Set-Content "$opusBuild/opus-runtime-runs.json"
