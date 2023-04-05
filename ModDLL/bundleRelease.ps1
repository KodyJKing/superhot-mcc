$bugSplatBin = $env:BUGSPLAT_DIR + "x64/Release/"

$compress = @{
    Path = 
        "./x64/Release/*.exe", "./x64/Release/*.dll", 
        "./releaseFiles/*",
        "../CREDITS.txt", "../LICENSE.txt", 
        "dll/include/FW1FontWrapper.dll", 
        ($bugSplatBin + "BugSplat64.dll"),
        ($bugSplatBin + "BsSndRpt64.exe"),
        ($bugSplatBin + "BugSplatRc64.dll")
    CompressionLevel = "Optimal"
    DestinationPath = "./SUPERHOTMCC.zip"
}
Compress-Archive @compress -Force