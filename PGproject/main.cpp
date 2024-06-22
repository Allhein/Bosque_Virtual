#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <irrKlang.h>

#include "shader_m.h"
#include "camera.h"
#include "model.h"
#include "Skybox.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, const Model& modelo);
bool CheckCollision(const Camera& camera, const Model& model);
void resolveCollision(Camera& camera, const Model& model);

enum class GameMode {
    FreeMode,
    NormalMode
};

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

GameMode getGameMode() {
    std::cout << "Seleccione el modo de juego:\n";
    std::cout << "1. Free Mode (sin colisiones)\n";
    std::cout << "2. Modo Normal (con colisiones)\n";
    int choice;
    std::cin >> choice;

    switch (choice) {
    case 1:
        return GameMode::FreeMode;
    case 2:
        return GameMode::NormalMode;
    default:
        std::cout << "Opción no válida, se seleccionará Free Mode por defecto.\n";
        return GameMode::FreeMode;
    }
}

int width, height;

// camera
Camera camera(glm::vec3(0.2f, 1.4f, 5.5f));
float lastX = width / 2.0f;
float lastY = height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

GameMode mode;

//audio
irrklang::ISoundEngine* SoundEngine = irrklang::createIrrKlangDevice();

int main()
{
    getUserResolution(width, height);
    mode = getGameMode();
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    SoundEngine->play2D("sonidos/forest_amb.ogg", true);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(width, height, "Virtual Forest", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
   //stbi_set_flip_vertically_on_load(true);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader shaderProgram("model_loading.vert", "model_loading.frag");
    Shader skyboxShader("skybox.vert", "skybox.frag");

    std::vector<std::string> faces{
        "skybox/miramar_ft.jpg",
        "skybox/miramar_bk.jpg",
        "skybox/miramar_up.jpeg",
        "skybox/miramar_dn.jpg",
        "skybox/miramar_rt.jpg",
        "skybox/miramar_lf.jpg"
    };

    // load models
    // -----------
    Model modelo("obj/source/Forest.obj");
    Skybox skybox(faces);

    shaderProgram.use();
    shaderProgram.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shaderProgram.setVec3("dirLight.ambient", 0.2f, 0.2f, 0.2f); // Luz ambiental
    shaderProgram.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f); // Luz difusa
    shaderProgram.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f); // luz especular
    shaderProgram.setFloat("material.shininess", 32.0f);


    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window, modelo);

        // render
        // ------
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        shaderProgram.use();

        if (mode == GameMode::NormalMode) {
            if (CheckCollision(camera, modelo)) {
                // Si hay colisión, realiza alguna acción
               // std::cout << "¡Colisión detectada!" << std::endl;
                resolveCollision(camera, modelo);
            }
        }

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));	// it's a bit too big for our scene, so scale it down
        shaderProgram.setMat4("model", model);
        modelo.Draw(shaderProgram);

        // draw skybox
        skybox.Draw(skyboxShader, view, projection);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window, const Model& modelo)
{
   /* if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.MovementSpeed *= 2.0f;
    else if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_RELEASE) {
        camera.MovementSpeed *= 0.5f;
    }*/

    if (mode == GameMode::NormalMode) {
        

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            glm::vec3 front = camera.Front;
            glm::vec3 newPos = camera.Position + front * camera.MovementSpeed * deltaTime;
            if (!CheckCollision(newPos, modelo))
                camera.Position = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            glm::vec3 front = camera.Front;
            glm::vec3 newPos = camera.Position - front * camera.MovementSpeed * deltaTime;
            if (!CheckCollision(newPos, modelo))
                camera.Position = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
            glm::vec3 right = camera.Right;
            glm::vec3 newPos = camera.Position - right * camera.MovementSpeed * deltaTime;
            if (!CheckCollision(newPos, modelo))
                camera.Position = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
            glm::vec3 right = camera.Right;
            glm::vec3 newPos = camera.Position + right * camera.MovementSpeed * deltaTime;
            if (!CheckCollision(newPos, modelo))
                camera.Position = newPos;
        }
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !CheckCollision(camera.Position + camera.Up * camera.MovementSpeed * deltaTime, modelo)) {
            camera.Position += camera.Up * camera.MovementSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !CheckCollision(camera.Position - camera.Up * camera.MovementSpeed * deltaTime, modelo)) {
            camera.Position -= camera.Up * camera.MovementSpeed * deltaTime;
        }
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
    }
    else {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.ProcessKeyboard(UP, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.ProcessKeyboard(DOWN, deltaTime);
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

bool CheckCollision(const Camera& camera, const Model& model) {
    // Obtenemos la posición de la cámara
    glm::vec3 cameraPos = camera.Position;

    // Iteramos sobre las mallas del modelo
    for (const auto& mesh : model.meshes) {
        // Obtenemos la posición y dimensiones de la caja de colisión de la malla
        glm::vec3 minBox = mesh.boundingBox.min;
        glm::vec3 maxBox = mesh.boundingBox.max;

        // Comprobamos si la posición de la cámara está dentro de la caja de colisión
        if (cameraPos.x >= minBox.x && cameraPos.x <= maxBox.x &&
            cameraPos.y >= minBox.y && cameraPos.y <= maxBox.y &&
            cameraPos.z >= minBox.z && cameraPos.z <= maxBox.z) {
            // Si la posición de la cámara está dentro de la caja de colisión, hay colisión
            return true;
        }
    }

    // Si ninguna malla tiene colisión con la cámara, retornamos false
    return false;
}

void resolveCollision(Camera& camera, const Model& model) {
    // Obtenemos la posición de la cámara
    glm::vec3 cameraPos = camera.Position;

    // Iteramos sobre las mallas del modelo
    for (const auto& mesh : model.meshes) {
        // Obtenemos la posición y dimensiones de la caja de colisión de la malla
        glm::vec3 minBox = mesh.boundingBox.min;
        glm::vec3 maxBox = mesh.boundingBox.max;

        // Comprobamos si la posición de la cámara está dentro de la caja de colisión
        if (cameraPos.x >= minBox.x && cameraPos.x <= maxBox.x &&
            cameraPos.y >= minBox.y && cameraPos.y <= maxBox.y &&
            cameraPos.z >= minBox.z && cameraPos.z <= maxBox.z) {
            // Resolvemos la colisión ajustando ligeramente la dirección del movimiento
            // Encuentra el vector desde la cámara hasta el punto medio de la caja de colisión
            glm::vec3 collisionVector = (minBox + maxBox) * 0.5f - cameraPos;

            // Normaliza el vector de colisión para que solo indique la dirección
            collisionVector = glm::normalize(collisionVector);

            // Ajusta la posición de la cámara para que esté justo fuera de la caja de colisión
            glm::vec3 closestPoint = glm::clamp(cameraPos, minBox, maxBox);
            camera.Position = closestPoint + collisionVector * 0.08f; // Deslizamiento mínimo

            break;
        }
    }
}