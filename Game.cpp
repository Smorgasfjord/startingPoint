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
#include <GL/glut.h>
#endif

#ifdef _WIN32
#pragma comment(lib, "glew32.lib")

#include <GL\glew.h>
#include <GL\glut.h>
#endif

#define MAX_XPOS 2.0
#define MAX_YPOS 2.0
#define MIN_XPOS -2.0
#define MIN_YPOS -2.0

#define MAX_SCALE 3.0
#define MIN_SCALE 0.25

#define LIGHTIDX 0

#define PI 3.14159265

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
#include "Modeling/CMesh.h"
#include "Engine/chunks.h"
#include "GameObject.h"

using namespace glm;
using namespace std;

/*data structure for the image used for  texture mapping */
typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

Image *TextureImage;

typedef struct RGB {
  GLubyte r;
  GLubyte g;
  GLubyte b;
} RGB;

RGB myimage[64][64];
RGB* g_pixel;

//forward declaration of image loading and texture set-up code
int ImageLoad(char *filename, Image *image);
GLvoid LoadTexture(char* image_file, int tex_id);

void transObj(int meshIdx, float x, float y, float z);
void scaleObj(int meshIdx, float x, float y, float z);
void rotateObj(int meshIdx, float x, float y, float z);

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

int g_mat_id =1;
int shadeMode=1;

vec3 g_trans(0);
mat4 microScale = scale(mat4(1.0f), vec3(0.1,0.1,0.1));
vec3 eyePos = vec3(0.0,0.0,-g_Camtrans);
vec3 lookAtPoint;// = eyePos + vec3(0.0,0.0,-1.0);
vec3 upVec = vec3(0.0,1.0,0.0);

static const float g_groundY = -.51;      // y coordinate of the ground
static const float g_groundSize = 50.0;   // half the ground length

GLint h_uLightPos;
GLint h_uLightColor;
GLint h_uCamPos;
GLint h_uInvTrans;
GLint h_uMatAmb, h_uMatDif, h_uMatSpec, h_uMatShine;
GLint h_uMode, h_aUV, h_uTexUnit;

//vector<ArgContainer> stackArgs;

//Handles to the shader data
GLint h_aPosition;
GLint h_aColor;
GLint h_aNormal;
GLint h_uModelMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;

vec3 rotStart;
mat4 trackBall;
int collisions, PLAYER, HAMMER, GROUND;
float friction = 30.0f;

//every object has one of these -- size() = number of objects
vector<GameObject> Objects; //name

//every Model has one of these -- size() = number of models
vector<GameModel> Models;

ChunkWorld worl;

//routines to load in a bmp files - must be 2^nx2^m and a 24bit bmp
GLvoid LoadTexture(char* image_file, int texID) {

  TextureImage = (Image *) malloc(sizeof(Image));
  if (TextureImage == NULL) {
    printf("Error allocating space for image");
    exit(1);
  }
  cout << "trying to load " << image_file << endl;
  if (!ImageLoad(image_file, TextureImage)) {
    exit(1);
  }
  /*  2d texture, level of detail 0 (normal), 3 components (red, green, blue),            */
  /*  x size from image, y size from image,                                              */
  /*  border 0 (normal), rgb color data, unsigned byte data, data  */
  glBindTexture(GL_TEXTURE_2D, texID);
  glTexImage2D(GL_TEXTURE_2D, 0, 3,
    TextureImage->sizeX, TextureImage->sizeY,
    0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); /*  cheap scaling when image bigg*/
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); /*  cheap scaling when image smal*/

}

/* BMP file loader loads a 24-bit bmp file only */

/*
* getint and getshort are help functions to load the bitmap byte by byte
*/
static unsigned int getint(FILE *fp) {
  int c, c1, c2, c3;

  /*  get 4 bytes */
  c = getc(fp);
  c1 = getc(fp);
  c2 = getc(fp);
  c3 = getc(fp);

  return ((unsigned int) c) +
    (((unsigned int) c1) << 8) +
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
  int c, c1;

  /* get 2 bytes*/
  c = getc(fp);
  c1 = getc(fp);

  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/*  quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  */

int ImageLoad(char *filename, Image *image) {
  FILE *file;
  unsigned long size;                 /*  size of the image in bytes. */
  unsigned long i;                    /*  standard counter. */
  unsigned short int planes;          /*  number of planes in image (must be 1)  */
  unsigned short int bpp;             /*  number of bits per pixel (must be 24) */
  char temp;                          /*  used to convert bgr to rgb color. */

  /*  make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }

  /*  seek through the bmp header, up to the width height: */
  fseek(file, 18, SEEK_CUR);

  /*  No 100% errorchecking anymore!!! */

  /*  read the width */    image->sizeX = getint (file);

  /*  read the height */
  image->sizeY = getint (file);

  /*  calculate the size (assuming 24 bits or 3 bytes per pixel). */
  size = image->sizeX * image->sizeY * 3;

  /*  read the planes */
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }

  /*  read the bpp */
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }

  /*  seek past the rest of the bitmap header. */
  fseek(file, 24, SEEK_CUR);

  /*  read the data.  */
  image->data = (char *) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0;
  }

  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }

  for (i=0;i<size;i+=3) { /*  reverse all of the colors. (bgr -> rgb) */
    temp = image->data[i];
    image->data[i] = image->data[i+2];
    image->data[i+2] = temp;
  }

  fclose(file); /* Close the file and release the filedes */

  /*  we're done. */
  return 1;
}

