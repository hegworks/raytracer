#pragma once

enum class ModelType : uint8_t
{
	SPHERE,
	CUBE,
	PLANE,
	DRAGON,
};

class ModelData
{
public:
	static std::string GetAddress(ModelType modelType)
	{
		switch(modelType)
		{
			case ModelType::SPHERE:
				return ASSETDIR + "Models/Primitives/SphereSmooth/SphereSmooth.glb";
			case ModelType::CUBE:
				return ASSETDIR + "Models/Primitives/Cube/Cube.obj";
			case ModelType::PLANE:
				return ASSETDIR + "Models/Primitives/Plane/Plane.obj";
			case ModelType::DRAGON:
				return ASSETDIR + "Models/dragon.glb";
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};
