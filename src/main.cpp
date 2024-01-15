#include <stdio.h>
#include <stdlib.h>
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
  int vao;
  int size;
} Mesh;

typedef struct {
  Shader *shader;
  int specularTexture;
  float shininess;
  int ambientTexture; // currently unused
  int diffuseTexture;
  int emissionValues;
  int emissionMap;
  glm::vec3 ambientColor;
  glm::vec3 specularColor;
  glm::vec3 diffuseColor;
} Material;

typedef struct {
  Mesh *mesh; // vertex array object, created with createMesh
  Material *mat; // material created with createMaterial
  glm::vec3 pos;
  glm::vec3 rot;
  glm::vec3 scale;
  float angle;
} GameObject;

typedef struct {
  GameObject *gameObject;
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

Material* createMaterial(Shader *shader, int specularTexture, float shininess, int diffuseTexture, glm::vec3 ambientColor, int emissionValues, int emissionMap)
{
  Material* mat = (Material *)malloc(sizeof(Material));

  mat->shader = shader;
  mat->specularTexture = specularTexture;
  mat->shininess = 16.0f;
  mat->diffuseTexture = diffuseTexture;
  mat->ambientColor = ambientColor;
  mat->emissionValues = emissionValues;
  mat->emissionMap = emissionMap;
  // only used by the light cubes; default vals for now
  mat->specularColor = glm::vec3(1.0f);
  mat->diffuseColor = glm::vec3(1.0f);
  // unused for now:
  mat->ambientTexture = -1;

  return mat;
};

void destroyMaterial(Material *mat)
{
  free(mat);
}

GameObject* createGameObject(Mesh *mesh, Material *mat, glm::vec3 pos)
{
  GameObject *gameObject = (GameObject *)malloc(sizeof(GameObject));
  gameObject->mesh = mesh;
  gameObject->mat = mat;
  gameObject->pos = pos;
  gameObject->rot = glm::vec3(1.0f);
  gameObject->scale = glm::vec3(1.0f);
  gameObject->angle = 0.0f;
  return gameObject;
}

void destroyGameObject(GameObject *gameObject)
{
  free(gameObject);
}

void renderGameObject(GameObject *gameObject, glm::mat4 view, glm::mat4 projection)
{
  Material *mat = gameObject->mat;
  Shader *shader = mat->shader;
  shader->use();

  unsigned int modelLoc = glGetUniformLocation(shader->ID, "model");
  unsigned int viewLoc = glGetUniformLocation(shader->ID, "view");
  unsigned int projLoc = glGetUniformLocation(shader->ID, "projection");

  shader->setInt("material.specular", mat->specularTexture);
  shader->setFloat("material.shininess", mat->shininess);
  shader->setInt("material.diffuse", mat->diffuseTexture);
  shader->setVec3f("material.ambient", mat->ambientColor.r, mat->ambientColor.g, mat->ambientColor.b);
  shader->setInt("material.emission", mat->emissionValues);
  shader->setInt("material.emission_map", mat->emissionMap);

  glm::mat4 model = glm::translate(glm::mat4(1.0f), gameObject->pos);
  model = glm::rotate(model, gameObject->angle, gameObject->rot);
  model = glm::scale(model, gameObject->scale);

  glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
  glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
  glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

  glBindVertexArray(gameObject->mesh->vao);
  glDrawArrays(GL_TRIANGLES, 0, gameObject->mesh->size);
}

void renderSkybox(GameObject *skybox, glm::mat4 view, glm::mat4 projection)
{
  Shader *shader = skybox->mat->shader;
  shader->use();

  glDepthMask(GL_FALSE);
  glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
  unsigned int skyboxViewLoc = glGetUniformLocation(shader->ID, "view");
  unsigned int skyboxProjLoc = glGetUniformLocation(shader->ID, "projection");
  glUniformMatrix4fv(skyboxViewLoc, 1, GL_FALSE, glm::value_ptr(skyboxView));
  glUniformMatrix4fv(skyboxProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
  glBindVertexArray(skybox->mesh->vao);
  glDrawArrays(GL_TRIANGLES, 0, skybox->mesh->size);
  glDepthMask(GL_TRUE);
}

void renderWalls(GameObject *wall, glm::mat4 view, glm::mat4 projection)
{
  for (unsigned int i = 0; i < 50; i++) {
    for (unsigned int j = 0; j < 50; j++) {
      glm::mat4 model = glm::mat4(1.0f);
      // setting the position of the wall cubes on the CPU
      model = glm::translate(model, glm::vec3(-25.0f, 0.0f, -25.0f));
      model = glm::translate(model, glm::vec3((float)i, 0.0f, (float)j));

      // pull translation from model matrix
      wall->pos = glm::vec3(model[3]);

      if (i == 0 || j == 0 || i == 49 - 1 || j == 49 - 1) {
        renderGameObject(wall, view, projection);
        // setting the position of the wall cubes on the GPU
        // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        // rendering
        // glDrawArrays(GL_TRIANGLES, 0, 36);

        if (((i == 0 || i == 49 - 1) && j % 2 == 0) || ((j == 0 || j == 49 - 1) && i % 2 == 0)) {
          // generating some extra cubes for a castle crenellation effect; CPU side
          model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
          wall->pos = glm::vec3(model[3]);
          renderGameObject(wall, view, projection);
          // gpu side
          // glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
          // rendering
          // glDrawArrays(GL_TRIANGLES, 0, 36);
        }
      }
    }
  }
}

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

void sendPointLightColors(Shader *shader, PointLight **lights, int numOfLights)
{
  char fieldName[32] = "pointLights";
  char formattedSpecifier[32];

  for (int i = 0; i < numOfLights; i++) {
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "ambient");
    shader->setVec3f(formattedSpecifier, lights[i]->gameObject->mat->ambientColor.r, lights[i]->gameObject->mat->ambientColor.g, lights[i]->gameObject->mat->ambientColor.b);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "diffuse");
    shader->setVec3f(formattedSpecifier, lights[i]->gameObject->mat->diffuseColor.r, lights[i]->gameObject->mat->diffuseColor.g, lights[i]->gameObject->mat->diffuseColor.b);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "specular");
    shader->setVec3f(formattedSpecifier, lights[i]->gameObject->mat->specularColor.r, lights[i]->gameObject->mat->specularColor.g, lights[i]->gameObject->mat->specularColor.b);
  }
}

