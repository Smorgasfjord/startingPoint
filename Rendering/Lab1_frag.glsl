#version 330

struct Material {
  vec3 aColor;
  vec3 dColor;
  vec3 sColor;
  float shine;
};

uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uCamPos;
uniform Material uMat;
uniform sampler2D uTexUnit;

in vec3 fPos;
in vec3 fNorm;
in vec2 fTexCoord;

out vec4 owtput;

void main() {
  vec3 diffuse, specular, ambient;
  vec3 norm = normalize(fNorm);
  vec3 light = normalize(uLightPos - fPos);
  vec3 rVec = reflect(-light, norm);
  vec3 view = normalize(uCamPos - fPos);
  vec4 texColor = texture2D(uTexUnit, fTexCoord);
  diffuse = uLColor*max(dot(fNorm,light),0.0)*uMat.dColor;
  specular = uLColor*pow(max(dot(rVec,view),0.0),uMat.shine)*uMat.sColor;
  ambient = uLColor*uMat.aColor;
  vec3 phong = (diffuse + specular + ambient);
  owtput = vec4(phong.x, phong.y, phong.z, 1.0) * vec4(texColor.xyz, 1.0);
}
