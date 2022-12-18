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

float width = 800.f;
float height = 800.f;

std::vector<glm::vec3> FEGO_logo_vertices;
std::vector<glm::vec2> FEGO_logo_uvs;
std::vector<glm::vec3> FEGO_logo_normals;

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

    GLfloat square2Vertices[] = {
        //without indices
		-0.5f, -0.5f, 0.0f,  // bottom left
		0.5f, -0.5f, 0.0f,  // bottom right
		0.5f, 0.5f, 0.0f,  // top right
		
		0.5f, 0.5f, 0.0f,  // top right
		-0.5f, 0.5f, 0.0f,  // top left
		-0.5f, -0.5f, 0.0f  // bottom left
    };
	

    // Set up vertex buffer objects (VBOs) and vertex array object (VAO)
    GLuint fego_logo, square2VBO, VAO;
    glGenVertexArrays(1, &VAO);

    GLfloat* g_color_buffer_data = new GLfloat[FEGO_logo_vertices.size() * 3];
    for (int v = 0; v < FEGO_logo_vertices.size(); v++) {
        g_color_buffer_data[3 * v + 0] = 1;
        g_color_buffer_data[3 * v + 1] = 0;
        g_color_buffer_data[3 * v + 2] = 0.5f;
    }

    glGenBuffers(1, &fego_logo);
    glGenBuffers(1, &square2VBO);
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

    // Set up the second VAO for the second square
    GLuint VAO2;
    glGenVertexArrays(1, &VAO2);
    glBindVertexArray(VAO2);

    for (int v = 0; v < 12; v++) {
        g_color_buffer_data[3 * v + 0] = 0;
        g_color_buffer_data[3 * v + 1] = 255;
        g_color_buffer_data[3 * v + 2] = 100;
    }

    glBindBuffer(GL_ARRAY_BUFFER, square2VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square2Vertices), square2Vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, 12 * 3 * sizeof(GLfloat), g_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set the clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

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
        glBindVertexArray(VAO2);
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);

        // Swap the buffers
        glfwSwapBuffers(window);
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    // Clean up
    glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &fego_logo);
    glDeleteBuffers(1, &square2VBO);
    glfwTerminate();

    return 0;
}

