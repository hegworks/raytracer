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
};

constexpr int NUM_MODEL_TYPES = 9;

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
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};