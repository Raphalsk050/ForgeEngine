// TIMESTAMP: 2025-05-10-20:55:00 - FORÇAR RECARREGAMENTO DE CACHE
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec2 a_TexCoord;
layout(location = 4) in int a_EntityID;

layout(std140, binding = 0) uniform Camera
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

layout(std140, binding = 2) uniform Transform
{
    mat4 u_Transform;
};

// Use explicit locations for each output component to avoid location overlap
layout(location = 0) out vec3 v_WorldPosition;
layout(location = 1) out vec3 v_Normal;
layout(location = 2) out vec2 v_TexCoord;
layout(location = 3) out vec3 v_ViewPosition;
layout(location = 4) out mat3 v_TBN; // This will use locations 4, 5, 6 (one per column)
layout(location = 7) out flat int v_EntityID;

void main()
{
    v_WorldPosition = vec3(u_Transform * vec4(a_Position, 1.0));

    // Normal matrix for proper normal transformation (inverse transpose of model matrix)
    mat3 normalMatrix = transpose(inverse(mat3(u_Transform)));
    v_Normal = normalize(normalMatrix * a_Normal);

    // Calculate TBN matrix for normal mapping
    vec3 T = normalize(normalMatrix * a_Tangent);
    vec3 N = v_Normal;
    // Re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    // Calculate bitangent
    vec3 B = cross(N, T);
    v_TBN = mat3(T, B, N);

    v_TexCoord = a_TexCoord;
    v_ViewPosition = u_CameraPosition;
    v_EntityID = a_EntityID;

    gl_Position = u_ViewProjection * vec4(v_WorldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

// Match the locations used in the vertex shader for each input component
layout(location = 0) in vec3 v_WorldPosition;
layout(location = 1) in vec3 v_Normal;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in vec3 v_ViewPosition;
layout(location = 4) in mat3 v_TBN; // Uses locations 4, 5, 6
layout(location = 7) in flat int v_EntityID;

layout(std140, binding = 1) uniform Light
{
    vec3 u_PointLightPosition;
    float u_PointLightIntensity;
    vec3 u_AmbientLightColor;
    float u_AmbientLightIntensity;
};

// Material properties
layout(std140, binding = 3) uniform Material
{
    vec4 u_MaterialAlbedoColor;
    float u_MaterialMetallic;
    float u_MaterialRoughness;
    int u_EntityID;
};

// Texture samplers
layout(binding = 0) uniform sampler2D u_AlbedoMap;
layout(binding = 1) uniform sampler2D u_NormalMap;
layout(binding = 2) uniform sampler2D u_MetallicMap;
layout(binding = 3) uniform sampler2D u_RoughnessMap;

// Constants
const float PI = 3.14159265359;

// PBR functions
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / max(denom, 0.001);
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}

void main()
{
    // Sample textures
    vec4 albedo = texture(u_AlbedoMap, v_TexCoord) * u_MaterialAlbedoColor;
    vec3 normal = normalize(v_TBN * (texture(u_NormalMap, v_TexCoord).rgb * 2.0 - 1.0));
    float metallic = texture(u_MetallicMap, v_TexCoord).r * u_MaterialMetallic;
    float roughness = texture(u_RoughnessMap, v_TexCoord).r * u_MaterialRoughness;

    // If no normal map is provided, use the mesh normal
    if (normal.x == 0.0 && normal.y == 0.0 && normal.z == 0.0)
    normal = v_Normal;

    // PBR calculations
    vec3 N = normalize(normal);
    vec3 V = normalize(v_ViewPosition - v_WorldPosition);

    // Calculate reflectance at normal incidence (F0)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo.rgb, metallic);

    // Lighting calculation
    vec3 Lo = vec3(0.0);

    // Point light calculation
    vec3 L = normalize(u_PointLightPosition - v_WorldPosition);
    vec3 H = normalize(V + L);

    // Calculate point light attenuation
    float distance = length(u_PointLightPosition - v_WorldPosition);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = vec3(1.0, 1.0, 1.0) * attenuation * u_PointLightIntensity;

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 nominator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = nominator / max(denominator, 0.001);

    // Add to outgoing radiance Lo
    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo.rgb / PI + specular) * radiance * NdotL;

    // Ambient lighting
    vec3 ambient = u_AmbientLightColor * albedo.rgb * u_AmbientLightIntensity;

    vec3 color = ambient + Lo;

    // HDR tonemapping
    color = color / (color + vec3(1.0));
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));

    o_Color = vec4(color, albedo.a);
    o_EntityID = v_EntityID;

    // Discard transparent fragments
    if (o_Color.a < 0.01)
    discard;
}