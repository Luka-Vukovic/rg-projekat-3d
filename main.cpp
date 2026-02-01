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
#include "Chair.hpp"

Cinema cinema = Cinema();
CinemaSeatsData allChairs;
bool needsChairUpdate = false;

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

// Slaba svetla za SELLING i PLAYING
const glm::vec3 LIGHT_AMBIENT_DIM = glm::vec3(0.08f, 0.08f, 0.08f);
const glm::vec3 LIGHT_DIFFUSE_DIM = glm::vec3(0.15f, 0.15f, 0.2f);
const glm::vec3 LIGHT_SPECULAR_DIM = glm::vec3(0.05f, 0.05f, 0.05f);

// Jaka svetla za ENTERING i LEAVING
const glm::vec3 LIGHT_AMBIENT_BRIGHT = glm::vec3(0.5f, 0.5f, 0.5f);
const glm::vec3 LIGHT_DIFFUSE_BRIGHT = glm::vec3(1.3f, 1.3f, 1.6f);
const glm::vec3 LIGHT_SPECULAR_BRIGHT = glm::vec3(0.3f, 0.3f, 0.3f);

const glm::vec3 SCREEN_LIGHT_COLOR = glm::vec3(0.9f, 0.85f, 0.75f);
const float SCREEN_LIGHT_INTENSITY = 1.8f;    // Pojačano jer delimo sa 3
const float SCREEN_LIGHT_RADIUS = 10.0f;      // Manji radijus jer imamo više izvora

// Pozicije svetla - levo, centralno, desno
glm::vec3 screenLightPositions[3] = {
    glm::vec3(SCREEN_CENTER.x - SCREEN_WIDTH * 0.33f, SCREEN_CENTER.y, SCREEN_Z + 2.5f),  // Levo
    glm::vec3(SCREEN_CENTER.x,                         SCREEN_CENTER.y, SCREEN_Z + 2.5f),  // Centralno
    glm::vec3(SCREEN_CENTER.x + SCREEN_WIDTH * 0.33f, SCREEN_CENTER.y, SCREEN_Z + 2.5f)   // Desno
};

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
glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 2.0);;
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);

// Funkcija za ray casting - pronalazi na koju stolicu je kliknuto
bool rayIntersectsChair(glm::vec3 rayOrigin, glm::vec3 rayDirection,
    glm::vec3 chairPos, float width, float height, float depth,
    int& row, int& col) {
    // AABB (Axis-Aligned Bounding Box) test
    // Izračunaj granice stolice
    glm::vec3 boxMin = glm::vec3(chairPos.x, chairPos.y - height, chairPos.z - depth);
    glm::vec3 boxMax = glm::vec3(chairPos.x + width, chairPos.y, chairPos.z);

    float tMin = 0.0f;
    float tMax = 1000.0f;  // Maksimalna udaljenost

    // Test za svaku osu
    for (int i = 0; i < 3; i++) {
        if (abs(rayDirection[i]) < 0.001f) {
            // Ray je paralelan sa ovom ravni
            if (rayOrigin[i] < boxMin[i] || rayOrigin[i] > boxMax[i]) {
                return false;
            }
        }
        else {
            float t1 = (boxMin[i] - rayOrigin[i]) / rayDirection[i];
            float t2 = (boxMax[i] - rayOrigin[i]) / rayDirection[i];

            if (t1 > t2) std::swap(t1, t2);

            tMin = std::max(tMin, t1);
            tMax = std::min(tMax, t2);

            if (tMin > tMax) return false;
        }
    }

    return tMin < tMax;
}

// Funkcija za pronalaženje stolice na koju je kliknuto
bool findClickedChair(glm::vec3 cameraPos, glm::vec3 cameraFront,
    const CinemaSeatsData& seats, int& clickedRow, int& clickedCol) {

    float closestDistance = 1000.0f;
    bool found = false;
    int hitCount = 0;

    for (size_t i = 0; i < seats.chairs.size(); i++) {
        int row = seats.seatIndices[i].first;
        int col = seats.seatIndices[i].second;

        if (rayIntersectsChair(cameraPos, cameraFront, seats.positions[i],
            seats.chairWidth, seats.chairHeight, seats.chairDepth,
            row, col)) {

            hitCount++;
            float distance = glm::length(seats.positions[i] - cameraPos);

            if (distance < closestDistance) {
                closestDistance = distance;
                clickedRow = row;
                clickedCol = col;
                found = true;
            }
        }
    }

    return found;
}

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

    if (cinema.GetCinemaState() != CinemaState::SELLING) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {

        int clickedRow = -1, clickedCol = -1;

        if (findClickedChair(cameraPos, cameraFront, allChairs, clickedRow, clickedCol)) {

            SeatState before = cinema.GetSeatState(clickedRow, clickedCol);

            cinema.ToggleSeat(clickedRow, clickedCol);

            SeatState after = cinema.GetSeatState(clickedRow, clickedCol);

            needsChairUpdate = true;
        }
    }
}

