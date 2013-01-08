varying vec4 Color;
varying float s;

vec3 RED = vec3(1.0,0.0,0.0);
vec3 GREEN = vec3(0.0,1.0,0.0);
vec3 BLUE = vec3(0.0,0.0,1.0);

vec3 chromaDepthColor(float s){
  vec3 YELLOW = mix(RED,GREEN,s);
  vec3 CYAN = mix(GREEN,BLUE,s);
  return mix(YELLOW,CYAN,s);

}

void main() {

  vec3 rgb3 = chromaDepthColor(s); 
  vec4 rgb = vec4(rgb3,1.0);

//  gl_FragColor = vec4(s,s,0.0,1.0);
//  gl_FragColor = rgb;
    gl_FragColor = rgb * Color;
//    gl_FragColor = Color;


}
