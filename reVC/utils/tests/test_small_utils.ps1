$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$utilsRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$utilsTestDir = Join-Path $utilsRoot 'build/small-utils-tests'
New-Item -ItemType Directory -Force -Path $utilsTestDir | Out-Null
$utilsRuns = 0
$utilsRejected = 0
foreach ($utilsCompiler in @('clang', 'gcc')) {
    foreach ($utilsLanguage in @('c', 'c++')) {
        foreach ($utilsOptimization in @('O0', 'O2')) {
            foreach ($utilsSizes in @(0, 1)) {
                $utilsName = "$utilsCompiler-$($utilsLanguage.Replace('+', 'p'))-$utilsOptimization-$utilsSizes"
                $utilsExecutable = Join-Path $utilsTestDir "$utilsName.exe"
                $utilsStandard = if ($utilsLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
                $utilsArgs = @('-x', $utilsLanguage, $utilsStandard, "-$utilsOptimization", '-Wall', '-Wextra', '-Werror', '-pedantic')
                if ($utilsSizes) { $utilsArgs += '-DCHECK_STRUCT_SIZES' }
                $utilsSource = Join-Path $PSScriptRoot 'small_utils.c'
                & $utilsCompiler @utilsArgs $utilsSource -o $utilsExecutable
                if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $utilsName" }
                & $utilsExecutable
                if ($LASTEXITCODE -ne 0) { throw "Utility test failed: $utilsName" }
                $utilsRuns++
                foreach ($utilsBad in @('UTIL_BAD_SIZE', 'UTIL_BAD_OFFSET')) {
                    $utilsExpectedFailure = ($utilsBad -eq 'UTIL_BAD_OFFSET') -or ($utilsSizes -eq 1)
                    $utilsOutput = & $utilsCompiler @utilsArgs "-D$utilsBad" '-fsyntax-only' $utilsSource 2>&1
                    $utilsFailed = $LASTEXITCODE -ne 0
                    $utilsOutput | Set-Content -LiteralPath (Join-Path $utilsTestDir "$utilsName-$utilsBad.log")
                    if ($utilsFailed -ne $utilsExpectedFailure) { throw "Incorrect validation result: $utilsName-$utilsBad" }
                    if ($utilsFailed) { $utilsRejected++ }
                }
            }
        }
    }
}
foreach ($utilsLanguage in @('c', 'c++')) {
    $utilsStandard = if ($utilsLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
    & clang -x $utilsLanguage $utilsStandard -fsyntax-only -D__MWERKS__ -DCHECK_STRUCT_SIZES -DUTIL_BAD_SIZE -DUTIL_BAD_OFFSET (Join-Path $PSScriptRoot 'small_utils.c')
    if ($LASTEXITCODE -ne 0) { throw "Legacy disabled-assertion compatibility failed: $utilsLanguage" }
}
Write-Output "Utility tests passed in $utilsRuns combinations; $utilsRejected invalid layouts rejected"
