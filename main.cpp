#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

//GLM biblioteke
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Util.h"
#include "Cuboid.hpp"
#include "Light.hpp"
#include "Cinema.hpp"

Cinema cinema = Cinema();

double enteringStart = NULL;
double playingStart = NULL;
double leavingStart = NULL;

unsigned int screenHeight = 1000;
unsigned int screenWidth = 1000;

float speed = 0.4f;

unsigned screenTextures[28];

// GLOBALNE PROMENLJIVE ZA PROSTORIJU
const float ROOM_WIDTH = 20.0f;
const float ROOM_HEIGHT = 10.0f;
const float ROOM_DEPTH = 30.0f;
const glm::vec3 ROOM_FRONT_TOP_LEFT = glm::vec3(-ROOM_WIDTH / 2, ROOM_HEIGHT - 2.0f, -8.0f);

// PLATNO
const float SCREEN_WIDTH = 16.33f;
const float SCREEN_HEIGHT = 7.0f;
const float SCREEN_Z = ROOM_FRONT_TOP_LEFT.z + 0.01f; // Skoro na zadnjem zidu
const glm::vec3 SCREEN_CENTER = glm::vec3(1.0f, ROOM_HEIGHT * 0.4f, SCREEN_Z);

// STEPENIŠTE
const int NUM_STEPS = 6;              // Broj stepenika
const float STEP_HEIGHT = 0.7f;       // Visina svakog koraka
const float STEP_DEPTH = 2.0f;        // Dužina svakog koraka
const float DISTANCE_FROM_SCREEN = 9.5f; // Udaljenost prvog koraka od platna

int endProgram(std::string message) {
    std::cout << message << std::endl;
    glfwTerminate();
    return -1;
}

unsigned int preprocessTexture(const char* filepath) {
    unsigned int texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    return texture;
}

bool firstMouse = true;
float lastX, lastY = 500.0f;
float yaw = -90.0f, pitch = 0.0f;
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

float fov = 45.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

bool numberKeysActive[10] = { false };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    /*for (int i = 1; i <= 9; i++) {
        if (key == GLFW_KEY_0 + i || key == GLFW_KEY_KP_0 + i) {
            if (action == GLFW_PRESS && !numberKeysActive[i]) {
                cinema.BuySeats(i);
                numberKeysActive[i] = true;
            }
            else if (action == GLFW_RELEASE) {
                numberKeysActive[i] = false;
            }
        }
    }

    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }*/

    if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
        if (enteringStart == NULL && playingStart == NULL && leavingStart == NULL) {
            enteringStart = glfwGetTime();
            cinema.SwitchState();
            cinema.GetRandomTakenSeats();
            // peopleInitialized = false;
        }
    }
}

// Funkcija za proveru da li je pozicija unutar prostorije
bool isPositionValid(glm::vec3 pos) {
    // Konstante za granice prostorije sa malim offsetom (0.5 jedinice od zida)
    const float MARGIN = 0.5f;

    float minX = -ROOM_WIDTH / 2.0f + MARGIN;
    float maxX = ROOM_WIDTH / 2.0f - MARGIN;
    float minY = ROOM_FRONT_TOP_LEFT.y - ROOM_HEIGHT + MARGIN;
    float maxY = ROOM_FRONT_TOP_LEFT.y - MARGIN;
    float minZ = ROOM_FRONT_TOP_LEFT.z + MARGIN;
    float maxZ = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH - MARGIN;

    // Provera granica prostorije
    if (pos.x < minX || pos.x > maxX) return false;
    if (pos.y < minY || pos.y > maxY) return false;
    if (pos.z < minZ || pos.z > maxZ) return false;

    // Provera kolizije sa platnom (malo ispred platna)
    float screenMinX = SCREEN_CENTER.x - SCREEN_WIDTH / 2.0f - MARGIN;
    float screenMaxX = SCREEN_CENTER.x + SCREEN_WIDTH / 2.0f + MARGIN;
    float screenMinY = SCREEN_CENTER.y - SCREEN_HEIGHT / 2.0f - MARGIN;
    float screenMaxY = SCREEN_CENTER.y + SCREEN_HEIGHT / 2.0f + MARGIN;
    float screenZ = SCREEN_Z + MARGIN;

    if (pos.x >= screenMinX && pos.x <= screenMaxX &&
        pos.y >= screenMinY && pos.y <= screenMaxY &&
        pos.z <= screenZ) {
        return false;
    }

    // Provera kolizije sa stepeništem
    float stepZ = SCREEN_Z + DISTANCE_FROM_SCREEN;
    float stepY = ROOM_FRONT_TOP_LEFT.y - ROOM_HEIGHT;

    for (int i = 0; i < NUM_STEPS; i++) {
        float currentStepDepth = STEP_DEPTH;
        if (i == NUM_STEPS - 1) {
            currentStepDepth = ROOM_DEPTH - stepZ + SCREEN_Z;
        }

        float stepMinZ = stepZ;
        float stepMaxZ = stepZ + currentStepDepth + MARGIN;
        float stepMinY = stepY;
        float stepMaxY = stepY + STEP_HEIGHT + MARGIN;

        // Ako je kamera ispod stepenice i unutar njenog Z i X opsega
        if (pos.y >= stepMinY && pos.y <= stepMaxY &&
            pos.z >= stepMinZ && pos.z <= stepMaxZ) {
            return false;
        }

        // Ako pokušava da prođe kroz stepenicu sa strane ili ispod
        if (pos.y < stepMaxY &&
            pos.z >= stepMinZ && pos.z <= stepMaxZ) {
            return false;
        }

        stepZ += STEP_DEPTH;
        stepY += STEP_HEIGHT;
    }

    return true;
}

