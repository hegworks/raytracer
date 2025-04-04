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
	HAMBURGER,
	SNAKE,
	LVL_SQUARE,
	LVL_TTORUS,
	LVL_TEAPOT0,
	LVL_TEAPOT1,
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
	ModelType::HAMBURGER,
	ModelType::SNAKE,
	ModelType::LVL_SQUARE,
	ModelType::LVL_TTORUS,
	ModelType::LVL_TEAPOT0,
	ModelType::LVL_TEAPOT1,
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
	"HAMBURGER",
	"SNAKE",
	"LVL_SQUARE",
	"LVL_TTORUS",
	"LVL_TEAPOT0",
	"LVL_TEAPOT1",
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
				return ASSETDIR + "Models/dragon2.glb";
			case ModelType::CORNELL:
				return ASSETDIR + "Models/cornell-box.obj";
			case ModelType::GLASS:
				return ASSETDIR + "Models/ikea_glass.glb";
			case ModelType::KENNY:
				return ASSETDIR + "Models/kennyscene.glb";
			case ModelType::HAMBURGER:
				return ASSETDIR + "Models/Hamburger/Hamburger.fbx";
			case ModelType::SNAKE:
				return ASSETDIR + "Models/NakedSnake/Naked_Snake.obj";
			case ModelType::LVL_SQUARE:
				return ASSETDIR + "Models/Game/square.glb";
			case ModelType::LVL_TTORUS:
				return ASSETDIR + "Models/Game/ttorus.glb";
			case ModelType::LVL_TEAPOT0:
				return ASSETDIR + "Models/Game/teapot.glb";
			case ModelType::LVL_TEAPOT1:
				return ASSETDIR + "Models/Game/teapot_1.obj";
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};