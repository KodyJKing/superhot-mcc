- Port activity based time scaling.
    - PlayerController
        - Port to C++.
        - Reverse engineer the `actions` field.
            - Controls shoot, jump, grenade, crouch, melee, flashlight
            - Doesn't control walk, zoom or look.

- Patch hitscan (misnomer) projectiles to have lower speed.

- Find a way to timescale first person weapon animations.
    - Look for \fp\fp in tag browser. Set a data breakpoint on an animation's tag.