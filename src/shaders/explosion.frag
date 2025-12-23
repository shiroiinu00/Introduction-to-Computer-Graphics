#version 330 core
in GS_OUT {
    vec3 normal;
    vec2 uv;
}fs_in;
out vec4 FragColor;
uniform sampler2D outTexture;

void main(){
    FragColor = texture(outTexture, fs_in.uv);
}