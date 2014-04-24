
#include "GameObject.h"

using namespace glm;

GLint __h_uInvTrans;
GLint __h_uMatAmb, __h_uMatDif, __h_uMatSpec, __h_uMatShine;
GLint __h_aUV, __h_uTexUnit;

//Handles to the shader data
GLint __h_aPosition;
GLint __h_aColor;
GLint __h_aNormal;
GLint __h_uModelMatrix;
int __ShadeProg;

void crappyInitFunc(GLint it, GLint ma, GLint md, GLint ms, GLint msh, GLint uv, 
      GLint tu, GLint pos, GLint c, GLint n, GLint mm, int sp) {
   __h_uInvTrans = it;
   __h_uMatAmb = ma;
   __h_uMatDif = md;
   __h_uMatSpec = ms;
   __h_uMatShine = msh;
   __h_aUV = uv;
   __h_aPosition = pos;
   __h_aColor = c;
   __h_aNormal = n;
   __h_uModelMatrix = mm;
   __h_uTexUnit = tu;
   __ShadeProg = sp;
}

/* helper function to set up material for shading */
void SetMaterial(int i) {
  glUseProgram(__ShadeProg);
  switch (i) {
    case 0:
        safe_glUniform3f(__h_uMatAmb, 0.4, 0.4, 0.6);
        safe_glUniform3f(__h_uMatDif, 0.6, 0.6, 0.6);
        safe_glUniform3f(__h_uMatSpec, 0.4, 0.4, 0.3);
        safe_glUniform1f(__h_uMatShine, 1.0);
        break;
    case 1:
        safe_glUniform3f(__h_uMatAmb, 0.2, 0.2, 0.2);
        safe_glUniform3f(__h_uMatDif, 0.2, 0.3, 0.3);
        safe_glUniform3f(__h_uMatSpec, 0.4, 0.4, 0.4);
        safe_glUniform1f(__h_uMatShine, 200.0);
        break;
    case 2:
    /* TO DO fill in another material that is greenish */
        //slime cube
        safe_glUniform3f(__h_uMatAmb, 0.1, 0.7, 0.1);
        safe_glUniform3f(__h_uMatDif, 0.3, 0.4, 0.3);
        safe_glUniform3f(__h_uMatSpec, 0.3, 0.5, 0.3);
        safe_glUniform1f(__h_uMatShine, 10.0);
        break;
    case 3:
        safe_glUniform3f(__h_uMatAmb, 0.1, 0.1, 0.1);
        safe_glUniform3f(__h_uMatDif, 0.2, 0.2, 0.2);
        safe_glUniform3f(__h_uMatSpec, 0.3, 0.3, 0.3);
        safe_glUniform1f(__h_uMatShine, 20.0);
        break;
  }
}

void initGameObjState(Transform_t *state) {
   state->pos = state->orient = vec3(0.0);
   state->scale = vec3(1.0);
   state->translate = state->scaling = state->rotation = state->transform
      = mat4(1.0);
}

bool containedIn(vec3 pt, vec3 min, vec3 max) {
   return pt.x >= min.x && pt.y >= min.y && pt.z >= min.z
          && pt.x <= max.x && pt.y <= max.y && pt.z <= max.z;
}

void GameObject::trans(float x, float y, float z) {
   int i;
   mat4 inmesh, *outmesh;
   vec4 newPos;

   state.pos.x += x;
   state.pos.y += y;
   state.pos.z += z;

   inmesh = state.translate;
   outmesh = &state.translate;

   *outmesh = inmesh * translate(mat4(1.0f), vec3(x,y,z));
   state.transform = (*outmesh) * state.rotation * state.scaling;
}

void GameObject::rot(float x, float y, float z) {
   int i;
   vec3 center;
   vec4 newPos;
   mat4 movTrans, rotTrans, retTrans, inmesh, *outmesh;

   center = state.pos;
   inmesh = state.rotation;
   outmesh = &state.rotation;

//   updateRotation(x,y);
   movTrans = translate(mat4(1.0f), -center);
   retTrans = translate(mat4(1.0f), center);
   rotTrans = rotate(mat4(1.0f), x, vec3(0.0f, 1.0f, 0.0f));
   rotTrans = rotate(mat4(1.0f), y, vec3(1.0f, 0.0f, 0.0f))*rotTrans;
   rotTrans = rotate(mat4(1.0f), z, vec3(0.0f, 0.0f, 1.0f))*rotTrans;
   *outmesh = retTrans * rotTrans * movTrans * inmesh;

   state.transform = state.translate * (*outmesh) * state.scaling;
}

