- Port activity based time scaling.
    - PlayerController
        - Port to C++.
        - Reverse engineer the `actions` field.
            - Controls shoot, jump, grenade, crouch, melee, flashlight
            - Doesn't control walk, zoom or look.

    - Hook the update function (the one calls updateEntity). We need this to do entity independent updates like updating time-scale.

- Implement animation interpolation.
    - Find offset and number of bone poses in entity class (varies by entity type iirc).
    - When `animFrame` updates, save the `relativePose` between the old and new frame. Do not let `animFrame` advance.
    - Each update, scale any unexpected pose changes by timescale and then apply `relativePose` (scaled by timescale).
    - Clear `relativePose` and advance `animFrame` after full motion has been applied.
    - Turn off interpolation when timescale is above cutoff (0.95?).
