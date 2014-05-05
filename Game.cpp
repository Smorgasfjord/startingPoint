/*
  Base code for program 3 for CSC 471
  OpenGL, glut and GLSL application
  Starts to loads in a .m mesh file 
  ADD: storing data into a VBO and drawing it
  Uses glm for matrix transforms
  I. Dunn and Z. Wood  (original .m loader by H. Hoppe)
*/

#include <iostream>

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif

#ifdef __unix__
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")

#include <GL\glew.h>
#include <GL\glut.h>
#endif

#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <string>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "glm/gtx/vector_query.hpp" //
#include "GameObject.h"
#include "GameModel.h"
#include "Engine/chunks.h"
#include "Modeling/CMesh.h"
#include "Platform.h"
#include "Mountain.h"
#include "oldTextureLoader.h"


using namespace glm;
using namespace std;

void transObj(int meshIdx, float x, float y, float z);
void scaleObj(int meshIdx, float x, float y, float z);
void rotateObj(int meshIdx, float x, float y, float z);



// Uniform Bindings Points
 GLuint matricesUniLoc = 1, materialUniLoc = 2;
//
// // The sampler uniform for textured models
// // we are assuming a single texture so this will
// //always be texture unit 0
 GLuint texUnit = 0;
//

//flag and ID to toggle on and off the shader
static float downPos[2], upPos[2], currPos[2], lastPos[2];
int shade = 0;
int ShadeProg;
static int selection;
static int progState;
static float g_width, g_height;
static float phi = 0, theta = 0;
float lightx, lighty, lightz;


float g_Camtrans = -6.0;
vec3 wBar;
vec3 uBar;
vec3 vBar;
static float g_scale = 1;

GLHandles handles;

int g_mat_id =1;
int shadeMode=1;

vec3 g_trans(0);
mat4 microScale = scale(mat4(1.0f), vec3(0.1,0.1,0.1));
vec3 eyePos = vec3(0.0,0.0,-g_Camtrans);
vec3 lookAtPoint;// = eyePos + vec3(0.0,0.0,-1.0);
vec3 upVec = vec3(0.0,1.0,0.0);

static const float g_groundY = -.51;      // y coordinate of the ground
static const float g_groundSize = 50.0;   // half the ground length

int collisions, PLAYER, HAMMER, GROUND;
float friction = 30.0f;

//Platform
GameModel platMod;
//vector<Platform> platforms;
//Platform placing;

//Mountain
GameModel mountMod;
//Mountain mount;

//every object has one of these -- size() = number of objects
vector<GameObject> Objects; //name

//every Model has one of these -- size() = number of models
vector<GameModel> Models;

ChunkWorld worl;

int InitObj(int model, int material, int tex, string name, float mass, int cG) {
   GameObject obj;
   obj = GameObject(name);
   obj.initialize(&Models[model],model,cG,handles);
   obj.setPhysProps(mass,0);
   Objects.push_back(obj);
   return Objects.size() - 1;
}

//Generates a random float within the range min-max
float randomFloat(float min, float max)
{
   return (max - min) * (rand() / (double) RAND_MAX) + min;
}


//Read in a .lvl file of the given name
/*vector<Platform> importLevel(std::string const & fileName)
{
   vector<Platform> plats;
   std::ifstream File;
   File.open(fileName.c_str());

   if (! File.is_open())
   {
      std::cerr << "Unable to open Level file: " << fileName << std::endl;
   }

   while (File)
   {
      string ReadString;
      string pos, size, rot;
      glm::vec3 position, sizeVec;
      float rotation;
      getline(File, ReadString);
      std::stringstream Stream(ReadString);

      //Platform data
      if(ReadString.find("Platform:") != std::string::npos)
      {
         getline(File, pos);
         getline(File, size);
         getline(File, rot);
         sscanf(pos.c_str(), "\t%f %f %f\n", &(position.x), &(position.y), &(position.z));
         sscanf(size.c_str(), "\t%f %f %f\n", &(sizeVec.x), &(sizeVec.y), &(sizeVec.z));
         sscanf(rot.c_str(), "\t%f\n", &rotation);
         plats.push_back(Platform(position, sizeVec, rotation, handles, platMod));
      }
   }
   File.close();
   return plats;
}

//Export the current platform layout to a .lvl file
void exportLevel()
{
   string fileName = "mountain.lvl";
   ofstream myfile;


   myfile.open ((char*)fileName.c_str());
   for (std::vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
      myfile << it->toString();
   }
   myfile.close();

}*/

