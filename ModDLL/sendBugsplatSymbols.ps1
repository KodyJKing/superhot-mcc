$buildDir = ".\x64\Release\"
$modDllPath = $buildDir + "SUPERHOTMCC.dll"
$version = (Get-Command $modDllPath).FileVersionInfo.FileVersion

$sendPdbs_path = $env:BUGSPLAT_DIR + "..\Tools\SendPdbs.exe"

& $sendPdbs_path /b "kody.j.king@gmail.com" /a "Superhot-MCC" /v $version /d $buildDir /f "SUPERHOTMCC.*" /verbose