#pragma once

namespace HaloCE::Mod {

    inline struct Settings {
        float timeScale = 0.1f;
        bool enableTimeScale = true;
        bool overrideTimeScale = false;
        bool poseInterpolation = true;
        bool timescaleDeadzoning = false;
        bool shieldLimitedTimeScale = false;

        float playerDamageScale = 3.0f;
        float npcDamageScale = 2.0f;
    } settings = {};

    void init();
    void free();
    void modThreadUpdate();
    float getGlobalTimeScale();
}