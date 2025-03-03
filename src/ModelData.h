﻿#pragma once

enum class ModelType : uint8_t
{
	SPHERE,
	CUBE,
	PLANE,
	DRAGON,
	CORNELL,
};

constexpr int NUM_MODEL_TYPES = 5;

inline constexpr ModelType ALL_MODEL_TYPES[NUM_MODEL_TYPES] =
{
	ModelType::SPHERE,
	ModelType::CUBE,
	ModelType::PLANE,
	ModelType::DRAGON,
	ModelType::CORNELL,
};

inline const std::string ALL_MODEL_NAMES[NUM_MODEL_TYPES] =
{
	"SPHERE",
	"CUBE",
	"PLANE",
	"DRAGON",
	"CORNELL",
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
				return ASSETDIR + "Models/dragon2.glb";
			case ModelType::CORNELL:
				return ASSETDIR + "Models/cornell-box.obj";
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};