
#ifndef __GAME_OBJECT_H__
#define __GAME_OBJECT_H__


#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "glm/gtx/vector_query.hpp" //
#include "GLSL_helper.h"
#include "mesh.h"
#include "CMeshLoaderSimple.h"
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

void crappyInitFunc(GLint a,GLint b,GLint c,GLint d,GLint e,GLint f,GLint g,GLint h,GLint i,GLint j,GLint k,int sp);
void initGameObjState(Transform_t *state);
void setMaterial(int i, int ShadeProg);

class ObjectMesh {
   public:
      Transform_t state;
      int meshIdx;
      int matIdx;
      int texIdx;
      int numFaces;
      GLuint posBuffObj;
      GLuint idxBuffObj;
      GLuint uvBuffObj;
      GLuint normBuffObj;

      ObjectMesh() 
      {
      }
      
      ObjectMesh(int m, int mat, int tex, int nf, GLuint p, GLuint i, GLuint u, GLuint n) :
         meshIdx(m),
         matIdx(mat),
         texIdx(tex),
         numFaces(nf),
         posBuffObj(p),
         idxBuffObj(i),
         uvBuffObj(u),
         normBuffObj(n)
      {
         initGameObjState(&state);
      }

      void render();
};

class ObjectModel {
   public:
      vector<ObjectMesh> meshes;
      int modelIdx;

      ObjectModel()
      {
      }

      ObjectModel(int mi) :
         modelIdx(mi)
      {
      }
};

class GameObject {
   public:
      ObjectModel model;
      string name;
      Transform_t state;
      SBoundingBox bounds;
      float mass;

      GameObject()
      {
      }

      GameObject(SBoundingBox box, float m, vec3 v, string n) :
         bounds(box),
         mass(m),
         name(n)
      {
         initGameObjState(&state);
         state.velocity = v;
      }
      
      int checkCollision(GameObject other);
      vec3 applyForce(vec3 force);
      vec3 setVelocity(vec3 vel);
      vec3 applyTransform(mat4 tran);
      float scaleMass(float scale);
      void trans(float x, float y, float z);
      void rescale(float x, float y, float z);
      void rot(float x, float y, float z);
      void draw();
      void update(double timeStep);
};

class ModelMesh {
   public:
      GLuint posBuffObj;
      GLuint idxBuffObj;
      GLuint uvBuffObj;
      GLuint normBuffObj;

      int numFaces;

      ModelMesh() 
      {
      }

      ModelMesh(GLuint p, GLuint i, GLuint u, GLuint n, int nf) :
         posBuffObj(p),
         idxBuffObj(i),
         uvBuffObj(u),
         normBuffObj(n),
         numFaces(nf)
      {
      }
};

class GameModel {
   public:
      vector<ModelMesh> meshes;
      Model model;
      int numMeshes;
      string fname;
      SBoundingBox bounds;

      GameModel()
      {
      }

      GameModel(Model m, int nm, string fn) :
         model(m),
         numMeshes(nm),
         fname(fn)
      {
      }

      void updateBounds();
};



#endif
