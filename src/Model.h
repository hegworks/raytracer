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
#include <utility>
#include <vector>

struct Texture
{
	int m_surfaceIndex;
	std::string m_type;
	aiString m_path;
};

class Model
{
public:
	Model(std::string const& path, const float2 textureCoordScale = float2(1), bool isRandZ = false)
	{
		//stbi_set_flip_vertically_on_load(shouldVerticallyFlipTexture);
		m_textureCoordScale = textureCoordScale;
		m_isRandZ = isRandZ;
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

	std::string GetStrippedFileName() const;
	int VertexToMeshIdx(uint prim) const;

private:
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	void TextureFromFile(const std::string& path);
	void TextureFromMemory(aiTexel* pcData, int mWidth);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene);

	bool m_isRandZ = false;
	inline static RNG rng;
	inline static uint seed = 123;
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

	processNode(scene->mRootNode, scene);
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

inline void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		processMesh(mesh, scene);
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

inline void Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	// process indices
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];

		for(unsigned int j = 0; j < face.mNumIndices; j++)
		{
			int idx = face.mIndices[j];

			VertexData& newVertexData = m_modelData.m_vertexDataList.emplace_back();
			newVertexData.m_normal = float3(mesh->mNormals[idx].x, mesh->mNormals[idx].y, mesh->mNormals[idx].z);
			float randZAddition = 0.0f;
			if(m_isRandZ)
			{
				const float randSign = RandomFloat() > 0.5f ? 1.0f : -1.0f;
				randZAddition = randSign * RandomFloat() * 2.0f;
			}
			float4 vert(mesh->mVertices[idx].x, mesh->mVertices[idx].y, mesh->mVertices[idx].z + randZAddition, 0);
			m_modelData.m_vertices.emplace_back(vert);
			if(mesh->mTextureCoords[0])
				newVertexData.m_texCoord = float2(mesh->mTextureCoords[0][idx].x * m_textureCoordScale.x, mesh->mTextureCoords[0][idx].y * m_textureCoordScale.y);
			else
				newVertexData.m_texCoord = float2(0);
		}
	}
	int verticesSize = static_cast<int>(m_modelData.m_vertices.size());
	m_modelData.m_meshVertexBorderList.emplace_back(verticesSize - 1);

	// process material
	Material& mat = m_modelData.m_meshMaterialList.emplace_back();
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	strcpy(mat.m_name, material->GetName().C_Str());
	aiColor4D diffuse;
	if(AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse))
	{
		mat.m_albedo = {diffuse.r,diffuse.g,diffuse.b};
	}

	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", scene);
	m_modelData.m_textureList.insert(m_modelData.m_textureList.end(), diffuseMaps.begin(), diffuseMaps.end());
}

inline std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<Texture> textures;
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
