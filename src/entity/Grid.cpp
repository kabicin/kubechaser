#include "entity/Grid.h"
#include <assert.h>
#include <iostream>

Grid::Grid(const std::shared_ptr<Camera>& camera)
{
    // set draw attributes for shader to reflect desired output
    da.color = true;
    da.lightSwitch = 0;

    for (int axis = 0; axis < 3; axis++) initialize(axis);
    for (int axis = 0; axis < 3; axis++) initializeLines(glm::vec3(5, 5, 5), axis);  

}

void Grid::initialize(int axis)
{
    assert(axis >= 0 && axis < 3);
    float pos[3] = {0.0f, 0.0f, 0.0f};
    pos[axis] = 0.5f;

    float gridColors[3];
    for (int i = 0 ; i < 3; i++) gridColors[i] = pos[i] == 0.5f ? 1.0f : 0.0f;
    PCVertex vertices[] = {
        // position         // color
        PCVertex(-pos[0], -pos[1], pos[2], gridColors[0], gridColors[1], gridColors[2]),
        PCVertex(pos[0], pos[1], -pos[2], gridColors[0], gridColors[1], gridColors[2])
    };

    // create VAO
    glGenVertexArrays(1, &VAO[axis]);
    glBindVertexArray(VAO[axis]);

    // create VBO
    glGenBuffers(1, &VBO[axis]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[axis]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PCVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PCVertex), reinterpret_cast<void*>(offsetof(PCVertex, Color)));
    glEnableVertexAttribArray(3);
}

void Grid::initializeLines(const glm::vec3& cartesianScale, int axis)
{
    assert(axis >= 0 && axis < 3);
    float x = 0.0f, y = 0.0f, z = 0.0f;
    int lines;
    if (axis == 0) 
    {
        x = 0.5f;
        lines = static_cast<int>(cartesianScale.x);
    }
    else if (axis == 1)
    {
        y = 0.5f;
        lines = static_cast<int>(cartesianScale.y);
    }
    else
    {
        z = 0.5f;
        lines = static_cast<int>(cartesianScale.z);
    }

    PCVertex vertices[200];
    int max_lines = std::min(100, lines * 5) + 2; // +2 for the x and z cartesian axes when no cross is drawn.
    float gridColors[3];
    for (int i = 0 ; i < 3; i++) gridColors[i] = GridColor;
    std::cout << "initializing (" << max_lines << ") line pairs" << std::endl;
    if (axis == 0 || axis == 2)
    {
        for (int i = 0; i < max_lines; i++)
        {
            vertices[2 * i] = PCVertex(-x, 0, z, gridColors[0], gridColors[1], gridColors[2]);
            vertices[2 * i + 1] = PCVertex(x, 0, -z, gridColors[0], gridColors[2], gridColors[2]);
        }
    }
    else
    {
        max_lines = 0;
    }

    // create VAO
    glGenVertexArrays(1, &lines_VAO[axis]);
    glBindVertexArray(lines_VAO[axis]);

    // create VBO
    glGenBuffers(1, &lines_VBO[axis]);
    glBindBuffer(GL_ARRAY_BUFFER, lines_VBO[axis]);
    num_lines[axis] = max_lines;
    glBufferData(GL_ARRAY_BUFFER, sizeof(PCVertex) * num_lines[axis] * 2, vertices, GL_STATIC_DRAW);

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PCVertex), (void*)0);
    glEnableVertexAttribArray(0);

    // color
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(PCVertex), reinterpret_cast<void*>(offsetof(PCVertex, Color)));
    glEnableVertexAttribArray(3);
}

void Grid::Draw(const std::shared_ptr<ShaderFactory>& factory, const std::shared_ptr<Camera>& camera, bool withCross)
{   
    for (int axis = 0; axis < 3; axis++)
    {
        // reset camera attributes
        ca.scale = glm::vec3(10, 10, 10);
        ca.translate = glm::vec3(0, 0, 0);
        ca.rotateXdeg = 0;
        ca.rotateYdeg = 0;
        factory->Use(da, camera->GetCameraPos());
        camera->Render(factory->GetShaderProgram(), ca);
        // draw cross
        if (withCross) {
            glBindVertexArray(VAO[axis]);
            glDrawArrays(GL_LINES, 0, 2);
        }

        // draw lines
        glBindVertexArray(lines_VAO[axis]);
        glm::vec3 new_translate = glm::vec3(axis == 0 ? 1 : 0, axis == 1 ? 1 : 0, axis == 2 ? 1 : 0);
        int signage = 0;
        int lines = withCross ? num_lines[axis] - 2 : num_lines[axis];
        for (int i = 0; i < lines; i++)
        {
            signage = (i % 2 == 0 ? 1 : -1);
            int ishift = withCross ? i + 2 : i;
            float offset = static_cast<float>((signage * (ishift/2)) / 25.0);
            ca.translate = glm::vec3(
                axis == 2 ? offset : 0, 
                0, 
                axis == 0 ? offset : 0);
           
            // update camera attributes to shaders
            camera->Render(factory->GetShaderProgram(), ca);
            // draw at a stride of 2 indexes
            glDrawArrays(GL_LINES, i * 2, 2);
        }
        ca.scale = glm::vec3(10, 10, 10);
        ca.translate = glm::vec3(0, 0, 0);
        ca.rotateXdeg = 0;
        ca.rotateYdeg = 0;
    }
}