void GameObject::rescale(float x, float y, float z) {
   int i;
   vec3 center, vScale, currScale;
   vec4 newPos;
   mat4 movTrans, sTrans, retTrans, inmesh, *outmesh;
   SBoundingBox temp;
   center = state.pos;
   vScale = vec3(x,y,z);
   currScale = state.scale;

   inmesh = state.scaling;
   outmesh = &state.scaling;
   
   movTrans = translate(mat4(1.0f), -center);
   retTrans = translate(mat4(1.0f), center);
   *outmesh = retTrans * scale(mat4(1.0f), vScale) * movTrans * inmesh;
   state.transform = state.translate * state.rotation * (*outmesh);

   currScale.x *= vScale.x;
   currScale.y *= vScale.y;
   currScale.z *= vScale.z;
   temp = SBoundingBox(bounds.left * x, bounds.bottom * y, bounds.back * z);
   temp.update(bounds.right * x, bounds.top * y, bounds.front * z);
   bounds = temp;
}


vec3 GameObject::checkCollision(GameObject other) {
   vec3 min = vec3(bounds.left, bounds.bottom, bounds.back) + state.pos;
   vec3 max = vec3(bounds.right, bounds.top, bounds.front) + state.pos;
   vec3 otherMin = vec3(other.bounds.left, other.bounds.bottom, other.bounds.back) + other.state.pos;
   vec3 otherMax = vec3(other.bounds.right, other.bounds.top, other.bounds.front) + other.state.pos;

   if (max.x > otherMin.x && min.x < otherMax.x && max.y > otherMin.y && 
         min.y < otherMax.y && max.z > otherMin.z && min.z < otherMax.z) {
      return state.velocity - other.state.velocity;
   }

   return vec3(0.0);
}

vec3 GameObject::applyForce(vec3 force) {
   vec3 deltaV;
   state.velocity += force / mass;
   return deltaV;
}

vec3 GameObject::setVelocity(vec3 vel) {
   vec3 forceApplied;
   state.velocity = vel;
   return forceApplied;
}

vec3 GameObject::applyTransform(mat4 tran) {
   vec3 forceApplied;
   state.transform *= tran;
   return forceApplied;
}

float GameObject::scaleMass(float scale) {
   return 1.0;
}

void GameObject::draw() {
   int i;
   glUseProgram(__ShadeProg);
   safe_glUniformMatrix4fv(__h_uModelMatrix, glm::value_ptr(state.transform));
   safe_glUniformMatrix4fv(__h_uInvTrans, 
      glm::value_ptr(glm::transpose(glm::inverse(state.transform))));

   for (i = 0; i < model.meshes.size(); i++) {
      model.meshes[i].render();
   }
}

void ObjectMesh::render() {
   //std::cout << "using buffer " << buff << "\n";
   glUseProgram(__ShadeProg);
   SetMaterial(matIdx);
   safe_glEnableVertexAttribArray(__h_aPosition);
   glBindBuffer(GL_ARRAY_BUFFER, posBuffObj);
   safe_glVertexAttribPointer(__h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(__h_aNormal);
   glBindBuffer(GL_ARRAY_BUFFER, normBuffObj);
   safe_glVertexAttribPointer(__h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

   safe_glEnableVertexAttribArray(__h_aUV);
   //if (shadeMode) {
   glBindBuffer(GL_ARRAY_BUFFER, uvBuffObj);
   //}
   safe_glVertexAttribPointer(__h_aUV, 2, GL_FLOAT, GL_FALSE, 0, 0);

   // draw!
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idxBuffObj);
   glDrawElements(GL_TRIANGLES, numFaces, GL_UNSIGNED_INT, 0);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

   //clean up
   safe_glDisableVertexAttribArray(__h_aPosition);
   safe_glDisableVertexAttribArray(__h_aUV);
   safe_glDisableVertexAttribArray(__h_aNormal);
}

void GameObject::update(double timeStep) {
   if (!grounded && gravityAffected && timeStep > 0) {
      state.velocity += vec3(0.0, -20.0, 0.0);
   }
   vec3 mov = state.velocity * (float)timeStep;
   trans(mov.x,mov.y,mov.z);
}
