#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

uniform vec3 viewPos;

uniform samplerCube environmentMap;

uniform float AIR_coeff;
uniform float GLASS_coeff;

// TODO: Implement glass shading with schlick method

void main()
{          
  vec3 N = normalize(fs_in.Normal);
  vec3 V = normalize(viewPos - fs_in.FragPos);
  vec3 I = -V;

  float eta = AIR_coeff / GLASS_coeff;

  vec3 R = reflect(I, N);
  vec3 T = refract(I, N, eta);

  vec3 reflectColor = texture(environmentMap, R).rgb;
  vec3 refractColor = texture(environmentMap, T).rgb;

  float cosTheta = clamp(dot(-I, N),0.0, 1.0);
  float r0 = (AIR_coeff - GLASS_coeff) / (AIR_coeff + GLASS_coeff);
  r0 = r0 * r0;
  float fresnel = r0 + (1.0 - r0) * pow(1.0 - cosTheta, 5.0);

  vec3 color = fresnel * reflectColor + (1.0 - fresnel) * refractColor;

  FragColor = vec4(color, 1.0); 
} 