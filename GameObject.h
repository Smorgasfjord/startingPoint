
#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__

//Don't ask me why; I had to do this to make X-code happy
#ifdef __APPLE__
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "glm/gtx/vector_query.hpp" //
#endif

#ifdef __unix__
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "glm/gtx/vector_query.hpp" //
#endif

#include "GameModel.h"
#include "Rendering/GLSL_helper.h"
#include "GLHandles.h"
#include "Modeling/CMeshLoaderSimple.h"
#include <stdlib.h>
#include <string>

using namespace std;
using namespace glm;

typedef struct trans_data Transform_t;
struct trans_data {
   vec3 pos;
   vec3 scale;
   vec3 orient;
   vec3 velocity;
   mat4 translate;
   mat4 scaling;
   mat4 rotation;
   mat4 transform;
};

void initGameObjState(Transform_t *state);
void setMaterial(int i, int ShadeProg);

class ObjectMesh {
   public:
      int meshIdx;
      MeshBufferData buffDat;

      ObjectMesh() 
      {
      }
      
      ObjectMesh(int m, MeshBufferData n) :
         meshIdx(m),
         buffDat(n)
      {
      }

      void render(GLHandles handle);
};

class ObjectNode {
   public:
      vector<ObjectMesh> meshes;
      vector<ObjectNode> children;
      string name;
      Transform_t state;

      ObjectNode()
      {
         initGameObjState(&state);
      }

      ObjectNode(string n) :
        name(n) 
      {
         initGameObjState(&state);
      }
      
      void initialize(ModelNode *modNod);
      void render(GLHandles handle, mat4 cumulative);
};

class GameObject {
   public:
      ObjectNode model;
      string name;
      SBoundingBox bounds;
      int modelIdx;
      int collisionGroup;
      int grounded;
      int gravityAffected;
      float mass;
      GLHandles handles;

      GameObject()
      {
      }

      GameObject(string n) :
         name(n)
      {
         grounded = 0;
         gravityAffected = 1;
         mass = 1.0;
         collisionGroup = 0;
      }
     
      void initialize(GameModel *gMod, int modIdx, int collGroup, GLHandles handle);
      void setPhysProps(float mass, int gravAffect);
      vec3 pos();
      vec3 vel();
      vec3 checkCollision(GameObject other);
      vec3 applyForce(vec3 force);
      vec3 addVelocity(vec3 vel);
      vec3 setVelocity(vec3 vel);
      vec3 applyTransform(mat4 tran);
      float scaleMass(float scale);
      void trans(float x, float y, float z);
      void rescale(float x, float y, float z);
      void rot(float x, float y, float z);
      void draw();
      virtual void update(double timeStep);
};

#endif
