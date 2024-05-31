Document the process of reversing *everything* here so we can find things again after the game updates. Keep it sorted so steps can be reproduced in order.

## Player entity

Grenade counts live on the player entity, so the quickest way to find the player entity is to throw grenades and scan for changes.

## Damage entity function

The `damageEntity` function is an important entry point for analysis. After you found the player entity, you can find their health at offset `0x9C` and their shield at offset `0xA0`.

## Player handle global

Set a breakpoint at the start of the damage entity function and print `RCX` (the first parameter) to find the player handle.

There should be a global that holds the player entity handle. When you scan for it, there will be a few results. You need to test on a few levels to find the correct one.

## EntityArray and EntityRecordArray

Since halo doesn't pass raw pointers around, you will need a way to translate entity handles to entity pointers. The begining of the `damageEntity` function should contain an example of this translation code. The damaged entity handle will be the first parameter to the function.

`EntityRecordArray` contains a list of `EntityRecord`s, which among other things, contain an offset into the `EntityArray`, which contains the actual entity data. `EntityRecord.entityArrayOffset` is at offset `0x8`. `EntityRecord` is 12 bytes long.

See [[Entity Handle to Entity Pointer]] for the specifics of how lookup works. Also see `Halo1.cpp:getEntityPointer` and `Halo1.cpp:getEntityListPointer` (EntityRecordArray is also called EntityList for historical reasons).

## TagArray

There should be a global that points to the map's tag array.

To find this, look for instructions that access the first field of any entity (the entity's tagId). Load that function up in Ghidra and look for a calculation that adds that field to a global. You may have to step inside a function call to find the global.

See the Tag structure in `Halo1.hpp:Tag`.

## Map address translation

Things like tag data and tag names are stored in the map file. Tag headers point to these, but the addresses must be translated from the expected map address (`mapBase`) to the actual memory address (`relocatedMapBase`). We do this in `Halo1.cpp:translateMapAddress`. To find the `mapBase` and `relocatedMapBase` load up `damageEntity` in Ghidra and look for a similar calculation. You should find a parameter being added to the difference of two globals somewhere in the function.

## UpdateEntity and Update functions

To find this function, set a breakpoint in `damageEntity`. Step out of the function several times (atleast 6), saving each call in your Cheat Engine address list. After seeing an indirect call, stepping out one more time will take you into the updateEntity function (this could change).

The call right above updateEntity will be the update function.

## PlayerController

`PlayerController` contains things like player angles and input data. It also contains a handle to the target under their reticle for some reason. Looking for this is the easiest way to find this structure.

### Find `PlayerController.targetHandle` in memory.
This field gives the handle to an enemy entity under your reticle if it turns your reticle red. The value is `FFFFFFFF` when no target is under the reticle. Scan for a 32-bit value with unknown initial value.

#### Verifying

You will find several results with the same value. 

Memory browse with floating point view and look for one that seems to contain your look angles. Look angles will be in radians with yaw in [0, 2pi] and pitch in [-pi/2, pi/2]. Yaw should be at offset `0x1C`.

The `PlayerController.targetHandle` should be at offset `0x1A4`, so verify that some instruction accesses the address you found with that offset.

The correct structure will also have the player handle at offset `0x10`.

# See Also

[Kavawuvi's map file documentation](https://opencarnage.net/index.php?/topic/6693-halo-map-file-structure-revision-212/)