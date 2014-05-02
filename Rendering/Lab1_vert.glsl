#version 330
in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;
 
out vec4 vPos;
out vec2 vTexCoord;
out vec3 vNorm;


struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

uniform mat4 uProjMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uModelMatrix;
uniform mat4 uInverseTranspose;
uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uCamPos;
uniform Material uMat;

uniform Matrices {    
   mat4 projMatrix;
   mat4 viewMatrix;
   mat4 modelMatrix;
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
