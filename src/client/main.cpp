// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include "matrix_math.cpp"


static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar * p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success){
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }
    glAttachShader(ShaderProgram, ShaderObj);
}

static GLuint CompileShaders()
{
  GLuint ShaderProgram = glCreateProgram();
  if (ShaderProgram == 0){
    exit(1);
  }

  
  const char * vs = "#version 330 \n \
      layout (location = 0) in vec3 Position; \n \
      uniform mat4 gWorld; \n \
      out vec4 Color; \n \
      void main() { \n \
      gl_Position = gWorld * vec4(Position, 1.0); \n \
      Color = vec4(1.0, 0.0, 0.0, 1.0);}";

  const char * fs = "#version 330 \n \
      in vec4 Color; \n \
      out vec4 FragColor; \n \
      void main() { \n \
      FragColor = Color; }";


  AddShader(ShaderProgram, vs, GL_VERTEX_SHADER);
  AddShader(ShaderProgram, fs, GL_FRAGMENT_SHADER);

  GLint success = 0;
  
  glLinkProgram(ShaderProgram);
  glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
  if (success == 0) exit(1);

  glValidateProgram(ShaderProgram);
  glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &success);
  if (success == 0) exit(1);

  return ShaderProgram;
}

void CreateAndFillBuffers(GLuint * VertexBuffer, GLuint * IndexBuffer, 
                          GLfloat * Vertices, unsigned int * Indices,
                          unsigned int NumVertices, unsigned int NumIndices)
{
  glGenBuffers(1, VertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, *VertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, NumVertices*sizeof(GLfloat), Vertices, GL_STATIC_DRAW);

  glGenBuffers(1, IndexBuffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *IndexBuffer);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, NumIndices*sizeof(unsigned int), Indices, GL_STATIC_DRAW);

}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Critic2", NULL, NULL);
    glfwMakeContextCurrent(window);
    gl3wInit();

    // Setup ImGui binding
    ImGui_ImplGlfwGL3_Init(window, true);
    
    //Setup up OpenGL stuff
    GLuint VertexArray;
    glGenVertexArrays(1, &VertexArray);
    glBindVertexArray(VertexArray);

    GLuint trishader = CompileShaders();
    GLuint gWorldLocation;
    gWorldLocation = glGetUniformLocation(trishader, "gWorld");
    
    Pipeline p;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Load sphere mesh
    GLuint IndexBuffer;
    GLuint VertexBuffer;
    GLuint IB2;
    GLuint VB2;
    unsigned int NumVertices = 3078;
    unsigned int NumIndices = 6144;
    static GLfloat * vertices = (GLfloat *) malloc(sizeof(GLfloat)*NumVertices);
    static unsigned int * indices = (unsigned int *) malloc(sizeof(unsigned int)*NumIndices);
    ReadMesh(vertices, indices, "./sphere.v", "./sphere.i");
    CreateAndFillBuffers(&VertexBuffer, &IndexBuffer, vertices, indices, 
                         NumVertices, NumIndices);

    CreateAndFillBuffers(&VB2, &IB2, vertices, indices, 
                         NumVertices, NumIndices);

    bool show_test_window = true;
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();


        static float sx=1.f, sy=1.f, sz=1.f;
        static float sf = 0.5f;
        static bool lockScale = true;
        static float rx = -20.f, ry = 20.f, rz = 0.f;
        static float tx = 0.f, ty = 0.f, tz = 0.f;
        {
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::Checkbox("Lock Scale Ratio", &lockScale);
            if(lockScale){
              ImGui::DragFloat("Scale", &sf, 0.005f);
            } else {
              ImGui::DragFloat("Scale X", &sx, 0.005f);
              ImGui::DragFloat("Scale Y", &sy, 0.005f);
              ImGui::DragFloat("Scale Z", &sz, 0.005f);
            }
            ImGui::DragFloat("Rotate X", &rx, 0.5f);
            ImGui::DragFloat("Rotate Y", &ry, 0.5f);
            ImGui::DragFloat("Rotate Z", &rz, 0.5f);

            ImGui::DragFloat("Translate X", &tx, 0.005f);
            ImGui::DragFloat("Translate Y", &ty, 0.005f);
            ImGui::DragFloat("Translate Z", &tz, 0.005f);
        }


        // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow
        if (!show_test_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
            ImGui::ShowTestWindow(&show_test_window);
        }

        // Rendering
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

       // p.SetPerspectiveProj(60.f, display_w, display_h, 1.f, 1000.f);

        p.Scale(sx*sf, sy*sf, sz*sf);
        p.Translate(tx, ty, tz); 
        p.Rotate(rx, ry, rz);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());

        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBuffer);
        glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);

        p.Scale(0.25f, 0.25f, 0.25f);
        p.Translate(0.5f, 0.f, .0f);
        glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, (const GLfloat *)p.GetTrans());
        glDrawElements(GL_TRIANGLES, NumIndices, GL_UNSIGNED_INT, 0);

        glDisableVertexAttribArray(0);
        
        glUseProgram(trishader);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        ImGui::Render();
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
