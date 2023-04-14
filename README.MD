### [See it in action.](https://www.youtube.com/watch?v=wjiKeEh5J7g)

## Building Debug

1. Install the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
2. Make sure the DXSDK_DIR environment variable is set.
3. Build with Visual Studio 2022.

## Building Release

1. Download the [BugSplat Native SDK](https://app.bugsplat.com/browse/download_item.php?item=native) and set the environment variable `BUGSPLAT_DIR` to point at `../BugSplatNative/BugSplat`
2. Follow the instructions from `Building Debug`

## Dev Install

To make sure you have all the necessary DLL dependencies, download the latest release and follow the instructions in the README.

## Running

1. Launch Halo MCC with EAC disabled.
2. To inject SUPERHOTMCC.dll either:
    - run SuperhotMCCLauncher.exe
    - or inject SUPERHOTMCC.dll with a DLL injector[^1]
    
[^1]: The cheat table in this repository has a hotkeys for injecting the Debug and Release DLLs. See `Dev Cheat Table`.

## Usage

This mod is developed for the Steam version of MCC. It probably won't work on the Window's Store version.

Only CE's single player campaign is supported at the moment.

Currently, you need to use an FOV of 90 for tracers to render at the right position. I'm working on a fix for that.

## Hotkeys

| Option              | Key |
| ------------------- | --- |
| Unload mod DLL      | F9  |
| Toggle time scaling | F3  |
| Toggle freeze time  | F2  |
| Single step time    | F1  |

### Debug overlay
| Option                 | Key           |
| ---------------------- | ------------- |
| Show overlay           | Numpad 1      |
| Show highlighted only  | Numpad 2      |
| Show all entity types  | Numpad 3      |
| Print highlighted[^2]     | P             |

[^2]: Printing a highlighted entity also copies its address to your clipboard.

### Dev Cheat Table
| Option             | Key |
| ------------------ | --- |
| Inject debug DLL   | F10 |
| Inject release DLL | F8  |
| Unload PDB         | F9  |
