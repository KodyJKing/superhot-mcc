[See it in action.](https://www.youtube.com/watch?v=8FDzfyl4kzY)

## Setup

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) and add MSBuild to your PATH. Location may vary. For me, it's located under `C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin`.

Don't forget to clone recursively to get submodules:

```powershell
git clone --recursive
```

For the intended experience, you will need to use [these maps](https://drive.google.com/file/d/1HuGOeBXWkw4GbMptUdh8bU79_zxIDhpp/view?usp=drive_link). Just back up your existing maps and replace them with these. They should be under `<MCCPath>/halo1/maps`. The map mods limit the speed of "hitscan" projectiles and add tracers to them. Eventually, I'll keep diffs for the modified tag files directly in this repo.

## Building and Running

```powershell
scripts/build.ps1
scripts/run_launcher.ps1
```

Halo MCC will need to be running when you run `run_launcher.ps1`.

## Workflow

When developing, run `scripts/watch_launcher.ps1` to build/run in watch mode. When you save a source file, this script will uninject, rebuild, and reinject the mod DLL.

If you're using Cheat Engine as part of your workflow, turn off symbols in the debug build configuration (`superhotmcc/premake5.lua`). Otherwise, Cheat Engine will hold on to the PDB file and prevent rebuilding.

## Scripts

All scripts assume you are running from the root of the repository.

### `build.ps1`

Builds mod once.

Arguments:

- `-Config` Configuration to build. Values are `Debug` and `Release`. Default is `Debug`.
- `-IDE` IDE premake will generate project files for. Default is `vs2022`.

### `run_launcher.ps1`

Injects mod into game once. MCC must already be running.

Arguments:

- `-Config` Configuration to run. Values are `Debug` and `Release`. Default is `Debug`.

- `-Arguments` Arguments to pass to the launcher executable. Optional.

### `watch_build.ps1`

Builds mod and recompiles on file change. 

Press `R` in the terminal to rebuild without waiting for a file change.

Arguments:

- Inherits from `build.ps1`.

### `watch_launcher.ps1`

Builds mod and runs launcher. Uninjects mod from game, recompiles and reruns launcher on file change. 

Press `R` in the terminal to rebuild without waiting for a file change.

Arguments:

- Inherits from `watch_build.ps1` and `run_launcher.ps1`.

### `package.ps1`

Packages mod into a `.zip` file. Creates a copy of xaudio2_9redist.dll that imports the mod.

Arguments:

- Inherits from `build.ps1`.

### `install_package.ps1`

Runs `package.ps1` and installs the mod into the game. This script is for testing, not for end users.

Arguments:

- Inherits from `package.ps1`.
- `-MCCPath` Path to the root of the MCC installation. Default is `C:\Program Files (x86)\Steam\steamapps\common\Halo The Master Chief Collection`.
- `-Uninstall` Uninstalls the mod instead of installing it.
