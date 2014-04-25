#include "CMeshLoaderSimple.h"
/* Base code for mesh for CPE 471 Rasterizer project */
/* Reads in an ASCII mesh into stl vectors (stored in CMesh class) */
/* heavily edited version of I. Dunn's c++ parser - ZJW */
#ifdef __APPLE__
#include <cmath>
#endif

#ifdef __unix__
#include <math.h>
#endif


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>
#include <string>
#include <streambuf>
#include <stdlib.h>
#include <assert.h>

using namespace std;

float calcTriArea(SVector3 v1, SVector3 v2, SVector3 v3) {
   return ((v2.X - v1.X) * (v3.Y - v1.Y) - (v2.Y - v1.Y) * (v3.X - v1.X)) * 0.5f;
}

void CMesh::GetBounds() {
   if (!Vertices.size()) {
      return;
   }
   Bounds = SBoundingBox(Vertices[0].pos.X, Vertices[0].pos.Y, Vertices[0].pos.Z);
   for (int i = 1; i < Vertices.size(); i++) {
      Bounds.update(Vertices[i].pos.X, Vertices[i].pos.Y, Vertices[i].pos.Z);
   }
}

/* code to parse through a .m file and write the data into a mesh */
int loadASCIIMesh(std::string const & fileName, CMesh *Mesh, SColor defColor) {
   std::ifstream File;
   File.open(fileName.c_str());

   if (! File.is_open())
   {
      std::cerr << "Unable to open mesh file: " << fileName << std::endl;
      return 0;
   }

   //CMesh * Mesh = new CMesh();

   while (File)
   {
      std::string ReadString;
      std::getline(File, ReadString);

      std::stringstream Stream(ReadString);

      std::string Label;
      Stream >> Label;

      if (Label.find("#") != std::string::npos)
      {
         // Comment, skip
         continue;
      }

      if ("Vertex" == Label)
      {
         int Index;
         Stream >> Index; // We don't care, throw it away

         SVector3 pos;
         Stream >> pos.X;
         Stream >> pos.Y;
         Stream >> pos.Z;

         SVertex Vertex;
         Vertex.pos = pos;
         Vertex.Color = defColor;
         Mesh->Vertices.push_back(Vertex);
      }
      else if ("Face" == Label)
      {
         int Index;
         Stream >> Index; // We don't care, throw it away

         int Vertex1, Vertex2, Vertex3;
         Stream >> Vertex1;
         Stream >> Vertex2;
         Stream >> Vertex3;

         STriangle Triangle;
         Triangle.vIdx1 = Vertex1 - 1;
         Triangle.vIdx2 = Vertex2 - 1;
         Triangle.vIdx3 = Vertex3 - 1;

         size_t Location;
         if ((Location = ReadString.find("{")) != std::string::npos) // there is a color
         {
            Location = ReadString.find("rgb=(");
            Location += 5; // rgb=( is 5 characters

            ReadString = ReadString.substr(Location);
            std::stringstream Stream(ReadString);
            float Color;
            Stream >> Color;
            Triangle.Color.Red = Color;
            Stream >> Color;
            Triangle.Color.Green = Color;
            Stream >> Color;
            Triangle.Color.Blue = Color;
         }

         Mesh->Triangles.push_back(Triangle);
      }
      else if ("" == Label)
      {
         // Just a new line, carry on...
      }
      else if ("Corner" == Label)
      {
         // We're not doing any normal calculations... (oops!)
      }
      else
      {
         std::cerr << "While parsing ASCII mesh: Expected 'Vertex' or 'Face' label, found '" << Label << "'." << std::endl;
      }
   }

   if (! Mesh->Triangles.size() || ! Mesh->Vertices.size())
   {
      delete Mesh;
      return 0;
   }

   return 1;
}

CMesh::CMesh()
{}

CMesh::~CMesh()
{}

/* Just an example of how to iterate over the mesh triangles */
void CMesh::PrintMesh() {
    for(unsigned int j = 0; j < Triangles.size(); j++)
    {
        SVertex Vertex;
        cout << "New Vertices from triangle " << j << endl;
        Vertex = Vertices[Triangles[j].vIdx1];
        cout << "V1 " << Vertex.pos.X << " " << Vertex.pos.Y << " " << Vertex.pos.Z << endl;
        Vertex = Vertices[Triangles[j].vIdx2];
        cout << "V2 " << Vertex.pos.X << " " << Vertex.pos.Y << " " << Vertex.pos.Z << endl;
        Vertex = Vertices[Triangles[j].vIdx3];
        cout << "V3 " << Vertex.pos.X << " " << Vertex.pos.Y << " " << Vertex.pos.Z << endl;
    }
}

bool CMesh::DepthCompare(STriangle t1, STriangle t2) {
   SBoundingBox box1, box2;

   box1 = t1.getBound(Vertices);
   box2 = t2.getBound(Vertices);

   if (box1.front <= box2.back) {
      return false;
   }
   else if (box1.back >= box2.front) {
      return true;
   }
   else {
      return (box1.front + box1.back) / 2.0 > (box2.front + box2.back) / 2.0;
   }
}