void sendPointLightPositions(Shader *shader, PointLight **lights, int numOfLights)
{
  char fieldName[32] = "pointLights";
  char formattedSpecifier[32];

  for (int i = 0; i < numOfLights; i++) {
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "pos");
    shader->setVec3f(formattedSpecifier, lights[i]->gameObject->pos.x, lights[i]->gameObject->pos.y, lights[i]->gameObject->pos.z);
  }
}

void sendPointLightAttenuations(Shader *shader, PointLight **lights, int numOfLights)
{
  char fieldName[32] = "pointLights";
  char formattedSpecifier[32];

  for (int i = 0; i < numOfLights; i++) {
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "linear");
    shader->setFloat(formattedSpecifier, lights[i]->linear);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "constant");
    shader->setFloat(formattedSpecifier, lights[i]->constant);
    snprintf(formattedSpecifier, 32, "%s[%d].%s", fieldName, i, "quadratic");
    shader->setFloat(formattedSpecifier, lights[i]->quadratic);
  }
}

PointLight *createPointLight(Mesh *mesh, Material *mat, glm::vec3 pos)
{
  PointLight *light = (PointLight *)malloc(sizeof(PointLight));
  light->gameObject = createGameObject(mesh, mat, pos);
  light->height = light->gameObject->pos.y;
  light->gameObject->mat->specularColor = glm::vec3(1.0f);
  light->gameObject->mat->diffuseColor = light->gameObject->mat->specularColor * 0.65f;
  light->gameObject->mat->ambientColor = light->gameObject->mat->specularColor * 0.3f;
  light->constant = 1.0f;
  light->linear = 0.15f;
  light->quadratic = 0.032f;
  return light;
}

