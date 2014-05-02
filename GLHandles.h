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
      GLuint uModelMatrix;
      GLuint uViewMatrix;
      GLuint uProjMatrix;
      GLuint uNormMatrix;
      GLuint uLightPos;
      GLuint uLightColor;
      GLuint uEyePos;
      GLuint uMatAmb;
      GLuint uMatDif;
      GLuint uMatSpec;
      GLuint uMatShine;

      GLHandles()
      {
      }

      void print() {
         printf("sp:%i, p:%i, n:%i, uv:%i, t:%i, mm:%i, vm:%i, pm:%i, nm:%i, lp:%i, lc:%i, ep:%i, ma:%i, md:%i, msp:%i, msh:%i\n",
               aPosition,aNormal,aUV,uTexUnit,uModelMatrix,uViewMatrix,uProjMatrix,
               uNormMatrix,uLightPos,uLightColor,uEyePos,uMatAmb,uMatDif,uMatSpec,uMatShine);
      }
};

#endif /* defined(___76_Lab1__GLHandles__) */
