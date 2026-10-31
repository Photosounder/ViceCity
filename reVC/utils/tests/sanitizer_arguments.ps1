$ErrorActionPreference='Stop'
$sanitizerArgumentsRoot=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$sanitizerArgumentsPython='C:/Users/user/AppData/Local/Programs/Python/Python310/python.exe'
$sanitizerArgumentsResults=@()
foreach($wrapper in Get-ChildItem $PSScriptRoot -Filter 'test_*.ps1'){
    $text=[IO.File]::ReadAllText($wrapper.FullName)
    $assignment=[regex]::Match($text,'(?m)^\s*(\$\w+Extra)=@\(\)\r?\n\s*if\(\$Sanitized\)\{\1\+=''--sanitized''\}')
    if(!$assignment.Success){continue}
    $variable=$assignment.Groups[1].Value.Substring(1)
    if($text -notmatch ('(?m)&\s+\$\w+Python\s+build/test_\w+\.py\s+@'+[regex]::Escape($variable)+'\s*$')){throw "Missing actual native argument splat in $($wrapper.Name)"}
    $tokens=$null;$errors=$null
    $null=[Management.Automation.Language.Parser]::ParseFile($wrapper.FullName,[ref]$tokens,[ref]$errors)
    if($errors.Count){throw "Invalid PowerShell syntax in $($wrapper.Name): $errors"}
    foreach($enabled in @($false,$true)){
        $literal=if($enabled){'$true'}else{'$false'}
        $probe='$Sanitized='+$literal+'; '+$assignment.Value+"`n& '"+$sanitizerArgumentsPython+"' -c 'import json,sys; print(json.dumps({""args"":sys.argv[1:]}))' @"+$variable
        $output=& ([scriptblock]::Create($probe))
        if($LASTEXITCODE -ne 0){throw "Python argument probe failed in $($wrapper.Name)"}
        $arguments=($output | ConvertFrom-Json).args
        if(($enabled -and ($arguments.Count -ne 1 -or $arguments[0] -ne '--sanitized')) -or (!$enabled -and $arguments.Count -ne 0)){throw "Incorrect sanitizer arguments in $($wrapper.Name): $output"}
        $sanitizerArgumentsResults+=@{wrapper=$wrapper.Name;sanitized=$enabled;arguments=$arguments}
    }
}
if($sanitizerArgumentsResults.Count -ne 78){throw "Expected both modes of all 39 sanitizer wrappers, checked $($sanitizerArgumentsResults.Count)"}
$sanitizerArgumentsOutput=Join-Path $sanitizerArgumentsRoot 'build/sanitizer-arguments-tests'
$null=[IO.Directory]::CreateDirectory($sanitizerArgumentsOutput)
[IO.File]::WriteAllText((Join-Path $sanitizerArgumentsOutput 'results.json'),($sanitizerArgumentsResults | ConvertTo-Json -Depth 4).Replace("`r`n","`n")+"`n")
Write-Output 'Verified actual Python arguments in both modes of all 39 PowerShell sanitizer wrappers'