/* Initialization of objects in the world */
/*void setWorld()
{
   //Add mountain
   mount = Mountain(glm::vec3(g_groundSize / 2, 0, g_groundSize / 2), handles, mountMod);

   //Read in a level if one is available, we probably want to make
   //this a more complex system of loading/saving
   platforms = importLevel("mountain.lvl");
}*/


/*void MakeBjorn() {
   LoadModel("Models/cylinder.obj");
   LoadModel("Models/sphere.obj");
   
}*/
/*
void LoadMesh(string fName) {
   CMesh mesh;
   GameModel mod;
   ModelMesh mmesh;
   float max=0.0;
   loadASCIIMesh(fName,&mesh,SColor());
   
   mod = GameModel(Model(), 1, fName);
   CMeshLoader::loadVertexBufferObjectFromMesh(fName, mmesh.numFaces, 
         mmesh.posBuffObj, mmesh.idxBuffObj, mmesh.normBuffObj, mmesh.uvBuffObj);
   mesh.centerMeshByExtents(SVector3(0.0));
   for (int i = 0; i < mesh.Vertices.size(); i++) {
      if (max < fabsf(mesh.Vertices[i].pos.X)) max = fabsf(mesh.Vertices[i].pos.X);
      if (max < fabsf(mesh.Vertices[i].pos.Y)) max = fabsf(mesh.Vertices[i].pos.Y);
      if (max < fabsf(mesh.Vertices[i].pos.Z)) max = fabsf(mesh.Vertices[i].pos.Z);
   }
   mmesh.numFaces *=3;
   mesh.resizeMesh(SVector3(1.0/max, 1.0/max, 1.0/max));
   mesh.GetBounds();
   mod.bounds = mesh.Bounds;
   mod.meshes.push_back(mmesh);
   Models.push_back(mod);
}*/

void LoadModel(string fName) {
   GameModel mod;
   mod = loadModel(fName, handles);
   Models.push_back(mod);

}

/* initialize the geomtry (including color)
   Change file name to load a different mesh file
*/
void InitGeom() {
   LoadModel("models/Orange.dae");
   LoadModel("models/bjorn_hammer.dae");
   PLAYER = InitObj(0,2,0,"player",1.0,0);
   InitObj(0,2,0,"moop",1.0,0);
   //GROUND = InitObj(1,0,0,"ground",0.0,2);
   //Objects[GROUND].gravityAffected = 0;
   //InitObj(0,1,0,"thing",5.0,1);
   HAMMER = InitObj(1,1,0,"hommer",5.0,0);
   
   //Positions[3] = vec3(0,0,25.0f);
   //rotateObj(3,0,-90.0f,0.0f);
   //lightx = lighty = 10.0f;
   //scaleObj(5,0.5,0.5,0.5f);
   //scaleObj(0,50.0,50.0,50.0f);
   //rotateObj(4,90.0f,0.0f,90.0f);
   //transObj(0,eyePos.x,eyePos.y,eyePos.z);
   //transObj(PLAYER,0.0,2.5,0.0f);
   lookAtPoint = Objects[0].pos();
   eyePos = lookAtPoint + vec3(0.0, 0.0, -g_Camtrans);
   //transObj(1,lightx,lighty,0.0);
   //transObj(5,0.0,5.5,0.0f);
   //Objects[5].setVelocity(vec3(0.5,0.5,0.5));
}

/* projection matrix */
void SetProjectionMatrix() {
   mat4 Projection = perspective(90.0f, (float)g_width/g_height, 0.1f, 100.f);
   glBindBuffer(GL_UNIFORM_BUFFER, handles.uMatricesBuff);
   glBufferSubData(GL_UNIFORM_BUFFER, ProjMatrixOffset, MatrixSize, value_ptr(Projection));
   glBindBuffer(GL_UNIFORM_BUFFER,0);
}