bool numberKeysActive[10] = { false };

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    for (int i = 1; i <= 9; i++) {
        if (key == GLFW_KEY_0 + i || key == GLFW_KEY_KP_0 + i) {
            if (action == GLFW_PRESS && !numberKeysActive[i]) {
                cinema.BuySeats(i);
                numberKeysActive[i] = true;
                needsChairUpdate = true;  // DODAJ OVO
            }
            else if (action == GLFW_RELEASE) {
                numberKeysActive[i] = false;
            }
        }
    }

    /*if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
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

    // Provera kolizije sa stepeništem (glavni koraci I međukoraci)
    float stepZ = SCREEN_Z + DISTANCE_FROM_SCREEN;
    float stepY = ROOM_FRONT_TOP_LEFT.y - ROOM_HEIGHT;

    // Dimenzije međukoraka (iste kao u createStaircase)
    float subStepWidth = ROOM_WIDTH * 0.25f;
    float subStepHeight = STEP_HEIGHT * 0.5f;
    float subStepDepth = STEP_DEPTH * 0.5f;

    for (int i = 0; i < NUM_STEPS; i++) {
        float currentStepDepth = STEP_DEPTH;
        if (i == NUM_STEPS - 1) {
            currentStepDepth = ROOM_DEPTH - stepZ + SCREEN_Z;
        }

        // === PROVERA GLAVNOG KORAKA ===
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

        // === PROVERA MEĐUKORAKA (levi međukorak) ===
        // Međukorak je pozicioniran na prednjoj ivici glavnog koraka, levo
        float subStepMinX = -ROOM_WIDTH / 2.0f;
        float subStepMaxX = -ROOM_WIDTH / 2.0f + subStepWidth + MARGIN;
        float subStepMinY = stepY;  // Počinje od poda glavnog koraka
        float subStepMaxY = stepY + subStepHeight + MARGIN;
        float subStepMinZ = stepZ;  // Prednja ivica glavnog koraka
        float subStepMaxZ = stepZ + subStepDepth + MARGIN;

        // Provera kolizije sa međukorakom
        if (pos.x >= subStepMinX && pos.x <= subStepMaxX &&
            pos.y >= subStepMinY && pos.y <= subStepMaxY &&
            pos.z >= subStepMinZ && pos.z <= subStepMaxZ) {
            return false;
        }

        // Provera ako pokušava proći kroz međukorak
        if (pos.x >= subStepMinX && pos.x <= subStepMaxX &&
            pos.y < subStepMaxY &&
            pos.z >= subStepMinZ && pos.z <= subStepMaxZ) {
            return false;
        }

        // Pomeramo se za sledeći korak (nazad i gore)
        stepZ += STEP_DEPTH;
        stepY += STEP_HEIGHT;
    }

    return true;
}

// Dodaj pre main():
void updateLightsForState(std::vector<Light>& lights, CinemaState state) {
    glm::vec3 ambient, diffuse, specular;

    if (state == CinemaState::ENTERING || state == CinemaState::LEAVING) {
        ambient = LIGHT_AMBIENT_BRIGHT;
        diffuse = LIGHT_DIFFUSE_BRIGHT;
        specular = LIGHT_SPECULAR_BRIGHT;
    }
    else {
        // SELLING ili PLAYING - slaba svetla
        ambient = LIGHT_AMBIENT_DIM;
        diffuse = LIGHT_DIFFUSE_DIM;
        specular = LIGHT_SPECULAR_DIM;
    }

    for (auto& light : lights) {
        light.ambient = ambient;
        light.diffuse = diffuse;
        light.specular = specular;
    }
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    for (int i = 0; i < 28; i++) {
        std::string path = "res/movie_frames/frame" + std::to_string(i + 1) + ".png";
        screenTextures[i] = preprocessTexture(path.c_str());
    }

    unsigned int availableChairTex = preprocessTexture("res/textures/blue_seat.jpg");
    unsigned int reservedChairTex = preprocessTexture("res/textures/yellow_seat.jpg");
    unsigned int boughtChairTex = preprocessTexture("res/textures/red_seat.jpg");

    unsigned int crosshairTex = preprocessTexture("res/textures/crosshair.png");

    unsigned int doorFullTex = preprocessTexture("res/textures/door_full.jpg");

    unsigned int doorLeftTex = preprocessTexture("res/textures/door_left_main.jpg");
    unsigned int doorRightTex = preprocessTexture("res/textures/door_right_main.jpg");
    unsigned int doorOutsideTex = preprocessTexture("res/textures/door_outside.jpg");
    unsigned int doorInsideTex = preprocessTexture("res/textures/door_inside.jpg");
    unsigned int doorTopTex = preprocessTexture("res/textures/door_top.jpg");
    unsigned int doorBottomTex = preprocessTexture("res/textures/door_bottom.jpg");

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Kreiranje shadera
    unsigned int unifiedShader = createShader("basic.vert", "basic.frag");
    glUseProgram(unifiedShader);
    glUniform1i(glGetUniformLocation(unifiedShader, "uTex"), 0);

    unsigned int uiShader = createShader("ui.vert", "ui.frag");

    // Definisanje boja za svaku stranu KORISTEĆI GLOBALNE KONSTANTE
    std::vector<glm::vec4> roomColors = {
        glm::vec4(0.32, 0.32, 0.32, 1.0),  // Prednja
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

    CuboidData leftDoor = createCuboid(
        glm::vec3(-9.53f, 0.4f, SCREEN_Z + 0.94f),
        0.12f,
        2.4f,
        0.93f,
        roomColors,
        {doorInsideTex, doorLeftTex, doorBottomTex, doorTopTex, doorRightTex, doorOutsideTex},
        false
    );

    CuboidData rightDoor = createCuboid(
        glm::vec3(-7.79f, 0.4f, SCREEN_Z + 0.94f),
        0.12f,
        2.4f,
        0.93f,
        roomColors,
        {doorInsideTex, doorLeftTex, doorBottomTex, doorTopTex, doorRightTex, doorOutsideTex},
        false
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

    RectangleData openedDoor = createRectangle(
        glm::vec3(-8.6f, -0.8f, SCREEN_Z + 0.01f),
        1.86f,
        2.4f,
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), // Bela boja
        0,  // Bez teksture (možeš dodati teksturu kasnije)
        true // Okrenut ka kameri
    );

    RectangleData closedDoor = createRectangle(
        glm::vec3(-8.6f, -0.8f, SCREEN_Z + 0.01f),
        1.86f,
        2.4f,
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), // Bela boja
        doorFullTex,  // Bez teksture (možeš dodati teksturu kasnije)
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

    allChairs = createCinemaSeats(
        cinema,
        0.8f,                    // Širina stolice
        1.2f,                    // Visina stolice
        0.8f,                    // Dubina stolice
        1.5f,                    // Razmak od desnog zida
        0.3f,                    // Razmak između stolica
        availableChairTex,       // Tekstura za dostupne stolice
        reservedChairTex,        // Tekstura za rezervisane stolice
        boughtChairTex,          // Tekstura za kupljene stolice
        ROOM_WIDTH,
        ROOM_HEIGHT,
        ROOM_DEPTH,
        ROOM_FRONT_TOP_LEFT.z,
        SCREEN_Z,
        ROOM_FRONT_TOP_LEFT.y - ROOM_HEIGHT,
        STEP_HEIGHT,
        STEP_DEPTH,
        DISTANCE_FROM_SCREEN
    );

    // Svetla na plafonu - koriste globalne konstante
    std::vector<Light> lights;

    float ceilingY = ROOM_FRONT_TOP_LEFT.y - 0.2f;
    float lightZ1 = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH * 0.25f;
    float lightZ2 = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH * 0.5f;
    float lightZ3 = ROOM_FRONT_TOP_LEFT.z + ROOM_DEPTH * 0.75f;

    // Čuvamo pozicije svetla za lakše ažuriranje
    std::vector<glm::vec3> lightPositions = {
        glm::vec3(-ROOM_WIDTH / 4, ceilingY, lightZ1),
        glm::vec3(ROOM_WIDTH / 4, ceilingY, lightZ1),
        glm::vec3(-ROOM_WIDTH / 4, ceilingY, lightZ2),
        glm::vec3(ROOM_WIDTH / 4, ceilingY, lightZ2),
        glm::vec3(-ROOM_WIDTH / 4, ceilingY, lightZ3),
        glm::vec3(ROOM_WIDTH / 4, ceilingY, lightZ3)
    };

    // Inicijalno kreiranje svetla (SELLING = dim)
    for (const auto& pos : lightPositions) {
        lights.push_back(Light(
            pos,
            LIGHT_AMBIENT_DIM,
            LIGHT_DIFFUSE_DIM,
            LIGHT_SPECULAR_DIM,
            1.0f, 0.045f, 0.0075f
        ));
    }

    RectangleData crosshair = create2DOverlay(
        0.0f, 0.0f,           // Centar ekrana (NDC koordinate)
        0.05f, 0.089f,         // Veličina (NDC koordinate - probaj različite vrednosti)
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), // Bela boja
        crosshairTex          // Tekstura
    );

    // Uniforme
    glm::mat4 model = glm::mat4(1.0f);
    unsigned int modelLoc = glGetUniformLocation(unifiedShader, "uM");

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

        static CinemaState lastState = CinemaState::SELLING;
        CinemaState currentState = cinema.GetCinemaState();
        if (currentState != lastState) {
            updateLightsForState(lights, currentState);
            lastState = currentState;
        }

        if (needsChairUpdate) {
            updateCinemaSeatsTextures(allChairs, cinema, availableChairTex, reservedChairTex, boughtChairTex);
            needsChairUpdate = false;
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
                needsChairUpdate = true;
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
                needsChairUpdate = true;
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
                needsChairUpdate = true;
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

        // Screen light - samo aktivno kad je PLAYING
        if (playingStart != NULL) {
            glUniform1i(glGetUniformLocation(unifiedShader, "useScreenLight"), 1);
            glUniform3fv(glGetUniformLocation(unifiedShader, "screenLightPositions"), 3, &screenLightPositions[0][0]);
            glUniform3fv(glGetUniformLocation(unifiedShader, "screenLightColor"), 1, &SCREEN_LIGHT_COLOR[0]);
            glUniform1f(glGetUniformLocation(unifiedShader, "screenLightIntensity"), SCREEN_LIGHT_INTENSITY);
            glUniform1f(glGetUniformLocation(unifiedShader, "screenLightRadius"), SCREEN_LIGHT_RADIUS);
        }
        else {
            glUniform1i(glGetUniformLocation(unifiedShader, "useScreenLight"), 0);
        }

        // Crtanje prostorije
        glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 0);
        drawCuboid(room);

        // Crtanje stepeništa
        drawStaircase(staircase);

        glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 1);
        drawCinemaSeats(allChairs);

        if ((cinema.GetCinemaState() == CinemaState::SELLING) || (cinema.GetCinemaState() == CinemaState::PLAYING)) {
            drawRectangle(closedDoor);
        }
        else {
            drawCuboid(leftDoor);
            drawCuboid(rightDoor);
            glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 0);
            drawRectangle(openedDoor);
        }

        if (playingStart != NULL) {
            glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 1);
        }
        else {
            // Ako film ne traje, platno je obična bela površina
            glUniform1i(glGetUniformLocation(unifiedShader, "useTex"), 0);
        }
        drawRectangle(screen);

        // --- CRTANJE CROSSHAIR-A ---
        if (cinema.GetCinemaState() == CinemaState::SELLING) {
            glDisable(GL_DEPTH_TEST);

            // Koristi UI shader umesto unifiedShader
            glUseProgram(uiShader);

            // Aktiviraj teksturu
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, crosshairTex);
            glUniform1i(glGetUniformLocation(uiShader, "uTex"), 0);
            glUniform1i(glGetUniformLocation(uiShader, "useTex"), 1);

            drawRectangle(crosshair);

            glEnable(GL_DEPTH_TEST);
        }

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
    for (auto& chair : allChairs.chairs) {
        for (auto& part : chair.parts) {
            glDeleteBuffers(1, &part.VBO);
            glDeleteVertexArrays(1, &part.VAO);
        }
    }

    glfwTerminate();
    return 0;
}