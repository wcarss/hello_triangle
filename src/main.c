#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define INFOLOG_LENGTH 512
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  printf("famebuffer_size changed: %d, %d\n", width, height);
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }
}

int main(int argc, char** argv)
{
  const int WINDOW_WIDTH = 1280;
  const int WINDOW_HEIGHT = 720;
  GLFWwindow* window;

  /* Initialize the library */
  if (!glfwInit()) {
    return -1;
  }

#ifdef __APPLE__
  /* We need to explicitly ask for a 4.1 context on OS X */
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Hello Triangle", NULL, NULL);

  if (window == NULL) {
    printf("Failed to create GLFW window\n");
    glfwTerminate();
    return -1;
  }

  /* Make the window's context current */
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    printf("Failed to initialize GLAD\n");
    return -1;
  }

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  /* vertex shader */
  unsigned int vertexShader;
  const char *vertexShaderSource = "#version 410 core\n"
                                   "layout (location = 0) in vec3 aPos;\n"
                                   "void main() {\n"
                                   "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
                                   "}";

  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShader);

  int vertexShaderSuccess;
  char vertexShaderInfoLog[INFOLOG_LENGTH];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vertexShaderSuccess);

  if (!vertexShaderSuccess) {
    glGetShaderInfoLog(vertexShader, INFOLOG_LENGTH, NULL, vertexShaderInfoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", vertexShaderInfoLog);
  }

  /* end vertex shader */

  /* fragment shader */
  unsigned int fragmentShader;
  const char *fragmentShaderSource = "#version 410 core\n"
                                     "out vec4 FragColor;\n"
                                     "void main() {\n"
                                     "  FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
                                     "}";

  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShader);

  int fragmentShaderSuccess;
  char fragmentShaderInfoLog[INFOLOG_LENGTH];
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragmentShaderSuccess);

  if (!fragmentShaderSuccess) {
    glGetShaderInfoLog(fragmentShader, INFOLOG_LENGTH, NULL, fragmentShaderInfoLog);
    printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", fragmentShaderInfoLog);
  }

  /* end fragment shader */

  /* shader program */
  unsigned int shaderProgram;
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  int shaderProgramSuccess;
  char shaderProgramInfoLog[INFOLOG_LENGTH];
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &shaderProgramSuccess);

  if (!shaderProgramSuccess) {
    glGetProgramInfoLog(shaderProgram, INFOLOG_LENGTH, NULL, shaderProgramInfoLog);
    printf("ERROR::SHADER::PROGRAM::LINK_FAILED\n%s\n", shaderProgramInfoLog);
  }

  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);
  /* end shader program */

  /* declare vertices */
  float vertices[] = {
    -0.5, -0.5, 0.0,
    0.5, -0.5, 0.0,
    0.0,  0.5, 0.0
  };

  unsigned int VBO;
  glGenBuffers(1, &VBO);

  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  /* end declare vertices */

  /* set up vertex attribute array */
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  /* end setting up vertex attribute array */

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    processInput(window);

    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

    /* *use* shader program */
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    /* draw the darn triangles */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
