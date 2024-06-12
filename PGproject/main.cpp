#include <iostream>
#include <vector>
#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Texture.h"
#include "shaderClass.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camara.h"
#include "Skybox.h"


// Estructura de Material
struct Material {
    glm::vec3 ambient = glm::vec3(1.0f);
    glm::vec3 diffuse = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(1.0f);
    float shininess = 32.0f;
    GLuint diffuseTexture = 0;
    GLuint specularTexture = 0;
};

// Prototipos de función
void processMesh(aiMesh* mesh, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Material>& materials);
void processNode(aiNode* node, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Material>& materials);
GLuint loadTexture(const char* path);

void loadModel(const char* path, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Material>& materials) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    processNode(scene->mRootNode, scene, vertices, indices, materials);
}

void processNode(aiNode* node, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Material>& materials) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, vertices, indices, materials);
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, vertices, indices, materials);
    }
}

void processMesh(aiMesh* mesh, const aiScene* scene, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<Material>& materials) {
    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        vertices.push_back(mesh->mVertices[i].x);
        vertices.push_back(mesh->mVertices[i].y);
        vertices.push_back(mesh->mVertices[i].z);

        if (mesh->mTextureCoords[0]) {
            vertices.push_back(mesh->mTextureCoords[0][i].x);
            vertices.push_back(mesh->mTextureCoords[0][i].y);
        }
        else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }

        if (mesh->mColors[0]) {
            vertices.push_back(mesh->mColors[0][i].r);
            vertices.push_back(mesh->mColors[0][i].g);
            vertices.push_back(mesh->mColors[0][i].b);
        }
        else {
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
        }

        vertices.push_back(mesh->mNormals[i].x);
        vertices.push_back(mesh->mNormals[i].y);
        vertices.push_back(mesh->mNormals[i].z);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    Material material;
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

        aiColor3D color;
        float shininess;

        mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
        material.ambient = glm::vec3(color.r, color.g, color.b);

        mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        material.diffuse = glm::vec3(color.r, color.g, color.b);

        mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
        material.specular = glm::vec3(color.r, color.g, color.b);

        mat->Get(AI_MATKEY_SHININESS, shininess);
        material.shininess = shininess;

        if (mat->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
            aiString str;
            mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
            material.diffuseTexture = loadTexture(str.C_Str());
        }

        if (mat->GetTextureCount(aiTextureType_SPECULAR) > 0) {
            aiString str;
            mat->GetTexture(aiTextureType_SPECULAR, 0, &str);
            material.specularTexture = loadTexture(str.C_Str());
        }
    }
    materials.push_back(material);
}

GLuint loadTexture(const char* path) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrComponents;
    unsigned char* data = stbi_load("Tolet.png", &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture: " << path << std::endl;
    }
    stbi_image_free(data);
    return textureID;
}

void getUserResolution(int& width, int& height) {
    std::cout << "Seleccione la resolución de la ventana:\n";
    std::cout << "1. 800x600\n";
    std::cout << "2. 1024x768\n";
    std::cout << "3. 1280x720\n";
    std::cout << "4. 1920x1080\n";
    std::cout << "5. Personalizada\n";
    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        width = 800;
        height = 600;
        break;
    case 2:
        width = 1024;
        height = 768;
        break;
    case 3:
        width = 1280;
        height = 720;
        break;
    case 4:
        width = 1920;
        height = 1080;
        break;
    case 5:
        std::cout << "Ingrese el ancho de la ventana: ";
        std::cin >> width;
        std::cout << "Ingrese la altura de la ventana: ";
        std::cin >> height;
        break;
    default:
        std::cout << "Opción no válida, se usará la resolución por defecto 800x600.\n";
        width = 800;
        height = 600;
        break;
    }
}

int main() {

    int width, height;
    getUserResolution(width, height);

    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, "Forest", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();

    glViewport(0, 0, 800, 800);

    Shader shaderProgram("default.vert", "default.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");

    std::vector<std::string> faces{
        "skybox/miramar_ft.jpg",
        "skybox/miramar_bk.jpg",
        "skybox/miramar_up.jpeg",
        "skybox/miramar_dn.jpg",
        "skybox/miramar_rt.jpg",
        "skybox/miramar_lf.jpg"
    };

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<Material> materials;
    loadModel("obj/source/BOG.obj", vertices, indices, materials);

    VAO VAO1;
    VAO1.Bind();

    VBO VBO1(vertices.data(), vertices.size() * sizeof(float));
    EBO EBO1(indices.data(), indices.size() * sizeof(unsigned int));

    VAO1.LinkAttrib(VBO1, 0, 3, GL_FLOAT, 11 * sizeof(float), (void*)0);
    VAO1.LinkAttrib(VBO1, 1, 2, GL_FLOAT, 11 * sizeof(float), (void*)(3 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 2, 3, GL_FLOAT, 11 * sizeof(float), (void*)(5 * sizeof(float)));
    VAO1.LinkAttrib(VBO1, 3, 3, GL_FLOAT, 11 * sizeof(float), (void*)(8 * sizeof(float)));

    VAO1.Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    glEnable(GL_DEPTH_TEST);

    shaderProgram.Activate();

    GLuint modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram.ID, "projection");

    GLuint diffuseLoc = glGetUniformLocation(shaderProgram.ID, "material.diffuse");
    GLuint specularLoc = glGetUniformLocation(shaderProgram.ID, "material.specular");
    GLuint shininessLoc = glGetUniformLocation(shaderProgram.ID, "material.shininess");

    glUniform1i(diffuseLoc, 0);
    glUniform1i(specularLoc, 1);

    Camera camara(width, height, glm::vec3(0.0f, 0.0f, 2.0f));

    Skybox skybox(faces);

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        static float lastFrame = 0.0f;
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        camara.Inputs(window);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderProgram.Activate();
        camara.updateMatrix(45.0f, 0.1f, 100.0f);
        camara.Matrix(shaderProgram, "camMatrix");

        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = camara.GetViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        for (const auto& material : materials) {
            glUniform1f(shininessLoc, material.shininess);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, material.diffuseTexture);
            glUniform1i(diffuseLoc, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, material.specularTexture);
            glUniform1i(specularLoc, 1);

            VAO1.Bind();
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            VAO1.Unbind();
        }
        skybox.Draw(skyboxShader, view, projection);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    VAO1.Delete();
    VBO1.Delete();
    EBO1.Delete();
    shaderProgram.Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}