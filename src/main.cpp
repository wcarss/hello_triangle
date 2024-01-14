#include <stdio.h>
#include <stdbool.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <shader.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define MAX_NUM_OF_LIGHTS 100
#define INFOLOG_LENGTH 512
#define WITHOUT_ATTRIBUTES 0
#define WITH_ATTRIBUTES 1
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

typedef struct {
  glm::vec3 pos;
  glm::vec3 front;
  glm::vec3 up;
  bool forwardPressed;
  bool backwardPressed;
  bool leftPressed;
  bool rightPressed;
  float speed;
  float height;
  bool shouldJump;
  bool jump;
  float jumpStart;
  float jumpSpeed;
  float maxFallSpeed;
  float maxJumpSpeed;
  float jumpTime;
  float fallSpeed;
  float gravity;
  bool canJump;
  float pitch;
  float yaw;
  float lightsUsedControl;
} Camera;

typedef struct {
  glm::vec3 specular;
  glm::vec3 diffuse;
  glm::vec3 ambient;
  glm::vec3 pos;
  float height;
  float constant;
  float linear;
  float quadratic;
} PointLight;

typedef struct {
  glm::vec3 specular;
  glm::vec3 diffuse;
  glm::vec3 ambient;
  glm::vec3 dir;
} DirLight;

typedef struct {
  Camera* cam;
} GameContext;

void setupCam(Camera* cam)
{
  cam->pos = glm::vec3(0.0f, 0.0f,  3.0f);
  cam->front = glm::vec3(0.0f, 0.0f, -1.0f);
  cam->up = glm::vec3(0.0f, 1.0f,  0.0f);
  cam->forwardPressed = false;
  cam->backwardPressed = false;
  cam->leftPressed = false;
  cam->rightPressed = false;
  cam->speed = 2.5f;
  cam->height = 0;
  cam->jump = false;
  cam->shouldJump = false;
  cam->jumpStart = 0.0f;
  cam->jumpSpeed = 0.0f;
  cam->maxJumpSpeed = 4.5f;
  cam->jumpTime = 1.29f;
  cam->fallSpeed = 0.0f;
  cam->maxFallSpeed = 7.0f;
  cam->gravity = 3.5f;
  cam->canJump = true;
  cam->pitch = 0.0f;
  cam->yaw = -90.0f;
  cam->lightsUsedControl = 1.0f;
}

void processCamera(Camera* cam, float deltaTime, float currentFrame)
{
  float deltaSpeed = cam->speed * deltaTime;

  if (!cam->jump && cam->shouldJump) {
    cam->shouldJump = false;
    cam->canJump = false;
    cam->jump = true;
    cam->jumpStart = currentFrame;
    cam->jumpSpeed = cam->maxJumpSpeed;
  }

  if (cam->forwardPressed) {
    cam->forwardPressed = false;
    cam->pos += deltaSpeed * cam->front;
  }

  if (cam->backwardPressed) {
    cam->backwardPressed = false;
    cam->pos -= deltaSpeed * cam->front;
  }

  if (cam->leftPressed) {
    cam->leftPressed = false;
    cam->pos -= glm::normalize(glm::cross(cam->front, cam->up)) * deltaSpeed;
  }

  if (cam->rightPressed) {
    cam->rightPressed = false;
    cam->pos += glm::normalize(glm::cross(cam->front, cam->up)) * deltaSpeed;
  }

  if (cam->jump == true) {
    float jumpStartDelta = currentFrame - cam->jumpStart;

    // jump runs for "2 seconds", or something.
    if (jumpStartDelta < cam->jumpTime) {
      cam->height += cam->jumpSpeed * deltaTime;
      cam->jumpSpeed -= cam->gravity * deltaTime;

      if (cam->jumpSpeed < 0.0f) {
        //printf("redundant jump speed call: %f\n", cam->jumpSpeed);
        cam->jumpSpeed = 0;
      }
    } else {
      cam->jump = false;
      cam->jumpStart = 0;
      cam->fallSpeed = cam->jumpSpeed;
    }
  } else if (cam->height > 0.1f) {
    cam->height -= cam->fallSpeed * deltaTime;
    cam->fallSpeed += cam->gravity * deltaTime;

    if (cam->fallSpeed > cam->maxFallSpeed) {
      cam->fallSpeed = cam->maxFallSpeed;
    }
  } else if (cam->height < 0.1f) {
    cam->canJump = true;
    cam->height = 0.0f;
    cam->fallSpeed = 0.0f;
  }

  cam->pos.y = cam->height;

  glm::vec3 direction;
  direction.x = cos(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
  direction.y = sin(glm::radians(cam->pitch));
  direction.z = sin(glm::radians(cam->yaw)) * cos(glm::radians(cam->pitch));
  cam->front = glm::normalize(direction);
}

void changeCameraAngles(Camera *cam, float xoffset, float yoffset)
{
  cam->yaw   += xoffset;
  cam->pitch += yoffset;

  // keeps us locked to sane angles
  if (cam->pitch > 89.0f) {
    cam->pitch =  89.0f;
  }

  if (cam->pitch < -89.0f) {
    cam->pitch = -89.0f;
  }
}

void sendPointLightColors(Shader* shader, PointLight * lights, int numOfLights)
{
  char fieldName[32] = "pointLights";
  char formattedSpecifier[32];

  for (int i = 0; i < numOfLights; i++) {
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "ambient");
    shader->setVec3f(formattedSpecifier, lights[i].ambient.r, lights[i].ambient.g, lights[i].ambient.b);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "diffuse");
    shader->setVec3f(formattedSpecifier, lights[i].diffuse.r, lights[i].diffuse.g, lights[i].diffuse.b);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "specular");
    shader->setVec3f(formattedSpecifier, lights[i].specular.r, lights[i].specular.g, lights[i].specular.b);
  }
}

