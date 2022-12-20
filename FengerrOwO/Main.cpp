#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "shader.hpp"
#include "controls.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "LoadObj.h"

GLFWwindow* window;

float width = 1000.f;
float height = 1000.f;

glm::vec3 grid_size = glm::vec3(10, 10, 10); // x y z

std::vector<std::vector<float>> blocks_positions_and_colors;

std::vector<glm::vec3> grid_vertices;
std::vector<glm::vec2> grid_uvs;
std::vector<glm::vec3> grid_normals;

std::vector<glm::vec3> FEGO_logo_vertices;
std::vector<glm::vec2> FEGO_logo_uvs;
std::vector<glm::vec3> FEGO_logo_normals;

std::vector<glm::vec3> cube_vertices;
std::vector<glm::vec2> cube_uvs;
std::vector<glm::vec3> cube_normals;

int main()
{
    // Initialize GLFW and create a window
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 
    
    window = glfwCreateWindow(width, height, "LEGO", NULL, NULL);
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (window == NULL) {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window); // Initialize GLEW
    glewExperimental = true; // Needed in core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, width / 2, height / 2);

    // Set up the shaders
    GLuint programID = LoadShaders("vertexShader.glsl", "fragmentShader.glsl");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint View = glGetUniformLocation(programID, "View");
    GLuint Model = glGetUniformLocation(programID, "View");

    bool res = loadObj("Assets/FEGO-logo.obj", FEGO_logo_vertices, FEGO_logo_uvs, FEGO_logo_normals);

    // Set up vertex buffer objects (VBOs) and vertex array object (VAO)
    GLuint fego_logo, VAO;
    glGenVertexArrays(1, &VAO);

    GLfloat* g_color_buffer_data = new GLfloat[FEGO_logo_vertices.size() * 3];
    for (int v = 0; v < FEGO_logo_vertices.size(); v++) {
        g_color_buffer_data[3 * v + 0] = 1;
        g_color_buffer_data[3 * v + 1] = 0;
        g_color_buffer_data[3 * v + 2] = 0.5f;
    }

    glGenBuffers(1, &fego_logo);
    glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, fego_logo);
    glBufferData(GL_ARRAY_BUFFER, FEGO_logo_vertices.size() * sizeof(glm::vec3), &FEGO_logo_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
	
    GLuint colorbuffer;
    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, FEGO_logo_vertices.size() * 3 * sizeof(GLfloat), g_color_buffer_data, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	

    bool grid_res = loadObj("Assets/grid.obj", grid_vertices, grid_uvs, grid_normals);

    GLuint grid_VBO, grid_VAO;

    glGenVertexArrays(1, &grid_VAO);
    glBindVertexArray(grid_VAO);

    for (int v = 0; v < grid_vertices.size(); v++) {
        g_color_buffer_data[3 * v + 0] = 255;
        g_color_buffer_data[3 * v + 1] = 255;
        g_color_buffer_data[3 * v + 2] = 100;
    }
    glGenBuffers(1, &grid_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(glm::vec3), &grid_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * 3 * sizeof(GLfloat), g_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    bool cube_res = loadObj("Assets/cube.obj", cube_vertices, cube_uvs, cube_normals);

    GLuint cube_VBO, cube_VAO;

    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    for (int v = 0; v < cube_vertices.size(); v++) {
        g_color_buffer_data[3 * v + 0] = 255;
        g_color_buffer_data[3 * v + 1] = 0;
        g_color_buffer_data[3 * v + 2] = 100;
    }
    glGenBuffers(1, &cube_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(glm::vec3), &cube_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * 3 * sizeof(GLfloat), g_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set the clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    //cull face
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

    // Run the main loop
    do{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        // Check for input
        glfwPollEvents();
		
        glUseProgram(programID);

        computeMatricesFromInputs(window, width, height);
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(View, 1, GL_FALSE, &ViewMatrix[0][0]);
        glUniformMatrix4fv(Model, 1, GL_FALSE, &ModelMatrix[0][0]);

        // Draw the first square
        glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, FEGO_logo_vertices.size());
        glBindVertexArray(0);

        // Draw the second square
        glBindVertexArray(grid_VAO);
        //draw procedural ground with grid
		for (int i = 0; i < 10; i++) {
			for (int j = 0; j < 10; j++) {
				ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(i * 1, 0, j * 1));
				MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
				glUniformMatrix4fv(View, 1, GL_FALSE, &ViewMatrix[0][0]);
				glUniformMatrix4fv(Model, 1, GL_FALSE, &ModelMatrix[0][0]);
				glDrawArrays(GL_TRIANGLES, 0, grid_vertices.size());
			}
		}
        glBindVertexArray(0);

		if()
        // Draw the second square
        glBindVertexArray(cube_VAO);
		
        //draw procedural ground with grid
         ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(2 * 1, 0, 0 * 1));
         MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
         glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
         glUniformMatrix4fv(View, 1, GL_FALSE, &ViewMatrix[0][0]);
         glUniformMatrix4fv(Model, 1, GL_FALSE, &ModelMatrix[0][0]);
         glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

        glBindVertexArray(0);

        // Swap the buffers
        glfwSwapBuffers(window);
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    // Clean up
    glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &fego_logo);
    glfwTerminate();

    return 0;
}

