#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "ShaderClass.h"

class Skybox {
public:
    GLuint textureID;
    GLuint VAO, VBO;

    Skybox(const std::vector<std::string>& faces);
    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection);
};

#endif