#include "Light.hpp"

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <string>

void setLightsUniforms(unsigned int shaderProgram, const std::vector<Light>& lights) {
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), lights.size());

    for (int i = 0; i < lights.size(); i++) {
        std::string base = "lights[" + std::to_string(i) + "]";

        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".position").c_str()),
            1, &lights[i].position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".ambient").c_str()),
            1, &lights[i].ambient[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".diffuse").c_str()),
            1, &lights[i].diffuse[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, (base + ".specular").c_str()),
            1, &lights[i].specular[0]);

        glUniform1f(glGetUniformLocation(shaderProgram, (base + ".constant").c_str()),
            lights[i].constant);
        glUniform1f(glGetUniformLocation(shaderProgram, (base + ".linear").c_str()),
            lights[i].linear);
        glUniform1f(glGetUniformLocation(shaderProgram, (base + ".quadratic").c_str()),
            lights[i].quadratic);
    }
}