int main(void)
{
    if (!glfwInit())
    {
        std::cout << "GLFW Biblioteka se nije ucitala! :(\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    screenWidth = mode->width;
    screenHeight = mode->height;

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Bioskop", monitor, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);
    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    for (int i = 0; i < 28; i++) {
        std::string path = "res/movie_frames/frame" + std::to_string(i + 1) + ".png";
        screenTextures[i] = preprocessTexture(path.c_str());
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Kreiranje shadera
    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    glUseProgram(unifiedShader);
    glUniform1i(glGetUniformLocation(unifiedShader, "uTex"), 0);

    // Definisanje boja za svaku stranu KORISTEĆI GLOBALNE KONSTANTE
    std::vector<glm::vec4> roomColors = {
        glm::vec4(0.05, 0.05, 0.05, 1.0),  // Prednja
        glm::vec4(0.32, 0.1, 0.1, 1.0),    // Leva
        glm::vec4(0.24, 0.16, 0.16, 1.0),  // Donja
        glm::vec4(0.2, 0.2, 0.2, 1.0),     // Gornja
        glm::vec4(0.32, 0.1, 0.1, 1.0),    // Desna
        glm::vec4(0.32, 0.1, 0.1, 1.0)     // Zadnja
    };

    // Kreiranje prostorije
    CuboidData room = createCuboid(
        ROOM_FRONT_TOP_LEFT,
        ROOM_WIDTH,
        ROOM_HEIGHT,
        ROOM_DEPTH,
        roomColors,
        {},
        true
    );

    // Kreiranje platna (beli pravougaonik)
    RectangleData screen = createRectangle(
        SCREEN_CENTER,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), // Bela boja
        0,  // Bez teksture (možeš dodati teksturu kasnije)
        true // Okrenut ka kameri
    );

    StaircaseData staircase = createStaircase(
        DISTANCE_FROM_SCREEN,
        STEP_HEIGHT,
        STEP_DEPTH,
        NUM_STEPS,
        glm::vec4(0.24, 0.16, 0.16, 1.0),
        ROOM_WIDTH,
        ROOM_HEIGHT,
        ROOM_DEPTH,
        ROOM_FRONT_TOP_LEFT.z,
        SCREEN_Z,
        ROOM_FRONT_TOP_LEFT.y - ROOM_HEIGHT
    );

    // Svetla na plafonu - koriste globalne konstante
    std::vector<Light> lights;

    float ceilingY = ROOM_FRONT_TOP_LEFT.y - 0.2f; // Malo ispod plafona
    float lightZ1 = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH * 0.25f;
    float lightZ2 = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH * 0.5f;
    float lightZ3 = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH * 0.75f;

    lights.push_back(Light(
        glm::vec3(-ROOM_WIDTH / 4, ceilingY, lightZ1),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.3f, 1.3f, 1.6f),
        glm::vec3(0.3f, 0.3f, 0.3f),
        1.0f, 0.045f, 0.0075f
    ));

    lights.push_back(Light(
        glm::vec3(ROOM_WIDTH / 4, ceilingY, lightZ1),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.3f, 1.3f, 1.6f),
        glm::vec3(0.3f, 0.3f, 0.3f),
        1.0f, 0.045f, 0.0075f
    ));

    lights.push_back(Light(
        glm::vec3(-ROOM_WIDTH / 4, ceilingY, lightZ2),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.3f, 1.3f, 1.6f),
        glm::vec3(0.3f, 0.3f, 0.3f),
        1.0f, 0.045f, 0.0075f
    ));

    lights.push_back(Light(
        glm::vec3(ROOM_WIDTH / 4, ceilingY, lightZ2),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.3f, 1.3f, 1.6f),
        glm::vec3(0.3f, 0.3f, 0.3f),
        1.0f, 0.045f, 0.0075f
    ));

    lights.push_back(Light(
        glm::vec3(-ROOM_WIDTH / 4, ceilingY, lightZ3),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.3f, 1.3f, 1.6f),
        glm::vec3(0.3f, 0.3f, 0.3f),
        1.0f, 0.045f, 0.0075f
    ));

    lights.push_back(Light(
        glm::vec3(ROOM_WIDTH / 4, ceilingY, lightZ3),
        glm::vec3(0.5f, 0.5f, 0.5f),
        glm::vec3(1.3f, 1.3f, 1.6f),
        glm::vec3(0.3f, 0.3f, 0.3f),
        1.0f, 0.045f, 0.0075f
    ));

    // Uniforme
    glm::mat4 model = glm::mat4(1.0f);
    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");

    glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 2.0);
    glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    unsigned int viewLoc = glGetUniformLocation(unifiedShader, "uV");

    glm::mat4 projectionP = glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    unsigned int projectionLoc = glGetUniformLocation(unifiedShader, "uP");

    // Render loop
    glUseProgram(unifiedShader);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));

    glClearColor(0.5, 0.5, 0.5, 1.0);

    // Omogućavanje depth testiranja i face cullinga - standardno i efikasno
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    bool hasOpenedDoor = false;

    while (!glfwWindowShouldClose(window))
    {
        double startTime = glfwGetTime();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // Čuvamo staru poziciju pre nego što je promenimo
        glm::vec3 oldCameraPos = cameraPos;
        glm::vec3 newCameraPos = cameraPos;

        // Kretanje kamere strelicama - kalkulišemo novu poziciju
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            newCameraPos += speed * glm::normalize(glm::vec3(cameraFront.z, 0, -cameraFront.x));
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            newCameraPos -= speed * glm::normalize(glm::vec3(cameraFront.z, 0, -cameraFront.x));
        }
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            newCameraPos += speed * glm::normalize(glm::vec3(cameraFront.x, cameraFront.y, cameraFront.z));
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            newCameraPos -= speed * glm::normalize(glm::vec3(cameraFront.x, cameraFront.y, cameraFront.z));
        }

        // Provera validnosti nove pozicije
        if (isPositionValid(newCameraPos)) {
            cameraPos = newCameraPos;
        }
        else {
            // Pokušaj da klizi duž zida - proveri X i Z odvojeno
            glm::vec3 tryX = glm::vec3(newCameraPos.x, oldCameraPos.y, oldCameraPos.z);
            glm::vec3 tryY = glm::vec3(oldCameraPos.x, newCameraPos.y, oldCameraPos.z);
            glm::vec3 tryZ = glm::vec3(oldCameraPos.x, oldCameraPos.y, newCameraPos.z);

            if (isPositionValid(tryX)) {
                cameraPos.x = newCameraPos.x;
            }
            if (isPositionValid(tryY)) {
                cameraPos.y = newCameraPos.y;
            }
            if (isPositionValid(tryZ)) {
                cameraPos.z = newCameraPos.z;
            }
        }

        if (enteringStart != NULL) {
            /*if (!peopleInitialized) {
                InitializePeople(seatWidth, seatHeight, aspect);
            }
            UpdatePeoplePositions(deltaTime, aspect);*/

            if (glfwGetTime() - enteringStart > 5) {
                enteringStart = NULL;
                cinema.SwitchState();
                //formDoorVAO(VAOdoor, VBOdoor, aspect);
                hasOpenedDoor = false;
                //cinema.SitOnSeats();
                playingStart = glfwGetTime();
            }
            else if (!hasOpenedDoor) {
                //formDoorVAO(VAOdoor, VBOdoor, aspect);
                hasOpenedDoor = true;
            }
        }
        else if (playingStart != NULL) {
            if (glfwGetTime() - playingStart > 20) {
                playingStart = NULL;
                cinema.SwitchState();
                cinema.ResetFrameCounter();
                screen.texture = 0;
                //formDoorVAO(VAOdoor, VBOdoor, aspect);
                //cinema.StandUp();
                leavingStart = glfwGetTime();
                //SetupPersonForExit();
            }
            else {
                cinema.IncreaseFrameCounter();
                screen.texture = screenTextures[cinema.GetMovieFrame()];
            }
        }
        else if (leavingStart != NULL) {
            //UpdatePeoplePositions(deltaTime, aspect);

            if (glfwGetTime() - leavingStart > 5) {
                leavingStart = NULL;
                cinema.SwitchState();
                //formDoorVAO(VAOdoor, VBOdoor, aspect);
                cinema.ResetSeats();
                cinema.ResetSelectedSeats();
                //ResetPeople();
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(unifiedShader);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        projectionP = glm::perspective(glm::radians(fov), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionP));

        glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 0);
        glUniform1i(glGetUniformLocation(unifiedShader, "transparent"), 0);

        setLightsUniforms(unifiedShader, lights);
        glUniform3fv(glGetUniformLocation(unifiedShader, "viewPos"), 1, &cameraPos[0]);

        // Crtanje prostorije
        glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 0);
        drawCuboid(room);

        // Crtanje stepeništa
        drawStaircase(staircase);

        if (playingStart != NULL) {
            glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 1);
        }
        else {
            // Ako film ne traje, platno je obična bela površina
            glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 0);
        }
        drawRectangle(screen);

        while (glfwGetTime() - startTime < 1.0 / 75) {}
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Čišćenje
    glDeleteBuffers(1, &room.VBO);
    glDeleteVertexArrays(1, &room.VAO);
    glDeleteBuffers(1, &screen.VBO);
    glDeleteVertexArrays(1, &screen.VAO);
    glDeleteProgram(unifiedShader);
    for (auto& step : staircase.steps) {
        glDeleteBuffers(1, &step.VBO);
        glDeleteVertexArrays(1, &step.VAO);
    }

    glfwTerminate();
    return 0;
}