static float CubePos[] = {
   -0.5, -0.5, -0.5,  //lower back left
   -0.5, 0.5, -0.5,   //upper back left
   0.5, 0.5, -0.5,    //upper back right
   0.5, -0.5, -0.5,   //lower back right
   -0.5, -0.5, 0.5,   //lower front left
   -0.5, 0.5, 0.5,    //upper front left
   0.5, 0.5, 0.5,     //upper front right
   0.5, -0.5, 0.5     //lower front right
};

float rootThree = sqrt(3.0);
static float CubeNorm[] = {
   -rootThree, -rootThree, -rootThree,  //lower back left
   -rootThree, rootThree, -rootThree,   //upper back left
   rootThree, rootThree, -rootThree,    //upper back right
   rootThree, -rootThree, -rootThree,   //lower back right
   -rootThree, -rootThree, rootThree,   //lower front left
   -rootThree, rootThree, rootThree,    //upper front left
   rootThree, rootThree, rootThree,     //upper front right
   rootThree, -rootThree, rootThree     //lower front right
};

static float CubeUV[] = {
   0.1, 0.1,
   0.1, 0.2,
   0.2, 0.1,
   0.2, 0.2,
   0.2, 0.3,
   0.3, 0.2,
   0.3, 0.3,
   0.4, 0.4
};

static unsigned int cubeIdx[] =
   {0, 1, 2, 0, 2, 3, 7, 6, 4, 4, 6, 5, 1, 5, 6, 1, 6, 2, 0, 3, 7, 0, 7, 4};

static void InitCube() {
   GameModel mod;
   ModelMesh mesh;
   mod = GameModel(Model(), 1, "cuben");
   mesh = ModelMesh(0,0,0,0,sizeof(cubeIdx)/sizeof(float));

   glGenBuffers(1, &mesh.posBuffObj);
   glGenBuffers(1, &mesh.normBuffObj);
   glGenBuffers(1, &mesh.uvBuffObj);
   glGenBuffers(1, &mesh.idxBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mesh.posBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubePos), CubePos, GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, mesh.normBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubeNorm), CubeNorm, GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(CubeUV), CubeUV, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.idxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIdx), cubeIdx, GL_STATIC_DRAW);
   mod.bounds = SBoundingBox(-0.5, -0.5, -0.5);
   mod.bounds.update(0.5, 0.5, 0.5);
   mod.meshes.push_back(mesh);
   Models.push_back(mod);   
}


static void InitGround() {

  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };

    float GrndUV[] = {
       0.0, 0.0,
       1.0, 0.0,
       1.0, 1.0,
       0.0, 1.0
    };

    unsigned int idx[] = {0, 1, 2, 0, 2, 3};
   GameModel mod;
   ModelMesh mesh;
   mod = GameModel(Model(), 1, "grond");
   mesh = ModelMesh(0,0,0,0,sizeof(idx)/sizeof(float));

   glGenBuffers(1, &mesh.posBuffObj);
   glGenBuffers(1, &mesh.normBuffObj);
   glGenBuffers(1, &mesh.uvBuffObj);
   glGenBuffers(1, &mesh.idxBuffObj);
   glBindBuffer(GL_ARRAY_BUFFER, mesh.posBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, mesh.normBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

   glBindBuffer(GL_ARRAY_BUFFER, mesh.uvBuffObj);
   glBufferData(GL_ARRAY_BUFFER, sizeof(GrndUV), GrndUV, GL_STATIC_DRAW);

   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.idxBuffObj);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

   mod.bounds = SBoundingBox(-g_groundSize, -0.001, -g_groundSize);
   mod.bounds.update(g_groundSize, 0.001, g_groundSize);
   mod.meshes.push_back(mesh);
   Models.push_back(mod);   
}

