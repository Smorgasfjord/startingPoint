//
//  Mountain.cpp
//  levelBuilder
//
//  Created by Taylor Woods on 4/16/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//
#ifndef MOUNTAIN_CPP
#define MOUNTAIN_CPP

#include "Mountain.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "GLSL_helper.h"
#include <sys/time.h>

Mountain::Mountain()
{
   
}

Mountain::Mountain(glm::vec3 pos, GLHandles hand, GameModel model)
{
   state.pos = pos;
   state.scale = glm::vec3(1.0f);
   rotation = 0;
   GameObject::handles = hand;
   state.velocity = glm::vec3(0);
   mod = model;
}

void Mountain::step()
{
   return;
}

/* Set up matrices to place model in the world */
void Mountain::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(GameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(GameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

float Mountain::getZ(float y)
{
   //ax + by + cz + d = 0 -> z = (-ax - by -d) / c
   //d = - ax0 -by0 - cz0
   //n = <a, b, c> = <0, 1, 1>
   //center of the plane: <x0, y0, z0> = <30, 30, 15>
   float d = (-1 * 15) + (1 * 15);
   float z = ((- 1 * y) - d) / -1;
   return z;
}

void Mountain::draw()
{
   glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
   //Enable handles
   safe_glEnableVertexAttribArray(handles.aPosition);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   SetModel(state.pos, state.scale, rotation);
   glBindBuffer(GL_ARRAY_BUFFER, mod.meshes[0].posBuffObj);
   safe_glVertexAttribPointer(handles.aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mod.meshes[0].idxBuffObj);
   safe_glEnableVertexAttribArray(handles.aNormal);
   
   glBindBuffer(GL_ARRAY_BUFFER, mod.meshes[0].normBuffObj);
   safe_glVertexAttribPointer(handles.aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
   
   glDrawElements(GL_TRIANGLES, mod.meshes[0].numFaces, GL_UNSIGNED_SHORT, 0);
   //clean up
	safe_glDisableVertexAttribArray(handles.aPosition);
	safe_glDisableVertexAttribArray(handles.aNormal);
   return;
}
#endif