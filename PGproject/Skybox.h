#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include "shader_m.h"

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    void Draw(Shader& shader, const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int loadCubemap(const std::vector<std::string>& faces);
    void setupSkybox();

    unsigned int cubemapTexture;
    unsigned int skyboxVAO, skyboxVBO;
};

#endif
