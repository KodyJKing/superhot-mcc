## Building

1. Install the [DirectX SDK](https://www.microsoft.com/en-us/download/details.aspx?id=6812)
2. Make sure the DXSDK_DIR environment variable is set.
3. Build with Visual Studio 19+. 

## Installing

1. Place `FW1FontWrapper.dll` under `\mcc\binaries\win64\`.
2. If you're an end user, place `SUPERHOTMCC.dll` under the same directory.

## Running

1. Launch Halo MCC with EAC disabled.
2. Disable fullscreen mode if active, you can use borderless windowed instead for fullscreen.
3. Load a Halo 1 level.
4. Use a DLL injector to inject `SUPERHOTMCC.dll` into `mcc-win64-shipping.exe`. I use cheat engine's DLL injector.

## Hotkeys

| Option              | Key |
| -------------       |---- |
| Unload mod DLL      | F9  |
| Toggle time scaling | F3  |
| Toggle freeze time  | F2  |
| Single step time    | F1  |

### Debug overlay
| Option                 | Key           |
| -------------          | ------------- |
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