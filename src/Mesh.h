#pragma once

#include <assimp/types.h>
#include <vector>

struct Vertex
{
	float3 m_position = float3(0);
	float3 m_normal = float3(0);
	float2 m_texCoords = float2(0);

	float3 m_tangent = float3(0);
	float3 m_bitangent = float3(0);
};

struct Texture
{
	unsigned int m_id;
	std::string m_type;
	aiString m_path;
};

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	// mesh data
	std::vector<Vertex>       m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture>      m_textures;
};

inline Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
{
	m_vertices = vertices;
	m_indices = indices;
	m_textures = textures;
}
