#pragma once

#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <iostream>
#include <map>
#include <stb_image.h>
#include <stdexcept>
#include <string>
#include <vector>

class Shader;
struct Texture;

class Model
{
public:
	Model(std::string const& path, float2 textureCoordScale = float2(1), bool shouldVerticallyFlipTexture = false)
	{
		stbi_set_flip_vertically_on_load(shouldVerticallyFlipTexture);
		m_textureCoordScale = textureCoordScale;
		loadModel(path);
	}
	~Model()
	{
		std::cout << "Destroying Model: " << m_directory << std::endl;
		for(unsigned int i = 0; i < m_texturesLoaded.size(); i++)
		{
			glDeleteTextures(1, &m_texturesLoaded[i].m_id);
		}
	}

	std::string GetDirectory() const { return m_directory; }
	std::string GetStrippedFileName() const;
	std::vector<Mesh> m_meshes;

private:
	std::string m_directory;
	std::vector<Texture> m_texturesLoaded;

	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	unsigned int TextureFromFile(const char* path, const std::string& directory);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

	float2 m_textureCoordScale;
};

inline void Model::loadModel(std::string path)
{
	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;
	const aiScene* scene = importer.ReadFile(path, flags);
	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
		return;
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

inline void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all the node's meshes (if any)
	for(unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		m_meshes.push_back(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for(unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

inline Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::vector<float4> triangles;

	for(unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// process vertex positions, normals and texture coordinates
		Vertex vertex;

		vertex.m_position = float3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.m_normal = float3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if(mesh->mTextureCoords[0])
			vertex.m_texCoords = float2(mesh->mTextureCoords[0][i].x * m_textureCoordScale.x, mesh->mTextureCoords[0][i].y * m_textureCoordScale.y);
		else
			vertex.m_texCoords = float2(0.0f, 0.0f);
		if(mesh->mTangents)
			vertex.m_tangent = float3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
		else
			vertex.m_tangent = float3(0.0f, 0.0f, 0.0f);
		if(mesh->mBitangents)
			vertex.m_bitangent = float3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
		else
			vertex.m_bitangent = float3(0.0f, 0.0f, 0.0f);
		vertices.push_back(vertex);
	}
	// process indices
	triangles.reserve(mesh->mNumFaces * 3);
	for(unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for(unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
			float4 trianglePoint(0);
			trianglePoint.x = mesh->mVertices[face.mIndices[j]].x;
			trianglePoint.y = mesh->mVertices[face.mIndices[j]].y;
			trianglePoint.z = mesh->mVertices[face.mIndices[j]].z;
			triangles.push_back(trianglePoint);
		}
	}
	// process material
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

	return {vertices, indices, textures, triangles, mesh->mNumFaces};
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
