#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/matrix_inverse.hpp> 
#include <glm/gtc/type_ptr.hpp> 

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;





// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 iepureModel;


// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(0.0f, 1.0f, 3.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.03f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
GLfloat angle;

// shaders
gps::Shader myBasicShader;






bool jump = false; // Flag pentru a indica dacă iepurele sare
float jumpHeight = 0.0f; // Înălțimea curentă a săriturii
float jumpSpeed = 0.05f; // Viteza săriturii
float maxJumpHeight = 0.5f; // Înălțimea maximă a săriturii

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    //TODO
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS && !jump) {
        jump = true;
    }

}
GLboolean mouseLeftButtonPressed = false;
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = xpos, lastY = ypos; // Ultima poziție a mouse-ului
    static bool firstClick = true;

    // Verificăm dacă butonul stâng al mouse-ului este apăsat
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        if (firstClick) {
            // Dacă este prima dată când este apăsat click-ul, salvăm poziția
            lastX = xpos;
            lastY = ypos;
            firstClick = false; // Nu mai este prima dată
        }

        // Calculăm mișcarea pe axele X și Y
        double xOffset = xpos - lastX;
        double yOffset = lastY - ypos; // inversăm pentru a corecta axa Y

        lastX = xpos; // actualizăm ultima poziție
        lastY = ypos;

        // Sensibilitate pentru mișcare
        float sensitivity = 0.1f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        // Actualizăm rotația camerei
        myCamera.rotate((float)yOffset, (float)xOffset);

        // Actualizăm matricea de vizualizare
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    }
    else {
        // Dacă butonul nu mai este apăsat, resetăm flag-ul de prima apăsare
        firstClick = true;
    }
}


void  initSkybox()
{
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/posx.jpg");
    faces.push_back("skybox/negx.jpg");
    faces.push_back("skybox/posy.jpg");
    faces.push_back("skybox/negy.jpg");
    faces.push_back("skybox/posz.jpg");
    faces.push_back("skybox/negz.jpg");
    mySkyBox.Load(faces);


}

gps::Model3D iepure;


void updateIepureJump() {
    static bool goingUp = true; // Indică dacă iepurele urcă sau coboară
    static float jumpHeight = 0.0f; // Înălțimea curentă a săriturii
    static float jumpSpeed = 0.02f; // Viteza săriturii
    static float maxJumpHeight = 0.5f; // Înălțimea maximă a săriturii

    if (jump) {
        if (goingUp) {
            jumpHeight += jumpSpeed;
            if (jumpHeight >= maxJumpHeight) {
                goingUp = false;
            }
        }
        else {
            jumpHeight -= jumpSpeed;
            if (jumpHeight <= 0.0f) {
                jumpHeight = 0.0f;
                jump = false;
                goingUp = true;
            }
        }
        // Actualizează doar matricea modelului iepurelui
        iepureModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, jumpHeight, 0.0f));
    }
}




void processMovement() {
    static float pitch = 0.0f;
    static float yaw = 0.0f;
    if (pressedKeys[GLFW_KEY_UP]) {
        myCamera.move(gps::MOVE_UP, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_DOWN]) {
        myCamera.move(gps::MOVE_DOWN, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

   
    if (pressedKeys[GLFW_KEY_X]) {
        angle += 1.0f; 
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    if (pressedKeys[GLFW_KEY_Z]) {
        angle -= 1.0f; 
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    if (pressedKeys[GLFW_KEY_1]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    if (pressedKeys[GLFW_KEY_2]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
    if (pressedKeys[GLFW_KEY_3]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    // Actualizează matricea de vizualizare și normalMatrix
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}




void initOpenGLWindow() {
    myWindow.Create(1920, 1000, "OpenGL Project Core");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    teapot.LoadModel("models/ferma/FINAL.obj");

    iepure.LoadModel("models/ferma/iepureFINAL.obj"); // Adaugă calea corectă către fișierul .obj
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
    iepureModel = glm::mat4(1.0f);


}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}
void renderIepure(gps::Shader shader) {
    shader.useShaderProgram();
    // Folosește matricea iepureModel pentru a actualiza uniforma "model"
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(iepureModel));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    iepure.Draw(shader);
}



void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene

    // render the teapot
    renderTeapot(myBasicShader);
    renderIepure(myBasicShader);
    mySkyBox.Draw(skyboxShader, view, projection);


}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initSkybox();
    initUniforms();
    
    setWindowCallbacks();




    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        updateIepureJump();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}