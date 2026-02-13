#include "Cuboid.hpp"

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Funkcija za kreiranje kvadra sa bojama ili teksturama
CuboidData createCuboid(glm::vec3 frontTopLeft, float width, float height, float depth,
    std::vector<glm::vec4> colors, std::vector<unsigned int> textures,
    bool inverted) {

    CuboidData cuboid;

    // Izračunavanje svih 8 tačaka kvadra
    float x = frontTopLeft.x;
    float y = frontTopLeft.y;
    float z = frontTopLeft.z;

    // Ako je inverted, menjamo stranu koja se smatra "prednjom"
    float depthDir = inverted ? depth : -depth;

    // 8 tačaka kvadra
    glm::vec3 ftl = frontTopLeft; // front top left
    glm::vec3 ftr = glm::vec3(x + width, y, z); // front top right
    glm::vec3 fbl = glm::vec3(x, y - height, z); // front bottom left
    glm::vec3 fbr = glm::vec3(x + width, y - height, z); // front bottom right
    glm::vec3 btl = glm::vec3(x, y, z + depthDir); // back top left
    glm::vec3 btr = glm::vec3(x + width, y, z + depthDir); // back top right
    glm::vec3 bbl = glm::vec3(x, y - height, z + depthDir); // back bottom left
    glm::vec3 bbr = glm::vec3(x + width, y - height, z + depthDir); // back bottom right

    // Normalizovani pravci (menjaju se ako je inverted)
    float normalDir = inverted ? -1.0f : 1.0f;

    std::vector<float> vertices;

    // Dodavanje 6 strana kvadra
    // Svaka strana: 4 vrha u TRIANGLE_FAN formatu (top-right, top-left, bottom-left, bottom-right)
    // Texture koordinate: (0,0) top-left, (1,0) top-right, (1,1) bottom-right, (0,1) bottom-left

    // 1. Prednja strana (z = frontTopLeft.z)
    glm::vec3 normal = glm::vec3(0, 0, normalDir);
    std::vector<glm::vec3> frontVerts = inverted ?
        std::vector<glm::vec3>{fbl, fbr, ftr, ftl} :
        std::vector<glm::vec3>{ ftr, ftl, fbl, fbr };
    std::vector<glm::vec2> texCoords = { {1,0}, {0,0}, {0,1}, {1,1} };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { frontVerts[i].x, frontVerts[i].y, frontVerts[i].z });
        vertices.insert(vertices.end(), { colors[0].r, colors[0].g, colors[0].b, colors[0].a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    // 2. Leva strana (x = frontTopLeft.x)
    normal = glm::vec3(-normalDir, 0, 0);
    std::vector<glm::vec3> leftVerts = inverted ?
        std::vector<glm::vec3>{bbl, fbl, ftl, btl} :
        std::vector<glm::vec3>{ ftl, btl, bbl, fbl };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { leftVerts[i].x, leftVerts[i].y, leftVerts[i].z });
        vertices.insert(vertices.end(), { colors[1].r, colors[1].g, colors[1].b, colors[1].a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    // 3. Donja strana (y = frontTopLeft.y - height)
    normal = glm::vec3(0, -normalDir, 0);
    std::vector<glm::vec3> bottomVerts = inverted ?
        std::vector<glm::vec3>{ fbl, bbl, bbr, fbr } :
        std::vector<glm::vec3>{ fbr, fbl, bbl, bbr };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { bottomVerts[i].x, bottomVerts[i].y, bottomVerts[i].z });
        vertices.insert(vertices.end(), { colors[2].r, colors[2].g, colors[2].b, colors[2].a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    // 4. Gornja strana (y = frontTopLeft.y)
    normal = glm::vec3(0, normalDir, 0);
    std::vector<glm::vec3> topVerts = inverted ?
        std::vector<glm::vec3>{ ftl, ftr, btr, btl } :
        std::vector<glm::vec3>{ btr, btl, ftl, ftr };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { topVerts[i].x, topVerts[i].y, topVerts[i].z });
        vertices.insert(vertices.end(), { colors[3].r, colors[3].g, colors[3].b, colors[3].a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    // 5. Desna strana (x = frontTopLeft.x + width)
    normal = glm::vec3(normalDir, 0, 0);
    std::vector<glm::vec3> rightVerts = inverted ?
        std::vector<glm::vec3>{ fbr, bbr, btr, ftr } :
        std::vector<glm::vec3>{ btr, ftr, fbr, bbr };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { rightVerts[i].x, rightVerts[i].y, rightVerts[i].z });
        vertices.insert(vertices.end(), { colors[4].r, colors[4].g, colors[4].b, colors[4].a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    // 6. Zadnja strana (z = frontTopLeft.z + depthDir)
    normal = glm::vec3(0, 0, -normalDir);
    std::vector<glm::vec3> backVerts = inverted ?
        std::vector<glm::vec3>{ btl, btr, bbr, bbl } :
        std::vector<glm::vec3>{ bbr, bbl, btl, btr };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { backVerts[i].x, backVerts[i].y, backVerts[i].z });
        vertices.insert(vertices.end(), { colors[5].r, colors[5].g, colors[5].b, colors[5].a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    // Kreiranje VAO i VBO
    glGenVertexArrays(1, &cuboid.VAO);
    glBindVertexArray(cuboid.VAO);

    glGenBuffers(1, &cuboid.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cuboid.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    unsigned int stride = 12 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Čuvanje tekstura
    for (int i = 0; i < 6; i++) {
        cuboid.textures[i] = (i < textures.size()) ? textures[i] : 0;
    }

    return cuboid;
}

// Funkcija za crtanje kvadra
void drawCuboid(const CuboidData& cuboid) {
    glBindVertexArray(cuboid.VAO);
    glActiveTexture(GL_TEXTURE0);

    for (int i = 0; i < 6; i++) {
        if (cuboid.textures[i] != 0) {
            glBindTexture(GL_TEXTURE_2D, cuboid.textures[i]);
        }
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }
}

// Funkcija za kreiranje pravougaonika (quad)
RectangleData createRectangle(glm::vec3 center, float width, float height,
    glm::vec4 color, unsigned int texture,
    bool facingCamera) {

    RectangleData rectangle;

    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    // Kreiramo pravougaonik sa centrom u zadatoj poziciji
    // Pravougaonik je u XY ravni, okrenut ka kameri (pozitivan Z pravac normale)
    glm::vec3 topLeft = glm::vec3(center.x - halfWidth, center.y + halfHeight, center.z);
    glm::vec3 topRight = glm::vec3(center.x + halfWidth, center.y + halfHeight, center.z);
    glm::vec3 bottomLeft = glm::vec3(center.x - halfWidth, center.y - halfHeight, center.z);
    glm::vec3 bottomRight = glm::vec3(center.x + halfWidth, center.y - halfHeight, center.z);

    // Normala (prema kameri ili od kamere)
    glm::vec3 normal = facingCamera ? glm::vec3(0, 0, -1) : glm::vec3(0, 0, 1);

    // Vertices: top-right, top-left, bottom-left, bottom-right (TRIANGLE_FAN)
    std::vector<float> vertices;
    std::vector<glm::vec3> verts = facingCamera ?
        std::vector<glm::vec3>{topRight, topLeft, bottomLeft, bottomRight} :
        std::vector<glm::vec3>{ topLeft, topRight, bottomRight, bottomLeft };

    std::vector<glm::vec2> texCoords = { {1,0}, {0,0}, {0,1}, {1,1} };

    for (int i = 0; i < 4; i++) {
        // Position
        vertices.insert(vertices.end(), { verts[i].x, verts[i].y, verts[i].z });
        // Color
        vertices.insert(vertices.end(), { color.r, color.g, color.b, color.a });
        // Texture coordinates
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        // Normal
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    glGenVertexArrays(1, &rectangle.VAO);
    glBindVertexArray(rectangle.VAO);

    glGenBuffers(1, &rectangle.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, rectangle.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    unsigned int stride = 12 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    rectangle.texture = texture;

    return rectangle;
}

// Funkcija za crtanje pravougaonika
void drawRectangle(const RectangleData& rectangle) {
    glBindVertexArray(rectangle.VAO);
    glActiveTexture(GL_TEXTURE0);

    if (rectangle.texture != 0) {
        glBindTexture(GL_TEXTURE_2D, rectangle.texture);
    }

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

RectangleData create2DOverlay(float x, float y, float width, float height,
    glm::vec4 color, unsigned int texture) {
    RectangleData rectangle;

    // x, y su u NDC koordinatama (-1 do 1)
    // width, height su takođe u NDC prostoru
    float halfW = width / 2.0f;
    float halfH = height / 2.0f;

    // Vertices u NDC prostoru
    glm::vec3 topLeft = glm::vec3(x - halfW, y + halfH, 0.0f);
    glm::vec3 topRight = glm::vec3(x + halfW, y + halfH, 0.0f);
    glm::vec3 bottomLeft = glm::vec3(x - halfW, y - halfH, 0.0f);
    glm::vec3 bottomRight = glm::vec3(x + halfW, y - halfH, 0.0f);

    glm::vec3 normal = glm::vec3(0, 0, 1);

    std::vector<float> vertices;
    std::vector<glm::vec3> verts = { topRight, topLeft, bottomLeft, bottomRight };
    std::vector<glm::vec2> texCoords = { {1,0}, {0,0}, {0,1}, {1,1} };

    for (int i = 0; i < 4; i++) {
        vertices.insert(vertices.end(), { verts[i].x, verts[i].y, verts[i].z });
        vertices.insert(vertices.end(), { color.r, color.g, color.b, color.a });
        vertices.insert(vertices.end(), { texCoords[i].x, texCoords[i].y });
        vertices.insert(vertices.end(), { normal.x, normal.y, normal.z });
    }

    glGenVertexArrays(1, &rectangle.VAO);
    glBindVertexArray(rectangle.VAO);

    glGenBuffers(1, &rectangle.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, rectangle.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    unsigned int stride = 12 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    rectangle.texture = texture;

    return rectangle;
}

StaircaseData createStaircase(float distanceFromScreen, float stepHeight, float stepDepth,
    int numSteps, glm::vec4 stepColor,
    float roomWidth, float roomHeight, float roomDepth, float roomFrontZ, float screenZ, float floorY) {

    StaircaseData staircase;

    // Konstante prostorije (iste kao u main.cpp)
    const float ROOM_WIDTH = roomWidth;
    const float ROOM_HEIGHT = roomHeight;
    const float ROOM_DEPTH = roomDepth;
    const float ROOM_FRONT_Z = roomFrontZ;
    const float ROOM_BACK_Z = ROOM_FRONT_Z + ROOM_DEPTH;
    const float SCREEN_Z = screenZ;
    const float FLOOR_Y = floorY;

    // Boja za sve stepenice
    std::vector<glm::vec4> colors(6, stepColor);

    // Početna pozicija prvog koraka
    float currentZ = SCREEN_Z + distanceFromScreen;
    float currentY = FLOOR_Y;

    for (int i = 0; i < numSteps; i++) {
        // Ako je poslednji korak, produžavamo ga do zadnjeg zida
        float currentStepDepth = stepDepth;
        if (i == numSteps - 1) {
            currentStepDepth = ROOM_DEPTH - currentZ + SCREEN_Z;
        }

        glm::vec3 frontTopLeft = glm::vec3(
            -ROOM_WIDTH / 2.0f,           
            currentY + stepHeight,        
            currentZ + currentStepDepth    
        );

        CuboidData step = createCuboid(
            frontTopLeft,
            ROOM_WIDTH,        // Puna širina prostorije
            stepHeight,        // Visina koraka
            currentStepDepth,  // Dubina koraka (poslednji je duži)
            colors,            // Boja
            {},                // Bez tekstura
            false              // Nije invertovano
        );

        staircase.steps.push_back(step);
        
        float subStepWidth = ROOM_WIDTH * 0.25f;
        float subStepHeight = stepHeight * 0.5f;  
        float subStepDepth = stepDepth * 0.5f;    

        glm::vec3 subStepFrontTopLeft = glm::vec3(
            -ROOM_WIDTH / 2.0f,                        
            currentY + subStepHeight,      
            currentZ                     
        );

        CuboidData subStep = createCuboid(
            subStepFrontTopLeft,
            subStepWidth,
            subStepHeight,
            subStepDepth,
            colors,
            {},
            false
        );

        staircase.steps.push_back(subStep);

        // Pomeramo se za sledeći korak (nazad i gore)
        currentZ += stepDepth;
        currentY += stepHeight;
    }

    return staircase;
}

void drawStaircase(const StaircaseData& staircase) {
    for (const auto& step : staircase.steps) {
        drawCuboid(step);
    }
}

// Funkcija za spajanje svih stepenica u JEDAN mesh
StaircaseData createStaircaseMerged(float distanceFromScreen, float stepHeight, float stepDepth,
    int numSteps, glm::vec4 stepColor,
    float roomWidth, float roomHeight, float roomDepth, float roomFrontZ, float screenZ, float floorY) {

    StaircaseData staircase;

    const float ROOM_WIDTH = roomWidth;
    const float ROOM_HEIGHT = roomHeight;
    const float ROOM_DEPTH = roomDepth;
    const float ROOM_FRONT_Z = roomFrontZ;
    const float SCREEN_Z = screenZ;
    const float FLOOR_Y = floorY;

    // Boja za sve stepenice
    std::vector<glm::vec4> colors(6, stepColor);

    // === SPOJENI VERTEKSI ZA SVE KORAKE ===
    std::vector<float> allVertices;

    float currentZ = SCREEN_Z + distanceFromScreen;
    float currentY = FLOOR_Y;

    // Dimenzije međukoraka
    float subStepWidth = ROOM_WIDTH * 0.25f;
    float subStepHeight = stepHeight * 0.5f;
    float subStepDepth = stepDepth * 0.5f;

    for (int i = 0; i < numSteps; i++) {
        float currentStepDepth = stepDepth;
        if (i == numSteps - 1) {
            currentStepDepth = ROOM_DEPTH - currentZ + SCREEN_Z;
        }

        // === GLAVNI KORAK ===
        glm::vec3 frontTopLeft = glm::vec3(
            -ROOM_WIDTH / 2.0f,
            currentY + stepHeight,
            currentZ + currentStepDepth
        );

        // Generiši vertekse za ovaj korak i dodaj ih u allVertices
        // Kopiraj logiku iz createCuboid, ali umesto pravljenja VAO/VBO,
        // samo dodaj vertekse u vektor

        // 8 tačaka glavnog koraka
        float x = frontTopLeft.x;
        float y = frontTopLeft.y;
        float z = frontTopLeft.z;
        float w = ROOM_WIDTH;
        float h = stepHeight;
        float d = -currentStepDepth;

        glm::vec3 ftl = frontTopLeft;
        glm::vec3 ftr = glm::vec3(x + w, y, z);
        glm::vec3 fbl = glm::vec3(x, y - h, z);
        glm::vec3 fbr = glm::vec3(x + w, y - h, z);
        glm::vec3 btl = glm::vec3(x, y, z + d);
        glm::vec3 btr = glm::vec3(x + w, y, z + d);
        glm::vec3 bbl = glm::vec3(x, y - h, z + d);
        glm::vec3 bbr = glm::vec3(x + w, y - h, z + d);

        // 6 strana × 4 vrha × (3 pos + 4 col + 2 tex + 3 nor) = 6*4*12 floata
        std::vector<glm::vec3> faces[6] = {
            {ftr, ftl, fbl, fbr},  // Front
            {ftl, btl, bbl, fbl},  // Left
            {fbr, fbl, bbl, bbr},  // Bottom
            {btr, btl, ftl, ftr},  // Top
            {btr, ftr, fbr, bbr},  // Right
            {bbr, bbl, btl, btr}   // Back
        };

        glm::vec3 normals[6] = {
            glm::vec3(0, 0, 1),   // Front
            glm::vec3(-1, 0, 0),  // Left
            glm::vec3(0, -1, 0),  // Bottom
            glm::vec3(0, 1, 0),   // Top
            glm::vec3(1, 0, 0),   // Right
            glm::vec3(0, 0, -1)   // Back
        };

        std::vector<glm::vec2> texCoords = { {1,0}, {0,0}, {0,1}, {1,1} };

        for (int f = 0; f < 6; f++) {
            for (int v = 0; v < 4; v++) {
                // Position
                allVertices.push_back(faces[f][v].x);
                allVertices.push_back(faces[f][v].y);
                allVertices.push_back(faces[f][v].z);
                // Color
                allVertices.push_back(stepColor.r);
                allVertices.push_back(stepColor.g);
                allVertices.push_back(stepColor.b);
                allVertices.push_back(stepColor.a);
                // TexCoord
                allVertices.push_back(texCoords[v].x);
                allVertices.push_back(texCoords[v].y);
                // Normal
                allVertices.push_back(normals[f].x);
                allVertices.push_back(normals[f].y);
                allVertices.push_back(normals[f].z);
            }
        }

        // === MEĐUKORAK (levi) ===
        glm::vec3 subFrontTopLeft = glm::vec3(
            -ROOM_WIDTH / 2.0f,
            currentY + subStepHeight,
            currentZ
        );

        x = subFrontTopLeft.x;
        y = subFrontTopLeft.y;
        z = subFrontTopLeft.z;
        w = subStepWidth;
        h = subStepHeight;
        d = -subStepDepth;

        ftl = subFrontTopLeft;
        ftr = glm::vec3(x + w, y, z);
        fbl = glm::vec3(x, y - h, z);
        fbr = glm::vec3(x + w, y - h, z);
        btl = glm::vec3(x, y, z + d);
        btr = glm::vec3(x + w, y, z + d);
        bbl = glm::vec3(x, y - h, z + d);
        bbr = glm::vec3(x + w, y - h, z + d);

        std::vector<glm::vec3> subFaces[6] = {
            {ftr, ftl, fbl, fbr},
            {ftl, btl, bbl, fbl},
            {fbr, fbl, bbl, bbr},
            {btr, btl, ftl, ftr},
            {btr, ftr, fbr, bbr},
            {bbr, bbl, btl, btr}
        };

        for (int f = 0; f < 6; f++) {
            for (int v = 0; v < 4; v++) {
                allVertices.push_back(subFaces[f][v].x);
                allVertices.push_back(subFaces[f][v].y);
                allVertices.push_back(subFaces[f][v].z);
                allVertices.push_back(stepColor.r);
                allVertices.push_back(stepColor.g);
                allVertices.push_back(stepColor.b);
                allVertices.push_back(stepColor.a);
                allVertices.push_back(texCoords[v].x);
                allVertices.push_back(texCoords[v].y);
                allVertices.push_back(normals[f].x);
                allVertices.push_back(normals[f].y);
                allVertices.push_back(normals[f].z);
            }
        }

        currentZ += stepDepth;
        currentY += stepHeight;
    }

    // === KREIRAJ JEDAN VAO/VBO ZA SVE ===
    CuboidData mergedStaircase;

    glGenVertexArrays(1, &mergedStaircase.VAO);
    glBindVertexArray(mergedStaircase.VAO);

    glGenBuffers(1, &mergedStaircase.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, mergedStaircase.VBO);
    glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(float), allVertices.data(), GL_STATIC_DRAW);

    unsigned int stride = 12 * sizeof(float);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(9 * sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    mergedStaircase.vertexCount = allVertices.size() / 12; // 12 floata po vertexu

    staircase.steps.push_back(mergedStaircase);

    return staircase;
}

// Dodaj i novu funkciju za crtanje
void drawStaircaseMerged(const StaircaseData& staircase) {
    if (staircase.steps.empty()) return;

    const CuboidData& merged = staircase.steps[0]; // Samo jedan step sada
    glBindVertexArray(merged.VAO);

    // Crtaj sve face-ove odjednom
    int totalFaces = merged.vertexCount / 4; // 4 vrha po face-u
    for (int i = 0; i < totalFaces; i++) {
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
    }
}