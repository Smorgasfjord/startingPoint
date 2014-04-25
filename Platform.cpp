//
//  Platform.cpp
//  levelBuilder
//
//  Created by Taylor Woods on 4/15/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//
#ifndef PLATFORM_CPP
#define PLATFORM_CPP

#include "Platform.h"
#include "Mountain.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "GLSL_helper.h"
#include <sys/time.h>
#include <stdio.h>
#include <string.h>

Platform::Platform()
{
   //This needs to be here for C++ to be happy, even though it doesn't seem to do anything
}

Platform::Platform(glm::vec3 pos, GLHandles hand, GameModel model)
{
   state.pos = pos;
   state.scale = glm::vec3(1.0f);
   rotation = 0;
   GameObject::handles = hand;
   state.velocity = glm::vec3(0);
   mod = model;
}

Platform::Platform(glm::vec3 pos, glm::vec3 size, float rotation, GLHandles hand, GameModel model)
{
   state.pos = pos;
   state.scale = size;
   rotation = rotation;
   GameObject::handles = hand;
   state.velocity = glm::vec3(0);
   mod = model;
}

string Platform::toString()
{
   char pos[30];
   char sizeStr[30];
   char rot[15];
   string str = "Platform: \n";
   sprintf(pos, "\t%f %f %f\n", state.pos.x, state.pos.y, state.pos.z);
   sprintf(sizeStr, "\t%f %f %f\n", state.scale.x, state.scale.y, state.scale.z);
   sprintf(rot, "\t%f\n", rotation);
   str.append(pos);
   str.append(sizeStr);
   str.append(rot);
   
   return str;
}

//Given a position determines if that position intersects the platform
//Returns false for no collision
//NOT FULLY IMPLEMENTED
bool Platform::detectCollision(glm::vec3 test)
{
   float distFromCenter = state.scale.x * .5;
   if(test.x <= state.pos.x + distFromCenter && test.x >= state.pos.x - distFromCenter &&
      test.y <= state.pos.y + .5 && test.y >= state.pos.y - .5)
   {
      
   }
   
   return false;
}

float Platform::getRot()
{
   return rotation;
}

void Platform::setRot(float val)
{
   rotation = val;
}

void Platform::step()
{
   return;
}

glm::vec3 Platform::getSize()
{
   return state.scale;
}

void Platform::stretch()
{
   state.scale.x += .05;
}

void Platform::shrink()
{
   state.scale.x -= .05;
}

glm::vec3 Platform::getPos()
{
   return state.pos;
}

void Platform::setXPos(float val)
{
   state.pos.x = val;
}

void Platform::setYPos(float val)
{
   state.pos.z = Mountain::getZ(val) - .1;
   state.pos.y = val;
}

/* Set up matrices to place model in the world */
void Platform::SetModel(glm::vec3 loc, glm::vec3 size, float rotation) {
   glm::mat4 Scale = glm::scale(glm::mat4(1.0f), size);
   glm::mat4 Trans = glm::translate(glm::mat4(1.0f), loc);
   glm::mat4 Rotate = glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 0, 1));
   Rotate *= glm::rotate(glm::mat4(1.0f), -rotation / 2, glm::vec3(0, 1, 0));
   
   glm::mat4 final = Trans * Rotate * Scale;
   safe_glUniformMatrix4fv(GameObject::handles.uModelMatrix, glm::value_ptr(final));
   safe_glUniformMatrix4fv(GameObject::handles.uNormMatrix, glm::value_ptr(glm::vec4(1.0f)));
}

void Platform::draw()
{
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