int InitObj(int model, int material, int tex, string name, float mass, int cG) {
   GameObject obj;
   GameModel *mod = &Models[model];
   ModelMesh *temp;
   obj = GameObject(mod->bounds, mass, vec3(0.0), name, cG);
   obj.model = ObjectModel(model);
   for (int i = 0; i < mod->numMeshes; i++) {
      obj.model.meshes.push_back(ObjectMesh(i,material,tex,mod->meshes[i].numFaces,mod->meshes[i].posBuffObj,
               mod->meshes[i].idxBuffObj,mod->meshes[i].uvBuffObj,mod->meshes[i].normBuffObj));
   }
   Objects.push_back(obj);
   return Objects.size() - 1;
}

/*void MakeBjorn() {
   LoadModel("Models/cylinder.obj");
   LoadModel("Models/sphere.obj");

}*/

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
}

void LoadModel(string fName) {
   Model moddl;
   int vCount(0), mIdx(0);
   GameModel mod;
   GLuint idx, pos, uv, norm;
   SBoundingBox box = SBoundingBox(0.0,0.0,0.0);
   Face temp;
   ifstream modelFile(fName.c_str());
   moddl.load(modelFile);
   std::cout << "Loading: " << fName << "\n";
   mod = GameModel(moddl, moddl.meshes().size(), fName);
   for (int i = 0; i < moddl.meshes().size(); i++) {
      std::cout << " Loading Mesh " << i << " - " << moddl.meshes()[i].name() << "\n";
      vCount = moddl.meshes()[i].makeVBO(&idx, &pos, &uv, &norm);
      mod.meshes.push_back(ModelMesh(pos,idx,uv,norm,vCount));
      for (int j = 0; j < moddl.meshes()[i].faces().size(); j++) {
         temp = moddl.meshes()[i].faces()[j];
         for (int k = 0; k < 3; k++) {
            box.update(moddl.verts()[temp.V[k].mVertex].x,
                  moddl.verts()[temp.V[k].mVertex].y,
                  moddl.verts()[temp.V[k].mVertex].z);
         }
      }
   }
   mod.bounds = box;
   Models.push_back(mod);
}

/* initialize the geomtry (including color)
   Change file name to load a different mesh file
*/
void InitGeom() {
   InitCube();
   InitGround();
   PLAYER = InitObj(0,2,0,"player",1.0,0);
   InitObj(0,3,0,"light",0.0,0);
   Objects[1].gravityAffected = 0;
   GROUND = InitObj(1,0,0,"ground",0.0,2);
   Objects[GROUND].gravityAffected = 0;
   LoadMesh("Models/bunny500.m");
   LoadMesh("Models/tyra_1k.m");
   InitObj(0,1,0,"thing",5.0,1);
   HAMMER = InitObj(3,1,0,"hommer",5.0,0);
   
   //Positions[3] = vec3(0,0,25.0f);
   //rotateObj(3,0,-90.0f,0.0f);
   lightx = lighty = 10.0f;
   scaleObj(5,0.5,0.5,0.5f);
   scaleObj(0,1.0,2.0,1.0f);
   rotateObj(4,90.0f,0.0f,90.0f);
   //transObj(0,eyePos.x,eyePos.y,eyePos.z);
   lookAtPoint = Objects[0].state.pos;
   transObj(1,lightx,lighty,0.0);
   transObj(5,0.0,5.5,0.0f);
   transObj(PLAYER,0.0,2.5,0.0f);
   Objects[5].setVelocity(vec3(0.5,0.5,0.5));
}

/* projection matrix */
void SetProjectionMatrix() {
  mat4 Projection = perspective(90.0f, (float)g_width/g_height, 0.1f, 100.f);
  safe_glUniformMatrix4fv(h_uProjMatrix, value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
  mat4 view = lookAt(eyePos, lookAtPoint, upVec);
  safe_glUniformMatrix4fv(h_uViewMatrix, value_ptr(view));
}

/* model transforms */
void SetModel(int tIdx) {
  mat4 Trans = translate( mat4(1.0f), g_trans);
  mat4 norm = translate( mat4(1.0f), g_trans);
  if (tIdx >= 0) {
      safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr((Objects[tIdx].state.transform)));
      norm = transpose(inverse(Objects[tIdx].state.transform));
      safe_glUniformMatrix4fv(h_uInvTrans, value_ptr(norm));
   }
   else {
      safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr(Trans));
      safe_glUniformMatrix4fv(h_uInvTrans, value_ptr(norm));
   }
}