/* camera controls - do not change */
void SetView() {
   mat4 view = lookAt(eyePos, lookAtPoint, upVec);
   glBindBuffer(GL_UNIFORM_BUFFER, handles.uMatricesBuff);
   glBufferSubData(GL_UNIFORM_BUFFER, ViewMatrixOffset, MatrixSize, value_ptr(view));
   glBindBuffer(GL_UNIFORM_BUFFER,0);
}

/* set the model transform to the identity */
void SetModelI() {
  mat4 tmp = mat4(1.0f);
   glBindBuffer(GL_UNIFORM_BUFFER, handles.uMatricesBuff);
   glBufferSubData(GL_UNIFORM_BUFFER, ModelMatrixOffset, MatrixSize, value_ptr(tmp));
   glBindBuffer(GL_UNIFORM_BUFFER,0);
}

void transObj(int meshIdx, float x, float y, float z) {
   Objects[meshIdx].trans(x,y,z);
}

void rotateObj(int meshIdx, float x, float y, float z) {
   Objects[meshIdx].rot(x,y,z);
}

void scaleObj(int meshIdx, float x, float y, float z) {
   Objects[meshIdx].rescale(x,y,z);
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName, int progIdx) {
   GLuint VS; //handles to shader object
   GLuint FS; //handles to frag shader object
   GLint vCompiled, fCompiled, linked; //status of shader
   
   cout << "Installing\n";
   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);

   cout << "Created\n";
   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);

   cout << "Linked Source\n";
   //compile shader and print log
   glCompileShader(VS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
   printShaderInfoLog(VS);

   //compile shader and print log
   glCompileShader(FS);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
   printShaderInfoLog(FS);

   if (!vCompiled || !fCompiled) {
       printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
       return 0;
   }

   //create a program object and attach the compiled shader
   ShadeProg = glCreateProgram();
   glAttachShader(ShadeProg, VS);
   glAttachShader(ShadeProg, FS);
   cout << "Attached\n";

   glBindFragDataLocation(ShadeProg, 0, "owtput");

   glBindAttribLocation(ShadeProg,(GLuint)0,"aPosition");
   glBindAttribLocation(ShadeProg,1,"aNormal");
   glBindAttribLocation(ShadeProg,2,"aUV");

   glLinkProgram(ShadeProg);
   glValidateProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);

   handles = GLHandles();
   handles.ShadeProg = ShadeProg;
   /* get handles to attribute data */
   handles.aPosition = (GLuint)0;//safe_glGetAttribLocation(ShadeProg, "aPosition");
   handles.aNormal = (GLuint)1;//safe_glGetAttribLocation(ShadeProg, "aNormal");
   handles.aUV = (GLuint)2;//safe_glGetAttribLocation(ShadeProg, "aUV");
   handles.uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
   handles.uMatrices = glGetUniformBlockIndex(ShadeProg, "uMatrices");
   handles.uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
   handles.uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   handles.uEyePos = safe_glGetUniformLocation(ShadeProg, "uCamPos");
   handles.uMat = glGetUniformBlockIndex(ShadeProg, "uMat");
   
   glUniformBlockBinding(ShadeProg, handles.uMatrices, matricesUniLoc);
   glUniformBlockBinding(ShadeProg, handles.uMat, materialUniLoc);
   handles.print();
   printf("sucessfully installed shader %d\n", ShadeProg);
   return 1;
}

/* Some OpenGL initialization */
void Initialize ()               // Any GL Init Code 
{
   // Start Of User Initialization
   glClearColor (0.9f, 1.0f, 0.9f, 1.0f);
   // Black Background
   glClearDepth (1.0f); // Depth Buffer Setup
   glDepthFunc (GL_LEQUAL);   // The Type Of Depth Testing
   glEnable (GL_DEPTH_TEST);// Enable Depth Testing
       /* texture specific settings */
   glEnable(GL_TEXTURE_2D);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glGenBuffers(1,&handles.uMatricesBuff);
   glBindBuffer(GL_UNIFORM_BUFFER, handles.uMatricesBuff);
   glBufferData(GL_UNIFORM_BUFFER, MatricesUniBufferSize,NULL,GL_DYNAMIC_DRAW);
   glBindBufferRange(GL_UNIFORM_BUFFER, matricesUniLoc, handles.uMatricesBuff, 0, MatricesUniBufferSize);
   glBindBuffer(GL_UNIFORM_BUFFER,0);
}

