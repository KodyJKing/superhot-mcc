param (
    [string] $Path,
    [string[]] $ArgumentList
)

# Run the game, and relaunch if it exits or crashes. Run from the game's install directory (path).

$workingDir = Split-Path -Path $Path -Parent
Write-Host "Running $Path with args: $ArgumentList from $workingDir"
do {
    Start-Process -FilePath $Path -WorkingDirectory $workingDir -ArgumentList $ArgumentList -Wait
} while ($LASTEXITCODE -ne 0)
