#ifndef SHADERPROGRAM_H
#define SHADERPROGRAM_H

#include <GL/glew.h>
#include <iostream>
#include <stdio.h>

void initShaders();
void freeShaders();

class ShaderProgram {
private:
  char *readFile(const char *fileName);
  GLuint loadShader(GLenum shaderType, const char *fileName);
  GLuint vertexShader;
  GLuint fragmentShader;

public:
  GLuint shaderProgram;
  ShaderProgram(const char *vertexShaderFile, const char *fragmentShaderFile);
  GLuint attrib(const char *variableName);
  GLuint uniform(const char *variableName);
  void use();
  ~ShaderProgram();
};

extern ShaderProgram *basicShader;
extern ShaderProgram *lampShader;
#endif