void sendPointLightPositions(Shader* shader, PointLight * lights, int numOfLights)
{
  char fieldName[32] = "pointLights";
  char formattedSpecifier[32];

  for (int i = 0; i < numOfLights; i++) {
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "pos");
    shader->setVec3f(formattedSpecifier, lights[i].pos.x, lights[i].pos.y, lights[i].pos.z);
  }
}

void sendPointLightAttenuations(Shader* shader, PointLight * lights, int numOfLights)
{
  char fieldName[32] = "pointLights";
  char formattedSpecifier[32];

  for (int i = 0; i < numOfLights; i++) {
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "linear");
    shader->setFloat(formattedSpecifier, lights[i].linear);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "constant");
    shader->setFloat(formattedSpecifier, lights[i].constant);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "quadratic");
    shader->setFloat(formattedSpecifier, lights[i].quadratic);
  }
}

void setupPointLightDefaults(PointLight *light)
{
  light->pos = glm::vec3(0.7f,  0.2f,  2.0f);
  light->height = light->pos.y;
  light->specular = glm::vec3(1.0f);
  light->diffuse = light->specular * 0.65f;
  light->ambient = light->specular * 0.3f;
  light->constant = 1.0f;
  light->linear = 0.15f;
  light->quadratic = 0.032f;
}

void updatePointLightColor(PointLight *light, glm::vec3 color)
{
  light->specular = color;
  light->diffuse = light->specular * 0.65f;
  light->ambient = light->specular * 0.3f;
}

void updateDirLightColor(PointLight *light, glm::vec3 color)
{
  light->specular = color;
  light->diffuse = light->specular * 0.65f;
  light->ambient = light->specular * 0.3f;
}

void setupDirLightDefaults(DirLight *light)
{
  light->dir = glm::vec3(0.2f,  -0.4f,  1.0f);
  light->specular = glm::vec3(1.0f);
  light->diffuse = light->specular * 0.65f;
  light->ambient = light->specular * 0.3f;
}

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

  GameContext* game = (GameContext *) glfwGetWindowUserPointer(window);
  changeCameraAngles(game->cam, xoffset, yoffset);
}

void processInput(GLFWwindow *window, Camera* cam)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  cam->shouldJump = false;

  if (cam->canJump && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    cam->shouldJump = true;
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    cam->forwardPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    cam->backwardPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    cam->leftPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    cam->rightPressed = true;
  }

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    cam->lightsUsedControl += 0.5f;

    if (cam->lightsUsedControl > MAX_NUM_OF_LIGHTS) {
      cam->lightsUsedControl = MAX_NUM_OF_LIGHTS;
    }
  }

  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    cam->lightsUsedControl -= 0.5f;

    if (cam->lightsUsedControl < 0) {
      cam->lightsUsedControl = 0;
    }
  }
}

