#include "Camera.h"
#include "objects.h"
#include "shaderprogram.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <bits/stdc++.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <stdlib.h>

int total_snake = 1;
int aspectRatio = 1;
int state = 1;
int coord_index = 1;
float distance = -0.05f;
float rotate_angle = 0.f;
float snake_coords[2] = {0.f, 0.f};
const GLuint WIDTH = 1080, HEIGHT = 800;
bool keys[1024];

// Camera , function LookAt
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
GLfloat lastX = WIDTH / 2.0;
GLfloat lastY = HEIGHT / 2.0;
bool firstMouse = true;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

void lookAt();
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void windowResizeCallback(GLFWwindow *window, int width, int height);
void initOpenglProgram(GLFWwindow *window);
void freeOpenglProgram(GLFWwindow *window);
void drawScene(GLFWwindow *window);
void initWindow(GLFWwindow *window);
void do_movement(void);
void generateRing(ShaderProgram *basicShader);
void update_direction(float angle);
GLuint readTexture(const char* filename) ;
void loadModel(std::string plik);

int main(int argc, char *argv[]) {
  GLFWwindow *window;
  if (!glfwInit()) {
    fprintf(stderr, "Can't initalize GLFW\n");
    exit(EXIT_FAILURE);
  }

  window = glfwCreateWindow(WIDTH, HEIGHT, "Snake3D", NULL, NULL);
  initWindow(window);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);
  if (int err = glewInit() != GLEW_OK) {
    fprintf(stderr, "Can't initalize GLEW: %s\n", glewGetErrorString(err));
    return -1;
  }

  initShaders();

  initOpenglProgram(window);

  while (!glfwWindowShouldClose(window)) {

    lookAt();
    drawScene(window);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwPollEvents();
    do_movement();
  }

  freeShaders();
  freeOpenglProgram(window);
  glfwTerminate();
  return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, GL_TRUE);

  if ((key == GLFW_KEY_D) && action == GLFW_PRESS) {
    update_direction(1.5708f);
    if (state == 0)
      distance *= -1.0f;
  }

  if ((key == GLFW_KEY_A) && action == GLFW_PRESS) {
    update_direction(-1.5708f);
    if (state == 1)
      distance *= -1.0f;
  }

  //  if (key >= 0 && key < 1024) {
  //    if (action == GLFW_PRESS)
  //      keys[key] = true;
  //    else if (action == GLFW_RELEASE)
  //      keys[key] = false;
  // }
}

void windowResizeCallback(GLFWwindow *window, int width, int height) {
  if (height == 0)
    return;
  aspectRatio = (float)width / (float)height;
  glViewport(0, 0, width, height);
}

void initOpenglProgram(GLFWwindow *window) {
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glfwSetWindowSizeCallback(window, windowResizeCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  glfwSetKeyCallback(window, key_callback);
  initObjects();
  loadModel(std::string("Gold_Band_Ring_FBX.fbx"));

  return;
}

void freeOpenglProgram(GLFWwindow *window) {
  glDeleteTextures(1, &map_texture);
  glDeleteTextures(1, &fence_texture);
  glDeleteTextures(1, &column_texture);
  glfwDestroyWindow(window);
  return;
}
 
void drawScene(GLFWwindow *window) {
  glEnable(GL_DEPTH_TEST);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  basicShader->use();
  generateMap(basicShader);
  for (int i = 0; i < 4; i++) {
    generateFence(basicShader, i);
    generateColumn(basicShader, i);
  }
  generateRing(basicShader);
  for (int i = 0; i < total_snake; i++)
    snakeData[i].snake_coords[coord_index] += distance;
  generateSnake(basicShader, total_snake);
  for (int i = 0; i < 2; i++) {
    if (snakeData[0].snake_coords[i] > 9.49f ||
        snakeData[0].snake_coords[i] < -10.12f) {
      for (int j = 0; j < total_snake; j++) {
        snakeData[j].snake_coords[0] = 0.f;
        snakeData[j].snake_coords[1] = 0.f + 0.625 * j;
      }
      coord_index = 1;
      state = 1;
      snakeData[0].rotate_angle = 0.f;
      distance = -0.05f;
      total_snake = 1;
    }
  }

  glfwSwapBuffers(window);
  return;
}


void load_favicon(GLFWwindow *window) {
  GLFWimage icons[1];
  icons[0].pixels = SOIL_load_image("images/favicon.png", &icons[0].width,
                                    &icons[0].height, 0, SOIL_LOAD_RGBA);
  glfwSetWindowIcon(window, 1, icons);
  SOIL_free_image_data(icons[0].pixels);
  return;
};

void initWindow(GLFWwindow *window) {
  if (!window) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }
  load_favicon(window);
  return;
}

void lookAt() {
  GLfloat currentFrame = glfwGetTime();
  deltaTime = currentFrame - lastFrame;
  lastFrame = currentFrame;

  glm::mat4 model = glm::mat4(1.0f);

  glm::mat4 view = glm::mat4(1.0f);
  view = camera.GetViewMatrix();
  glm::mat4 projection = glm::mat4(1.0f);
  model = glm::rotate(model, 0.0f, glm::vec3(0.5f, 1.0f, 0.0f));
  view =
      glm::rotate(view, snakeData[0].rotate_angle, glm::vec3(0.0f, 1.0f, 0.0f));
  view = glm::translate(view, glm::vec3(-snakeData[0].snake_coords[0], -1.2f,
                                        -snakeData[0].snake_coords[1]));
  // view = glm::rotate(view, 1.5708f, glm::vec3(1.0f, 0.0f, 0.0f));
  // view = glm::translate(view, glm::vec3(0.0f, -20.0f, 0.0f));
  projection = glm::perspective(camera.Zoom, (GLfloat)WIDTH / (GLfloat)HEIGHT,
                                0.1f, 100.0f);

  // TODO Refactoring
  GLint modelLoc = glGetUniformLocation(basicShader->shaderProgram, "model");
  GLint viewLoc = glGetUniformLocation(basicShader->shaderProgram, "view");
  GLint projLoc = glGetUniformLocation(basicShader->shaderProgram, "projection");

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
  if (firstMouse) {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  GLfloat xoffset = xpos - lastX;
  GLfloat yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

void do_movement() {
  if (keys[GLFW_KEY_W])
    camera.ProcessKeyboard(FORWARD, deltaTime);
  if (keys[GLFW_KEY_S])
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (keys[GLFW_KEY_A])
    camera.ProcessKeyboard(LEFT, deltaTime);
  if (keys[GLFW_KEY_D])
    camera.ProcessKeyboard(RIGHT, deltaTime);
}

void update_direction(float angle) {
  for (int i = 1; i < total_snake; i++) {
    for (int j = 0; j < 2; j++)
      snakeData[i].snake_coords[j] = snakeData[i - 1].snake_coords[j];
    snakeData[i].rotate_angle = snakeData[i - 1].rotate_angle;
  }
  if (distance >= 0) {
    snakeData[0].snake_coords[coord_index] =
        (ceil(snakeData[0].snake_coords[coord_index] / 0.625f)) * 0.625f;
  } else {
    snakeData[0].snake_coords[coord_index] =
        (floor(snakeData[0].snake_coords[coord_index] / 0.625f)) * 0.625f;
  }
  coord_index = (coord_index + 1) % 2;
  state = (state + 1) % 2;
  snakeData[0].rotate_angle += angle;
}


