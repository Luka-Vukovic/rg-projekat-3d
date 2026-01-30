#ifndef CHAIR_HPP
#define CHAIR_HPP
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Cuboid.hpp"
#include "Cinema.hpp"

struct ChairData {
    std::vector<CuboidData> parts;
};

// Funkcija koja kreira celu stolicu na osnovu zadatih dimenzija i teksture
ChairData createChair(glm::vec3 position, float width, float height, float depth, unsigned int texture);

// Funkcija za crtanje svih delova stolice
void drawChair(const ChairData& chair);

struct CinemaSeatsData {
    std::vector<ChairData> chairs;
    std::vector<glm::vec3> positions;
    std::vector<std::pair<int, int>> seatIndices; // (row, col) za svaku stolicu
    float chairWidth;   // Čuvamo dimenzije za ažuriranje
    float chairHeight;
    float chairDepth;
};

// Funkcija za kreiranje svih stolica u bioskopu
CinemaSeatsData createCinemaSeats(
    Cinema& cinema,
    float chairWidth,
    float chairHeight,
    float chairDepth,
    float rightWallMargin,
    float chairSpacing,
    unsigned int availableTexture,
    unsigned int reservedTexture,
    unsigned int boughtTexture,
    float roomWidth,
    float roomHeight,
    float roomDepth,
    float roomFrontZ,
    float screenZ,
    float floorY,
    float stepHeight,
    float stepDepth,
    float distanceFromScreen
);

// Funkcija za ažuriranje tekstura stolica na osnovu stanja u Cinema objektu
void updateCinemaSeatsTextures(CinemaSeatsData& seats, const Cinema& cinema,
    unsigned int availableTexture,
    unsigned int reservedTexture,
    unsigned int boughtTexture);

void drawCinemaSeats(const CinemaSeatsData& seats);

#endif
#pragma once