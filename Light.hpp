#ifndef LIGHT_HPP
#define LIGHT_HPP

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <string>

struct Light {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    Light(glm::vec3 pos,
        glm::vec3 amb = glm::vec3(0.1f),
        glm::vec3 diff = glm::vec3(0.8f),
        glm::vec3 spec = glm::vec3(1.0f),
        float con = 1.0f,
        float lin = 0.09f,
        float quad = 0.032f)
        : position(pos), ambient(amb), diffuse(diff), specular(spec),
        constant(con), linear(lin), quadratic(quad) {}
};

void setLightsUniforms(unsigned int shaderProgram, const std::vector<Light>& lights);

#endif
#pragma once