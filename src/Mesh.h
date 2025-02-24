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
	std::vector<Vertex> m_vertices;
	std::vector<unsigned int> m_indices;
	std::vector<Texture> m_textures;
	std::vector<float4> m_triangles;
	uint32_t m_numFaces = 0;

	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures, const std::vector<float4>& triangles, const uint32_t& numFaces)
	{
		m_vertices = vertices;
		m_indices = indices;
		m_textures = textures;
		m_numFaces = numFaces;
		m_triangles = triangles;
	}
};
