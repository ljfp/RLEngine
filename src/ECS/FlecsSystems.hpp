#pragma once

#include "FlecsGameWorld.hpp"

inline constexpr const char* InputPhaseName = "InputPhase";
inline constexpr const char* MovementPhaseName = "MovementPhase";
inline constexpr const char* ProjectilePhaseName = "ProjectilePhase";
inline constexpr const char* AnimationPhaseName = "AnimationPhase";
inline constexpr const char* CollisionDetectPhaseName = "CollisionDetectPhase";
inline constexpr const char* CollisionResponsePhaseName = "CollisionResponsePhase";
inline constexpr const char* CameraPhaseName = "CameraPhase";
inline constexpr const char* ScriptPhaseName = "ScriptPhase";
inline constexpr const char* RenderBeginPhaseName = "RenderBeginPhase";
inline constexpr const char* RenderWorldPhaseName = "RenderWorldPhase";
inline constexpr const char* RenderUiPhaseName = "RenderUiPhase";
inline constexpr const char* RenderDebugPhaseName = "RenderDebugPhase";
inline constexpr const char* RenderEndPhaseName = "RenderEndPhase";
inline constexpr const char* CleanupPhaseName = "CleanupPhase";

void RegisterScriptComponents(flecs::world& World);
void RegisterScriptSystems(flecs::world& World);
void RegisterInputSystems(flecs::world& World);
void RegisterKeyboardControlSystems(flecs::world& World);
void RegisterMovementSystems(flecs::world& World);
void RegisterProjectileSystems(flecs::world& World);
void RegisterAnimationSystems(flecs::world& World);
void RegisterCollisionSystems(flecs::world& World);
void RegisterCameraSystems(flecs::world& World);
void RegisterRenderSystems(flecs::world& World);
void RegisterCleanupSystems(flecs::world& World);
