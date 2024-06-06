#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        float timeScale = 0.1f;
        bool enableTimeScale = true;
        bool overrideTimeScale = false;
        bool poseInterpolation = true;
        bool timescaleDeadzoning = false;
        bool shieldLimitedTimeScale = false;
    } settings = {};

    void init();
    void free();
    void modThreadUpdate();
    float getGlobalTimeScale();
}