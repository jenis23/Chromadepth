attribute vec4 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;

varying vec4 Color;
varying vec2 texCoord;
varying float depth;
varying float s;

uniform mat4 ModelViewProjection;
uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 TextureMatrix;

uniform vec3 ambientLight;
uniform vec3 light0Color;
uniform vec3 light0Position;

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform float nearPlane;
uniform float farPlane;



void main() {
  gl_Position = ModelViewProjection*vertexPosition;
//  texCoord = (TextureMatrix * vec4(vertexTexCoord, 0.0, 1.0)).st;

  vec3 P = vec3(ModelViewMatrix * vertexPosition);
  vec3 N = normalize(NormalMatrix * vertexNormal);
  vec3 L = normalize(light0Position - P);

  depth = sqrt(dot(P,P));  
//  depth = -P.z;
  s = (depth - nearPlane)/(farPlane - nearPlane);
  vec3 I_ambient = materialAmbient * ambientLight;
  float cos_theta = dot(L,N);
  vec3 I_diffuse = materialDiffuse * light0Color * max(0.0, cos_theta);
  vec3 I_specular = vec3(0.0, 0.0, 0.0);

  if (cos_theta > 0.0) {
    vec3 R = reflect(-L,N);
    vec3 V = normalize(-P);
    float cos_alpha = dot(R,V);
    I_specular = materialSpecular * light0Color * 
      pow(max(0.0, cos_alpha), materialShininess);
  }

  vec3 I = I_ambient + I_diffuse + I_specular;
    Color = vec4(I, 1.0);
  //   Color = vec4(1.0,0.10,0.10,1.0);
}
