param(
	[ValidateSet('Release', 'Debug')]
	[string]$Config = 'Release',
	[string]$Label = 'beta'
)
$ErrorActionPreference = 'Stop'

$repository = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$buildPath = Join-Path $repository "out\$Config"
$outputPath = Join-Path $repository 'out\installer'

$versionHeader = Get-Content (Join-Path $repository 'Telegram\SourceFiles\core\version.h') -Raw
if ($versionHeader -notmatch 'AppVersionStr = "([0-9.]+)"') {
	throw 'AppVersionStr not found in version.h.'
}
$version = $Matches[1]

foreach ($file in 'Lumigram.exe', 'modules\x64\d3d\d3dcompiler_47.dll') {
	if (-not (Test-Path (Join-Path $buildPath $file))) {
		throw "Missing $file in $buildPath. Build the $Config configuration first."
	}
}

$compiler = @(
	"$env:LOCALAPPDATA\Programs\Inno Setup 6\ISCC.exe",
	"${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
	"$env:ProgramFiles\Inno Setup 6\ISCC.exe"
) | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $compiler) {
	throw 'Inno Setup 6 is not installed (winget install --id JRSoftware.InnoSetup -e).'
}

$name = "Lumigram-Setup-x64-$version"
if ($Label) {
	$name += "-$Label"
}
New-Item -ItemType Directory -Force $outputPath | Out-Null
& $compiler `
	"/DMyAppVersion=$version" `
	"/DMyAppVersionFull=$version" `
	'/DMyBuildTarget=win64' `
	'/DUnsigned' `
	"/DReleasePath=$buildPath" `
	"/DMyOutputBaseFilename=$name" `
	"/O$outputPath" `
	(Join-Path $repository 'Telegram\build\setup.iss')
if ($LASTEXITCODE -ne 0) {
	throw "Inno Setup failed with exit code $LASTEXITCODE."
}
Join-Path $outputPath "$name.exe"
