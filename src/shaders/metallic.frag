#version 330 core

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
} fs_in;

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Light light;
uniform vec3 viewPos;

// environment mapping
uniform samplerCube environmentMap;

// hyperparameter
uniform float bias;
uniform float alpha;
uniform float lightIntensity;

// TODO: Implement metallic shading

void main() 
{
    vec3 N = normalize(fs_in.Normal);
    vec3 V = normalize(viewPos - fs_in.FragPos); // from fragment to eyes
    vec3 R = reflect(-V, N); // point to environment

    vec3 envColor = texture(environmentMap, R).rgb;
    vec3 L = normalize(light.position - fs_in.FragPos);
    float diff = max(dot(N, L) * lightIntensity, 0.0);

    vec3 base = vec3(diff + bias);
    vec3 color = mix(base, envColor, alpha);

    FragColor = vec4(color, 1.0);

}	
