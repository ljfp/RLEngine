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
#include "../ECS/FlecsGameWorld.hpp"

#include <fstream>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>
#include <string>

LevelLoader::LevelLoader()
{
	spdlog::info("LevelLoader created");
}

LevelLoader::~LevelLoader()
{
	spdlog::info("LevelLoader destroyed");
}

void LevelLoader::LoadLevel(sol::state& LuaState, flecs::world& World, const std::unique_ptr<AssetManager>& AssetManager, SDL_Renderer* Renderer, uint8_t LevelNumber)
{
	sol::load_result Script = LuaState.load_file("./assets/scripts/Level" + std::to_string(LevelNumber) + ".lua");
	if (!Script.valid())
	{
		sol::error Error = Script;
		std::string ErrorMessage = Error.what();
		spdlog::error("Error loading script: {}", ErrorMessage);
		return;
	}

	LuaState.script_file("./assets/scripts/Level" + std::to_string(LevelNumber) + ".lua");
	spdlog::info("Level {} loaded", LevelNumber);

	sol::table Level = LuaState["Level"];

	sol::table Assets = Level["assets"];
	uint16_t i = 0;
	while (true)
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
			spdlog::info("Texture with AssetID {} added to the AssetManager", Asset["id"].get<std::string>());
		}
		if (AssetType == "font")
		{
			AssetManager->AddFont(Asset["id"], Asset["file"], Asset["font_size"]);
			spdlog::info("Font with AssetID {} and size {} added to the AssetManager", Asset["id"].get<std::string>(), Asset["font_size"].get<int>());
		}
		i++;
	}

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
			TilemapFile.get(ch);
            //TODO: This only works for single digit tile indices. Need to update the tilemap file format to support multiple digit indices and update this code to parse them correctly.
			uint16_t SourceRectangleY = static_cast<uint16_t>((ch - '0') * TileSize);
			TilemapFile.get(ch);
            //TODO: This only works for single digit tile indices. Need to update the tilemap file format to support multiple digit indices and update this code to parse them correctly.
			uint16_t SourceRectangleX = static_cast<uint16_t>((ch - '0') * TileSize);
			TilemapFile.ignore();

			flecs::entity Tile = World.entity();
			ApplyGameplayTag(World, Tile, "Tiles");
			Tile.set<TransformComponent>(TransformComponent(glm::vec2(x * (MapScale * TileSize), y * (MapScale * TileSize)), glm::vec2(MapScale, MapScale), 0.0));
			Tile.set<SpriteComponent>(SpriteComponent(MapTextureAssetID, TileSize, TileSize, 0, false, SourceRectangleX, SourceRectangleY));
		}
	}
	TilemapFile.close();

	Game::MapWidth = static_cast<uint16_t>(MapNumColumns * TileSize * MapScale);
	Game::MapHeight = static_cast<uint16_t>(MapNumRows * TileSize * MapScale);
	World.set<MapBounds>(MapBounds{ Game::MapWidth, Game::MapHeight });

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
		flecs::entity NewEntity = World.entity();

		sol::optional<std::string> Tag = AnEntity["tag"];
		if (Tag != sol::nullopt)
		{
			ApplyGameplayTag(World, NewEntity, *Tag);
		}

		sol::optional<std::string> Group = AnEntity["group"];
		if (Group != sol::nullopt)
		{
			ApplyGameplayTag(World, NewEntity, *Group);
		}

		sol::optional<sol::table> HasComponents = AnEntity["components"];
		if (HasComponents != sol::nullopt)
		{
			sol::table Components = AnEntity["components"];

			sol::optional<sol::table> Transform = Components["transform"];
			if (Transform != sol::nullopt)
			{
				NewEntity.set<TransformComponent>(TransformComponent
				(
					glm::vec2(Components["transform"]["position"]["x"], Components["transform"]["position"]["y"]),
					glm::vec2(Components["transform"]["scale"]["x"].get_or(1.0), Components["transform"]["scale"]["y"].get_or(1.0)),
					static_cast<double>(Components["transform"]["rotation"].get_or(0.0))
				));
			}

			sol::optional<sol::table> RigidBody = Components["rigidbody"];
			if (RigidBody != sol::nullopt)
			{
				NewEntity.set<RigidBodyComponent>(RigidBodyComponent
				(
					glm::vec2(Components["rigidbody"]["velocity"]["x"].get_or(0.0), Components["rigidbody"]["velocity"]["y"].get_or(0.0))
				));
			}

			sol::optional<sol::table> Sprite = Components["sprite"];
			if (Sprite != sol::nullopt)
			{
				NewEntity.set<SpriteComponent>(SpriteComponent
				(
					Components["sprite"]["texture_asset_id"],
					Components["sprite"]["width"],
					Components["sprite"]["height"],
					Components["sprite"]["z_index"].get_or(1),
					Components["sprite"]["fixed"].get_or(false),
					Components["sprite"]["src_rect_x"].get_or(0),
					Components["sprite"]["src_rect_y"].get_or(0)
				));
			}

			sol::optional<sol::table> Animation = Components["animation"];
			if (Animation != sol::nullopt)
			{
				NewEntity.set<AnimationComponent>(AnimationComponent
				(
					Components["animation"]["num_frames"].get_or(1),
					Components["animation"]["speed_rate"].get_or(1),
					Components["animation"]["loop"].get_or(false)
				));
			}

			sol::optional<sol::table> BoxCollider = Components["boxcollider"];
			if (BoxCollider != sol::nullopt)
			{
				NewEntity.set<BoxColliderComponent>(BoxColliderComponent
				(
					Components["boxcollider"]["width"],
					Components["boxcollider"]["height"],
					glm::vec2(Components["boxcollider"]["offset"]["x"].get_or(0.0), Components["boxcollider"]["offset"]["y"].get_or(0.0))
				));
			}

			sol::optional<sol::table> Health = Components["health"];
			if (Health != sol::nullopt)
			{
				NewEntity.set<HealthComponent>(HealthComponent(static_cast<uint8_t>(Components["health"]["health_percentage"].get_or(100))));
			}

			sol::optional<sol::table> ProjectileEmitter = Components["projectile_emitter"];
			if (ProjectileEmitter != sol::nullopt)
			{
				NewEntity.set<ProjectileEmitterComponent>(ProjectileEmitterComponent
				(
					glm::vec2(Components["projectile_emitter"]["projectile_velocity"]["x"], Components["projectile_emitter"]["projectile_velocity"]["y"]),
					static_cast<uint16_t>(Components["projectile_emitter"]["repeat_frequency"].get_or(1)) * 1000,
					static_cast<uint16_t>(Components["projectile_emitter"]["projectile_duration"].get_or(10)) * 1000,
					static_cast<uint8_t>(Components["projectile_emitter"]["hit_percentage_damage"].get_or(10)),
					Components["projectile_emitter"]["friendly"].get_or(false)
				));
			}

			sol::optional<sol::table> CameraFollow = Components["camera_follow"];
			if (CameraFollow != sol::nullopt)
			{
				NewEntity.add<CameraFollowComponent>();
			}

			sol::optional<sol::table> KeyboardControl = Components["keyboard_controller"];
			if (KeyboardControl != sol::nullopt)
			{
				NewEntity.set<KeyboardControlComponent>(KeyboardControlComponent
				(
					glm::vec2(Components["keyboard_controller"]["up_velocity"]["x"].get_or(0.0), Components["keyboard_controller"]["up_velocity"]["y"].get_or(0.0)),
					glm::vec2(Components["keyboard_controller"]["down_velocity"]["x"].get_or(0.0), Components["keyboard_controller"]["down_velocity"]["y"].get_or(0.0)),
					glm::vec2(Components["keyboard_controller"]["left_velocity"]["x"].get_or(0.0), Components["keyboard_controller"]["left_velocity"]["y"].get_or(0.0)),
					glm::vec2(Components["keyboard_controller"]["right_velocity"]["x"].get_or(0.0), Components["keyboard_controller"]["right_velocity"]["y"].get_or(0.0))
				));
			}

			sol::optional<sol::table> TextLabel = Components["text_label"];
			if (TextLabel != sol::nullopt)
			{
				NewEntity.set<TextLabelComponent>(TextLabelComponent
				(
					glm::vec2(Components["text_label"]["position"]["x"], Components["text_label"]["position"]["y"]),
					Components["text_label"]["text"],
					Components["text_label"]["font_id"],
					SDL_Color
					{
						Components["text_label"]["color"]["r"],
						Components["text_label"]["color"]["g"],
						Components["text_label"]["color"]["b"],
						255
					},
					Components["text_label"]["fixed"].get_or(false)
				));
			}

			sol::optional<sol::table> Script = Components["on_update_script"];
			if (Script != sol::nullopt)
			{
				sol::function Funct = Components["on_update_script"][0];
				NewEntity.set<ScriptComponent>(ScriptComponent(Funct));
			}
		}

		i++;
	}
}