void destroyPointLight(PointLight *pointLight)
{
  destroyGameObject(pointLight->gameObject);
  free(pointLight);
}

void updatePointLightColor(PointLight *light, glm::vec3 color)
{
  light->gameObject->mat->specularColor = color;
  light->gameObject->mat->diffuseColor = light->gameObject->mat->specularColor * 0.65f;
  light->gameObject->mat->ambientColor = light->gameObject->mat->specularColor * 0.3f;
}

void updateDirLightColor(DirLight *light, glm::vec3 color)
{
  light->specular = color;
  light->diffuse = light->specular * 0.65f;
  light->ambient = light->specular * 0.3f;
}

void updatePointLights(PointLight **pointLights, int lightsUsed)
{
  // light placement -- this is updating their positions in the CPU and GPU, but not rendering the light cubes themselves
  for (int i = 0; i < lightsUsed; i++) {
    GameObject *localGameObj = pointLights[i]->gameObject;
    localGameObj->scale = glm::vec3(0.1f * (((i + 1) * 2) % 7));
    float distance = sqrt(localGameObj->pos.x * localGameObj->pos.x + localGameObj->pos.z * localGameObj->pos.z);
    localGameObj->pos.x = distance * sin(glfwGetTime() * (i % 11 + 1) / (2.0f + (i % 3) * 1.5));
    localGameObj->pos.y = pointLights[i]->height + sin(glfwGetTime() * (i % 11 + 1) / 5.0f) * 1.3f;
    localGameObj->pos.z = distance * cos(glfwGetTime() * (i % 11 + 1) / (2.0f + (i % 3) * 1.5));
    glm::vec3 lightColor;
    lightColor.x = abs(sin(glfwGetTime() * (i % 7 + 1) * 0.15f));
    lightColor.y = abs(sin(glfwGetTime() * (i % 11 + 1) * 0.17f));
    lightColor.z = abs(sin(glfwGetTime() * (i % 9 + 1) * 0.13f));
    updatePointLightColor(pointLights[i], lightColor);
  }
}

void renderPointLightCubes(Shader *lightCubeShader, PointLight **pointLights, int lightsUsed, glm::mat4 view, glm::mat4 projection)
{
  // iterating over the lights to render their models
  for (int i = 0; i < lightsUsed; i++) {
    // tell the lightCubeShader what's what
    lightCubeShader->use();
    lightCubeShader->setVec3f("light.specular", pointLights[i]->gameObject->mat->specularColor.r, pointLights[i]->gameObject->mat->specularColor.g, pointLights[i]->gameObject->mat->specularColor.b);
    renderGameObject(pointLights[i]->gameObject, view, projection);
  }
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

Mesh *createMesh(float *vertices, unsigned int numVertices, unsigned int array_size, int with_attributes)
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

  Mesh *mesh = (Mesh *)malloc(sizeof(Mesh));
  mesh->vao = VAO;
  mesh->size = numVertices;
  return mesh;
}

