#include "LevelLoader.hpp"
#include "Game.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/ScriptComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TextLabelComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../ECS/ECS.hpp"

#include <iostream>
#include <fstream>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <sol/sol.hpp>
#include <string>

LevelLoader::LevelLoader()
{
	spdlog::info("LevelLoader created");
}

LevelLoader::~LevelLoader()
{
	spdlog::info("LevelLoader destroyed");
}

void LevelLoader::LoadLevel(sol::state& LuaState, const std::unique_ptr<Registry>& Registry, const std::unique_ptr<AssetManager>& AssetManager, SDL_Renderer* Renderer, uint8_t LevelNumber)
{
	// We should check the syntax of the script before executing it.
	sol::load_result Script = LuaState.load_file("./assets/scripts/Level" + std::to_string(LevelNumber) + ".lua");
	if (!Script.valid())
	{
		sol::error Error = Script;
		std::string ErrorMessage = Error.what();
		spdlog::error("Error loading script: {}", ErrorMessage);
		return;
	}

	// Load the entities and components from assets/scripts/Level<n>.lua
	LuaState.script_file("./assets/scripts/Level" + std::to_string(LevelNumber) + ".lua");
	spdlog::info("Level {} loaded", LevelNumber);

	// Get the Level table from the script
	sol::table Level = LuaState["Level"];

	// Read the level assets from the table
	sol::table Assets = Level["assets"];
	uint16_t i = 0;
	while(true)
	{
		sol::optional<sol::table> HasAsset = Assets[i];
		if (HasAsset == sol::nullopt)
		{
			break;
		}
		sol::table Asset = Assets[i];
		std::string AssetType = Asset["type"];
		if (AssetType == "texture")
		{
			AssetManager->AddTexture(Renderer, Asset["id"], Asset["file"]);
			spdlog::info("Texture with AssetID {} added to the AssetManager", std::string(Asset["id"]));
		}
		if (AssetType == "font")
		{
			AssetManager->AddFont(Asset["id"], Asset["file"], Asset["font_size"]);
			spdlog::info("Font with AssetID {} and size {} added to the AssetManager", std::string(Asset["id"]), uint8_t(Asset["font_size"]));
		}
		i++;
	}

	// Read the level tilemap information from the table
	sol::table Tilemap = Level["tilemap"];
	std::string MapFilePath = Tilemap["map_file"];
	std::string MapTextureAssetID = Tilemap["texture_asset_id"];
	uint16_t MapNumRows = Tilemap["num_rows"];
	uint16_t MapNumColumns = Tilemap["num_cols"];
	uint16_t TileSize = Tilemap["tile_size"];
	double MapScale = Tilemap["scale"];
	std::fstream TilemapFile;
	TilemapFile.open(MapFilePath, std::ios::in);
	for (uint16_t y = 0; y < MapNumRows; y++)
	{
		for (uint16_t x = 0; x < MapNumColumns; x++)
		{
			char ch;
			// Read the first digit
			TilemapFile.get(ch);
			uint16_t SourceRectangleY = std::atoi(&ch) * TileSize;
			// Read the second digit
			TilemapFile.get(ch);
			uint16_t SourceRectangleX = std::atoi(&ch) * TileSize;
			// Skip the comma
			TilemapFile.ignore();

			Entity Tile = Registry->CreateEntity();
			Tile.Group("Tiles");
			Tile.AddComponent<TransformComponent>(glm::vec2(x * (MapScale * TileSize), y * (MapScale * TileSize)), glm::vec2(MapScale, MapScale), 0.0);
			Tile.AddComponent<SpriteComponent>(MapTextureAssetID, TileSize, TileSize, 0, false, SourceRectangleX, SourceRectangleY);
		}
	}
	TilemapFile.close();
	Game::MapWidth = MapNumColumns * TileSize * MapScale;
	Game::MapHeight = MapNumRows * TileSize * MapScale;

	// Read the level entities and components from the table
	sol::table Entities = Level["entities"];
	i = 0;
	while (true)
	{
		sol::optional<sol::table> HasEntity = Entities[i];
		if (HasEntity == sol::nullopt)
		{
			break;
		}

		sol::table AnEntity = Entities[i];
		Entity NewEntity = Registry->CreateEntity();

		// Tag entity if it has a tag.
		sol::optional<std::string> Tag = AnEntity["tag"];
		if (Tag != sol::nullopt)
		{
			NewEntity.Tag(AnEntity["tag"]);
		}

		// Group entity if it belongs to a group.
		sol::optional<std::string> Group = AnEntity["group"];
		if (Group != sol::nullopt)
		{
			NewEntity.Group(AnEntity["group"]);
		}

		// Add components to the entity
		sol::optional<sol::table> HasComponents = AnEntity["components"];
		if (HasComponents != sol::nullopt)
		{
			// Check for TransformComponent
			sol::optional<sol::table> Transform = AnEntity["components"]["transform"];
			if (Transform != sol::nullopt)
			{
				NewEntity.AddComponent<TransformComponent>
				(
					glm::vec2
					(
						AnEntity["components"]["transform"]["position"]["x"],
						AnEntity["components"]["transform"]["position"]["y"]
					),
					glm::vec2
					(
						AnEntity["components"]["transform"]["scale"]["x"].get_or(1.0),
						AnEntity["components"]["transform"]["scale"]["y"].get_or(1.0)
					),
					static_cast<double>(AnEntity["components"]["transform"]["rotation"].get_or(0.0))
				);
			}

			// Check for RigidBodyComponent
			sol::optional<sol::table> RigidBody = AnEntity["components"]["rigidbody"];
			if (RigidBody != sol::nullopt)
			{
				NewEntity.AddComponent<RigidBodyComponent>
				(
					glm::vec2
					(
						AnEntity["components"]["rigidbody"]["velocity"]["x"].get_or(0.0),
						AnEntity["components"]["rigidbody"]["velocity"]["y"].get_or(0.0)
					)
				);
			}

			// Check for SpriteComponent
			sol::optional<sol::table> Sprite = AnEntity["components"]["sprite"];
			if (Sprite != sol::nullopt)
			{
				NewEntity.AddComponent<SpriteComponent>
				(
					AnEntity["components"]["sprite"]["texture_asset_id"],
					AnEntity["components"]["sprite"]["width"],
					AnEntity["components"]["sprite"]["height"],
					AnEntity["components"]["sprite"]["z_index"].get_or(1),
					AnEntity["components"]["sprite"]["fixed"].get_or(false),
					AnEntity["components"]["sprite"]["src_rect_x"].get_or(0),
					AnEntity["components"]["sprite"]["src_rect_y"].get_or(0)
				);
			}

			// Check for AnimationComponent
			sol::optional<sol::table> Animation = AnEntity["components"]["animation"];
			if (Animation != sol::nullopt)
			{
				NewEntity.AddComponent<AnimationComponent>
				(
					AnEntity["components"]["animation"]["num_frames"].get_or(1),
					AnEntity["components"]["animation"]["speed_rate"].get_or(1),
					AnEntity["components"]["animation"]["loop"].get_or(false)
				);
			}

			// Check for BoxColliderComponent
			sol::optional<sol::table> BoxCollider = AnEntity["components"]["boxcollider"];
			if (BoxCollider != sol::nullopt)
			{
				NewEntity.AddComponent<BoxColliderComponent>
				(
					AnEntity["components"]["boxcollider"]["width"],
					AnEntity["components"]["boxcollider"]["height"],
					glm::vec2
					(
						AnEntity["components"]["boxcollider"]["offset"]["x"].get_or(0.0),
						AnEntity["components"]["boxcollider"]["offset"]["y"].get_or(0.0)
					)
				);
			}

			// Check for HealthComponent
			sol::optional<sol::table> Health = AnEntity["components"]["health"];
			if (Health != sol::nullopt)
			{
				NewEntity.AddComponent<HealthComponent>
				(
					static_cast<uint8_t>(AnEntity["components"]["health"]["health_percentage"].get_or(100))
				);
			}

			// Check for ProjectileEmitterComponent
			sol::optional<sol::table> ProjectileEmitter = AnEntity["components"]["projectile_emitter"];
			if (ProjectileEmitter != sol::nullopt)
			{
				NewEntity.AddComponent<ProjectileEmitterComponent>
				(
					glm::vec2
					(
						AnEntity["components"]["projectile_emitter"]["projectile_velocity"]["x"],
						AnEntity["components"]["projectile_emitter"]["projectile_velocity"]["y"]
					),
					static_cast<uint16_t>(AnEntity["components"]["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
					static_cast<uint16_t>(AnEntity["components"]["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
					static_cast<uint8_t>(AnEntity["components"]["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
					AnEntity["components"]["projectile_emitter"]["friendly"].get_or(false)
				);
			}

			// Check for CameraFollowComponent
			sol::optional<sol::table> CameraFollow = AnEntity["components"]["camera_follow"];
			if (CameraFollow != sol::nullopt)
			{
				NewEntity.AddComponent<CameraFollowComponent>();
			}

			// Check for KeyboardControlComponent
			sol::optional<sol::table> KeyboardControl = AnEntity["components"]["keyboard_controller"];
			if (KeyboardControl != sol::nullopt)
			{
				NewEntity.AddComponent<KeyboardControlComponent>
				(
					glm::vec2
					(
						AnEntity["components"]["keyboard_controller"]["up_velocity"]["x"].get_or(0.0),
						AnEntity["components"]["keyboard_controller"]["up_velocity"]["y"].get_or(0.0)
					),
					glm::vec2
					(
						AnEntity["components"]["keyboard_controller"]["down_velocity"]["x"].get_or(0.0),
						AnEntity["components"]["keyboard_controller"]["down_velocity"]["y"].get_or(0.0)
					),
					glm::vec2
					(
						AnEntity["components"]["keyboard_controller"]["left_velocity"]["x"].get_or(0.0),
						AnEntity["components"]["keyboard_controller"]["left_velocity"]["y"].get_or(0.0)
					),
					glm::vec2
					(
						AnEntity["components"]["keyboard_controller"]["right_velocity"]["x"].get_or(0.0),
						AnEntity["components"]["keyboard_controller"]["right_velocity"]["y"].get_or(0.0)
					)
				);
			}

			// Check for TextLabelComponent
			sol::optional<sol::table> TextLabel = AnEntity["components"]["text_label"];
			if (TextLabel != sol::nullopt)
			{
				NewEntity.AddComponent<TextLabelComponent>
				(
					glm::vec2
					(
						AnEntity["components"]["text_label"]["position"]["x"],
						AnEntity["components"]["text_label"]["position"]["y"]
					),
					AnEntity["components"]["text_label"]["text"],
					AnEntity["components"]["text_label"]["font_id"],
					SDL_Color
					{
						AnEntity["components"]["text_label"]["color"]["r"],
						AnEntity["components"]["text_label"]["color"]["g"],
						AnEntity["components"]["text_label"]["color"]["b"]
					},
					AnEntity["components"]["text_label"]["fixed"].get_or(false)
				);
			}

			// Check for Script
			sol::optional<sol::table> Script = AnEntity["components"]["on_update_script"];
			if (Script != sol::nullopt)
			{
				sol::function Funct = AnEntity["components"]["on_update_script"][0];
				NewEntity.AddComponent<ScriptComponent>(Funct);
			}
		}
		i++;
	}
}