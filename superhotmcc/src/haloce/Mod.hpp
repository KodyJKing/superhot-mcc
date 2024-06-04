#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        float timeScale = 0.1f;
        bool enableTimeScale = true;
        bool poseInterpolation = true;
    } settings = {};

    void init();
    void free();
    void modThreadUpdate();
}