#pragma once

#include "Material.h"
#include "ModelData.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <tmpl8math.h>
#include <utility>
#include <vector>

#include "PerlinGenerator.h"

struct Texture
{
	int m_surfaceIndex;
	std::string m_type;
	aiString m_path;
};

class Model
{
public:
	Model(std::string const& path, const float2 textureCoordScale = float2(1), const bool isRandZ = false, const bool isInvertMetallic = false)
	{
		//stbi_set_flip_vertically_on_load(shouldVerticallyFlipTexture);

		m_textureCoordScale = textureCoordScale;
		m_isRandZ = isRandZ;
		m_isInvertMetallic = isInvertMetallic;

		printf("Loading Model:%s\n", path.c_str());
		loadModel(path);
		size_t pos = path.find_last_of("/\\");
		m_fileName = (pos != std::string::npos) ? path.substr(pos + 1) : path;
	}
	~Model()
	{
		std::cout << "Destroying Model: " << m_directory << std::endl;
		for(int i = 0; i < m_modelData.m_surfaceList.size(); ++i)
		{
			FREE64(m_modelData.m_surfaceList[i].pixelsF);
			FREE64(m_modelData.m_surfaceList[i].pixels);
		}
	}

	struct ALIGNED(32) VertexData
	{
		float3 m_normal = 0;
		float dummy0 = 0;
		float2 m_texCoord = 0;
		float2 dummy1 = 0;
	};

	struct ALIGNED(64) ModelData
	{
		std::vector<Material> m_meshMaterialList; /// idx of material of each mesh
		std::vector<int> m_meshVertexBorderList; /// last idx of m_vertices of each mesh
		std::vector<Texture> m_textureList;
		std::vector<Surface> m_surfaceList;
		std::vector<int> m_surfaceIndexList;
		std::vector<VertexData> m_vertexDataList;
		std::vector<float4> m_vertices;
		ModelType m_type;
#ifdef _DEBUG
		char dummy[31];
#else
		char dummy[23];
#endif
	};

	Model(ModelData modelData)
	{
		m_modelData = std::move(modelData);
	}

	ModelData m_modelData;

	float2 m_textureCoordScale = 1;
	std::string m_directory;
	std::string m_fileName;
	std::vector<Texture> m_texturesLoaded;
	int m_fallbackTextureIdx = -1;

	std::string GetStrippedFileName() const;
	int VertexToMeshIdx(uint prim) const;

private:
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform);
	void processMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform);
	void TextureFromFile(const std::string& path);
	void TextureFromMemory(aiTexel* pcData, int mWidth);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene);

	bool m_isRandZ = false;
	bool m_isInvertMetallic = false;
};

inline void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_CalcTangentSpace;
	const aiScene* scene = importer.ReadFile(path, flags);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::string e = importer.GetErrorString();
		std::cerr << "ERROR::ASSIMP::IMPORTER " << e;
		throw std::runtime_error(importer.GetErrorString());
	}
	m_directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene, {});
}

inline std::string Model::GetStrippedFileName() const
{
	std::string result = m_directory.substr(m_directory.find_last_of('/') + 1);
	result = result.substr(result.find_last_of('\\') + 1);
	return result;
}

inline int Model::VertexToMeshIdx(const uint prim) const
{
	const int numMeshes = static_cast<int>(m_modelData.m_meshVertexBorderList.size());
	if(numMeshes == 1) return 0;
	for(int i = 0; i < numMeshes; ++i)
	{
		const int borderVertexIdx = m_modelData.m_meshVertexBorderList[i];
		if(static_cast<int>(prim) <= borderVertexIdx)
		{
			return i;
		}
	}
	throw runtime_error("Prim idx is bigger than all the vertex borders ids");

}

inline void Model::processNode(aiNode* node, const aiScene* scene, aiMatrix4x4 parentTransform)
{
	aiMatrix4x4 nodeTransform = parentTransform * node->mTransformation;

	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene, nodeTransform);
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, nodeTransform);
	}
}

//#define FULLY_RANDOM
#define SINE
//#define PERLIN
//#define FIXED_MOVE

