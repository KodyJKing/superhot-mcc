- Port activity based time scaling.
    - PlayerController
        - ✓ Port to C++.
        - ✓ Reverse engineer the `actions` field.
        - Replace calls to `GetAsyncKeyState`.

- Find a way to timescale first person weapon animations.
    - Look for \fp\fp in tag browser. Set a data breakpoint on an animation's tag.

- Look through the updateEntity function for discrete updates like when entities randomly update their look direction. Time scale these updates by skipping them at a certain rate or probability.

- ✓ Apply timescaling to projectile trails.

- Make map mods to that make game look like superhot.
    - ✓ Replace bullet contrails with a red line. 
    - Replace textures to make the environment white, weapons/projectiles black, and enemies/trails red.


✓