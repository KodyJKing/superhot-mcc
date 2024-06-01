#include <Windows.h>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "Overlay.hpp"
#include "DX11Hook.hpp"

namespace Overlay {

    void initializeContext(HWND targetWindow) {
        if (ImGui::GetCurrentContext( ))
            return;
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(targetWindow);
    }

    void render() {
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("SuperHot MCC Overlay");
        ImGui::Text("Hello, world!");
        ImGui::End();

        ImGui::EndFrame();
    }

    void init() {
        HWND targetWindow = DX11Hook::findMainWindow();
        DX11Hook::hook(targetWindow);
    }

    void free() {
        DX11Hook::unhook();
    }

}