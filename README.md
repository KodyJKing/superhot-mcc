Note, this mod also depends on a map mod which I haven't published yet. You don't *technically* need it, but gameplay will be unbalanced without it.

## Setup

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) and add MSBuild to your PATH. Location may vary. For me, it's located under `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`.

Don't forget to clone recursively to get submodules:

```powershell
git clone --recursive
```

## Workflow

When developing, run `scripts/watch_launcher.ps1` to build/run in watch mode. When you save a file, this script will uninject, rebuild, and reinject the mod DLL.

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