void CMesh::GenerateNormals() {
   SVector3 vert1, vert2, vert3, norm, temp, tempy;
   float len;
   for (int i = 0; i < Vertices.size(); i++) {
      Normals.push_back(SVector3(0.0f));
   }
   for (int i = 0; i < Triangles.size(); i++) {
      vert1 = Vertices[Triangles[i].vIdx1].pos;
      vert2 = Vertices[Triangles[i].vIdx2].pos;
      vert3 = Vertices[Triangles[i].vIdx3].pos;
      
      temp = vert3 - vert2;
      tempy = vert1 - vert2;
      norm = temp.crossProduct(tempy);
      norm = norm / norm.length();

      Normals[Triangles[i].vIdx1] += norm;
      Normals[Triangles[i].vIdx2] += norm;
      Normals[Triangles[i].vIdx3] += norm;
   }
   for (int i = 0; i < Normals.size(); i++) {
      Normals[i] = Normals[i] / Normals[i].length();
   }
}



int wToPixelX(float width, float height, float xW) {
   return ((width > height ? height / width : 1.0) * xW + 1.0) * width / 2.0;
}

int wToPixelY(float width, float height, float yW) {
   return ((width < height ? width / height : 1.0) * yW + 1.0) * height / 2.0;
}

float pToWorldX(float width, float height, int xP) {
   return (width > height ? width / height : 1.0) * (2.0 * (float)xP / width - 1.0);
}

float pToWorldY(float width, float height, int yP) {
   return (width < height ? height / width : 1.0) * (2.0 * (float)yP / height - 1.0);
}

/* center the mesh */
void CMesh::centerMeshByExtents(SVector3 const & CenterLocation) {
    /*SVector3 Center;
    GetBounds();
    Center = Bounds.center();
    SVector3 VertexOffset = CenterLocation - Center;
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->pos += VertexOffset;*/

    if (Vertices.size() < 2)
        return;

    SVector3 Min, Max;
    {
        std::vector<SVertex>::const_iterator it = Vertices.begin();
        Min = it->pos;
        Max = it->pos;
        for (; it != Vertices.end(); ++ it)
        {
            if (Min.X > it->pos.X)
                Min.X = it->pos.X;
            if (Min.Y > it->pos.Y)
                Min.Y = it->pos.Y;
            if (Min.Z > it->pos.Z)
                Min.Z = it->pos.Z;

            if (Max.X < it->pos.X)
                Max.X = it->pos.X;
            if (Max.Y < it->pos.Y)
                Max.Y = it->pos.Y;
            if (Max.Z < it->pos.Z)
                Max.Z = it->pos.Z;
        }
    }

    SVector3 Center = (Max + Min) / 2.0;

    SVector3 VertexOffset = CenterLocation - Center;
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->pos += VertexOffset;

}

/* resize the mesh */
void CMesh::resizeMesh(SVector3 const & Scale)
{
/*    if (Vertices.size() < 2)
        return;

    SVector3 Extent = (SVector3(Bounds.right, Bounds.top, Bounds.front) - 
                       SVector3(Bounds.left, Bounds.bottom, Bounds.back));
    SVector3 Resize = Scale / std::max(Extent.X, std::max(Extent.Y, Extent.Z));
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->pos *= Resize;*/
       if (Vertices.size() < 2)
        return;

    SVector3 Min, Max;
    {
        std::vector<SVertex>::const_iterator it = Vertices.begin();
        Min = it->pos;
        Max = it->pos;
        for (; it != Vertices.end(); ++ it)
        {
            if (Min.X > it->pos.X)
                Min.X = it->pos.X;
            if (Min.Y > it->pos.Y)
                Min.Y = it->pos.Y;
            if (Min.Z > it->pos.Z)
                Min.Z = it->pos.Z;

            if (Max.X < it->pos.X)
                Max.X = it->pos.X;
            if (Max.Y < it->pos.Y)
                Max.Y = it->pos.Y;
            if (Max.Z < it->pos.Z)
                Max.Z = it->pos.Z;
        }
    }

    SVector3 Extent = (Max - Min);
    SVector3 Resize = Scale / std::max(Extent.X, std::max(Extent.Y, Extent.Z));
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->pos *= Resize;

}

/* resize the mesh */
void CMesh::rotateMesh(SVector3 const & Rotation)
{
/*    if (Vertices.size() < 2)
        return;

    SVector3 Extent = (SVector3(Bounds.right, Bounds.top, Bounds.front) - 
                       SVector3(Bounds.left, Bounds.bottom, Bounds.back));
    SVector3 Resize = Scale / std::max(Extent.X, std::max(Extent.Y, Extent.Z));
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it)
        it->pos *= Resize;*/
       if (Vertices.size() < 2)
        return;
    float cosx, cosy, sinx, siny;
    cosx = cos(Rotation.X);
    cosy = cos(Rotation.Y);
    sinx = sin(Rotation.X);
    siny = sin(Rotation.Y);
    for (std::vector<SVertex>::iterator it = Vertices.begin(); it != Vertices.end(); ++ it) {
        it->pos = SVector3(it->pos.X*cosx + it->pos.Z*sinx, it->pos.Y, it->pos.Z*cosx - it->pos.X*sinx);
        it->pos = SVector3(it->pos.X, it->pos.Y*cosy + it->pos.Z*siny, it->pos.Y*siny + it->pos.Z*cosy);
    }

}