void Update(double timeStep) {
   vec3 min, max, force;
   Objects[0].model.state.velocity *= exp(-friction * timeStep);
   for (int i = 0; i < Objects.size(); i++) {
      if (length(Objects[i].vel()) > 0.0001) {
         Objects[i].update(timeStep);
         Objects[i].grounded = 0;
         for (int j = 0; j < Objects.size(); j++) {
            if (j != i && Objects[i].collisionGroup != Objects[j].collisionGroup && length(force = Objects[i].checkCollision(Objects[j])) > 0.0001) {
               if (i == HAMMER) {
                  printf("%s collides with %s\n", Objects[i].name.c_str(), Objects[j].name.c_str());
                  Objects[PLAYER].applyForce(-force * Objects[HAMMER].mass * (float)timeStep * 10.0f);
                  collisions++;
               }
               else {
                  Objects[i].update(-timeStep);
                  Objects[i].addVelocity(-force);
               }
               if (j == GROUND) {
                  Objects[i].grounded = 1;
               }
               break;
            }
         }
      }
   }
}

/* Main display function */
void Draw (void)
{
   int meshCount = 0;
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Start our shader   
   glUseProgram(handles.ShadeProg);

   /* only set the projection and view matrix once */
   SetProjectionMatrix();
   SetView();
   //SetModel(-1);

   safe_glUniform3f(handles.uLightPos, lightx,lighty,lightz);
   //Objects[LIGHTIDX].state.transform = translate(mat4(1.0f), vec3(lightx,lighty,lightz))*microScale;
   safe_glUniform3f(handles.uLightColor, 1.0,1.0,1.0);
   safe_glUniform3f(handles.uEyePos, eyePos.x,eyePos.y,eyePos.z);

    //set up the texture unit
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    safe_glUniform1i(handles.uTexUnit, 0);

    //printf("%d\n", Objects.size());
      //data set up to access the vertices and color
   for (int i = 0; i < Objects.size(); i++) {
      Objects[i].draw();
   }
   
   //mount.draw();

   /*glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
   if(placeMode)
   {
      SetMaterial(2);
      placing.draw();
   }*/

   /*for (vector<Platform>::iterator it = platforms.begin(); it != platforms.end(); ++ it) {
      it->draw();
   }*/

   //disable the shader
   glUseProgram(0);
   glDisable(GL_TEXTURE_2D);
}

/* Reshape */
void ReshapeGL (GLFWwindow * window, int width, int height) {
        g_width = (float)width;
        g_height = (float)height;
        glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));
}


float p2wx(double in_x) {
  if (g_width > g_height) {
     return g_width / g_height * (2.0 * in_x / g_width - 1.0);
  }
  else {
     return 2.0 * in_x / g_width - 1.0;
  }
}

float p2wy(double in_y) {
  //flip glut y
  in_y = g_height - in_y;
  if (g_width < g_height) {
     return g_height / g_width * (2.0 * in_y / g_height - 1.0);
  }
  else {
     return 2.0 * in_y / g_height - 1.0;
  }
}

int w2px(float in_x) {
   if (g_width > g_height) {
      return (in_x*g_height/g_width+1.0)*g_width/2.0;
   }
   else {
      return (in_x+1.0)*g_width/2.0;
   }
}

int w2py(float in_y) {
   if (g_width < g_height) {
      return g_height - (in_y*g_width/g_height+1.0)*g_height/2.0;
   }
   else {
      return g_height - (in_y+1.0)*g_height/2.0;
   }
}


