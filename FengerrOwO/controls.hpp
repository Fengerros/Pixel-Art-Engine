#pragma once
void computeMatricesFromInputs(GLFWwindow* window, float width, float height);
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getPlayerDirection();
glm::vec3 getPlayerPosition();