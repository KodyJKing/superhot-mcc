param(
    [string]$Config = "Debug",
    [string]$Arguments = ""
)

$ArgumentList = $Arguments -split " "

& "./bin/$Config-Win64/superhotmcc-launcher/superhotmcc-launcher.exe" $ArgumentList | Out-Default
