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

float pitch = 0.0f;
float yaw = -90.0f;

typedef struct {
  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;
  float speed;
  float height;
  bool jump;
  float jumpstart;
  float jumpspeed;
  float fall;
} Camera;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  printf("famebuffer_size changed: %d, %d\n", width, height);
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  static float lastX = 0, lastY = 0;
  static bool firstMouse = true;

  if (firstMouse) { // initially set to true; prevents a big jump at first load
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
  lastX = xpos;
  lastY = ypos;

  // controls how much the mouse actually moves the camera
  const float sensitivity = 0.4f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  yaw   += xoffset;
  pitch += yoffset;

  // keeps us locked to sane angles
  if (pitch > 89.0f) {
    pitch =  89.0f;
  }

  if (pitch < -89.0f) {
    pitch = -89.0f;
  }
}

void processInput(GLFWwindow *window, Camera* cam, float *textureSwap, float deltaTime, float currentTime)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  float deltaSpeed = cam->speed * deltaTime;

  if (!cam->jump && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    cam->jump = true;
    cam->jumpstart = currentTime;
    cam->jumpspeed = 3.0f;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cam->pos += deltaSpeed * cam->front;
    cam->pos.y = cam->height;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cam->pos -= deltaSpeed * cam->front;
    cam->pos.y = cam->height;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cam->pos -= glm::normalize(glm::cross(cam->front, cam->up)) * deltaSpeed;
    cam->pos.y = cam->height;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cam->pos += glm::normalize(glm::cross(cam->front, cam->up)) * deltaSpeed;
    cam->pos.y = cam->height;
  }

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    *textureSwap += 0.01;
    std::cout << "textureSwap up: " << *textureSwap << std::endl;
  } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    *textureSwap -= 0.01;
    std::cout << "textureSwap down: " << *textureSwap << std::endl;
  }
}

void setupCam(Camera* cam)
{
  cam->pos = glm::vec3(0.0f, 0.0f,  3.0f);
  cam->front = glm::vec3(0.0f, 0.0f, -1.0f);
  cam->up = glm::vec3(0.0f, 1.0f,  0.0f);
  cam->speed = 2.5f;
  cam->height = 0;
  cam->jump = false;
  cam->jumpstart = 0.0f;
  cam->jumpspeed = 0.0f;
  cam->fall = 3.6f;
}

