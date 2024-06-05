#include <Windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "Overlay.hpp"
#include "DX11Hook.hpp"
#include "halomcc/HaloMCC.hpp"
#include "haloce/UI.hpp"
#include "Licenses.hpp"
#include "overlay/ESP.hpp"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Overlay {

    HWND gameWindow;

    HWND getGameWindow() {
        return gameWindow;
    }

    struct Settings {
        bool showESP = false;
    } settings = {};

    void initializeContext(HWND targetWindow) {
        if (ImGui::GetCurrentContext( ))
            return;
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(targetWindow);
    }

    void credits() {
        ImGui::SeparatorText("Credits");
        if ( ImGui::CollapsingHeader("Dear ImGui") ) ImGui::TextWrapped(Licenses::imGui);
        if ( ImGui::CollapsingHeader("AsmJit") ) ImGui::TextWrapped(Licenses::asmJit);
        if ( ImGui::CollapsingHeader("MinHook") ) ImGui::TextWrapped(Licenses::minHook);
        if ( ImGui::CollapsingHeader("UniversalHookX") ) ImGui::TextWrapped(Licenses::universalHookX);
        ImGui::TextWrapped("Thanks to Kavawuvi for their documentation of the Halo CE map and tag format.");
    }

    void mainModWindow() {
        ImGui::Begin("Superhot MCC");

        // Default window size
        auto winSize = ImVec2(300, 400);
        ImGui::SetWindowSize(winSize, ImGuiCond_Once);
        // Default position to bottom right
        auto displaySize = ImGui::GetIO().DisplaySize;
        ImGui::SetWindowPos(ImVec2(displaySize.x - winSize.x - 10, displaySize.y - winSize.y - 10), ImGuiCond_Once);

        // Tabs
        if (ImGui::BeginTabBar("SuperHot MCC Tabs")) {
            if (ImGui::BeginTabItem("Settings")) {
                ImGui::Checkbox("ESP", &settings.showESP);
                if (ImGui::IsItemHovered()) ImGui::SetTooltip("Show debug ESP (F1)");

                HaloCE::Mod::UI::settings();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("About")) {
                ImGui::Text("Superhot MCC 1.0.0 by Kody King");
                credits();
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Debug")) {
                HaloCE::Mod::UI::debug();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }        

        ImGui::End();
    }

    void esp() {
        ESP::updateScreenSize();

        ImGui::Begin(
            "__ESP", 0,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs
        );
        {

            // Set Window position to top left corner
            ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);

            // Set Window size to full screen
            ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);

            HaloCE::Mod::UI::esp();
        }
        ImGui::End();
    }

    void checkHotkeys() {
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false))
            settings.showESP = !settings.showESP;
        HaloCE::Mod::UI::checkHotKeys();
    }

    void render() {
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        checkHotkeys();

        if ( HaloMCC::isPauseMenuOpen() )
            mainModWindow();
        if ( settings.showESP )
            esp();

        ImGui::EndFrame();
    }

    namespace WndProcHook {
        // Original WndProc
        WNDPROC originalWndProc = nullptr;

        LRESULT CALLBACK hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            auto result = ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

            bool blockMessage = false;
            switch (msg) {
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                case WM_CHAR:
                case WM_SYSCHAR:
                    blockMessage = ImGui::GetIO().WantCaptureKeyboard;
                    break;
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_XBUTTONDOWN:
                case WM_XBUTTONUP:
                case WM_MOUSEWHEEL:
                case WM_MOUSEHWHEEL:
                    blockMessage = ImGui::GetIO().WantCaptureMouse;
                    break;
                case WM_INPUT:
                    blockMessage = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
                    break;
            }

            if (blockMessage)
                return result;

            return CallWindowProc(originalWndProc, hWnd, msg, wParam, lParam);
        }

        void hook(HWND targetWindow) {
            originalWndProc = (WNDPROC) SetWindowLongPtr(targetWindow, GWLP_WNDPROC, (LONG_PTR) hkWndProc);
        }

        void unhook(HWND targetWindow) {
            SetWindowLongPtr(targetWindow, GWLP_WNDPROC, (LONG_PTR) originalWndProc);
        }
    }

    void init() {
        gameWindow = DX11Hook::findMainWindow();
        DX11Hook::hook(gameWindow);
        WndProcHook::hook(gameWindow);
    }

    void free() {
        WndProcHook::unhook(gameWindow);
        DX11Hook::unhook();
    }

}