#ifndef OBJMODEL_H
#define OBJMODEL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <QtMath>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

#include "model.h"
#include "mesh.h"
#include "aabb.h"


class ObjModel: public Model
{
public:
    /*  Model Data */
    vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    vector<Mesh> meshes;
    string directory;
    string path;
    bool gammaCorrection;

    /*  Functions   */
    // constructor, expects a filepath to a 3D model.
    ObjModel(string const &path, bool gamma, QOpenGLFunctions_3_3_Core &core) : gammaCorrection(gamma)
    {
        type = OBJMODEL;
        this->path = path;
        loadModel(path, core);
    }

    ~ObjModel() {
        foreach(Texture texture, textures_loaded){
            stbi_image_free(texture.data);
        }
    }

    // draws the model, and thus all its meshes
    void Draw(QOpenGLShaderProgram& shader, QOpenGLFunctions_3_3_Core &core)
    {
        size_t size = meshes.size();
        for(size_t i = 0; i < size; i++)
            meshes[i].Draw(shader, core);
    }

    bool intersect(Intersection& intersection){
        // save world coordinate ray
        Ray orig_ray = intersection.m_ray;
        intersection.m_ray = orig_ray.transform(modelMatrix.inverted());
        // init for this model
        intersection.model_intersected = false;
        intersection.m_model_t = kRayTMax;
        // aabb
        if(!aabb.intersect(intersection)){
            intersection.m_ray = orig_ray;
            return false;
        }
        size_t size = meshes.size();
        for(size_t i = 0; i < size; i++){
            meshes[i].intersect(intersection);
        }
        intersection.m_ray = orig_ray;
        if(intersection.model_intersected) {
            Point world_intersect = Point(modelMatrix * intersection.m_model_intersect.toQVector3D());
            float world_t = (world_intersect.m_x - intersection.m_ray.m_origin.m_x) / intersection.m_ray.m_direction.m_x;
            if(world_t < intersection.m_t){
                intersection.m_pModel = this;
                intersection.m_intersect = world_intersect;
                intersection.m_t = world_t;
                intersection.m_normal = Vector(modelMatrix * intersection.m_model_normal.toQVector3D()).normalized();

            }
        }
        return intersection.model_intersected;
    }

private:
    /*  Functions   */
    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(string const &path, QOpenGLFunctions_3_3_Core &core)
    {
        // read file via ASSIMP
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
        // check for errors
        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
        // retrieve the directory path of the filepath
        directory = path.substr(0, path.find_last_of('/'));

        // process ASSIMP's root node recursively
        processNode(scene->mRootNode, scene, core);
    }

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene, QOpenGLFunctions_3_3_Core &core)
    {
        // process each mesh located at the current node
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            // the node object only contains indices to index the actual objects in the scene.
            // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene, core));
        }
        // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, core);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene, QOpenGLFunctions_3_3_Core &core)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Face> faces;
        vector<Texture> textures;
        AABB mesh_aabb;

        // Walk through each of the mesh's vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // AABB
            mesh_aabb.xmin = min(mesh_aabb.xmin, vector.x);
            mesh_aabb.xmax = max(mesh_aabb.xmax, vector.x);
            mesh_aabb.ymin = min(mesh_aabb.ymin, vector.y);
            mesh_aabb.ymax = max(mesh_aabb.ymax, vector.y);
            mesh_aabb.zmin = min(mesh_aabb.zmin, vector.z);
            mesh_aabb.zmax = max(mesh_aabb.zmax, vector.z);

            // normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            // texture coordinates
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
            vertices.push_back(vertex);
        }
        // update model's AABB
        aabb.xmin = min(mesh_aabb.xmin, aabb.xmin);
        aabb.xmax = max(mesh_aabb.xmax, aabb.xmax);
        aabb.ymin = min(mesh_aabb.ymin, aabb.ymin);
        aabb.ymax = max(mesh_aabb.ymax, aabb.ymax);
        aabb.zmin = min(mesh_aabb.zmin, aabb.zmin);
        aabb.zmax = max(mesh_aabb.zmax, aabb.zmax);

        // now walk through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            Face mesh_face;
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
                mesh_face.vertexIndices.push_back(face.mIndices[j]);
            }
            faces.push_back(mesh_face);
        }
        // process materials
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
        // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
        // Same applies to other texture as the following list summarizes:
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse", core);
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular", core);
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal", core);
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height", core);
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

        // return a mesh object created from the extracted mesh data
        return Mesh(vertices, indices, faces, textures, mesh_aabb, core);
    }

    unsigned int TextureFromFile(const char *path, const string &directory, bool gamma, unsigned char* &data, int &width, int &height, int &nrComponents, QOpenGLFunctions_3_3_Core &core)
    {
        string filename = string(path);
        filename = directory + '/' + filename;

        unsigned int textureID;
        core.glGenTextures(1, &textureID);

        data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            core.glBindTexture(GL_TEXTURE_2D, textureID);
            core.glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            core.glGenerateMipmap(GL_TEXTURE_2D);

            core.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            core.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            core.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            core.glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

//            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }

        return textureID;
    }

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName, QOpenGLFunctions_3_3_Core &core)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            // check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                    break;
                }
            }
            if(!skip)
            {   // if texture hasn't been loaded already, load it
                Texture texture;
                unsigned char* data;
                int width, height, nrComponents;
                texture.id = TextureFromFile(str.C_Str(), this->directory, false, data, width, height, nrComponents, core);
                texture.type = typeName;
                texture.path = str.C_Str();
                texture.data = data;
                texture.width = width;
                texture.height = height;
                texture.nrComponents = nrComponents;
                textures.push_back(texture);
                textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
            }
        }
        return textures;
    }

    Color sampleFromTexture(Texture &texture, float u, float v)
    {
        float x = (texture.width - 1)* u;
        float y = (texture.height - 1) * v;
        int x1 = qFloor(x);
        int x2 = qCeil(x);
        int y1 = qFloor(y);
        int y2 = qCeil(y);
        float u2 = x - x1;
        float v2 = y - y1;
        Color color11 = readFromTexture(texture, x1, y1);
        Color color12 = readFromTexture(texture, x1, y2);
        Color color21 = readFromTexture(texture, x2, y1);
        Color color22 = readFromTexture(texture, x2, y2);
        return color11*u2*v2 + color12*u2*(1-v2)+ color21*(1-u2)*v2 + color22*(1-u2)*(1-v2);
    }

    Color readFromTexture(Texture &texture, int x, int y)
    {
        int index = (y * texture.width + x) * texture.nrComponents;
        float r = texture.data[index] / 255.0;
        float g = texture.data[index+1] / 255.0;
        float b = texture.data[index+2] / 255.0;
        float a = texture.data[index+3] / 255.0;
        return Color(r*a, g*a, b*a);
    }
};

#endif
