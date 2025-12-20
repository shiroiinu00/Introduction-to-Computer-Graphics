#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VS_OUT{
    vec3 Color;
    vec2 TexCoord;
} vs_out;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float gloss; // shininess
};

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


// TODO:
// Implement Gouraud shading

void main()
{
	vec4 worldPos = model * vec4(aPos, 1.0);
    vec3 FragPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * aNormal);

    vec3 L = normalize(light.position - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);

    // Phong specular
    float spec = 0.0;
    if(diff > 0.0){
        spec = pow(max(dot(R, V), 0.0), material.gloss);
    }

    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * material.diffuse * diff;
    vec3 specular = light.specular * material.specular * spec;

    vs_out.Color = ambient + diffuse + specular;
    vs_out.TexCoord = aTexCoord;

    gl_Position = projection * view * worldPos;
}