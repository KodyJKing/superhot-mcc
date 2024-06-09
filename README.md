## Setup

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) and add MSBuild to your PATH. Location may vary. For me, it's located under `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`.

Don't forget to clone recursively to get submodules:

```powershell
git clone --recursive
```

## Workflow

When developing, run `scripts/watch_launcher.ps1` to build/run in watch mode. When you save a file, this script will uninject, rebuild, and reinject the mod DLL.

For the moment, you will need to reinject if you exit a Halo 1 map and then re-enter it. This is because the game unloads `halo1.dll` when you exit to menu. Reinitialization will be implemented soon.

## Scripts

### `build.ps1`

Builds mod once.

Arguments:

- `-Config` Configuration to build. Values are `Debug` and `Release`. Default is `Debug`.
- `-IDE` IDE premake will generate project files for. Default is `vs2022`.

### `watch_build.ps1`

Builds mod and recompiles on file change. 

Press `R` in the terminal to rebuild without waiting for a file change.

Arguments:

- Inherits from `build.ps1`.

### `watch_launcher.ps1`

Builds mod and runs launcher. Uninjects mod from game, recompiles and reruns launcher on file change. 

Press `R` in the terminal to rebuild without waiting for a file change.

Arguments:

- Inherits from `watch_build.ps1`.

- `-Arguments` Arguments to pass to the launcher executable. Optional.
