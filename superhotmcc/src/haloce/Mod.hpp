#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        float timeScale = 0.5f;
    } settings = {};

    void init();
    void free();
    void modThreadUpdate();
}