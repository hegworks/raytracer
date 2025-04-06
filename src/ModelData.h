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
	LVL_SQUARE_FULL,
	LVL_TTORUS,
	LVL_TTORUS_FULL,
	LVL_TEAPOT,
	WHITE_ROOM,
};

constexpr int NUM_MODEL_TYPES = 16;

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
	"HAMBURGER",
	"SNAKE",
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
			case ModelType::LVL_SQUARE_FULL:
				return ASSETDIR + "Models/Game/square_full.glb";
			case ModelType::LVL_TTORUS:
				return ASSETDIR + "Models/Game/ttorus.glb";
			case ModelType::LVL_TTORUS_FULL:
				return ASSETDIR + "Models/Game/ttorus_full.glb";
			case ModelType::LVL_TEAPOT:
				return ASSETDIR + "Models/Game/teapot.glb";
			case ModelType::WHITE_ROOM:
				//return ASSETDIR + "Models/Game/andys_room.glb";
				//return ASSETDIR + "Models/Game/toy_box.glb";
				//return ASSETDIR + "Models/Game/final_bedroom.glb";
				//return ASSETDIR + "Models/Game/time-room.glb";
				//return ASSETDIR + "Models/Game/the_morning_room.glb";
				//return ASSETDIR + "Models/Game/abandoned_brick_room.glb";
				//return ASSETDIR + "Models/Game/90-th_retro_room_with_dendy.glb";
				//return ASSETDIR + "Models/Game/bedroom_interior.glb";
				//return ASSETDIR + "Models/Game/Home2_Night.glb";
				//return ASSETDIR + "Models/Game/the_king_s_hall.glb";
				//return ASSETDIR + "Models/Game/adiha.fbx";
				//return ASSETDIR + "Models/Game/worm_hole_but_sphere.glb";
				return ASSETDIR + "Models/Game/room_by_night.glb";
				//return ASSETDIR + "Models/Game/geometric_1.glb";
				//return ASSETDIR + "Models/Game/spheres.glb";
		}
		throw std::runtime_error("Unhandled ModelType");
	}
};