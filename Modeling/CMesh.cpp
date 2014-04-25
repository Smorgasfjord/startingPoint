
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
#include "CMesh.h"
#include "../glm/glm.hpp"
#include "../glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "../glm/gtc/type_ptr.hpp" //value_ptr
#include "../glm/gtx/vector_query.hpp" //


bool CMeshLoader::loadVertexBufferObjectFromMesh(std::string const & fileName, 
      int & TriCount, GLuint & posBuff, GLuint & IdxBuff, GLuint & NorBuff, GLuint & ClrBuff)
{
   CMesh * Mesh = loadASCIIMesh(fileName);
   if (! Mesh)
      return false;
   glm::vec3 Norm, Color, v1, v2, v3;
   SVector3 vert1, vert2, vert3;
   SColor triCol;
   TriCount = Mesh->Triangles.size();
   float verts[Mesh->Vertices.size()*3], clrs[TriCount*3];
   float norms[Mesh->Vertices.size()*3];
   unsigned int idxs[TriCount*3];
   float normLen, colorLen;
   unsigned int vIdx1, vIdx2, vIdx3;
   float max = 0.0, MAX = 1.0, MIN = 0.2;
    //ADD code to resize the mesh and to reposition it at the origin
   std::fill(norms, norms+Mesh->Vertices.size()*3, 0.0);
   std::fill(clrs,clrs+TriCount*3, -1.0);
   Mesh->centerMeshByExtents(SVector3(0.0,0.0,0.0));
   
   for (int i = 0; i < Mesh->Vertices.size(); i++) {
      if (max < fabsf(Mesh->Vertices[i].pos.X)) max = fabsf(Mesh->Vertices[i].pos.X);
      if (max < fabsf(Mesh->Vertices[i].pos.Y)) max = fabsf(Mesh->Vertices[i].pos.Y);
      if (max < fabsf(Mesh->Vertices[i].pos.Z)) max = fabsf(Mesh->Vertices[i].pos.Z);
   }
   
   Mesh->resizeMesh(SVector3(MAX/max, MAX/max, MAX/max));

   for (int i = 0; i < Mesh->Vertices.size(); i++) {
      verts[i*3] = Mesh->Vertices[i].pos.X;
      verts[i*3+1] = Mesh->Vertices[i].pos.Y;
      verts[i*3+2] = Mesh->Vertices[i].pos.Z;
   }

    //ADD code to create a VBO for the mesh
   glGenBuffers(1, &posBuff);
   glBindBuffer(GL_ARRAY_BUFFER, posBuff);
   glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    
   for (int i = 0; i < TriCount; i++) {
      vIdx1 = idxs[i*3] = Mesh->Triangles[i].vIdx1;
      vIdx2 = idxs[i*3+1] = Mesh->Triangles[i].vIdx2;
      vIdx3 = idxs[i*3+2] = Mesh->Triangles[i].vIdx3;

      triCol = Mesh->Triangles[i].Color;
      vert1 = Mesh->Vertices[vIdx1].pos;
      v1 = glm::vec3(vert1.X, vert1.Y, vert1.Z);
      vert2 = Mesh->Vertices[vIdx2].pos;
      v2 = glm::vec3(vert2.X, vert2.Y, vert2.Z);
      vert3 = Mesh->Vertices[vIdx3].pos;
      v3 = glm::vec3(vert3.X, vert3.Y, vert3.Z);

      Norm = glm::cross(v3-v2,v1-v2);
      normLen = glm::length(Norm);
 
      clrs[vIdx1*3] = (triCol.Red+(clrs[vIdx1*3] < 0.0 ? triCol.Red : clrs[vIdx1*3]))/2.0;
      clrs[vIdx2*3] = (triCol.Red+(clrs[vIdx2*3] < 0.0 ? triCol.Red : clrs[vIdx2*3]))/2.0;
      clrs[vIdx3*3] = (triCol.Red+(clrs[vIdx3*3] < 0.0 ? triCol.Red : clrs[vIdx3*3]))/2.0;
 
      clrs[vIdx1*3+1] = (triCol.Green+(clrs[vIdx1*3+1] < 0.0 ? triCol.Green : clrs[vIdx1*3+1]))/2.0;
      clrs[vIdx2*3+1] = (triCol.Green+(clrs[vIdx2*3+1] < 0.0 ? triCol.Green : clrs[vIdx2*3+1]))/2.0;
      clrs[vIdx3*3+1] = (triCol.Green+(clrs[vIdx3*3+1] < 0.0 ? triCol.Green : clrs[vIdx3*3+1]))/2.0;
 
      clrs[vIdx1*3+2] = (triCol.Blue+(clrs[vIdx1*3+2] < 0.0 ? triCol.Blue : clrs[vIdx1*3+2]))/2.0;
      clrs[vIdx2*3+2] = (triCol.Blue+(clrs[vIdx2*3+2] < 0.0 ? triCol.Blue : clrs[vIdx2*3+2]))/2.0;
      clrs[vIdx3*3+2] = (triCol.Blue+(clrs[vIdx3*3+2] < 0.0 ? triCol.Blue : clrs[vIdx3*3+2]))/2.0;
     
      norms[vIdx1*3] += Norm.x/normLen;
      norms[vIdx2*3] += Norm.x/normLen;
      norms[vIdx3*3] += Norm.x/normLen;

      norms[vIdx1*3+1] += Norm.y/normLen;
      norms[vIdx2*3+1] += Norm.y/normLen;
      norms[vIdx3*3+1] += Norm.y/normLen;

      norms[vIdx1*3+2] += Norm.z/normLen;
      norms[vIdx2*3+2] += Norm.z/normLen;
      norms[vIdx3*3+2] += Norm.z/normLen;
   }
 
   glGenBuffers(1, &IdxBuff);
   glBindBuffer(GL_ARRAY_BUFFER, IdxBuff);
   glBufferData(GL_ARRAY_BUFFER, sizeof(idxs), idxs, GL_STATIC_DRAW);

   for (int j = 0; j < Mesh->Vertices.size()*3; j+=3) {
      Norm = glm::vec3(norms[j], norms[j+1], norms[j+2]);
      normLen = glm::length(Norm);
      norms[j] /= normLen;
      norms[j+1] /= normLen;
      norms[j+2] /= normLen;
   }

   glGenBuffers(1, &NorBuff);
   glBindBuffer(GL_ARRAY_BUFFER, NorBuff);
   glBufferData(GL_ARRAY_BUFFER, sizeof(norms), norms, GL_STATIC_DRAW);

   glGenBuffers(1, &ClrBuff);
   glBindBuffer(GL_ARRAY_BUFFER, ClrBuff);
   glBufferData(GL_ARRAY_BUFFER, sizeof(clrs), clrs, GL_STATIC_DRAW);
	return true;
}

CMesh * const CMeshLoader::loadASCIIMesh(std::string const & fileName)
{
	std::ifstream File;
	File.open(fileName.c_str());

	if (! File.is_open())
	{
		std::cerr << "Unable to open mesh file: " << fileName << std::endl;
		return 0;
	}

	CMesh * Mesh = new CMesh();

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

	return Mesh;
}