int loadTexture(int tex_number, const char *path)
{
  int width, height, nrChannels;
  unsigned int texture;
  glGenTextures(1, &texture);
  std::cout << "loadTexture texture id set to: " << texture << " for path " << path << std::endl;
  glActiveTexture(GL_TEXTURE0 + tex_number);
  glBindTexture(GL_TEXTURE_2D, texture);

  unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);

  if (data) {
    int pathLen = strlen(path);

    if (strcmp(&path[pathLen - 4], ".jpg") == 0) {
      printf("got jpg for path: %s\n", path);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    } else {
      printf("got non-jpg for path: %s\n", path);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  } else {
    std::cout << "Failed to load texture" << std::endl;
  }

  stbi_image_free(data);
  return texture;
}

int loadObject(float *vertices, unsigned int array_size, int with_attributes)
{
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
  glBufferData(GL_ARRAY_BUFFER, array_size, vertices, GL_STATIC_DRAW);

  // bind the element buffer
  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  /* end declare vertices */

  /* set up attribute arrays */
  if (with_attributes == WITH_ATTRIBUTES) {
    // vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture attributes
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
  } else {
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
  }

  return VAO;
}

unsigned int loadCubemap(int tex_number, std::vector<std::string> faces)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);
  std::cout << "cubemap textureID set to: " << textureID << std::endl;
  glActiveTexture(GL_TEXTURE0 + textureID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

  int width, height, nrChannels;

  for (unsigned int i = 0; i < faces.size(); i++) {
    unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);

    if (data) {
      if (nrChannels == 3) {
        std::cout << "3 channels and w,h " << width << ", " << height << " at path: " << faces[i] << std::endl;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
                    );
      } else if (nrChannels == 4) {
        if (width != height) {
          height = width;
        }

        std::cout << "4 channels and w,h " << width << ", " << height << " at path: " << faces[i] << std::endl;
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data
                    );
      }

      stbi_image_free(data);
    } else {
      std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
      stbi_image_free(data);
    }
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return textureID;
}

