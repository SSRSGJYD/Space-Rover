#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace[5];
} fs_in;


struct Light {
    int type; // 1 for PointLight, 2 for DirLight
    vec3 attrib; // direction or position
    vec3 color;
};

struct PointLight {
    vec3 position;
    vec3 color;
};

struct DirLight {
    vec3 direction;
	vec3 color;
};

uniform sampler2D texture_diffuse1;
uniform sampler2D shadowMap[5];
uniform Light lights[5];

uniform int numLights;
uniform bool useShadow;
uniform vec3 ambientStrength;
uniform vec3 specularStrength;
uniform float n;
uniform vec3 viewPos; 

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap, vec3 lightDir)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    // vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), n);
    // combine results
    vec3 ambient = ambientStrength * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = diff * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 specular = specularStrength * spec * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    return (ambient + diffuse + specular) * light.color;
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec4 lightSpaceMatrix, sampler2D shadowMap)
{
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), n);
    // combine results
    vec3 ambient = ambientStrength * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 diffuse = diff * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    vec3 specular = specularStrength * spec * vec3(texture(texture_diffuse1, fs_in.TexCoords));
    // shadow
    float shadow = 0;
    if(useShadow){
        shadow = ShadowCalculation(lightSpaceMatrix, shadowMap, lightDir);   
    }
    // shadow = 0.9;
    vec3 result = (ambient + (1.0 - shadow) * (diffuse + specular)) * light.color;   
    return result;
}

void main()
{           
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 result = vec3(0.0, 0.0, 0.0);
    for(int i=0; i<numLights; ++i){
        Light light = lights[i];
        int type = light.type;
        if(1 == type) { // point light
            PointLight pointLight;
            pointLight.position = light.attrib;
            pointLight.color = light.color;
            result += CalcPointLight(pointLight, normal, fs_in.FragPos, viewDir, fs_in.FragPosLightSpace[i], shadowMap[i]);
        }
    }
    FragColor = vec4(result, 1.0);
}