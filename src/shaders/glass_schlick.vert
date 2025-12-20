#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out VS_OUT{
    vec3 FragPos;
    vec3 Normal;
}vs_out;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


// TODO: Implement glass shading with schlick method

void main()
{
   vec4 worldPos = model * vec4(aPos, 1.0);
   vs_out.FragPos = worldPos.xyz;

   mat3 normalMatrix = transpose(inverse(mat3(model)));
   vs_out.Normal = normalize(normalMatrix * aNormal);

   gl_Position = projection * view * worldPos;
}