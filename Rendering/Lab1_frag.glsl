#version 330

layout (std140) uniform uMat {
  vec4 aColor;
  vec4 dColor;
  vec4 sColor;
  vec4 eColor;
  float shine;
  int texCount;
};

uniform vec3 uLightPos;
uniform vec3 uLColor;
uniform vec3 uCamPos;
uniform sampler2D uTexUnit;

in vec4 vPos;
in vec3 vNorm;
in vec2 vTexCoord;

out vec4 owtput;

void main() {
  vec3 diffuse, specular, ambient;
  vec3 norm = normalize(vNorm);
  vec3 light = normalize(uLightPos - vPos.xyz);
  vec3 rVec = reflect(-light, norm);
  vec3 view = normalize(uCamPos - vPos.xyz);
  vec4 texColor;
  if (texCount != 0) {
    texColor = texture2D(uTexUnit, vTexCoord);
  }
  else {
     texColor = vec4(1.0);
  }
  diffuse = uLColor*max(dot(vNorm,light),0.0)*dColor.xyz;
  specular = uLColor*pow(max(dot(rVec,view),0.0),shine)*sColor.xyz;
  ambient = uLColor*aColor.xyz;
  vec3 phong = (diffuse + specular + ambient);
  owtput = vec4(phong.x, phong.y, phong.z, 1.0) * vec4(texColor.xyz, 1.0);
}
