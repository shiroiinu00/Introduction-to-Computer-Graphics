#version 330 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aTexCoord;

out VS_OUT{
    vec3 pos;
    vec3 normal;
    vec2 uv;
} vs_out;

uniform mat4 model, view, projection;
void main(){
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_out.pos = worldPos.xyz;
    vs_out.normal = mat3(transpose(inverse(model))) * aNormal;
    vs_out.uv = aTexCoord;
    gl_Position = projection * view * worldPos;
}