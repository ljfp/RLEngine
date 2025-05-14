#pragma once

#include "../ECS/FlecsBridge.hpp"
#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <string>

class RenderDebugGUISystemFlecs : public FlecsBridgeSystem {
public:
    RenderDebugGUISystemFlecs() = default;
    
    void Update(SDL_Renderer* renderer, std::unique_ptr<FlecsBridge>& registry, SDL_Rect& camera, bool isDebug = false) {
        // Only render if debug mode is on
        if (!isDebug) {
            return;
        }
        
        // Start a new ImGui frame
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        
        // Display entity statistics
        ImGui::Begin("Entity Statistics");
        
        // Get the Flecs world
        auto& world = registry->GetWorld();
        int entityCount = 0;
        
        // Count entities
        world.each([&entityCount](flecs::entity e) {
            entityCount++;
            return true;
        });
        
        ImGui::Text("Entities: %d", entityCount);
        
        // Show tags and groups
        ImGui::Separator();
        ImGui::Text("Tags:");
        
        // Display tags as a tree
        std::vector<std::string> tags;
        world.each([&tags](flecs::entity e) {
            if (e.is_alive() && !e.has(flecs::ChildOf)) {
                // Convert flecs::string_view to std::string
                flecs::string_view nameView = e.name();
                std::string name = nameView.c_str() ? std::string(nameView.c_str()) : "";
                if (name.length() > 0) {
                    tags.push_back(name);
                }
            }
            return true;
        });
        
        for (const auto& tag : tags) {
            if (ImGui::TreeNode(tag.c_str())) {
                // Show entities with this tag
                world.each([tag](flecs::entity e) {
                    flecs::entity tagEntity = e.world().entity(tag.c_str());
                    if (e.has(tagEntity)) {
                        char buf[32];
                        std::snprintf(buf, 32, "Entity ID: %u", (unsigned int)e.id());
                        ImGui::Text("%s", buf);
                    }
                    return true;
                });
                ImGui::TreePop();
            }
        }
        
        // Display groups
        ImGui::Separator();
        ImGui::Text("Groups:");
        
        std::vector<std::string> groups;
        world.each([&groups](flecs::entity e) {
            if (e.is_alive()) {
                // Convert flecs::string_view to std::string
                flecs::string_view nameView = e.name();
                std::string name = nameView.c_str() ? std::string(nameView.c_str()) : "";
                if (name.length() > 0) {
                    // Check if this entity is used as a parent/group
                    bool isGroup = false;
                    e.world().each([&](flecs::entity child) {
                        if (child.has(flecs::ChildOf, e)) {
                            isGroup = true;
                            return false;  // Stop iteration once we find one child
                        }
                        return true;
                    });
                    
                    if (isGroup) {
                        groups.push_back(name);
                    }
                }
            }
            return true;
        });
        
        for (const auto& group : groups) {
            if (ImGui::TreeNode(group.c_str())) {
                flecs::entity groupEntity = world.entity(group.c_str());
                
                // Show entities in this group
                world.each([groupEntity](flecs::entity e) {
                    if (e.has(flecs::ChildOf, groupEntity)) {
                        char buf[32];
                        std::snprintf(buf, 32, "Entity ID: %u", (unsigned int)e.id());
                        ImGui::Text("%s", buf);
                    }
                    return true;
                });
                ImGui::TreePop();
            }
        }
        
        ImGui::End();
        
        // Render ImGui
        ImGui::Render();
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    }
    
    void Run(flecs::iter& it) override {
        // This is not used directly since this system needs renderer access
        // Update is called manually
    }
    
    void SetupQuery(flecs::world& world, flecs::system& system) override {
        // No query needed for this system
    }
}; 