[See it in action.](https://www.youtube.com/watch?v=wjiKeEh5J7g)

## Building

1. Install the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
2. Make sure the DXSDK_DIR environment variable is set.
3. Build with Visual Studio 2022. 

## Installing

1. Place `FW1FontWrapper.dll` (under ModDLL/dll/include) and `SuperhotMCC.ini` under `\mcc\binaries\win64\`.
2. If you're an end user, place `SUPERHOTMCC.dll` and `SuperhotMCCLauncher.exe` under the same directory.

## Running

1. Launch Halo MCC with EAC disabled.
2. Run SuperhotMCCLauncher.exe or inject SUPERHOTMCC.dll with a DLL injector.

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
| Print highlighted*     | P             |

**Printing a highlighted entity also copies its address to your clipboard.*

### Dev Cheat Table
| Option     | Key |
| ---------- | --- |
| Inject DLL | F10 |
| Unload PDB | F9  |
