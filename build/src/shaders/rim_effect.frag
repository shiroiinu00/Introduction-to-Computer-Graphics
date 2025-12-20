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

uniform vec3 rimColor;
uniform float rimStrength;
uniform float rimPower;

// TODO: Implement bling-phong shading

void main()
{
	vec3 N = normalize(fs_in.Normal);
    vec3 L = normalize(light.position - fs_in.FragPos);
    vec3 V = normalize(viewPos - fs_in.FragPos);
    vec3 H = normalize(L + V); // halfway vector

    // Lambert diffuse
    float diff = max(dot(N, L), 0.0);

    // Blinn-Phong specular
    float spec = 0.0;
    if(diff > 0.0){
        spec = pow(max(dot(N, H), 0.0), material.gloss);
    }

    // A / D / S
    vec3 ambient = light.ambient * material.ambient;
    vec3 diffuse = light.diffuse * material.diffuse * diff;
    vec3 specular = light.specular * material.specular * spec;

    vec3 texColor = texture(ourTexture, fs_in.TexCoord).rgb;
    vec3 baseColor = (ambient + diffuse) * texColor + specular;

    float NdotV = max(dot(N, V), 0.0);
    float rim   = pow(1.0 - NdotV, rimPower);
    rim *= rimStrength; // 總強度

    vec3 rimTerm = rimColor * rim;

    vec3 finalColor = baseColor + rimTerm;

    FragColor = vec4(finalColor, 1.0);

}