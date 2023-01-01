#include <stdio.h>
#include <stdlib.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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

float width = 900.f;
float height = 900.f;

glm::vec3 grid_size = glm::vec3(10, 10, 10); // x y z

std::vector<glm::vec3> block_position;
std::vector<glm::vec3> block_color;

std::vector<glm::vec3> grid_vertices;
std::vector<glm::vec2> grid_uvs;
std::vector<glm::vec3> grid_normals;

std::vector<glm::vec3> FEGO_logo_vertices;
std::vector<glm::vec2> FEGO_logo_uvs;
std::vector<glm::vec3> FEGO_logo_normals;

std::vector<glm::vec3> cube_vertices;
std::vector<glm::vec2> cube_uvs;
std::vector<glm::vec3> cube_normals;

bool undo_pressed = false;
bool first_mouse = false;
bool mouse_visible = false;
bool key_pressed = false;


void set_block_position(glm::vec3 position, glm::vec3 color)
{
	block_position.push_back(position);
	block_color.push_back(color);
}

void undo_block() {
	if (block_position.size() > 0)
	{
		block_position.pop_back();
		block_color.pop_back();
	} 
    else
    {
		std::cout << "No blocks to remove" << std::endl;
    }
}

int main()
{

    // Initialize GLFW and create a window
    glewExperimental = true; // Needed for core profile
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 8); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL 
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(width, height, "Pixel-arting 3D", NULL, NULL);
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

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

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
	
	GLuint ProjectionID = glGetUniformLocation(programID, "Projection");
    GLuint ViewID = glGetUniformLocation(programID, "View");
    GLuint ModelID = glGetUniformLocation(programID, "Model");

	GLuint LightDirectionID = glGetUniformLocation(programID, "LightDirection");
    GLuint LightPositionID = glGetUniformLocation(programID, "LightPosition");
    GLuint AmbientID = glGetUniformLocation(programID, "ambient_color");
    GLuint DiffuseID = glGetUniformLocation(programID, "diffuse_color");
    GLuint SpecularID = glGetUniformLocation(programID, "specular_color");

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

    GLfloat* grid_color_buffer_data = new GLfloat[grid_vertices.size() * 3];
    for (int v = 0; v < grid_vertices.size(); v++) {
        grid_color_buffer_data[3 * v + 0] = 1;
        grid_color_buffer_data[3 * v + 1] = 1;
        grid_color_buffer_data[3 * v + 2] = 1;
    }
    glGenBuffers(1, &grid_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, grid_VBO);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * sizeof(glm::vec3), &grid_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * 3 * sizeof(GLfloat), grid_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

	GLuint grid_normal;
	glGenBuffers(1, &grid_normal);
	glBindBuffer(GL_ARRAY_BUFFER, grid_normal);
	glBufferData(GL_ARRAY_BUFFER, grid_normals.size() * sizeof(glm::vec3), &grid_normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    bool cube_res = loadObj("Assets/cube.obj", cube_vertices, cube_uvs, cube_normals);
    GLuint cube_VBO, cube_VAO;

    glGenVertexArrays(1, &cube_VAO);
    glBindVertexArray(cube_VAO);

    GLfloat* cube_color_buffer_data = new GLfloat[cube_vertices.size() * 3];
    for (int v = 0; v < cube_vertices.size(); v++) {
        cube_color_buffer_data[3 * v + 0] = 0;
        cube_color_buffer_data[3 * v + 1] = 0;
        cube_color_buffer_data[3 * v + 2] = 0;
    }
    glGenBuffers(1, &cube_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);
    glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * sizeof(glm::vec3), &cube_vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &colorbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
    glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * 3 * sizeof(GLfloat), cube_color_buffer_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(1);

	GLuint cube_normal;
	glGenBuffers(1, &cube_normal);
	glBindBuffer(GL_ARRAY_BUFFER, cube_normal);
	glBufferData(GL_ARRAY_BUFFER, cube_normals.size() * sizeof(glm::vec3), &cube_normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Set the clear color
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    //cull face
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	//glFrontFace(GL_CCW);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	
    
    std::cout << "\nLoaded!\n\n";
    std::cout << "gui - click the \"m\" key on your keyboard\n";
    std::cout << "poligon mode - press the \"p\" key on your keyboard\n";
    std::cout << "ESC to close app\n";

    do{
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start the ImGUI frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a window for block insertion
        ImGui::Begin("Insert Block");
        static glm::vec3 block_pos = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 block_col = glm::vec3(1.0f, 1.0f, 1.0f);
        static glm::vec2 grid_size = glm::vec2(10.0f, 10.0f);
        static glm::vec3 grid_col = glm::vec3(1.0f, 1.0f, 1.0f);
        static glm::vec3 grid_start_col = glm::vec3(1.0f, 0.0f, 0.0f);
        static glm::vec3 app_back_col = glm::vec3(0.2f, 0.3f, 0.3f);
        static glm::vec3 light_dir = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 ambient_col = glm::vec3(0.0f, 0.0f, 0.0f);
        static glm::vec3 diffuse_col = glm::vec3(1.0f, 1.0f, 1.0f);
        static glm::vec3 specular_col = glm::vec3(1.0f, 1.0f, 1.0f);
        
        ImGui::InputFloat3("Position", &block_pos[0]);
        ImGui::ColorEdit3("Color", &block_col[0]);
        
        if (ImGui::Button("Insert"))
        {
            set_block_position(block_pos, block_col);
        }
        if (ImGui::Button("Undo"))
        {
            undo_block();
        }

        ImGui::Text("App Settings");
        ImGui::ColorEdit3("App background color", &app_back_col[0]);

        ImGui::Text("Grid Settings");
        ImGui::InputFloat2("Grid size", &grid_size[0]);
        ImGui::ColorEdit3("Grid color", &grid_col[0]);
        ImGui::ColorEdit3("Grid start color", &grid_start_col[0]);
        
        ImGui::Text("Light Settings");
		ImGui::InputFloat3("Light Direction", &light_dir[0]);
        ImGui::InputFloat3("Light Position", &light_pos[0]);
		ImGui::ColorEdit3("Ambient Color", &ambient_col[0]);
		ImGui::ColorEdit3("Diffuse Color", &diffuse_col[0]);
		ImGui::ColorEdit3("Specular Color", &specular_col[0]);
        ImGui::End();


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

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            if (!key_pressed) {
                mouse_visible = !mouse_visible;
                if (mouse_visible) {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
                    first_mouse = true;
                }
                else {
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    if (first_mouse) {
                        glfwSetCursorPos(window, width / 2, height / 2);
                        glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
                        first_mouse = false;
                    }
                }
                key_pressed = true;
            }
        }
        else {
            key_pressed = false;
        }

        if (!mouse_visible) {
            computeMatricesFromInputs(window, width, height);
            //light_dir = getPlayerDirection();
            light_pos = glm::vec3(getPlayerPosition().x, getPlayerPosition().y, getPlayerPosition().z);
        }

        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
		glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
        glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
        glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
		
		glUniform3f(LightDirectionID, light_dir.x, light_dir.y, light_dir.z);
        glUniform3f(LightPositionID, light_pos.x, light_pos.y, light_pos.z);
		glUniform3f(AmbientID, ambient_col.x, ambient_col.y, ambient_col.z);
		glUniform3f(DiffuseID, diffuse_col.x, diffuse_col.y, diffuse_col.z);
		glUniform3f(SpecularID, specular_col.x, specular_col.y, specular_col.z);

        // Draw logo
        //glBindVertexArray(VAO);
		//glDrawArrays(GL_TRIANGLES, 0, FEGO_logo_vertices.size());
        //glBindVertexArray(0);

        glBindVertexArray(grid_VAO);
		for (int i = 0; i < grid_size.x; i++) {
			for (int j = 0; j < grid_size.y; j++) {
				ModelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(i, 0, j));
                glUniformMatrix4fv(ProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
				glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
				glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);

                if (i == 0 && j == 0) {
                    for (int v = 0; v < grid_vertices.size(); v++) {
                        grid_color_buffer_data[3 * v + 0] = grid_start_col[0];
                        grid_color_buffer_data[3 * v + 1] = grid_start_col[1];
                        grid_color_buffer_data[3 * v + 2] = grid_start_col[2];
                    }
                }
                else
                {
                    for (int v = 0; v < grid_vertices.size(); v++) {
                        grid_color_buffer_data[3 * v + 0] = grid_col[0];
                        grid_color_buffer_data[3 * v + 1] = grid_col[1];
                        grid_color_buffer_data[3 * v + 2] = grid_col[2];
                    }
                }

                glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
                glBufferData(GL_ARRAY_BUFFER, grid_vertices.size() * 3 * sizeof(GLfloat), grid_color_buffer_data, GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glEnableVertexAttribArray(1);

				glDrawArrays(GL_TRIANGLES, 0, grid_vertices.size());
			}
		}
        glBindVertexArray(0);



        if (block_position.size() != 0) {
            for (int i = 0; i < block_position.size(); i++) {
                // Draw the second square
                glBindVertexArray(cube_VAO);

                for (int v = 0; v < cube_vertices.size(); v++) {
                    cube_color_buffer_data[3 * v + 0] = block_color[i].x;
                    cube_color_buffer_data[3 * v + 1] = block_color[i].y;
                    cube_color_buffer_data[3 * v + 2] = block_color[i].z;
                }

                glBindBuffer(GL_ARRAY_BUFFER, colorbuffer);
                glBufferData(GL_ARRAY_BUFFER, cube_vertices.size() * 3 * sizeof(GLfloat), cube_color_buffer_data, GL_STATIC_DRAW);
                glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
                glEnableVertexAttribArray(1);

                ModelMatrix = glm::translate(glm::mat4(1.0), block_position[i]);
                MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
                glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
                glUniformMatrix4fv(ViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
                glUniformMatrix4fv(ModelID, 1, GL_FALSE, &ModelMatrix[0][0]);
                glDrawArrays(GL_TRIANGLES, 0, cube_vertices.size());

                glBindVertexArray(0);
            }
        }
        else
        {
            //debug
            //std::cout << "lista pusta\n";
        }

        // Render ImGUI
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap the buffers
        glfwSwapBuffers(window);

        glClearColor(app_back_col.x, app_back_col.y, app_back_col.z, 1.0f);
    } while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
        glfwWindowShouldClose(window) == 0);

    // Clean up
    glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &fego_logo);
    glfwTerminate();

    // Cleanup ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}

