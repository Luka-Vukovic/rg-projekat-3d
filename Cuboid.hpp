#ifndef CUBOID_HPP
#define CUBOID_HPP

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Struktura za čuvanje podataka o kvadru
struct CuboidData {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int textures[6];
};

CuboidData createCuboid(glm::vec3 frontTopLeft, float width, float height, float depth,
    std::vector<glm::vec4> colors, std::vector<unsigned int> textures = {},
    bool inverted = false);

void drawCuboid(const CuboidData& cuboid);

struct RectangleData {
    unsigned int VAO;
    unsigned int VBO;
    unsigned int texture;
};

// Funkcija za kreiranje pravougaonika (quad)
RectangleData createRectangle(glm::vec3 center, float width, float height,
    glm::vec4 color, unsigned int texture = 0,
    bool facingCamera = true);

// Funkcija za crtanje pravougaonika
void drawRectangle(const RectangleData& rectangle);

// Funkcija za kreiranje 2D overlay pravougaonika (za UI elemente)
RectangleData create2DOverlay(float x, float y, float width, float height,
    glm::vec4 color, unsigned int texture = 0);

struct StaircaseData {
    std::vector<CuboidData> steps;
};

StaircaseData createStaircase(float distanceFromScreen, float stepHeight, float stepDepth,
    int numSteps, glm::vec4 stepColor,
    float roomWidth, float roomHeight, float roomDepth, float roomFrontZ, float screenZ, float floorY);

void drawStaircase(const StaircaseData& staircase);

#endif
#pragma once
