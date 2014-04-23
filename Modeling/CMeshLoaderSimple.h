#ifndef _CMESHLOADER_H_
#define _CMESHLOADER_H_

#ifdef __APPLE__
#include <cmath>
#endif

#ifdef __unix__
#include <math.h>
#endif

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <streambuf>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "types.h"

using namespace std;

class SVector3 {
 public:
   float X, Y, Z;

   SVector3()
      : X(0), Y(0), Z(0) {}

   SVector3(float in)
      : X(in), Y(in), Z(in) {}

   SVector3(float in_x, float in_y, float in_z)
      : X(in_x), Y(in_y), Z(in_z) {}


   SVector3 crossProduct(SVector3 const & v) const {
      return SVector3(Y*v.Z - v.Y*Z, v.X*Z - X*v.Z, X*v.Y - v.X*Y);
   }

   float dotProduct(SVector3 const & v) const {
      return X*v.X + Y*v.Y + Z*v.Z;
   }

   float length() const {
      return sqrtf(X*X + Y*Y + Z*Z);
   }

   SVector3 operator + (SVector3 const & v) const {
      return SVector3(X+v.X, Y+v.Y, Z+v.Z);
   }

   SVector3 & operator += (SVector3 const & v) {
      X += v.X;
      Y += v.Y;
      Z += v.Z;

      return * this;
   }

   SVector3 operator - (SVector3 const & v) const {
      return SVector3(X-v.X, Y-v.Y, Z-v.Z);
   }

   SVector3 & operator -= (SVector3 const & v) {
      X -= v.X;
      Y -= v.Y;
      Z -= v.Z;

      return * this;
   }

   SVector3 operator * (SVector3 const & v) const {
      return SVector3(X*v.X, Y*v.Y, Z*v.Z);
   }

   SVector3 & operator *= (SVector3 const & v) {
      X *= v.X;
      Y *= v.Y;
      Z *= v.Z;

      return * this;
   }

   SVector3 operator / (SVector3 const & v) const {
      return SVector3(X/v.X, Y/v.Y, Z/v.Z);
   }

   SVector3 & operator /= (SVector3 const & v) {
      X /= v.X;
      Y /= v.Y;
      Z /= v.Z;

      return * this;
   }

   SVector3 operator * (float const s) const {
      return SVector3(X*s, Y*s, Z*s);
   }

   SVector3 & operator *= (float const s) {
      X *= s;
      Y *= s;
      Z *= s;

      return * this;
   }

   SVector3 operator / (float const s) const {
      return SVector3(X/s, Y/s, Z/s);
   }

   SVector3 & operator /= (float const s) {
      X /= s;
      Y /= s;
      Z /= s;

      return * this;
   }

   bool operator == (SVector3 const & oth) {
      return X == oth.X && Y == oth.Y && Z == oth.Z;
   }


};

class SColor {
 public:

    float Red, Green, Blue, Alpha;

    SColor()
        : Red(0.7f), Green(0.7f), Blue(0.7f), Alpha(1.0f) {}

    SColor(float r, float g, float b)
        : Red(r), Green(g), Blue(b), Alpha(1.0f) {}

    SColor(float r, float g, float b, float a)
        : Red(r), Green(g), Blue(b), Alpha(a) {}

    SColor & scale(float scale) {
       Red *= scale;
       Green *= scale;
       Blue *= scale;
       return *this;
    }

};

class SVertex {
 public:

    SVector3 pos;
    SColor Color;

};

class SBoundingBox {
 public:

   float left, right, bottom, top, front, back;

   SBoundingBox()
       : left(0.0f), right(0.0f), bottom(0.0f), top(0.0f), front(0.0f), back(0.0f) {}

   SBoundingBox(float x, float y, float z)
       : left(x), right(x), bottom(y), top(y), front(z), back(z) {}

   void update(float x, float y, float z) {
      left = left < x ? left : x;
      right = right > x ? right : x;
      top = top > y ? top : y;
      bottom = bottom < y ? bottom : y;
      front = front > z ? front : z;
      back = back < z ? back : z;
      return;
   }

   SVector3 center() {
      return SVector3((right+left)/2.0, (top+bottom)/2.0, (front+back)/2.0);
   }


};

class STriangle {
 public:

   unsigned int vIdx1, vIdx2, vIdx3;
   SColor Color;
   SBoundingBox box;

   STriangle()
       : vIdx1(0), vIdx2(0), vIdx3(0), Color(SColor()), box(SBoundingBox()) {}

   SBoundingBox getBound(std::vector<SVertex> verts) {
      box = SBoundingBox(verts[vIdx1].pos.X, verts[vIdx1].pos.Y, verts[vIdx1].pos.Z);
      box.update(verts[vIdx2].pos.X, verts[vIdx2].pos.Y, verts[vIdx2].pos.Z);
      box.update(verts[vIdx3].pos.X, verts[vIdx3].pos.Y, verts[vIdx3].pos.Z);
      return box;
   }


};

class LightSource {
 public:
   SVector3 minBound, maxBound;
   color_t col;
   float strength;

   LightSource() :
      minBound(0),
      maxBound(0),
      strength(0)
   { }

   LightSource(SVector3 min, SVector3 max, color_t c, float s) :
      minBound(min.X, min.Y, min.Z),
      maxBound(max.X, max.Y, max.Z),
      strength(s)
   {
      srand(time(NULL));
      col = c;
   }

   SVector3 randPt() {
      float x,y,z;
      x = (maxBound.X - minBound.X)*((float)rand()/(float)RAND_MAX);
      y = (maxBound.Y - minBound.Y)*((float)rand()/(float)RAND_MAX);
      z = (maxBound.Z - minBound.Z)*((float)rand()/(float)RAND_MAX);

      return SVector3(minBound.X + x, minBound.Y + y, minBound.Z + z);
   }
      
};

class CMesh {
 public:
   SBoundingBox Bounds;
   std::vector<SVertex> Vertices;
   std::vector<STriangle> Triangles;
   std::vector<SVector3> Normals;

   CMesh();
   void PrintMesh();
   bool DepthCompare(STriangle t1, STriangle t2);
   void GetBounds();
   void GenerateNormals();

   ~CMesh();
   void centerMeshByExtents(SVector3 const & CenterLocation);
   void rotateMesh(SVector3 const & Rotation);
   void resizeMesh(SVector3 const & Scale);
};

int wToPixelX(float, float, float);
int wToPixelY(float, float, float);
float pToWorldX(float, float, int);
float pToWorldY(float, float, int);
int loadASCIIMesh(std::string const & fName, CMesh* m, SColor cl);

#endif
