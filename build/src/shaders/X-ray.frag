#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

out vec4 FragColor;


uniform vec3 viewPos;

uniform vec3 xrayColor;
uniform float xrayStrength;
uniform float xrayPower;

uniform sampler2D ourTexture;

// TODO: Implement bling-phong shading

void main()
{
	vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    

    float edge = 1.0 - max(dot(N, V), 0.0);
    edge = pow(edge, xrayPower);

    vec3 baseXray = xrayColor * edge * xrayStrength;

    
    vec3 texColor = texture(ourTexture, fs_in.TexCoord).rgb;
    vec3 color =  texColor * 0.2 + baseXray;

    FragColor = vec4(color, 1.0);

}