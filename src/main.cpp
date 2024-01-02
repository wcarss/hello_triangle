#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define INFOLOG_LENGTH 512
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  printf("famebuffer_size changed: %d, %d\n", width, height);
  glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window, float* offset, float *textureSwap)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    offset[0] -= 0.01;
  } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    offset[0] += 0.01;
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    offset[1] -= 0.01;
  } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    offset[1] += 0.01;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    *textureSwap += 0.01;
    std::cout << "textureSwap up: " << *textureSwap << std::endl;
  } else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    *textureSwap -= 0.01;
    std::cout << "textureSwap down: " << *textureSwap << std::endl;
  }
}

int main(int argc, char** argv)
{
  const int WINDOW_WIDTH = 1280;
  const int WINDOW_HEIGHT = 720;
  GLFWwindow* window;
  float offset[3] = {0.0f, 0.0f, 0.0f};
  float textureSwap = 0.2;

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

  /* texture loading */
  // flip images to a right-side-up view:
  stbi_set_flip_vertically_on_load(true);

  int width1, height1, nrChannels1;
  unsigned int texture1;
  glGenTextures(1, &texture1);
  glBindTexture(GL_TEXTURE_2D, texture1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  unsigned char *data1 = stbi_load("images/container.jpg", &width1, &height1, &nrChannels1, 0);

  if (data1) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width1, height1, 0, GL_RGB, GL_UNSIGNED_BYTE, data1);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data1);

  int width2, height2, nrChannels2;
  unsigned int texture2;
  glGenTextures(1, &texture2);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, texture2);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  unsigned char *data2 = stbi_load("images/awesomeface.png", &width2, &height2, &nrChannels2, 0);

  if (data2) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width2, height2, 0, GL_RGBA, GL_UNSIGNED_BYTE, data2);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data2);

  /* end texture loading */

  Shader ourShader("shaders/shader.vs", "shaders/shader.fs");

  /* declare vertices */
  float vertices[] = {
    // positions          // colors           // texture coords
    0.5f,  0.5f, 0.0f,    1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
    0.5f, -0.5f, 0.0f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left
  };
  unsigned int indices[] = {  // note that we start from 0!
    0, 1, 3,   // first triangle
    1, 2, 3    // second triangle
  };

  // declare and generate the vertex array, vertex buffer, and element index buffer
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  unsigned int EBO;
  glGenBuffers(1, &EBO);

  // bind the vertex array
  glBindVertexArray(VAO);

  // bind the array buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // bind the element buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  /* end declare vertices */

  /* set up attribute arrays */
  // vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // color attributes
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture attributes
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  /* end setting up attribute arrays */

  // un-comment to use wireframe mode:
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // set up textures in shader:
  ourShader.use(); // don't forget to activate the shader before setting uniforms!
  glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0); // set it manually
  ourShader.setInt("texture2", 1); // or with shader class

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    processInput(window, offset, &textureSwap);

    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, glm::vec3(0.5f, -0.5f, 0.0f));
    trans = glm::rotate(trans, (float)glfwGetTime(), glm::vec3(0.0f, 0.0f, 1.0f));

    ourShader.use();
    ourShader.setVec3f("offset", offset[0], offset[1], offset[2]);
    unsigned int transformLoc = glGetUniformLocation(ourShader.ID, "transform");
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
    ourShader.setFloat("textureSwap", textureSwap);

    /* draw the darn triangles, using the vertex array element array buffer */
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glm::mat4 trans2 = glm::mat4(1.0f);
    trans2 = glm::translate(trans2, glm::vec3(-0.5f, 0.5f, 0.0f));
    // std::min(0.0, sin(glfwGetTime())));
    float time = (float)glfwGetTime();
    float sinTime = abs(sin(time));
    trans2 = glm::scale(trans2, glm::vec3(sinTime, sinTime, 0.0f));
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans2));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
