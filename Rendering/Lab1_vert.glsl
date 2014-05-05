#version 330
in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;
 
out vec4 vPos;
out vec3 vNorm;
out vec2 vTexCoord;

layout (std140) uniform uMatrices {    
   mat4 uProjMatrix;
   mat4 uViewMatrix;
   mat4 uModelMatrix;
   mat4 uInverseTranspose;
};
 
void main() {
  vec4 vPosition, transNorm;
  
  /* First model transforms */
  vPosition = uModelMatrix* vec4(aPosition.x, aPosition.y, aPosition.z, 1.0);
  vPosition = uViewMatrix* vPosition;
  vPos = uProjMatrix*vPosition;
  transNorm = uInverseTranspose * vec4(aNormal.x, aNormal.y, aNormal.z, 0.0);
  /* TO DO REPLACE THIS WITH lighting!!!! */
  vNorm = normalize(vec3(transNorm.x, transNorm.y, transNorm.z));
  vTexCoord = aUV;
}
