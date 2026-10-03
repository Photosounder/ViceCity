$ErrorActionPreference = 'Stop'
$env:Path = 'C:\msys\ucrt64\bin;C:\msys\usr\bin;' + $env:Path
$linkRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$linkTestDir = Join-Path $linkRoot 'build/link-list-tests'
New-Item -ItemType Directory -Force -Path $linkTestDir | Out-Null
$linkRuns = 0
foreach ($linkCompiler in @('clang', 'gcc')) {
    foreach ($linkLanguage in @('c', 'c++')) {
        foreach ($linkOptimization in @('O0', 'O2')) {
            foreach ($linkCustomAssert in @(0, 1)) {
                $linkName = "$linkCompiler-$($linkLanguage.Replace('+', 'p'))-$linkOptimization-$linkCustomAssert"
                $linkExecutable = Join-Path $linkTestDir "$linkName.exe"
                $linkStandard = if ($linkLanguage -eq 'c') { '-std=c11' } else { '-std=c++17' }
                $linkArgs = @('-x', $linkLanguage, $linkStandard, "-$linkOptimization", '-Wall', '-Wextra', '-Werror', '-pedantic', '-Wno-missing-field-initializers')
                if ($linkCustomAssert) { $linkArgs += '-DLINK_PREDEFINED_ASSERT' }
                $linkArgs += @((Join-Path $PSScriptRoot 'link_list.c'), '-o', $linkExecutable)
                & $linkCompiler @linkArgs
                if ($LASTEXITCODE -ne 0) { throw "Compilation failed: $linkName" }
                & $linkExecutable | Out-Null
                if ($LASTEXITCODE -ne 0) { throw "Linked list test failed: $linkName" }
                $linkRuns++
            }
        }
    }
}
Write-Output "Linked list tests passed in $linkRuns compiler/language/optimization/assertion combinations"
