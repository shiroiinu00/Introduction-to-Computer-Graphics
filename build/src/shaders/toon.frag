#version 330 core

in VS_OUT{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoord;
} fs_in;

out vec4 FragColor;

struct Material{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float gloss; // shininess
};

struct Light{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

uniform sampler2D ourTexture;

uniform float diffLevels;
uniform float specThreshold;
uniform float outlineStrength;

// TODO: Implement bling-phong shading

void main()
{
	vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(light.position - fs_in.FragPos);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 H = normalize(L + V); // halfway vector

    // Lambert diffuse
    float diffRaw = max(dot(N, L), 0.0);

    float levels = max(diffLevels, 1.0);
    float diffToon = floor(diffRaw * levels) / levels;
    
    // Blinn-Phong specular
    float specToon = 0.0;
    if(diffRaw > 0.0){
        float specRaw = pow(max(dot(N, H), 0.0), material.gloss);
        specToon = specRaw > specThreshold ? 1.0 : 0.0;
    }

    // A / D / S
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * material.diffuse * diffToon;
    vec3 specular = light.specular * material.specular * specToon;

    vec3 texColor = texture(ourTexture, fs_in.TexCoord).rgb;
    vec3 baseColor = (ambient + diffuse) * texColor + specular;

    FragColor = vec4(baseColor, 1.0);

}