void destroyMesh(Mesh *mesh)
{
  free(mesh);
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
  int awesomeface_index = 4;
  setupCam(&cam);
  game.cam = &cam;
  float deltaTime = 0.0f; // Time between current frame and last frame
  float lastFrame = 0.0f; // Time of last frame
  //glm::vec3 lightPos(0.2f, 1.0f, 2.0f);
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
  PointLight **pointLights = (PointLight **)malloc(sizeof(PointLight *) * MAX_NUM_OF_LIGHTS);
  int numFlyingCubes = 10;
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
  GameObject **flyingCubes = (GameObject **)malloc(sizeof(GameObject *) * numFlyingCubes);
  DirLight dirLight;

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
  unsigned int container2_emission_map = loadTexture(4, "images/container2_emission_map.png");
  unsigned int generic01 = loadTexture(5, "images/altdev/generic-07.png");
  unsigned int generic02 = loadTexture(6, "images/altdev/generic-12.png");
  unsigned int awesomeface = loadTexture(7, "images/awesomeface.png");
  unsigned int matrixTexture = loadTexture(8, "images/matrix.jpg");
  unsigned int blankTexture = loadTexture(9, "images/1x1.png");

  // for some reason the skybox is flipped differently
  stbi_set_flip_vertically_on_load(false);
  unsigned int skyboxTexture = loadCubemap(10, vfaces);

  /* end texture loading */
  Shader lightingShader("shaders/lighting_shader.vs", "shaders/lighting_shader.fs");
  Shader lightCubeShader("shaders/light_cube_shader.vs", "shaders/light_cube_shader.fs");
  Shader skyboxShader("shaders/skybox_shader.vs", "shaders/skybox_shader.fs");

  glm::vec3 defaultAmbientColor = glm::vec3(0.2f);
  //                                            (shader,           specular,            shininess,  diffuse,       ambient,             emissionVals,  emissionMap);
  Material *containerMaterial   = createMaterial(&lightingShader,  blankTexture,        16.0f,      container,     defaultAmbientColor, blankTexture,  blankTexture);
  Material *container2Material  = createMaterial(&lightingShader,  container2_specular, 16.0f,      container2,    defaultAmbientColor, matrixTexture, container2_emission_map);
  Material *awesomefaceMaterial = createMaterial(&lightingShader,  blankTexture,        64.0f,      awesomeface,   defaultAmbientColor, awesomeface,   awesomeface);
  Material *generic01Material   = createMaterial(&lightingShader,  blankTexture,        16.0f,      generic01,     defaultAmbientColor, blankTexture,  blankTexture);
  Material *generic02Material   = createMaterial(&lightingShader,  blankTexture,        16.0f,      generic02,     defaultAmbientColor, blankTexture,  blankTexture);
  Material *skyboxMaterial      = createMaterial(&skyboxShader,    blankTexture,        16.0f,      skyboxTexture, defaultAmbientColor, blankTexture,  blankTexture);

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

  Mesh *cubeMesh = createMesh(vertices_cube, 36, sizeof(vertices_cube), WITH_ATTRIBUTES);
  Mesh *planeMesh = createMesh(vertices_plane, 6, sizeof(vertices_plane), WITH_ATTRIBUTES);
  Mesh *skyboxMesh = createMesh(vertices_skybox, 36, sizeof(vertices_skybox), WITHOUT_ATTRIBUTES);

  for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
    glm::vec3 pos;
    // point light material is all blanks -- materials will need to be refactored later
    // also though: we need 1 material / light, because they are all unique colors, changing at their own rates!
    // corresponding frees in destroyPointLight
    Material *pointLightMaterial  = createMaterial(&lightCubeShader, blankTexture,        16.0f,      blankTexture,  defaultAmbientColor, blankTexture,  blankTexture);

    if (i < 10) {
      pos = pointLightPositions[i];
    } else {
      pos = glm::vec3((i % 11) * 0.3f, (i % 13) * 0.3f, (i % 17) * 0.6f);
    }

    pointLights[i] = createPointLight(cubeMesh, pointLightMaterial, pos);
  }

  for (int i = 0; i < numFlyingCubes; i++) {
    if (i == awesomeface_index) {
      flyingCubes[i] = createGameObject(cubeMesh, awesomefaceMaterial, cubePositions[i]);
    } else {
      flyingCubes[i] = createGameObject(cubeMesh, container2Material, cubePositions[i]);
    }
  }

  GameObject *plane = createGameObject(planeMesh, generic02Material, glm::vec3(-50.0f, -0.5f, -50.0f));
  GameObject *wall = createGameObject(cubeMesh, generic01Material, glm::vec3(0.0f));
  GameObject *skybox = createGameObject(skyboxMesh, skyboxMaterial, glm::vec3(0.0f));

  // un-comment to use wireframe mode:
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // set up textures in shader:
  skyboxShader.use();
  skyboxShader.setInt("skybox", skyboxTexture);
  lightingShader.use(); // don't forget to activate the shader before setting uniforms!
  lightingShader.setInt("skybox", skyboxTexture);

  // set up lighting -- actually, these seem unused!
  sendPointLightAttenuations(&lightingShader, pointLights, MAX_NUM_OF_LIGHTS);

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

    glm::mat4 view = glm::lookAt(cam.pos, cam.pos + cam.front, cam.up);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 1280.0f / 720.0f, 0.1f, 100.0f);

    // lights begin
    lightingShader.use(); // used for everything kinda
    int lightsUsed = (int)floor(cam.lightsUsedControl);
    lightingShader.setInt("lightsUsed", lightsUsed);
    lightingShader.setVec3f("viewPos", cam.pos.x, cam.pos.y, cam.pos.z);  // this is the "player cam pos" :/

    // updating pointLight pos+color in the lightingShader on the GPU:
    updatePointLights(pointLights, lightsUsed);
    sendPointLightColors(&lightingShader, pointLights, lightsUsed);
    sendPointLightPositions(&lightingShader, pointLights, lightsUsed);

    // updating positions of each flyingCube in cpu
    for (unsigned int i = 0; i < 10; i++) {
      float angle = 20.0f * i;

      if (i == awesomeface_index) {
        flyingCubes[i]->rot = glm::vec3(1.0f, 1.0f, 0.5f);
        flyingCubes[i]->angle = (float)glfwGetTime() * glm::radians(angle);
      } else if (i % 3 == 0) {
        flyingCubes[i]->rot = glm::vec3(1.0f, 0.3f, 0.5f);
        flyingCubes[i]->angle = (float)glfwGetTime() * glm::radians(angle);
      } else if (i % 2 == 0) {
        flyingCubes[i]->rot = glm::vec3(0.3f, 0.1f, 0.5f);
        flyingCubes[i]->angle = (float)glfwGetTime() * glm::radians(angle);
      } else if (i <= 10) {
        flyingCubes[i]->rot = glm::vec3(1.0f, 0.3f, 0.5f);
        flyingCubes[i]->angle = angle; // no change to angle
      }
    }

    renderSkybox(skybox, view, projection);
    renderWalls(wall, view, projection);
    renderGameObject(plane, view, projection);

    for (int i = 0; i < numFlyingCubes; i++) {
      renderGameObject(flyingCubes[i], view, projection);
    }

    renderPointLightCubes(&lightCubeShader, pointLights, lightsUsed, view, projection);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);

    /* Poll for and process events */
    glfwPollEvents();
  }

  for (int i = 0; i < MAX_NUM_OF_LIGHTS; i++) {
    // these materials are created one-per-light so the lights can be unique colors
    destroyMaterial(pointLights[i]->gameObject->mat);
    destroyPointLight(pointLights[i]);
  }

  for (int i = 0; i < numFlyingCubes; i++) {
    destroyGameObject(flyingCubes[i]);
  }

  free(flyingCubes);
  free(pointLights);
  destroyMaterial(containerMaterial);
  destroyMaterial(container2Material);
  destroyMaterial(awesomefaceMaterial);
  destroyMaterial(generic01Material);
  destroyMaterial(generic02Material);
  destroyMaterial(skyboxMaterial);
  destroyMesh(cubeMesh);
  destroyMesh(planeMesh);
  destroyMesh(skyboxMesh);

  glfwTerminate();
  return 0;
}
