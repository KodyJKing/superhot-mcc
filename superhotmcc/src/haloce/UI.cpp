#include "imgui.h"
#include "Mod.hpp"

namespace HaloCE::Mod::UI {

    void settings() {
        ImGui::Text( "Time Scale" );
        ImGui::SliderFloat( "##timeScale", &HaloCE::Mod::settings.timeScale, 0.0f, 1.0f );
    }

    void about() {
        // Nothing.
    }

    void debug() {

    }

}