#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        float timeScale = 0.1f;
        bool enableTimeScale = false;
    } settings = {};

    void init();
    void free();
    void modThreadUpdate();
}