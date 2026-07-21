param(
    [string]$Destination = $(if ($env:RUNNER_TEMP) { Join-Path $env:RUNNER_TEMP 'iwyu' } else { Join-Path $PSScriptRoot '..\build\tools\iwyu' })
)

$ErrorActionPreference = 'Stop'
$clang = Get-Command clang -ErrorAction Stop
$versionText = & $clang.Source --version
if ($versionText -notmatch 'clang version\s+(\d+)') {
    throw 'Unable to determine the installed Clang major version'
}
$branch = "clang_$($Matches[1])"
$source = Join-Path $Destination 'source'
$build = Join-Path $Destination 'build'
if (-not (Test-Path -LiteralPath $source)) {
    git clone --depth 1 --branch $branch https://github.com/include-what-you-use/include-what-you-use.git $source
    if ($LASTEXITCODE -ne 0) { throw "Failed to clone IWYU branch $branch" }
}
$llvmRoot = Split-Path (Split-Path $clang.Source -Parent) -Parent
cmake -S $source -B $build -G Ninja "-DCMAKE_PREFIX_PATH=$llvmRoot\lib\cmake"
if ($LASTEXITCODE -ne 0) { throw 'IWYU configure failed' }
cmake --build $build --parallel
if ($LASTEXITCODE -ne 0) { throw 'IWYU build failed' }
$tool = Get-ChildItem -LiteralPath $build -Recurse -Filter include-what-you-use.exe | Select-Object -First 1
if (-not $tool) { throw 'IWYU executable was not produced' }
if ($env:GITHUB_PATH) {
    $tool.DirectoryName | Out-File -FilePath $env:GITHUB_PATH -Append -Encoding utf8
}
Write-Output $tool.FullName