inline void Model::processMesh(aiMesh* mesh, const aiScene* scene, const aiMatrix4x4& transform)
{
	for(uint i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for(uint j = 0; j < face.mNumIndices; ++j)
		{
			const uint idx = face.mIndices[j];

			float randZAddition = 0.0f;
			if(m_isRandZ)
			{
#if defined(FULLY_RANDOM)
				constexpr float amplitude = 0.5f;
				const float randSign = RandomFloat() > 0.5f ? 1.0f : -1.0f;
				randZAddition = randSign * RandomFloat() * amplitude;


#elif defined(SINE)
				constexpr float frequency = 5.0f, amplitude = 1.5f;
				const float axis = mesh->mVertices[idx].x;
				randZAddition = sin(axis * frequency) * amplitude;


#elif defined (PERLIN)
				PerlinGenerator::m_numOctaves = 1;
				PerlinGenerator::m_amplitude = 0.5f;
				PerlinGenerator::m_persistence = 0.5f;
				randZAddition = PerlinGenerator::noise3D(mesh->mVertices[idx].x, mesh->mVertices[idx].y, mesh->mVertices[idx].z);


#elif defined (FIXED_MOVE)
				constexpr float frequency = 0.5f;
				constexpr float amplitude = 0.85f;
				const float axis = mesh->mVertices[idx].z;
				const int sectionIndex = static_cast<int>(axis / frequency);
#if 0
				randZAddition = (sectionIndex * amplitude);
#elif 0
				randZAddition = (sectionIndex % 2 == 0 ? 1.0f : -1.0f) * (sectionIndex * amplitude);
#elif 0
				randZAddition = amplitude * sinf(sectionIndex * TWOPI * 0.1f);
#elif 1
				randZAddition = amplitude * sinf(sectionIndex + axis * frequency);
#endif



#endif
			}

			aiMatrix3x3 normalMatrix = aiMatrix3x3(transform);
			normalMatrix.Inverse().Transpose();
			const aiVector3D transformedPos = transform * mesh->mVertices[idx];
			const aiVector3D transformedNormal = mesh->HasNormals() ? normalMatrix * mesh->mNormals[idx] : aiVector3D(0, 0, 1);

			float3 position(transformedPos.x, transformedPos.y, transformedPos.z + randZAddition);

			const float3 normal = float3(transformedNormal.x, transformedNormal.y, transformedNormal.z);

			float2 texCoord = float2(0);
			if(mesh->HasTextureCoords(0))
			{
				texCoord = float2(
					mesh->mTextureCoords[0][idx].x * m_textureCoordScale.x,
					mesh->mTextureCoords[0][idx].y * m_textureCoordScale.y
				);
			}

			VertexData vertexData;
			vertexData.m_normal = normal;
			vertexData.m_texCoord = texCoord;

			m_modelData.m_vertexDataList.push_back(vertexData);
			m_modelData.m_vertices.emplace_back(float4(position, 0));
		}
	}

	const int vertCount = static_cast<int>(m_modelData.m_vertices.size());
	m_modelData.m_meshVertexBorderList.emplace_back(vertCount - 1);

	// process material
	Material& mat = m_modelData.m_meshMaterialList.emplace_back();
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	strcpy(mat.m_name, material->GetName().C_Str());

	aiColor3D diffuse(DEFAULT_ALBEDO), emisssion(0.0f);
	float metallic(0.0f), roughness(0.0f), ior(1.0f), transmission(1.0f);
	material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
	material->Get(AI_MATKEY_COLOR_EMISSIVE, emisssion);
	material->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
	material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
	material->Get(AI_MATKEY_REFRACTI, ior); // ior is not exported to formats like fbx obj glb etc. this is line is mostly useless
	material->Get(AI_MATKEY_TRANSMISSION_FACTOR, transmission);

#if 0
	printf("Material: %s\n", mat.m_name);
	printf("Diffuse: %f %f %f\n", diffuse.r, diffuse.g, diffuse.b);
	printf("Emissive: %f %f %f\n", emisssion.r, emisssion.g, emisssion.b);
	printf("Roughness: %f\n", roughness);
	printf("Metallic: %f\n", metallic);
	printf("IOR: %f\n", ior);
	printf("Transmission: %f\n", transmission);
#endif

	mat.m_albedo = {diffuse.r,diffuse.g,diffuse.b};
	if(abs(mat.m_albedo.x - 0.6f) <= EPS && abs(mat.m_albedo.y - 0.6f) <= EPS && abs(mat.m_albedo.z - 0.6f) <= EPS) mat.m_albedo = DEFAULT_ALBEDO;

	if(emisssion.r > EPS || emisssion.g > EPS || emisssion.b > EPS)
	{
		mat.m_type = Material::Type::EMISSIVE;
		mat.m_albedo = {emisssion.r,emisssion.g,emisssion.b};
		mat.m_factor0 = 10.0f; // intensity
	}
	else if(ior > 1.0f)
	{
		mat.m_type = Material::Type::REFRACTIVE;
		mat.m_factor0 = transmission;
		mat.m_factor1 = ior;
	}
	else
	{
		mat.m_type = Material::Type::PATH_TRACED;
		mat.m_factor0 = 1.0f - roughness;
		if(m_isInvertMetallic) metallic = 1.0f - metallic;
		if(dot(mat.m_albedo, mat.m_albedo) > 3.0f - EPS) mat.m_albedo *= DEFAULT_ALBEDO;
	}

	if(material->GetTextureCount(aiTextureType_DIFFUSE) > 0 && mat.m_type == Material::Type::PATH_TRACED)
	{
		mat.m_albedo = DEFAULT_ALBEDO;
	}

	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
	m_modelData.m_textureList.insert(m_modelData.m_textureList.end(), diffuseMaps.begin(), diffuseMaps.end());
}

