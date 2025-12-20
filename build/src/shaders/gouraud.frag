#version 330 core

in VS_OUT{
    vec3 Color;
    vec2 TexCoord;
} fs_in;

out vec4 FragColor;


uniform sampler2D ourTexture;
// TODO:
// Implement Gouraud shading

void main()
{
	vec3 texColor = texture(ourTexture, fs_in.TexCoord).rgb;
    vec3 finalColor = fs_in.Color * texColor;
    FragColor = vec4(finalColor, 1.0);
}