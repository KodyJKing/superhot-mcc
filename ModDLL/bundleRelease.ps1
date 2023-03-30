$compress = @{
    Path = "./x64/Release/*.exe", "./x64/Release/*.dll", "./releaseFiles/*", "dll/include/FW1FontWrapper.dll", "../CREDITS.txt", "../LICENSE.txt"
    CompressionLevel = "Optimal"
    DestinationPath = "./SUPERHOTMCC.zip"
}
Compress-Archive @compress -Force