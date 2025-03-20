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
#include <vector>

struct Texture
{
	unsigned int m_id;
	std::string m_type;
	aiString m_path;
};

class Model
{
public:
	Model(std::string const& path, float2 textureCoordScale = float2(1), bool shouldVerticallyFlipTexture = false)
	{
		stbi_set_flip_vertically_on_load(shouldVerticallyFlipTexture);
		m_textureCoordScale = textureCoordScale;
		printf("Loading Model:%s\n", path.c_str());
		loadModel(path);
		m_modelData.m_initialized = true;
		size_t pos = path.find_last_of("/\\");
		m_fileName = (pos != std::string::npos) ? path.substr(pos + 1) : path;
	}
	~Model()
	{
		std::cout << "Destroying Model: " << m_directory << std::endl;
		for(unsigned int i = 0; i < m_texturesLoaded.size(); i++)
		{
			glDeleteTextures(1, &m_texturesLoaded[i].m_id);
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
		std::vector<VertexData> m_vertexDataList;
		std::vector<float4> m_vertices;
		ModelType m_type;
		bool m_initialized = false;
		char dummy[6];
	};

	ModelData m_modelData;

	float2 m_textureCoordScale = 1;
	std::string m_directory;
	std::string m_fileName;
	std::vector<Texture> m_texturesLoaded;

	std::string GetStrippedFileName() const;
	int VertexToMeshIdx(int prim);

private:
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	void processMesh(aiMesh* mesh, const aiScene* scene);
	unsigned int TextureFromFile(const char* path, const std::string& directory);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

inline void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
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

inline int Model::VertexToMeshIdx(int prim)
{
	int numMeshes = static_cast<int>(m_modelData.m_meshVertexBorderList.size());
	if(numMeshes == 1) return 0;
	for(int i = 0; i < numMeshes; ++i)
	{
		int borderVertexIdx = m_modelData.m_meshVertexBorderList[i];
		if(prim <= borderVertexIdx)
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
			float4 vert(mesh->mVertices[idx].x, mesh->mVertices[idx].y, mesh->mVertices[idx].z, 0);
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

	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	m_modelData.m_textureList.insert(m_modelData.m_textureList.end(), diffuseMaps.begin(), diffuseMaps.end());
}

inline unsigned int Model::TextureFromFile(const char* path, const std::string& directory)
{
	unsigned int texture;
	glGenTextures(1, &texture);

	int width, height, nrChannels;
	std::string fileName = directory + '/' + std::string(path);

	unsigned char* textureData = stbi_load(fileName.c_str(), &width, &height, &nrChannels, 0);
	if(!textureData)
	{
		std::cout << "ERROR::STBI::LOAD at file " << fileName << std::endl;
		throw std::runtime_error("ERROR::STBI::LOAD at file " + fileName);
	}

	GLenum format;
	if(nrChannels == 1)
		format = GL_RED;
	else if(nrChannels == 3)
		format = GL_RGB;
	else if(nrChannels == 4)
		format = GL_RGBA;
	else
	{
		std::cout << "ERROR::TextureFromFile::InvalidNrChannels at file " << fileName << std::endl;
		throw std::runtime_error("ERROR::TextureFromFile::InvalidNrChannels at file " + fileName);
	}

	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, textureData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(textureData);

	return texture;
}

inline std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
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
			texture.m_id = TextureFromFile(texturePath.c_str(), m_directory);
			texture.m_type = typeName;
			texture.m_path = str.C_Str();
			textures.push_back(texture);
			m_texturesLoaded.push_back(texture);
		}
	}
	return textures;
}