//the keyboard callback to change the values to the transforms
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
   float speed = 10.0;
   vec3 temp;
   if (action == GLFW_PRESS || action == GLFW_REPEAT) {
      switch( key ) {
       /* WASD keyes effect view/camera transform */
       //case GLFW_KEY_W:
       //  temp = wBar*speed;
       //  break;
       //case GLFW_KEY_S:
       //  temp = wBar*-speed;
       //  break;
       case GLFW_KEY_A:
         temp = uBar*speed;
         break;
       case GLFW_KEY_D:
         temp = uBar*-speed;
         break;
       case GLFW_KEY_N:
         shadeMode = (shadeMode + 1) % 2;
         break;
       case GLFW_KEY_Q: case GLFW_KEY_ESCAPE :
         glfwSetWindowShouldClose(window, GL_TRUE);
         return;
       default:
         temp = Objects[0].vel();
      }
      Objects[0].setVelocity(temp*vec3(1.0,1.0,1.0));
      lookAtPoint = Objects[0].pos();
      eyePos = lookAtPoint + wBar * g_Camtrans;

      wBar = normalize(lookAtPoint-eyePos);
      uBar = normalize(cross(upVec,wBar));
      vBar = cross(wBar,uBar);
   }
}

void Martian(GLFWwindow *window, double x, double y) {

   currPos[0] = p2wx(x);
   currPos[1] = p2wy(y);


   //glfwSetCursorPos(window, (double)g_width/2, (double)g_height/2);
}

static void error_callback(int error, const char* description) {
   fprintf(stderr, "%s", description);
}

int main( int argc, char *argv[] ) {
   GLFWwindow* window;
   int width, height, inc, obj;
   char title[256];
   char texture[] = "Models/Dargon.bmp";
 
   worl = ChunkWorld(50,50,50);

   glfwSetErrorCallback(error_callback);

   if (!glfwInit())
      exit(EXIT_FAILURE);
   g_width = 640;
   g_height = 480;
   window = glfwCreateWindow(g_width, g_height, "Collision!", NULL, NULL);

   if (!window) {
      glfwTerminate();
      exit(EXIT_FAILURE);
   }
   glfwMakeContextCurrent(window);
   
   glewExperimental = GL_TRUE;
   GLenum err = glewInit();
   if (GLEW_OK != err)
   {
        /* Problem: glewInit failed, something is seriously wrong. */
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
   }
   //LoadTexture(texture,0);
   //install the shader
   if (!InstallShader(textFileRead((char *)"Rendering/Lab1_vert.glsl"), textFileRead((char *)"Rendering/Lab1_frag.glsl"),0)) {
      printf("Error installing shader!\n");
      return 0;
   }
   Initialize();
   InitGeom();
   glfwSetKeyCallback(window, key_callback);
   glfwSetCursorPosCallback(window, Martian);
   //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
   glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_FALSE);
   glfwSetFramebufferSizeCallback(window, ReshapeGL);
   wBar = normalize(lookAtPoint-eyePos);
   uBar = normalize(cross(upVec,wBar));
   vBar = cross(wBar,uBar);
   while (!glfwWindowShouldClose(window)) {
      double timey = glfwGetTime(), lastTime = glfwGetTime();
      vec3 mousePos, move;
      sprintf(title, "%i %s", collisions, "Collisions!"); 
      glfwSetWindowTitle(window, title);
      glfwGetFramebufferSize(window, &width, &height);
      inc = (int)timey;
      timey = glfwGetTime();
      mousePos = Objects[0].pos() + vec3(currPos[0], currPos[1], 0.0) * -g_Camtrans;
      move = mousePos - Objects[HAMMER].pos();
      Objects[HAMMER].setVelocity(move*(float)(1.0/((timey-lastTime)*2.0)));
      Update((timey-lastTime)*2.0);
      lastTime = glfwGetTime();
      glViewport(0, 0, width, height);
      lookAtPoint = Objects[0].pos();
      eyePos = lookAtPoint + wBar * g_Camtrans;
      wBar = normalize(lookAtPoint-eyePos);
      uBar = normalize(cross(upVec,wBar));
      vBar = cross(wBar,uBar);
      Draw();
      glfwSwapBuffers(window);
      glfwPollEvents();
   }
   glfwDestroyWindow(window);
   glfwTerminate();
   exit(EXIT_SUCCESS);
}