int main(int argc, char** argv)
{
  const int WINDOW_WIDTH = 1280;
  const int WINDOW_HEIGHT = 720;
  GLFWwindow* window;
  GameContext game;
  Camera cam;
  setupCam(&cam);
  game.cam = &cam;
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame
  //glm::vec3 lightPos(0.2f, 1.0f, 2.0f);
  PointLight pointLights[MAX_NUM_OF_LIGHTS];
  DirLight dirLight;
  glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, 3.3f, -4.0f),
    glm::vec3(-3.0f, 0.8f, -6.0f),
    glm::vec3(0.0f,  1.0f, -3.0f),
    glm::vec3(1.0f,  2.0f, -2.0f),
    glm::vec3(1.5f,  7.0f, -3.0f),
    glm::vec3(5.0f,  0.0f, -5.0f),
    glm::vec3(0.8f,  0.3f, -2.0f),
    glm::vec3(3.5f,  6.0f, -3.0f),
    glm::vec3(0.5f,  8.0f, -3.0f)
  };

  for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
    setupPointLightDefaults(&pointLights[i]);

    if (i < 10) {
      pointLights[i].pos = pointLightPositions[i];
    } else {
      pointLights[i].pos = glm::vec3((i % 11) * 0.3f, (i % 13) * 0.3f, (i % 17) * 0.6f);
    }

    pointLights[i].height = pointLights[i].pos.y;
  }

  setupDirLightDefaults(&dirLight);

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

  glfwSetWindowUserPointer(window, (void *)&game);

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glEnable(GL_DEPTH_TEST);

  /* texture loading */
  std::vector<std::string> vfaces = {
    "images/skybox/tutorial/right.jpg",
    "images/skybox/tutorial/left.jpg",
    "images/skybox/tutorial/top.jpg",
    "images/skybox/tutorial/bottom.jpg",
    "images/skybox/tutorial/front.jpg",
    "images/skybox/tutorial/back.jpg"
  };
  std::vector<std::string> kfaces = {
    "images/skybox/kenney_voxel_pack/skybox_side1.png",
    "images/skybox/kenney_voxel_pack/skybox_side2.png",
    "images/skybox/kenney_voxel_pack/skybox_top.png",
    "images/skybox/kenney_voxel_pack/skybox_bottom.png",
    "images/skybox/kenney_voxel_pack/skybox_side3.png",
    "images/skybox/kenney_voxel_pack/skybox_side4.png"
  };


  // flip the rest of the images around vertically
  stbi_set_flip_vertically_on_load(true);
  unsigned int container = loadTexture(1, "images/container.jpg");
  unsigned int container2 = loadTexture(2, "images/container2.png");
  unsigned int container2_specular = loadTexture(3, "images/container2_specular.png");
  unsigned int generic01 = loadTexture(4, "images/altdev/generic-07.png");
  unsigned int generic02 = loadTexture(5, "images/altdev/generic-12.png");
  unsigned int awesomeface = loadTexture(6, "images/awesomeface.png");
  unsigned int matrix = loadTexture(7, "images/matrix.jpg");
  unsigned int blank = loadTexture(8, "images/1x1.png");
  unsigned int container2_emission_map = loadTexture(9, "images/container2_emission_map.png");
  stbi_set_flip_vertically_on_load(false);
  unsigned int cubemapTexture = loadCubemap(10, vfaces);

  /* end texture loading */
  Shader lightingShader("shaders/lighting_shader.vs", "shaders/lighting_shader.fs");
  Shader lightCubeShader("shaders/light_cube_shader.vs", "shaders/light_cube_shader.fs");
  Shader skyboxShader("shaders/skybox_shader.vs", "shaders/skybox_shader.fs");

  /* declare vertices */
  float vertices_cube[] = {
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

  float vertices_skybox[] = {
    // positions
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
    1.0f,  1.0f, -1.0f,
    1.0f,  1.0f,  1.0f,
    1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f, -1.0f,
    1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    1.0f, -1.0f,  1.0f
  };
  //unsigned int indices[] = {  // note that we start from 0!
  //  0, 1, 3,   // first triangle
  //  1, 2, 3    // second triangle
  //};

  unsigned int VAO = loadObject(vertices_cube, sizeof(vertices_cube), WITH_ATTRIBUTES);
  unsigned int VAO_plane = loadObject(vertices_plane, sizeof(vertices_plane), WITH_ATTRIBUTES);
  unsigned int lightCubeVAO = loadObject(vertices_cube, sizeof(vertices_cube), WITH_ATTRIBUTES);
  unsigned int VAO_skybox = loadObject(vertices_skybox, sizeof(vertices_skybox), WITHOUT_ATTRIBUTES);

  // un-comment to use wireframe mode:
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // set up textures in shader:
  skyboxShader.use();
  skyboxShader.setInt("skybox", cubemapTexture);
  lightingShader.use(); // don't forget to activate the shader before setting uniforms!
  lightingShader.setInt("skybox", cubemapTexture);

  // set up lighting
  lightingShader.setVec3f("objectColor", 1.0f, 0.5f, 0.31f);
  lightingShader.setVec3f("lightColor",  1.0f, 1.0f, 1.0f);
  sendPointLightAttenuations(&lightingShader, pointLights, MAX_NUM_OF_LIGHTS);

  glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, 2.2f, -2.5f),
    glm::vec3(-3.8f, 2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(3.3f, 3.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f),
  };

  lightingShader.setVec3f("dirLight.dir", dirLight.dir.x, dirLight.dir.y, dirLight.dir.z);
  lightingShader.setVec3f("dirLight.diffuse", dirLight.diffuse.r, dirLight.diffuse.g, dirLight.diffuse.b);
  lightingShader.setVec3f("dirLight.ambient", dirLight.ambient.r, dirLight.ambient.g, dirLight.ambient.b);

  /* Loop until the user closes the window */
  while (!glfwWindowShouldClose(window)) {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    processInput(window, &cam);
    processCamera(&cam, deltaTime, currentFrame);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear the buffers

    glDepthMask(GL_FALSE);
    skyboxShader.use();

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);
    glm::mat4 skybox_view = glm::mat4(glm::mat3(view));

    unsigned int skyboxViewLoc = glGetUniformLocation(skyboxShader.ID, "view");
    unsigned int skyboxProjLoc = glGetUniformLocation(skyboxShader.ID, "projection");
    glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(skybox_view));
    glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO_skybox);
    //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthMask(GL_TRUE);

    lightingShader.use();
    int lightsUsed = (int)floor(cam.lightsUsedControl);
    lightingShader.setInt("lightsUsed", lightsUsed);

    /* Render here */
    unsigned int modelLoc = glGetUniformLocation(lightingShader.ID, "model");
    unsigned int viewLoc = glGetUniformLocation(lightingShader.ID, "view");
    unsigned int projLoc = glGetUniformLocation(lightingShader.ID, "projection");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    lightingShader.setVec3f("viewPos", cam.pos.x, cam.pos.y, cam.pos.z);

    // revolving lights
    for (int i = 0; i < lightsUsed; i++) {
      float distance = sqrt(pointLights[i].pos.x * pointLights[i].pos.x + pointLights[i].pos.z * pointLights[i].pos.z);
      pointLights[i].pos.x = distance * sin(glfwGetTime() * (i % 11 + 1) / (2.0f + (i % 3) * 1.5));
      pointLights[i].pos.y = pointLights[i].height + sin(glfwGetTime() * (i % 11 + 1) / 5.0f) * 1.3f;
      pointLights[i].pos.z = distance * cos(glfwGetTime() * (i % 11 + 1) / (2.0f + (i % 3) * 1.5));
      glm::vec3 lightColor;
      lightColor.x = abs(sin(glfwGetTime() * (i % 7 + 1) * 0.15f));
      lightColor.y = abs(sin(glfwGetTime() * (i % 11 + 1) * 0.17f));
      lightColor.z = abs(sin(glfwGetTime() * (i % 9 + 1) * 0.13f));
      updatePointLightColor(&pointLights[i], lightColor);
    }

    sendPointLightColors(&lightingShader, pointLights, lightsUsed);
    sendPointLightPositions(&lightingShader, pointLights, lightsUsed);

    /* begin to draw the actual triangles, using the vertex array buffer */
    glBindVertexArray(VAO);

    lightingShader.setFloat("material.shininess", 16.0f);
    lightingShader.setVec3f("material.ambient", 1.0f, 0.5f, 0.31f);
    lightingShader.setInt("material.emission", matrix);
    lightingShader.setInt("material.emission_map", container2_emission_map);
    lightingShader.setInt("material.diffuse", container2);
    lightingShader.setInt("material.specular", container2_specular);

    for (unsigned int i = 0; i < 10; i++) {
      glm::mat4 model = glm::translate(glm::mat4(1.0f), cubePositions[i]);
      float angle = 20.0f * i;
      int awesomeface_index = 4;

      if (i == awesomeface_index) {
        lightingShader.setFloat("material.shininess", 64.0f);
        lightingShader.setInt("material.diffuse", awesomeface);
        lightingShader.setInt("material.specular", blank);
        lightingShader.setInt("material.emission", awesomeface);
        lightingShader.setInt("material.emission_map", awesomeface);
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

      if (i == awesomeface_index) {
        lightingShader.setFloat("material.shininess", 16.0f);
        lightingShader.setInt("material.emission", matrix);
        lightingShader.setInt("material.diffuse", container2);
        lightingShader.setInt("material.specular", container2_specular);
        lightingShader.setInt("material.emission_map", container2_emission_map);
      }
    }

    lightingShader.setInt("material.specular", blank);
    lightingShader.setInt("material.emission", blank);
    lightingShader.setInt("material.emission_map", blank);
    lightingShader.setInt("material.diffuse", generic01);

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

    lightingShader.setInt("material.diffuse", generic02);
    model = glm::translate(glm::mat4(1.0f) , glm::vec3(-50.0f, -0.5f, -50.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(VAO_plane);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    for (int i = 0; i < lightsUsed; i++) {
      model = glm::translate(glm::mat4(1.0f), pointLights[i].pos);
      model = glm::scale(model, glm::vec3(0.1f * (((i + 1) * 2) % 7)));
      lightCubeShader.use();
      lightCubeShader.setVec3f("light.specular", pointLights[i].specular.r, pointLights[i].specular.g, pointLights[i].specular.b);
      unsigned int lightCubeModelLoc = glGetUniformLocation(lightCubeShader.ID, "model");
      unsigned int lightCubeViewLoc = glGetUniformLocation(lightCubeShader.ID, "view");
      unsigned int lightCubeProjLoc = glGetUniformLocation(lightCubeShader.ID, "projection");

      glUniformMatrix4fv(lightCubeModelLoc, 1, GL_FALSE, glm::value_ptr(model));
      glUniformMatrix4fv(lightCubeViewLoc, 1, GL_FALSE, glm::value_ptr(view));
      glUniformMatrix4fv(lightCubeProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
      glBindVertexArray(lightCubeVAO);
      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  glfwTerminate();
  return 0;
}
