Document the process of reversing *everything* here so we can find things again after the game updates. Keep it sorted so steps can be reproduced in order.

## Player entity

Grenade counts live on the player entity. Finding them is the fastest way to find the player entity. They are single byte values. Frags should be at offset `0x2FC`.

## Damage entity function

The `damageEntity` function is an important entry point for analysis. After you found the player entity, you can find their health at offset `0x9C` and their shield at offset `0xA0`. Just set a data breakpoint on health and step on a grenade.

## EntityArray and EntityRecordArray

Rather than passing around raw pointers, halo uses 32-bit handles which are used to index entities. We need two globals to translate handles to pointers: `EntityArray` and `EntityRecordArray`. You can find these in the `damageEntity` function. The first thing the function does is translate the handle for the damaged entity to a pointer. The handle is the first parameter to the function.

See [[Entity Handle to Entity Pointer]] for the specifics of how lookup works. Also see `Halo1.cpp:getEntityPointer` and `Halo1.cpp:getEntityListPointer` (EntityRecordArray is also called EntityList for historical reasons).

## Player handle global

Set a breakpoint at the start of the damage entity function and print `RCX` (the first parameter) to find the player handle.

There should be a global that holds the player entity handle. When you scan for it, there will be a few results. You need to test on a few levels to find the correct one.


## TagArray

There should be a global that points to the map's tag array.

To find this, look for instructions that access the first field of any entity (the entity's tagId). Load that function up in Ghidra and look for a calculation that adds that field to a global. You may have to step inside a function call to find the global.

See `Halo1.hpp:Tag` and `Halo1.cpp:getTag`.

## Map address translation

Things like tag data and tag names are stored in the map file. Tag headers point to these, but the addresses must be translated from the expected map address (`mapBase`) to the actual memory address (`relocatedMapBase`). We do this in `Halo1.cpp:translateMapAddress`. To find the `mapBase` and `relocatedMapBase` load up `damageEntity` in Ghidra and look for a similar calculation. You should find a parameter being added to the difference of two globals somewhere in the function.

## UpdateEntity and Update functions

To find `updateEntity`, set a breakpoint in `damageEntity`. Step out of the function several times (atleast 6), saving each call in your Cheat Engine address list. After seeing an indirect call, stepping out one more time will take you into the updateEntity function (this could change).

The call right above `updateEntity` will be the `update` function.

## PlayerController

`PlayerController` contains things like player angles and input data. It also contains a handle to the target under their reticle for some reason. Looking for this is the easiest way to find this structure.

You will find several results with the same value. 

Memory browse with floating point view and look for one that seems to contain your look angles. Look angles will be in radians with yaw in [0, 2pi] and pitch in [-pi/2, pi/2]. Yaw should be at offset `0x1C`.

The `PlayerController.targetHandle` should be at offset `0x1A4`, so verify that some instruction accesses the address you found with that offset.

The correct structure will also have the player handle at offset `0x10`.

## Camera

The easiest way to find the camera structure is to scan for fov changes as you change zoom on a sniper/pistol. The fov is stored in radians. It is at offset `0x20` from the camera structure. The camera is a global, so it will be at a static address (green results in Cheat Engine). You may see another fov value before the camera structure. The one you want will be after position, forward and up vectors. See `Halo1.hpp:Camera`.

# See Also

[Kavawuvi's map file documentation](https://opencarnage.net/index.php?/topic/6693-halo-map-file-structure-revision-212/)