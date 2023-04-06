$bugSplatBin = $env:BUGSPLAT_DIR + "x64\Release\"

$buildDir = ".\x64\Release\"
$modDllPath = $buildDir + "SUPERHOTMCC.dll"
$version = (Get-Command $modDllPath).FileVersionInfo.FileVersion

New-Item -ItemType Directory -Force -Path ".\out\"
$outpath = ".\out\SUPERHOTMCC-" + $version + ".zip"

$compress = @{
    Path = 
        ($buildDir + "*.exe"), 
        ($buildDir + "*.dll"), 
        ".\releaseFiles\*",
        "..\CREDITS.txt", 
        "..\LICENSE.txt", 
        "dll\include\FW1FontWrapper.dll", 
        ($bugSplatBin + "BugSplat64.dll"),
        ($bugSplatBin + "BsSndRpt64.exe"),
        ($bugSplatBin + "BugSplatRc64.dll")
    CompressionLevel = "Optimal"
    DestinationPath = $outpath
}
Compress-Archive @compress -Force