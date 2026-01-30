#include "Chair.hpp"
#include "Cuboid.hpp"
#include "Cinema.hpp"
#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Funkcija za dobijanje teksture na osnovu stanja sedišta
unsigned int getTextureForSeatState(SeatState state,
    unsigned int availableTexture,
    unsigned int reservedTexture,
    unsigned int boughtTexture) {
    switch (state) {
    case SeatState::AVAILABLE:
        return availableTexture;
    case SeatState::RESERVED:
    case SeatState::RESERVED_SITTING:
        return reservedTexture;
    case SeatState::BOUGHT:
    case SeatState::BOUGHT_SITTING:
        return boughtTexture;
    default:
        return availableTexture;
    }
}

ChairData createChair(glm::vec3 pos, float w, float h, float d, unsigned int tex) {
    ChairData chair;
    std::vector<glm::vec4> defaultColors(6, glm::vec4(1.0f));
    std::vector<unsigned int> textures(6, tex);

    // Proporcije
    float armWidth = w * 0.2f;
    float armHeight = h * 0.5f;
    float seatHeight = h * 0.1f;
    float backrestThickness = d * 0.2f;
    float middleWidth = w - (2 * armWidth);
    float upperBackHeight = h - armHeight;
    float frontDepth = d - backrestThickness;

    // 1. GORNJI NASLON
    chair.parts.push_back(createCuboid(pos, w, upperBackHeight, backrestThickness, defaultColors, textures));

    // 2. DONJI NASLON
    glm::vec3 backrestPos2 = pos + glm::vec3(armWidth, -upperBackHeight, 0);
    chair.parts.push_back(createCuboid(backrestPos2, middleWidth, (armHeight * 0.4f + seatHeight), backrestThickness, defaultColors, textures));

    // 3. LEVI RUKOHVAT
    chair.parts.push_back(createCuboid(glm::vec3(pos.x, pos.y - upperBackHeight, pos.z), armWidth, armHeight, d, defaultColors, textures));

    // 4. DESNI RUKOHVAT
    glm::vec3 rightArmPos = glm::vec3(pos.x + w - armWidth, pos.y - upperBackHeight, pos.z);
    chair.parts.push_back(createCuboid(rightArmPos, armWidth, armHeight, d, defaultColors, textures));

    // 5. SEDIŠTE
    float frontZ = pos.z - backrestThickness;
    glm::vec3 seatPos = glm::vec3(pos.x + armWidth, pos.y - upperBackHeight - (armHeight * 0.4f), frontZ);
    chair.parts.push_back(createCuboid(seatPos, middleWidth, seatHeight, frontDepth, defaultColors, textures));

    return chair;
}

void drawChair(const ChairData& chair) {
    for (const auto& part : chair.parts) {
        drawCuboid(part);
    }
}

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
) {
    CinemaSeatsData cinemaSeats;

    // Čuvamo dimenzije za kasnije ažuriranje
    cinemaSeats.chairWidth = chairWidth;
    cinemaSeats.chairHeight = chairHeight;
    cinemaSeats.chairDepth = chairDepth;

    int rows = 6;
    int cols = 12;

    // Desni zid je na poziciji roomWidth/2
    float rightWallX = roomWidth / 2.0f;

    // Početak prvog reda stolica (najdalje desno)
    float startX = rightWallX - rightWallMargin - chairWidth;

    // Izračunavanje Z pozicije za prvi red
    float firstStepZ = screenZ + distanceFromScreen;

    // Prolazimo kroz sve redove
    for (int row = 0; row < rows; row++) {
        // Z pozicija zadnjeg dela stepenice za ovaj red
        float currentStepDepth = stepDepth;

        float stepBackZ = firstStepZ + (row * stepDepth) + currentStepDepth;

        // Y pozicija vrha stepenice za ovaj red
        float stepTopY = floorY + (row + 1) * stepHeight;

        // Pozicija stolice je na zadnjem delu stepenice
        float chairZ = stepBackZ - chairDepth * 0.1;
        float chairY = stepTopY;

        // Prolazimo kroz sve kolone u ovom redu
        float currentX = startX;

        for (int col = 0; col < cols; col++) {
            // Pozicija stolice (zadnje gornje levo teme)
            glm::vec3 chairPosition = glm::vec3(currentX, chairY + chairHeight, chairZ);

            // Dobijanje teksture na osnovu stanja sedišta
            SeatState state = cinema.GetSeatState(row, col);
            unsigned int texture = getTextureForSeatState(state, availableTexture, reservedTexture, boughtTexture);

            // Kreiranje stolice
            ChairData chair = createChair(chairPosition, chairWidth, chairHeight, chairDepth, texture);

            cinemaSeats.chairs.push_back(chair);
            cinemaSeats.positions.push_back(chairPosition);
            cinemaSeats.seatIndices.push_back({ row, col });

            // Pomeranje na sledeću poziciju ulevo
            currentX -= (chairWidth + chairSpacing);
        }
    }

    return cinemaSeats;
}

void updateCinemaSeatsTextures(CinemaSeatsData& seats, const Cinema& cinema,
    unsigned int availableTexture,
    unsigned int reservedTexture,
    unsigned int boughtTexture) {
    for (size_t i = 0; i < seats.chairs.size(); i++) {
        int row = seats.seatIndices[i].first;
        int col = seats.seatIndices[i].second;

        SeatState state = cinema.GetSeatState(row, col);
        unsigned int texture = getTextureForSeatState(state, availableTexture, reservedTexture, boughtTexture);

        // Ažuriraj teksture za sve delove stolice
        for (auto& part : seats.chairs[i].parts) {
            for (int j = 0; j < 6; j++) {
                part.textures[j] = texture;
            }
        }
    }
}

void drawCinemaSeats(const CinemaSeatsData& seats) {
    for (const auto& chair : seats.chairs) {
        drawChair(chair);
    }
}