//
//  GLHandles.h
//  476_Lab1
//
//  Created by Taylor Woods on 4/11/14.
//  Copyright (c) 2014 Taylor Woods. All rights reserved.
//

#ifndef ___76_Lab1__GLHandles__
#define ___76_Lab1__GLHandles__

#include <stdio.h>
#include <iostream>

#ifdef __APPLE__
#include <OPENGL/gl.h>
#endif

#ifdef __unix__
#include <GL/glew.h>
#include <GL/glut.h>
#endif

class GLHandles
{
   public:
      int ShadeProg;
      GLuint aPosition;
      GLuint aNormal;
      GLuint aUV;
      GLuint uTexUnit;
      GLuint uMatrices;
      GLuint uMatricesBuff;
      GLuint uLightPos;
      GLuint uLightColor;
      GLuint uEyePos;
      GLuint uMat;

      GLHandles()
      {
      }

      void print() {
         printf("sprog:%i, pos:%i, n:%i, uv:%i, tex:%i, mx%i, mxb%i, lp:%i, lc:%i, ep:%i, mat:%i\n",
               ShadeProg, aPosition,aNormal,aUV,uTexUnit,uMatrices,uMatricesBuff,uLightPos,uLightColor,uEyePos,uMat);
      }
};

#endif /* defined(___76_Lab1__GLHandles__) */
