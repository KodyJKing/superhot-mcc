#include "imgui.h"
#include "Mod.hpp"
#include "overlay/ESP.hpp"
#include "UI.hpp"
#include "Halo1.hpp"
#include "halomcc/HaloMCC.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <thread>
#include "utils/Strings.hpp"
#include "memory/Memory.hpp"
#include "TimeScale.hpp"
#include "TagBrowser.hpp"

namespace HaloCE::Mod::UI {

    void renderESP();
    void checkHotKeys();
    void devTab();

    bool showEsp = false;

    void topLevelRender() {
        checkHotKeys();

        if (showEsp) {
            Overlay::ESP::beginESPWindow("__ESP");
            renderESP();
            Overlay::ESP::endESPWindow();
        }
    }

    void checkHotKeys() {
        if (ImGui::IsKeyPressed( ImGuiKey_F1, false ))
            showEsp = !showEsp;
        if (ImGui::IsKeyPressed( ImGuiKey_F2, false ))
            HaloCE::Mod::settings.enableTimeScale = !HaloCE::Mod::settings.enableTimeScale;
        if (ImGui::IsKeyPressed( ImGuiKey_F3, false ))
            HaloCE::Mod::settings.poseInterpolation = !HaloCE::Mod::settings.poseInterpolation;
        if (ImGui::IsKeyPressed( ImGuiKey_F4, false ))
            HaloCE::Mod::settings.timescaleDeadzoning = !HaloCE::Mod::settings.timescaleDeadzoning;
        if (ImGui::IsKeyPressed( ImGuiKey_F5, false ))
            HaloCE::Mod::settings.shieldLimitedTimeScale = !HaloCE::Mod::settings.shieldLimitedTimeScale;
    }


    void mainWindowTabs() {
        if (ImGui::BeginTabItem("Dev")) {
            devTab();
            ImGui::EndTabItem();
        }
    }