/* set the model transform to the identity */
void SetModelI() {
  mat4 tmp = mat4(1.0f);
  safe_glUniformMatrix4fv(h_uModelMatrix, value_ptr(tmp));
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

   VS = glCreateShader(GL_VERTEX_SHADER);
   FS = glCreateShader(GL_FRAGMENT_SHADER);

   //load the source
   glShaderSource(VS, 1, &vShaderName, NULL);
   glShaderSource(FS, 1, &fShaderName, NULL);

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

   glLinkProgram(ShadeProg);
   /* check shader status requires helper functions */
   printOpenGLError();
   glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
   printProgramInfoLog(ShadeProg);

   glUseProgram(ShadeProg);

   /* get handles to attribute data */
   h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
   h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal");
   h_aColor = safe_glGetAttribLocation(ShadeProg, "aClr");
   h_aUV = safe_glGetAttribLocation(ShadeProg, "aUV");
   h_uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
   h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
   h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
   h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
   h_uInvTrans = safe_glGetUniformLocation(ShadeProg, "uInverseTranspose");
   h_uLightPos = safe_glGetUniformLocation(ShadeProg, "uLightPos");
   h_uLightColor = safe_glGetUniformLocation(ShadeProg, "uLColor");
   h_uCamPos = safe_glGetUniformLocation(ShadeProg, "uCamPos");
   h_uMode = safe_glGetUniformLocation(ShadeProg, "uMode");
   h_uMatAmb = safe_glGetUniformLocation(ShadeProg, "uMat.aColor");
   h_uMatDif = safe_glGetUniformLocation(ShadeProg, "uMat.dColor");
   h_uMatSpec = safe_glGetUniformLocation(ShadeProg, "uMat.sColor");
   h_uMatShine = safe_glGetUniformLocation(ShadeProg, "uMat.shine");
   
   crappyInitFunc(h_uInvTrans,h_uMatAmb,h_uMatDif,h_uMatSpec,h_uMatShine,
         h_aUV,h_uTexUnit,h_aPosition,h_aColor,h_aNormal,h_uModelMatrix,ShadeProg);
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
}

void Update(double timeStep) {
   vec3 min, max, force;
   map<int,int> dels;
   Objects[0].state.velocity *= exp(-friction * timeStep);
   for (int i = 0; i < Objects.size(); i++) {
      if (length(Objects[i].state.velocity) > 0.0001) {
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
                  Objects[i].state.velocity -= force;
               }
               if (j == GROUND) {
                  Objects[i].grounded = 1;
               }
               break;
            }
         }
      }
   }
   for (int i = Objects.size() - 1; i >= 0; i--) {
      if (dels.count(i)) {
         Objects.erase(Objects.begin() + i);
      }
   }
}

/* Main display function */
void Draw (void)
{
   int meshCount = 0;
   glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   //Start our shader   
   glUseProgram(ShadeProg);

   /* only set the projection and view matrix once */
   SetProjectionMatrix();
   SetView();
   //SetModel(-1);

   safe_glUniform1i(h_uMode, shadeMode);
   safe_glUniform3f(h_uLightPos, lightx,lighty,lightz);
   //Objects[LIGHTIDX].state.transform = translate(mat4(1.0f), vec3(lightx,lighty,lightz))*microScale;
   safe_glUniform3f(h_uLightColor, 1.0,1.0,1.0);
   safe_glUniform3f(h_uCamPos, eyePos.x,eyePos.y,eyePos.z);

    //set up the texture unit
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    safe_glUniform1i(h_uTexUnit, 0);

    //printf("%d\n", Objects.size());
      //data set up to access the vertices and color
   for (int i = 0; i < Objects.size(); i++) {
      Objects[i].draw();
   }
   
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
         temp = Objects[0].state.velocity;
      }
      Objects[0].setVelocity(temp*vec3(1.0,0.0,1.0));
      lookAtPoint = Objects[0].state.pos;
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
    Initialize();

   //LoadTexture((char *)"Models/Dargon.bmp", 0);

   //test the openGL version
   getGLversion();
   //install the shader
   if (!InstallShader(textFileRead((char *)"Rendering/Lab1_vert.glsl"), textFileRead((char *)"Rendering/Lab1_frag.glsl"),0)) {
      printf("Error installing shader!\n");
      return 0;
   }
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
      if (inc < (int)timey) {
         obj = InitObj(2,0,0,"spawm",50.0, 1);
         transObj(obj,(float)(inc%7),0.0,(float)(inc%5));
         scaleObj(obj,0.05,0.05,0.05);
         Objects[obj].setVelocity(vec3((lastTime - inc)*-((float)(inc%4)-1.5), 0.0, (timey - inc)*((float)(inc%8)-3.5)));
      }

      mousePos = Objects[0].state.pos + vec3(currPos[0], currPos[1], 0.0) * -g_Camtrans;
      move = mousePos - Objects[HAMMER].state.pos;
      Objects[HAMMER].setVelocity(move*(float)(1.0/((timey-lastTime)*2.0)));
      Update((timey-lastTime)*2.0);
      lastTime = glfwGetTime();
      glViewport(0, 0, width, height);
      lookAtPoint = Objects[0].state.pos;
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