inline std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<Texture> textures;

	const uint textureCount = mat->GetTextureCount(type);
	if(textureCount == 0)
	{
		if(m_fallbackTextureIdx != -1) // fallback texture has been loaded before
		{
			textures.push_back(m_texturesLoaded[m_fallbackTextureIdx]);
			m_modelData.m_surfaceIndexList.emplace_back(m_texturesLoaded[m_fallbackTextureIdx].m_surfaceIndex);
		}
		else
		{
			Texture texture;
			const std::string path = ASSETDIR + "Models/4x4white.png";
			std::cout << "Loading fallback texture at:  " << path << std::endl;
			TextureFromFile(path);
			texture.m_type = typeName;
			texture.m_path = path;
			texture.m_surfaceIndex = static_cast<int>(m_modelData.m_surfaceList.size()) - 1;
			textures.push_back(texture);
			m_texturesLoaded.push_back(texture);
			m_fallbackTextureIdx = static_cast<int>(m_texturesLoaded.size()) - 1;
			m_modelData.m_surfaceIndexList.emplace_back(texture.m_surfaceIndex);
		}
		return textures;
	}

	for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool wasTextureLoadedBefore = false;
		for(unsigned int j = 0; j < m_texturesLoaded.size(); j++)
		{
			if(std::strcmp(m_texturesLoaded[j].m_path.data, str.C_Str()) == 0)
			{
				textures.push_back(m_texturesLoaded[j]);
				m_modelData.m_surfaceIndexList.emplace_back(m_texturesLoaded[j].m_surfaceIndex);
				wasTextureLoadedBefore = true;
				break;
			}
		}
		if(!wasTextureLoadedBefore)
		{
			// if texture path contains backslash, change it to be the last part after backslash (fix for some fbx files)
			std::string texturePath = str.C_Str();
			size_t pos = texturePath.find_last_of('\\');
			if(pos != std::string::npos)
			{
				texturePath = texturePath.substr(pos + 1);
			}
			std::cout << "Loading: " << m_directory << "/" << texturePath.c_str() << std::endl;

			Texture texture;
			std::string path = m_directory + '/' + std::string(texturePath);

			aiString textureFile;
			mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), textureFile);
			if(const aiTexture* tex = scene->GetEmbeddedTexture(textureFile.C_Str()))
			{
				TextureFromMemory(tex->pcData, tex->mWidth);
			}
			else
			{
				TextureFromFile(path);
			}
			texture.m_type = typeName;
			texture.m_path = str.C_Str();
			texture.m_surfaceIndex = static_cast<int>(m_modelData.m_surfaceList.size()) - 1;
			textures.push_back(texture);
			m_texturesLoaded.push_back(texture);
			m_modelData.m_surfaceIndexList.emplace_back(texture.m_surfaceIndex);
		}
	}
	return textures;
}

inline void Model::TextureFromFile(const std::string& path)
{
	Surface& surface = m_modelData.m_surfaceList.emplace_back(path.c_str());
	surface.ownBuffer = false;
}

inline void Model::TextureFromMemory(aiTexel* pcData, int mWidth)
{
	Surface& surface = m_modelData.m_surfaceList.emplace_back(pcData, mWidth);
	surface.ownBuffer = false;
}
