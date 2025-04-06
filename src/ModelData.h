#pragma once

enum class ModelType : uint8_t
{
	PLANE,
	SPHERE,
	CUBE,
	TORUS,
	DRAGON,
	CORNELL,
	GLASS,
	FLAKE,
	KENNY,
	LVL_SQUARE,
	LVL_SQUARE_FULL,
	LVL_TTORUS,
	LVL_TTORUS_FULL,
	LVL_TEAPOT,
	WHITE_ROOM,
};

constexpr int NUM_MODEL_TYPES = 14;

inline constexpr ModelType ALL_MODEL_TYPES[NUM_MODEL_TYPES] =
{
	ModelType::PLANE,
	ModelType::SPHERE,
	ModelType::CUBE,
	ModelType::TORUS,
	ModelType::DRAGON,
	ModelType::CORNELL,
	ModelType::GLASS,
	ModelType::KENNY,
	ModelType::LVL_SQUARE,
	ModelType::LVL_SQUARE_FULL,
	ModelType::LVL_TTORUS,
	ModelType::LVL_TTORUS_FULL,
	ModelType::LVL_TEAPOT,
	ModelType::WHITE_ROOM,
};

inline const std::string ALL_MODEL_NAMES[NUM_MODEL_TYPES] =
{
	"PLANE",
	"SPHERE",
	"CUBE",
	"TORUS",
	"DRAGON",
	"CORNELL",
	"GLASS",
	"KENNY",
	"LVL_SQUARE",
	"LVL_SQUARE_FULL",
	"LVL_TTORUS",
	"LVL_TTORUS_FULL",
	"LVL_TEAPOT",
	"WHITE_ROOM",
};

class ModelData
{
public:
	static std::string GetAddress(ModelType modelType)
	{
		switch(modelType)
		{
			case ModelType::PLANE:
				return ASSETDIR + "Models/Primitives/Plane/Plane.obj";
			case ModelType::SPHERE:
				return ASSETDIR + "Models/Primitives/SphereSmooth/SphereSmooth.glb";
			case ModelType::CUBE:
				return ASSETDIR + "Models/Primitives/Cube/Cube.obj";
			case ModelType::TORUS:
				return ASSETDIR + "Models/Primitives/Torus/Torus.glb";
			case ModelType::DRAGON:
				return ASSETDIR + "Models/dragon.glb";
			case ModelType::CORNELL:
				return ASSETDIR + "Models/cornell-box.obj";
			case ModelType::GLASS:
				return ASSETDIR + "Models/ikea_glass.glb";
			case ModelType::KENNY:
				return ASSETDIR + "Models/kennyscene.glb";
			case ModelType::LVL_SQUARE:
				return ASSETDIR + "Models/Game/square.glb";
			case ModelType::LVL_SQUARE_FULL:
				return ASSETDIR + "Models/Game/square_full.glb";
			case ModelType::LVL_TTORUS:
				return ASSETDIR + "Models/Game/ttorus.glb";
			case ModelType::LVL_TTORUS_FULL:
				return ASSETDIR + "Models/Game/ttorus_full.glb";
			case ModelType::LVL_TEAPOT:
				return ASSETDIR + "Models/Game/teapot.glb";
			case ModelType::WHITE_ROOM:
				//maybe:
				//return ASSETDIR + "Models/Game/spheres.glb"; // use in start screen
				//return ASSETDIR + "Models/Game/andys_room.glb"; // remove roof

				//return ASSETDIR + "Models/Game/Neon Calligraphy School.glb"; // make mats emissive in blender
				//return ASSETDIR + "Models/Game/neon_mask.glb"; // change emissive mat colors in blender

				//return ASSETDIR + "Models/Game/neon_spinners.glb"; // edit mats in blender
				//return ASSETDIR + "Models/Game/arcade_machine.glb"; // edit mats in blender


				//def:
				// glass dragon
				return ASSETDIR + "Models/Game/balloon_dog.glb"; //change materials in blender to 0.8 & 0.15
				//return ASSETDIR + "Models/Game/summer_drink.glb"; //100% change refractiveness and color of foam, glass, cocktail in code
				//return ASSETDIR + "Models/Game/swapfiets.glb"; // fix materials in blender (remove emissive, make metallic etc.)
				//return ASSETDIR + "Models/Game/chair.glb"; // increase metallic and smoothness of screws
				//return ASSETDIR + "Models/Game/metal_bucket.glb"; // increase smoothness in blender
				//return ASSETDIR + "Models/Game/guitar.glb"; // make some parts metallic in blender
				//return ASSETDIR + "Models/Game/miniature_cat.glb"; // level obj
				//return ASSETDIR + "Models/Game/geometric_1.glb"; // use as level
				//return ASSETDIR + "Models/Game/90-th_retro_room_with_dendy.glb"; // use in a level

				//return ASSETDIR + "Models/Game/raymatic.glb"; // use in start screen - logo

				//return ASSETDIR + "Models/Game/room_by_night.glb"; // def. use somewhere
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};