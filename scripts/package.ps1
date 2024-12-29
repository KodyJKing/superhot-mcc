param(
    [string]$Config = "Debug",
    [string]$IDE = "vs2022",
    [string]$MCCPath = "C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection"
)

# Build the project.
& "./scripts/build.ps1" -Config $Config -IDE $IDE

# Create fresh package directory.
$PackagePath = "bin\$Config-Win64\package"
if (Test-Path $PackagePath) { Remove-Item -Path $PackagePath -Recurse -Force }
New-Item -Path $PackagePath -ItemType Directory -Force

Copy-Item -Path superhotmcc\shipfiles\* -Destination $PackagePath -Recurse -Force
Copy-Item -Path bin\$Config-Win64\superhotmcc\superhotmcc.dll -Destination $PackagePath\superhotmcc.dll -Force
Copy-Item -Path "$MCCPath\MCC\Binaries\Win64\xaudio2_9redist.dll" -Destination $PackagePath\xaudio2_9redist.dll -Force

# Add superhotmcc.dll to xaudio2_9redist.dll's import table.
Set-Location $PackagePath # Needs to be done from the package directory to avoid absolute import paths.
& "..\..\..\vendor\Detours\bin.X64\setdll.exe" "-d:superhotmcc.dll" "xaudio2_9redist.dll"
Set-Location "..\..\.."

# Zip the package directory.
$ConfigLower = $Config.ToLower()
$ZipPath = "bin\$Config-Win64\superhotmcc-$ConfigLower.zip"
Compress-Archive -Path $PackagePath\* -DestinationPath $ZipPath -Force

# Cleanup package directory.
Remove-Item -Path $PackagePath -Recurse -Force