    void devTab() {

        if (ImGui::CollapsingHeader("Damage Scale", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::SliderFloat("Player", &HaloCE::Mod::settings.playerDamageScale, 0.0f, 10.0f, "%.1f");
            ImGui::SliderFloat("NPC", &HaloCE::Mod::settings.npcDamageScale, 0.0f, 10.0f, "%.1f");
        }

        if (ImGui::CollapsingHeader("Time Scale")) {
            ImGui::Checkbox("Enable Time Scale", &HaloCE::Mod::settings.enableTimeScale);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable time scaling (F2)");

            ImGui::Checkbox("Override Time Scale", &HaloCE::Mod::settings.overrideTimeScale);
            int timeScalePercent = (int) (HaloCE::Mod::settings.timeScale * 100.0f);
            ImGui::PushItemWidth(200);
            ImGui::SliderInt( "Scale", &timeScalePercent, 0, 100, "%d%%" );
            ImGui::PopItemWidth();
            HaloCE::Mod::settings.timeScale = timeScalePercent / 100.0f;

            ImGui::Checkbox("Time Scale Deadzoning", &HaloCE::Mod::settings.timescaleDeadzoning);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Deadzone time scaling to prevent game from freezing (F4)");

            ImGui::Checkbox("Pose Interpolation", &HaloCE::Mod::settings.poseInterpolation);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Enable pose interpolation (F3)");

            ImGui::Checkbox("Shield Limited Time Scale", &HaloCE::Mod::settings.shieldLimitedTimeScale);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Limit time scaling when shields are down (F5)");
        }

        if (ImGui::CollapsingHeader("Tools")) {
            // Translate map address
            ImGui::BeginChild("##Translate Map Address", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
                ImGui::Text("Translate Map Address");
                static char address[255] = {0};
                ImGui::InputText("##Address", address, 255);
                static uint32_t mapAddress = 0;
                try {
                    mapAddress = std::stoul( address, nullptr, 16 );
                } catch (...) {
                    mapAddress = 0;
                }
                if (mapAddress) {
                    uint64_t translated = Halo1::translateMapAddress( mapAddress );
                    ImGui::Text("-> %p", (void*) translated);
                    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy translated address to clipboard");
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                        char text[255] = {0};
                        snprintf( text, 255, "%p", (void*) translated );
                        ImGui::SetClipboardText( text );
                    }
                }
            ImGui::EndChild();

            if (ImGui::Button("Tag Browser"))
                showTagBrowser = !showTagBrowser;
            if (showTagBrowser) tagBrowser();

            ImGui::Checkbox("ESP", &showEsp);
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("Toggle ESP (F1)");
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // ESP
    // Todo: Move into Halo1ESP.cpp

    struct ESPSettings {
        bool anchorHighlight = false;
        float fovScale = 0.637f;
        float maxDistance = 100.0f;
        struct Filter {
            bool biped = true;
            bool vehicle = false;
            bool weapon = false;
            bool projectile = false;
            bool scenery = false;
        } filter = {};
    } espSettings = {};

    bool shouldDisplay(Halo1::Entity* entity) {
        auto c = (Halo1::EntityCategory) entity->entityCategory;
        if (c == Halo1::EntityCategory_Biped)      return espSettings.filter.biped;
        if (c == Halo1::EntityCategory_Vehicle)    return espSettings.filter.vehicle;
        if (c == Halo1::EntityCategory_Weapon)     return espSettings.filter.weapon;
        if (c == Halo1::EntityCategory_Projectile) return espSettings.filter.projectile;
        if (c == Halo1::EntityCategory_Scenery)    return espSettings.filter.scenery;
        return false;
    }

    void filterSettings() {
        ImGui::Checkbox("Biped", &espSettings.filter.biped);
        ImGui::Checkbox("Vehicle", &espSettings.filter.vehicle);
        ImGui::Checkbox("Weapon", &espSettings.filter.weapon);
        ImGui::Checkbox("Projectile", &espSettings.filter.projectile);
        ImGui::Checkbox("Scenery", &espSettings.filter.scenery);
    
    }

    Halo1::Entity* highlightEntity = nullptr;

    void highlightedEntityDetails() {
        Halo1::Entity* entity = highlightEntity;

        if (entity == nullptr || !Memory::isAllocated( (uintptr_t) entity ) )
            return;

        static struct View {
            bool pos = true;
            bool vel;
            bool health;
            bool shield;
            bool tag;
            bool tagID;
            bool tagCC;
            bool tagPath = true;
            bool animation;
            bool bones;
        } view = {};

        bool paused = HaloMCC::isGamePaused();

        #define VIEW_TOGGLE( name ) if (paused) { ImGui::Checkbox("##" #name, &view.name); ImGui::SameLine(); }

        auto tag = entity->tag();
        if ( (!paused || ImGui::CollapsingHeader("Tag")) && tag ) {
            VIEW_TOGGLE(tag);
            if (paused || view.tag) ImGui::Text("Tag: %p", tag);

            VIEW_TOGGLE(tagID);
            if (paused || view.tagID) ImGui::Text("Tag ID: %X", entity->tagID);

            std::string groupID = tag->groupIDStr();
            VIEW_TOGGLE(tagCC);
            if (paused || view.tagCC) ImGui::Text("Tag GroupID: %s", groupID.c_str());
            
            VIEW_TOGGLE(tagPath);
            const char* path = tag->getResourcePath();
            const char* nullPath = "null";
            if (!path) path = nullPath;
            if (paused || view.tagPath) ImGui::Text("Tag Path: %s", path);
        }

        if (!paused || ImGui::CollapsingHeader("Entity") ) {
            auto pos = entity->pos;
            auto vel = entity->vel;
            VIEW_TOGGLE(pos);
            if (paused || view.pos) ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
            VIEW_TOGGLE(vel);
            if (paused || view.vel) ImGui::Text("Velocity: %.2f, %.2f, %.2f", vel.x, vel.y, vel.z);
            
            VIEW_TOGGLE(health);
            if (paused || view.health) ImGui::Text("Health: %.2f", entity->health);
            VIEW_TOGGLE(shield);
            if (paused || view.shield) ImGui::Text("Shield: %.2f", entity->shield);
            
            VIEW_TOGGLE(animation);
            if (paused || view.animation) ImGui::Text("AnimSet: %X, Anim: %d, Frame: %d, %d bones", entity->animSetTagID, entity->animId, entity->animFrame, entity->boneCount());

            VIEW_TOGGLE(bones);
            if (paused || view.bones) {
                auto boneTransforms = entity->getBoneTransforms();
                ImGui::Text("Bones: %p", boneTransforms);
                if (view.bones && boneTransforms) {
                    ImGui::BeginChild("Bones", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);
                        for (int i = 0; i < entity->boneCount(); i++) {
                            auto bone = boneTransforms[i];
                            auto pos = bone.translation;
                            auto rot = bone.rotation;
                            ImGui::Text("%02d {%.2f, %.2f, %.2f, %.2f} {%.2f, %.2f, %.2f}", i, rot.x, rot.y, rot.z, rot.w, pos.x, pos.y, pos.z);
                        }
                    ImGui::EndChild();
                }
            }
        }

    }

    void espWindow() {
        bool paused = HaloMCC::isGamePaused();
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
        if (!paused)
            flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove 
                  | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar
                  | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs;

        ImGui::Begin("ESP", 0, flags);
        
        // Tabs
        if (ImGui::BeginTabBar("ESP Tabs")) {
            
            if (ImGui::BeginTabItem("Entities")) {
                ImGui::Text("%p", highlightEntity);

                // Copy button
                ImGui::SameLine();
                bool copyHotkeyDown = ImGui::GetIO().KeyShift && ImGui::IsKeyPressed( ImGuiKey_C, false );
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
                bool anchorHotkeyDown = ImGui::GetIO().KeyShift && ImGui::IsKeyPressed( ImGuiKey_V, false );
                if (anchorHotkeyDown) espSettings.anchorHighlight = !espSettings.anchorHighlight;

                if (paused && ImGui::CollapsingHeader("Filter")) {
                    int maxDistInt = (int) espSettings.maxDistance;
                    ImGui::SliderInt("Max Distance", &maxDistInt, 1, 200, "%d");
                    espSettings.maxDistance = (float) maxDistInt;
                    filterSettings();
                }

                highlightedEntityDetails();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Camera")) {
                if (paused) ImGui::SliderFloat("Fov Adjust", &espSettings.fovScale, 0.0f, 2.0f);
                Camera& camera = Overlay::ESP::camera;
                ImGui::Text( "Pos: %.2f %.2f %.2f", camera.pos.x, camera.pos.y, camera.pos.z );
                ImGui::Text( "Fwd: %.2f %.2f %.2f", camera.fwd.x, camera.fwd.y, camera.fwd.z );
                // ImGui::Text( "Up: %.2f %.2f %.2f", camera.up.x, camera.up.y, camera.up.z );
                ImGui::Text( "Fov: %.2f", camera.fov );
                // ImGui::Text("Screen: %.2f x %.2f", camera.width, camera.height);
                ImGui::EndTabItem();
            }

            #define PLAYER_CONTROLLER_TAB
            #ifdef PLAYER_CONTROLLER_TAB
            if (ImGui::BeginTabItem("PlayerController")) {
                auto playerController = Halo1::getPlayerControllerPointer();
                ImGui::Text("%p", playerController);

                uint32_t actions = playerController->actions;
                for (int i = 0; i < 32; i++) {
                    if (i > 0 && i % 8 != 0)
                        ImGui::SameLine();
                    char text[255] = {0};
                    snprintf( text, 255, "##flag%d", i );
                    ImGui::CheckboxFlags( text, &actions, 1 << i );
                }

                ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                #define SHOW_ACTION_1( name ) ImGui::TextColored( (actions & Halo1::PlayerActionFlags::name) ? green : white, #name);
                #define SHOW_ACTION_2( name ) ImGui::SameLine(); SHOW_ACTION_1( name );
                SHOW_ACTION_1(crouch);
                SHOW_ACTION_2(jump);
                SHOW_ACTION_2(flash);
                SHOW_ACTION_2(melee);
                SHOW_ACTION_1(reload);
                SHOW_ACTION_2(shoot);
                SHOW_ACTION_2(grenade1);
                SHOW_ACTION_2(grenade2);
                #undef SHOW_ACTION_2
                #undef SHOW_ACTION_1

                ImGui::Text("WalkX %.2f", playerController->walkX);
                ImGui::Text("WalkY %.2f", playerController->walkY);

                ImGui::EndTabItem();
            }
            #endif

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    Vec3 displayPos(Halo1::Entity* entity) {
        return entity->rootBonePos;
        // if (
        //     // entity->entityCategory == Halo1::EntityCategory_Scenery ||
        //     entity->parentHandle != NULL_HANDLE
        // )
        //     return entity->rootBonePos;
        // return entity->pos;
    }

    void renderESP() {

        if (!Halo1::isGameLoaded())
            return;

        ImGui::ProgressBar( getGlobalTimeScale(), ImVec2(200.0f, 0.0f) );

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
            if (shouldDisplay( entity )) {
                Vec3 pos = displayPos( entity );
                auto toEntity = pos - camera.pos;
                if (toEntity.length() < espSettings.maxDistance)
                    entitiesToDraw.push_back( entity );
            }
        } );

        if (!espSettings.anchorHighlight) {
            int highlightIndex = -1;
            float maxDot = -1.0f;
            for (int i = 0; i < entitiesToDraw.size(); i++) {
                auto entity = entitiesToDraw[i];
                Vec3 pos = displayPos( entity );
                auto toEntity = pos - camera.pos;
                if (toEntity.length() > espSettings.maxDistance) continue;
                toEntity = toEntity.normalize();
                float dot = toEntity.dot( camera.fwd );
                if (dot > maxDot) {
                    maxDot = dot;
                    highlightIndex = i;
                }
            }
            highlightEntity = highlightIndex >= 0 ? entitiesToDraw[highlightIndex] : nullptr;
        }

        bool gamePaused = HaloMCC::isGamePaused();
        byte alpha = gamePaused ? 0x40 : 0xFF;
        for (Halo1::Entity* entity : entitiesToDraw) {
            auto color = IM_COL32( 255, 255, 255, alpha );
            float radius = 0.1f;
            if (entity == highlightEntity) {
                color = IM_COL32( 64, 64, 255, alpha );
                radius = 0.2f;
            }
            Vec3 pos = displayPos( entity );
            ESP::drawCircle( pos, radius, color, true );
        }

    }

}