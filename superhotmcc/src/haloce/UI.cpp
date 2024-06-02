#include "imgui.h"
#include "Mod.hpp"
#include "overlay/ESP.hpp"
#include "UI.hpp"
#include "Halo1.hpp"
#include "halomcc/HaloMCC.hpp"
#include <iostream>
#include <vector>

namespace HaloCE::Mod::UI {

    void settings() {
        ImGui::SliderFloat( "Time Scale", &HaloCE::Mod::settings.timeScale, 0.0f, 1.0f );
    }

    void debug() {}

    /////////////////////////////////////////////////////////////////////////////////////////////
    // ESP

    struct ESPSettings {
        bool anchorHighlight = false;
        float fovScale = 0.637f;
    } espSettings = {};

    Halo1::Entity* highlightEntity = nullptr;

    void espWindow() {
        bool unpaused = !HaloMCC::isGamePaused();
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;
        if (unpaused) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.35f);
            flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
        }

        ImGui::Begin("ESP", 0, flags);
        
        // Tabs
        if (ImGui::BeginTabBar("ESP Tabs")) {
            if (ImGui::BeginTabItem("Entities")) {

                ImGui::Text("%p", highlightEntity);

                // Copy button
                ImGui::SameLine();
                bool copyHotkeyDown = ImGui::GetIO().KeyShift && ImGui::IsKeyPressed( ImGuiKey_C, true );
                if (copyHotkeyDown) ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 255, 0, 255));
                if (ImGui::Button("copy") || copyHotkeyDown) {
                    char text[255] = {0};
                    snprintf( text, 255, "%p", highlightEntity );
                    ImGui::SetClipboardText( text );
                }
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Copy pointer to clipboard (Shift+C)");
                if (copyHotkeyDown) ImGui::PopStyleColor();

                ImGui::SameLine();
                ImGui::Checkbox("anchor", &espSettings.anchorHighlight);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Keep current entity highlighted (Shift-V)");
                bool anchorHotkeyDown = ImGui::GetIO().KeyShift && ImGui::IsKeyPressed( ImGuiKey_V, true );
                if (anchorHotkeyDown) espSettings.anchorHighlight = !espSettings.anchorHighlight;

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Camera")) {
                Camera& camera = Overlay::ESP::camera;
                ImGui::Text( "Pos: %.2f %.2f %.2f", camera.pos.x, camera.pos.y, camera.pos.z );
                ImGui::Text( "Fwd: %.2f %.2f %.2f", camera.fwd.x, camera.fwd.y, camera.fwd.z );
                // ImGui::Text( "Up: %.2f %.2f %.2f", camera.up.x, camera.up.y, camera.up.z );
                ImGui::Text( "Fov: %.2f", camera.fov );
                // ImGui::Text("Screen: %.2f x %.2f", camera.width, camera.height);
                if (!unpaused)
                    ImGui::SliderFloat("Fov Adjust", &espSettings.fovScale, 0.0f, 2.0f);
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();

        if (unpaused) {
            ImGui::PopStyleVar();

        }
    }

    void esp() {

        if (!Halo1::isGameLoaded())
            return;

        espWindow();

        auto haloCam = Halo1::getPlayerCameraPointer();

        namespace ESP = Overlay::ESP;

        Camera& camera = ESP::camera;

        camera.pos = haloCam->pos;
        camera.fwd = haloCam->fwd;
        camera.up = haloCam->up;
        camera.fov = haloCam->fov * espSettings.fovScale;
        camera.verticalFov = true;

        std::vector<Halo1::Entity*> entitiesToDraw;
        Halo1::foreachEntityRecord( [&]( Halo1::EntityRecord* rec ) {
            auto entity = rec->entity();
            if (entity->entityCategory != Halo1::EntityCategory_Biped)
                return;
            entitiesToDraw.push_back( entity );
        } );


        if (!espSettings.anchorHighlight) {
            int highlightIndex = -1;
            float maxDot = -1.0f;
            for (int i = 0; i < entitiesToDraw.size(); i++) {
                auto entity = entitiesToDraw[i];
                auto toEntity = entity->pos - camera.pos;
                // if (toEntity.z < 0.1f)
                //     continue;
                toEntity = toEntity.normalize();
                float dot = toEntity.dot( camera.fwd );
                if (dot > maxDot) {
                    maxDot = dot;
                    highlightIndex = i;
                }
            }
            highlightEntity = highlightIndex >= 0 ? entitiesToDraw[highlightIndex] : nullptr;
        }

        for (Halo1::Entity* entity : entitiesToDraw) {
            auto color = IM_COL32( 255, 255, 255, 255 );
            float radius = 0.1f;
            if (entity == highlightEntity) {
                color = IM_COL32( 255, 255, 0, 255 );
                radius = 0.2f;
            }
            ESP::drawCircle( entity->pos, radius, color, true );
        }

    }

}