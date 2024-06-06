#include <thread>
#include <mutex>
#include <string>
#include "imgui.h"
#include "Halo1.hpp"

namespace HaloCE::Mod::UI {

    bool showTagBrowser = false;
    void tagBrowser() {
        ImGui::Begin("Tag Browser", &showTagBrowser, ImGuiWindowFlags_AlwaysAutoResize);
        
            //////////////////////////////////////////////////////////////////////////
            // Pagination
            static int tagsPerPage = 50;
            static int page = 0;
            ImGui::InputInt("Page size", &tagsPerPage);
            if (tagsPerPage < 1) tagsPerPage = 1;
            ImGui::InputInt("Page", &page);
            if (ImGui::IsWindowHovered())
                page -= (int) ImGui::GetIO().MouseWheel;
            if (page < 0) page = 0;

            //////////////////////////////////////////////////////////////////////////
            // Search
            
            static char search[512] = {0};
            static char lastSearch[512] = {0};
            static std::vector<int> searchResults;

            ImGui::InputText("Search", search, 512);
            bool focused = ImGui::IsItemActive();
            
            bool searchChanged = strcmp(search, lastSearch) != 0;

            // Debounce search
            static uint64_t lastSearchTick = GetTickCount64();
            uint64_t tick = GetTickCount64();
            bool debounce = focused && (tick - lastSearchTick < 200);
            bool tooShort = focused && strlen(search) < 3;
            
            static std::mutex searchMutex;
            if (!tooShort && !debounce && searchChanged && searchMutex.try_lock() ) {
                searchMutex.unlock();
                
                std::thread searchThread = std::thread( [&] {
                    std::lock_guard<std::mutex> lock(searchMutex);
                    int i = 0;
                    std::string searchStr = search;
                    std::transform(searchStr.begin(), searchStr.end(), searchStr.begin(), ::tolower);

                    bool hasLastSearch = lastSearch[0] != 0;
                    bool lastSearchIsPrefix = hasLastSearch && searchStr.find(lastSearch) == 0;
                    if (lastSearchIsPrefix) {
                        // Results will be a subset of previous results.
                        std::vector<int> newResults;
                        for (int index : searchResults) {
                            auto tag = Halo1::getTag(index);
                            auto path = tag->getResourcePath();
                            std::string pathStr = path;
                            std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), ::tolower);
                            if (pathStr.find(searchStr) != std::string::npos)
                                newResults.push_back(index);
                        }
                        searchResults = newResults;
                    } else {
                        // Search from scratch
                        searchResults.clear();
                        while (true) {
                            auto tag = Halo1::getTag(i);
                            if (!Halo1::tagExists(tag)) break;
                            auto path = tag->getResourcePath();
                            std::string pathStr = path;
                            std::transform(pathStr.begin(), pathStr.end(), pathStr.begin(), ::tolower);
                            if (pathStr.find(searchStr) != std::string::npos)
                                searchResults.push_back(i);
                            i++;
                        }

                    }
                    strcpy_s( lastSearch, searchStr.c_str() );
                    lastSearchTick = GetTickCount64();
                } );
                searchThread.detach();

            }

            //////////////////////////////////////////////////////////////////////////
            // Render tag item
            auto renderTag = [&](int index) {
                if (index < 0) {
                    ImGui::Text("");
                    return;
                }

                auto tag = Halo1::getTag(index);
                if (Halo1::tagExists(tag)) {
                    auto path = tag->getResourcePath();
                    if (!Halo1::validTagPath(path)) {
                        ImGui::Text("%04d NULL", index);
                    } else {
                        char text[2048] = {0};

                        ImGui::Text("%04d", index);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy index to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( std::to_string(index).c_str() );

                        // ImGui::SameLine();
                        // sprintf_s( text, "%X", tag->tagID );
                        // ImGui::Text(text);
                        // if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy TagID to clipboard");
                        // if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        // ImGui::SameLine();
                        // sprintf_s( text, "%p", tag );
                        // ImGui::Text( "%s", text );
                        // if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy tag pointer to clipboard");
                        // if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        ImGui::SameLine();
                        auto groupID = tag->groupIDStr();
                        ImGui::Text("%s", groupID.c_str());
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy GroupID to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( groupID.c_str() );

                        ImGui::SameLine();
                        auto data = tag->getData();
                        sprintf_s( text, "%p", data );
                        ImGui::Text("%s", text);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy data pointer to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        ImGui::SameLine();
                        ImGui::Text("%s", path);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy path to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( path );
                    }
                } else {
                    ImGui::Text("%04d NULL", index);
                }
            };

            //////////////////////////////////////////////////////////////////////////

            if (!tooShort)  {
                int numPages = (int) ceil( searchResults.size() / (float) tagsPerPage );
                ImGui::SameLine();
                ImGui::Text("(%d results, %d pages)", searchResults.size(), numPages);
                if (page >= numPages) page = numPages - 1;
                if (page < 0) page = 0;
            }

            ImGui::Separator();

            if (!tooShort && search[0] != 0) {
                // Render search results
                if (searchMutex.try_lock()) {
                    int pageBase = page * tagsPerPage;
                    for (int i = 0; i < tagsPerPage; i++) {
                        int j = pageBase + i;
                        int index = -1;
                        if (j < searchResults.size())
                            index = searchResults[j];
                        renderTag(index);
                    }
                    searchMutex.unlock();
                } else {
                    ImGui::Text("Searching...");
                }
            } else {
                // Render all tags
                for (int i = 0; i < tagsPerPage; i++) {
                    int index = page * tagsPerPage + i;
                    auto tag = Halo1::getTag(index);
                    renderTag(index);
                }
            }

        ImGui::End();
    }

}