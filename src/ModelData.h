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
	SCN_RAYMATIC,
	SCN_SPHERES,
	SCN_ROOM_LEVEL,
	SCN_ROOM_MAIN,
	LVL_SQUARE,
	LVL_SQUARE_FULL,
	LVL_BUCKET,
	LVL_COCKTAIL,
	LVL_BALLOON_DOG,
	LVL_CHAIR,
	LVL_SPINNER,
	LVL_CAT,
	LVL_DRAGON,
	LVL_GUITAR,
	LVL_RAYMATIC,
};

constexpr int NUM_MODEL_TYPES = 23;

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
	ModelType::SCN_RAYMATIC,
	ModelType::SCN_SPHERES,
	ModelType::SCN_ROOM_LEVEL,
	ModelType::SCN_ROOM_MAIN,
	ModelType::LVL_SQUARE,
	ModelType::LVL_SQUARE_FULL,
	ModelType::LVL_BUCKET,
	ModelType::LVL_COCKTAIL,
	ModelType::LVL_BALLOON_DOG,
	ModelType::LVL_CHAIR,
	ModelType::LVL_SPINNER,
	ModelType::LVL_CAT,
	ModelType::LVL_DRAGON,
	ModelType::LVL_GUITAR,
	ModelType::LVL_RAYMATIC,
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
	"SCN_RAYMATIC",
	"SCN_SPHERES",
	"SCN_ROOM_LEVEL",
	"SCN_ROOM_MAIN",
	"LVL_SQUARE",
	"LVL_SQUARE_FULL",
	"LVL_BUCKET",
	"LVL_COCKTAIL",
	"LVL_BALLOON_DOG",
	"LVL_CHAIR",
	"LVL_SPINNER",
	"LVL_CAT",
	"LVL_DRAGON",
	"LVL_GUITAR",
	"LVL_RAYMATIC",
};

class ModelData
{
public:
	static std::string GetAddress(const ModelType modelType)
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
			case ModelType::SCN_RAYMATIC:
				return ASSETDIR + "Models/Game/Scene/raymatic.glb";
			case ModelType::SCN_SPHERES:
				return ASSETDIR + "Models/Game/Scene/spheres.glb";
			case ModelType::SCN_ROOM_LEVEL:
				return ASSETDIR + "Models/Game/Scene/room_Level.glb";
			case ModelType::SCN_ROOM_MAIN:
				return ASSETDIR + "Models/Game/Scene/room_by_night.glb";
			case ModelType::LVL_SQUARE:
				return ASSETDIR + "Models/Game/square.glb";
			case ModelType::LVL_SQUARE_FULL:
				return ASSETDIR + "Models/Game/square_full.glb";
			case ModelType::LVL_BUCKET:
				return ASSETDIR + "Models/Game/metal_bucket.glb"; // increase smoothness in blender
			case ModelType::LVL_COCKTAIL:
				return ASSETDIR + "Models/Game/summer_drink.glb"; //100% change refractiveness and color of foam, glass, cocktail in code
			case ModelType::LVL_BALLOON_DOG:
				return ASSETDIR + "Models/Game/balloon_dog.glb"; //change materials in blender to 0.8 & 0.15
			case ModelType::LVL_CHAIR:
				return ASSETDIR + "Models/Game/chair.glb"; // increase metallic and smoothness of screws
			case ModelType::LVL_SPINNER:
				return ASSETDIR + "Models/Game/neon_spinners.glb"; // edit mats in blender
			case ModelType::LVL_CAT:
				return ASSETDIR + "Models/Game/miniature_cat.glb";
			case ModelType::LVL_DRAGON:
				return ASSETDIR + "Models/Game/dragon.glb";
			case ModelType::LVL_GUITAR:
				return ASSETDIR + "Models/Game/guitar.glb"; // make some parts metallic in blender
			case ModelType::LVL_RAYMATIC:
				return ASSETDIR + "Models/Game/raymatic.glb"; // use in start screen - logo
				//return ASSETDIR + "Models/Game/arcade_machine.glb"; // edit mats in blender // RESERVE
		}
		throw std::runtime_error("Unhandled ModelType");
	}

	static bool GetIsReverseMetallic(const ModelType modelType)
	{
		switch(modelType)
		{
			case ModelType::PLANE:
			case ModelType::SPHERE:
			case ModelType::CUBE:
			case ModelType::TORUS:
			case ModelType::DRAGON:
			case ModelType::CORNELL:
			case ModelType::GLASS:
			case ModelType::FLAKE:
			case ModelType::KENNY:
			case ModelType::SCN_RAYMATIC:
			case ModelType::SCN_SPHERES:
			case ModelType::SCN_ROOM_MAIN:
			case ModelType::LVL_SQUARE:
			case ModelType::LVL_SQUARE_FULL:
			case ModelType::LVL_CAT:
			case ModelType::LVL_DRAGON:
			case ModelType::SCN_ROOM_LEVEL:
			case ModelType::LVL_BALLOON_DOG:
			case ModelType::LVL_COCKTAIL:
			case ModelType::LVL_CHAIR:
				return false;
			case ModelType::LVL_BUCKET:
			case ModelType::LVL_SPINNER:
			case ModelType::LVL_GUITAR:
			case ModelType::LVL_RAYMATIC:
				return true;
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};