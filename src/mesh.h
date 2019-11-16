#ifndef MESH_H
#define MESH_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

#include "aabb.h"
#include "raytracing/basic.h"

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
};

struct Face {
    std::vector<unsigned int> vertexIndices;
};

struct Texture {
    unsigned int id;
    string type;
    string path;
    unsigned char* data;
    int width;
    int height;
    int nrComponents;
};

class Mesh {
public:
    /*  Mesh Data  */
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Face> faces;
    vector<Texture> textures;
    unsigned int VAO;
    AABB aabb;

    /*  Functions  */
    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Face> faces,
         vector<Texture> textures, AABB aabb,
         QOpenGLFunctions_3_3_Core &core)
    {
        this->vertices = vertices;
        this->indices = indices;
        this->faces = faces;
        this->textures = textures;
        this->aabb = aabb;

        // now that we have all the required data, set the vertex buffers and its attribute pointers.
        setupMesh(core);
    }

    // render the mesh
    void Draw(QOpenGLShaderProgram& shader, QOpenGLFunctions_3_3_Core &core)
    {
        // bind appropriate textures
        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            core.glActiveTexture(GL_TEXTURE0 + i); // active proper texture unit before binding
            // retrieve texture number (the N in diffuse_textureN)
            string number;
            string name = textures[i].type;
            if(name == "texture_diffuse")
				number = std::to_string(diffuseNr++);
			else if(name == "texture_specular")
				number = std::to_string(specularNr++); // transfer unsigned int to stream
            else if(name == "texture_normal")
				number = std::to_string(normalNr++); // transfer unsigned int to stream
             else if(name == "texture_height")
			    number = std::to_string(heightNr++); // transfer unsigned int to stream

													 // now set the sampler to the correct texture unit
            shader.setUniformValue((name + number).c_str(), i);
            // and finally bind the texture
            core.glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }

        // draw mesh
        core.glBindVertexArray(VAO);
        core.glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        core.glBindVertexArray(0);

        // always good practice to set everything back to defaults once configured.
        core.glActiveTexture(GL_TEXTURE0);
    }

    bool intersect(Intersection& intersection){
        if(!aabb.intersect(intersection)){
            return false;
        }
        bool flag = false;
        size_t num_face = faces.size();
        for(size_t i=0; i<num_face; i++){
            size_t num_vertice = faces[i].vertexIndices.size();
            for(size_t j=1; j<=num_vertice-2; j++){
                if(intersectTriangle(faces[i], j, intersection)){
                    flag = true;
                }
            }
        }
        return flag;
    }

private:
    /*  Render data  */
    unsigned int VBO, EBO;

    /*  Functions    */
    // initializes all the buffer objects/arrays
    void setupMesh(QOpenGLFunctions_3_3_Core &core)
    {
        // create buffers/arrays
        core.glGenVertexArrays(1, &VAO);
        core.glGenBuffers(1, &VBO);
        core.glGenBuffers(1, &EBO);

        core.glBindVertexArray(VAO);
        // load data into vertex buffers
        core.glBindBuffer(GL_ARRAY_BUFFER, VBO);
        // A great thing about structs is that their memory layout is sequential for all its items.
        // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
        // again translates to 3/2 floats which translates to a byte array.
        core.glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        core.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        core.glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // set the vertex attribute pointers
        // vertex Positions
        core.glEnableVertexAttribArray(0);
        core.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        core.glEnableVertexAttribArray(1);
        core.glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        core.glEnableVertexAttribArray(2);
        core.glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        core.glEnableVertexAttribArray(3);
        core.glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        core.glEnableVertexAttribArray(4);
        core.glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

        core.glBindVertexArray(0);
    }

    bool intersectTriangle(Face& face, unsigned int tri, Intersection& intersection)
    {
        unsigned int v0 = face.vertexIndices[0];
        unsigned int v1 = face.vertexIndices[tri];
        unsigned int v2 = face.vertexIndices[tri + 1];

        Vector v0To1(Vector(vertices[v1].Position - vertices[v0].Position));
        Vector v0To2(Vector(vertices[v2].Position - vertices[v0].Position));
        Vector gnormal = cross(v0To1, v0To2);
        float det = -dot(intersection.m_ray.m_direction, gnormal);
        if (det == 0.0f)
            return false;

        Vector rOriginToV0 = Vector(vertices[v0].Position) - intersection.m_ray.m_origin;
        Vector rayVertCross = cross(intersection.m_ray.m_direction, rOriginToV0);
        Vector rOriginToV1 = Vector(vertices[v1].Position) - intersection.m_ray.m_origin;
        float invDet = 1.0f / det;

        // Calculate barycentric gamma coord
        float gamma = -dot(rOriginToV1, rayVertCross) * invDet;
        if (gamma < 0.0f || gamma > 1.0f)
            return false;

        Vector rOriginToV2 = Vector(vertices[v2].Position) - intersection.m_ray.m_origin;

        // Calculate barycentric beta coord
        float beta = dot(rOriginToV2, rayVertCross) * invDet;
        if (beta < 0.0f || beta + gamma > 1.0f)
            return false;

        float t = -dot(rOriginToV0, gnormal) * invDet;
        if (t < kRayTMin || t >= intersection.m_t)
            return false;

        float alpha = 1.0f - beta - gamma;

        // Calculate shading normal...
        Vector shadingNormal;
        Vector normal1(vertices[v0].Normal);
        Vector normal2(vertices[v1].Normal);
        Vector normal3(vertices[v2].Normal);

        // Weight normals at each vertex by barycentric coords to create
        // the interpolated normal at the intersection point.
        shadingNormal = (normal1 * alpha) +
                        (normal2 * beta) +
                        (normal3 * gamma);
        shadingNormal.normalize();

        if(intersection.model_intersected){
            if(t < intersection.m_model_t){
                intersection.m_model_t = t;
                intersection.m_model_intersect = intersection.model_intersect(t);
                intersection.m_model_normal = shadingNormal;
                return true;
            }
        }
        else{
            intersection.model_intersected = true;
            intersection.m_model_t = t;
            intersection.m_model_intersect = intersection.model_intersect(t);
            intersection.m_model_normal = shadingNormal;
            return true;
        }
        return false;
    }
};
#endif