int main(int argc, char** argv)
{
  const int WINDOW_WIDTH = 1280;
  const int WINDOW_HEIGHT = 720;
  GLFWwindow* window;
  float textureSwap = 0.2;
  int texSelect = 0;
  Camera cam;
  setupCam(&cam);
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame
  glm::vec3 lightPos(0.2f, 1.0f, 2.0f);

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
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

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

  int width3, height3, nrChannels3;
  unsigned int texture3;
  glGenTextures(1, &texture3);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, texture3);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  unsigned char *data3 = stbi_load("images/altdev/generic-02.png", &width3, &height3, &nrChannels3, 0);

  if (data3) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width3, height3, 0, GL_RGBA, GL_UNSIGNED_BYTE, data3);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data3);

  int width4, height4, nrChannels4;
  unsigned int texture4;
  glGenTextures(1, &texture4);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, texture4);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  unsigned char *data4 = stbi_load("images/altdev/generic-12.png", &width4, &height4, &nrChannels4, 0);

  if (data4) {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width4, height4, 0, GL_RGBA, GL_UNSIGNED_BYTE, data4);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data4);

  /* end texture loading */

  Shader lightingShader("shaders/lighting_shader.vs", "shaders/lighting_shader.fs");
  Shader lightCubeShader("shaders/light_cube_shader.vs", "shaders/light_cube_shader.fs");

  /* declare vertices */
  float vertices[] = {
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,
    0.5f,  -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
    0.5f,   0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
    0.5f,   0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, 0.0f, -1.0f,   0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,
    0.5f,  -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,    0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,

    0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
    0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    1.0f, 1.0f,
    0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
    0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 1.0f,
    0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    0.0f, 0.0f,
    0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,    1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,
    0.5f,  -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 1.0f,
    0.5f,  -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    0.5f,  -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,   0.0f, -1.0f, 0.0f,   0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f,
    0.5f,   0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 1.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
    0.5f,   0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,    0.0f, 1.0f
  };

  float vertices_plane[] = {
    0.0f,   0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
    100.0f, 0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    100.0f, 0.0f,
    100.0f, 0.0f, 100.0f,  0.0f, 1.0f, 0.0f,    100.0f, 100.0f,
    0.0f,   0.0f, 0.0f,    0.0f, 1.0f, 0.0f,    0.0f, 0.0f,
    0.0f,   0.0f, 100.0f,  0.0f, 1.0f, 0.0f,    0.0f, 100.0f,
    100.0f, 0.0f, 100.0f,  0.0f, 1.0f, 0.0f,    100.0f, 100.0f,
  };

  //unsigned int indices[] = {  // note that we start from 0!
  //  0, 1, 3,   // first triangle
  //  1, 2, 3    // second triangle
  //};

  // declare and generate the vertex array, vertex buffer, and element index buffer
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  //unsigned int EBO;
  //glGenBuffers(1, &EBO);

  // bind the vertex array
  glBindVertexArray(VAO);

  // bind the array buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // bind the element buffer
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  /* end declare vertices */

  /* set up attribute arrays */
  // vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture attributes
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
  /* end setting up attribute arrays */

  /* plane */

  unsigned int VAO_plane;
  glGenVertexArrays(2, &VAO_plane);
  unsigned int VBO_plane;
  glGenBuffers(2, &VBO_plane);
  //unsigned int EBO;
  //glGenBuffers(1, &EBO);

  // bind the vertex array
  glBindVertexArray(VAO_plane);

  // bind the array buffer
  glBindBuffer(GL_ARRAY_BUFFER, VBO_plane);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_plane), vertices_plane, GL_STATIC_DRAW);

  // bind the element buffer
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  /* end declare vertices */

  /* set up attribute arrays */
  // vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture attributes
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  unsigned int lightCubeVAO;
  glGenVertexArrays(1, &lightCubeVAO);
  glBindVertexArray(lightCubeVAO);
  // we only need to bind to the VBO, the original container's VBO's data already contains the data we need.
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // set the vertex attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // normals
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture attributes
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  /* end plane */

  // un-comment to use wireframe mode:
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // set up textures in shader:
  lightingShader.use(); // don't forget to activate the shader before setting uniforms!
  texSelect = 0;
  lightingShader.setInt("texSelect", texSelect); // use blended texture by default
  lightingShader.setInt("texture1", 0); // or with shader class
  lightingShader.setInt("texture2", 1); // or with shader class
  lightingShader.setInt("texture3", 2); // or with shader class
  lightingShader.setInt("texture4", 3); // or with shader class
  lightingShader.setVec3f("lightPos", lightPos.x, lightPos.y, lightPos.z);

  // set up lighting
  lightingShader.setVec3f("objectColor", 1.0f, 0.5f, 0.31f);
  lightingShader.setVec3f("lightColor",  1.0f, 1.0f, 1.0f);

  glm::vec3 cubePositions[] = {
    glm::vec3(0.1f,  0.1f,  0.1f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f),

    glm::vec3(-5.0f,  0.0f, -5.0f),
    glm::vec3(-5.0f,  0.0f, -4.0f),
    glm::vec3(-5.0f,  0.0f, -3.0f),
    glm::vec3(-5.0f,  0.0f, -2.0f),
    glm::vec3(-5.0f,  0.0f, -1.0f),
    glm::vec3(-5.0f,  0.0f, 0.0f),
    glm::vec3(-5.0f,  0.0f, 1.0f),
    glm::vec3(-5.0f,  0.0f, 2.0f),
    glm::vec3(-5.0f,  0.0f, 3.0f),
    glm::vec3(-5.0f,  0.0f, 4.0f),
  };

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    float currentTime = glfwGetTime();

    if (cam.jump == true) {
      float jumpstartdelta = currentTime - cam.jumpstart;

      if (jumpstartdelta < 2) {
        cam.height += cam.jumpspeed * deltaTime;
        cam.jumpspeed -= cam.fall * deltaTime;
      } else {
        cam.jump = false;
        cam.jumpstart = 0;
      }
    } else if (cam.height > 0.0f && cam.jump != true) {
      cam.height -= cam.fall * deltaTime;
    }

    if (cam.height < 0.0f) {
      cam.height = 0.0f;
    }

    cam.pos.y = cam.height;

    processInput(window, &cam, &textureSwap, deltaTime, currentTime);

    /* Render here */
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cam.front = glm::normalize(direction);
    view = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    lightingShader.use();
    unsigned int modelLoc = glGetUniformLocation(lightingShader.ID, "model");
    unsigned int viewLoc = glGetUniformLocation(lightingShader.ID, "view");
    unsigned int projLoc = glGetUniformLocation(lightingShader.ID, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    lightingShader.setFloat("textureSwap", textureSwap);
    lightPos.x = 5 * sin(glfwGetTime() * 5.0f / 8.0f);
    lightPos.z = 5 * cos(glfwGetTime() * 5.0f / 8.0f);
    lightingShader.setVec3f("lightPos", lightPos.x, lightPos.y, lightPos.z);
    lightingShader.setVec3f("viewPos", cam.pos.x, cam.pos.y, cam.pos.z);

    /* draw the darn triangles, using the vertex array element array buffer */
    glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding texture
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texture3);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, texture4);
    glBindVertexArray(VAO);
    //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    texSelect = 0;
    lightingShader.setInt("texSelect", texSelect); // use blended texture by default

    for (unsigned int i = 0; i < 10; i++) {
      glm::mat4 model = glm::mat4(1.0f);

      model = glm::translate(model, cubePositions[i]);
      float angle = 20.0f * i;

      if (i % 6 == 0) {
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 1.0f, 0.5f));
      } else if (i % 3 == 0) {
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      } else if (i % 2 == 0) {
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(angle), glm::vec3(0.3f, 0.1f, 0.5f));
      } else if (i <= 10) {
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
      }

      glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    texSelect = 3;
    lightingShader.setInt("texSelect", texSelect); // use other texture

    for (unsigned int i = 0; i < 50; i++) {
      for (unsigned int j = 0; j < 50; j++) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-25.0f, 0.0f, -25.0f));
        model = glm::translate(model, glm::vec3((float)i, 0.0f, (float)j));

        if (i == 0 || j == 0 || i == 49 - 1 || j == 49 - 1) {
          glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
          glDrawArrays(GL_TRIANGLES, 0, 36);
          model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));

          if (((i == 0 || i == 49 - 1) && j % 2 == 0) || ((j == 0 || j == 49 - 1) && i % 2 == 0)) {
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawArrays(GL_TRIANGLES, 0, 36);
          }
        }
      }
    }

    texSelect = 4;
    lightingShader.setInt("texSelect", texSelect); // use other texture
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-50.0f, -0.5f, -50.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(VAO_plane);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    lightCubeShader.use();
    unsigned int lightCubeModelLoc = glGetUniformLocation(lightCubeShader.ID, "model");
    unsigned int lightCubeViewLoc = glGetUniformLocation(lightCubeShader.ID, "view");
    unsigned int lightCubeProjLoc = glGetUniformLocation(lightCubeShader.ID, "projection");

    glUniformMatrix4fv(lightCubeModelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(lightCubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(lightCubeProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glBindVertexArray(lightCubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
