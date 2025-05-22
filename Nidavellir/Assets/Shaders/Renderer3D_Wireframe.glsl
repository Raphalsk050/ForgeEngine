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

// Movendo o uniforme para um bloco uniforme adequado
layout(std140, binding = 2) uniform Transform
{
    mat4 u_Transform;
};

layout(location = 0) out flat int v_EntityID;

void main()
{
    vec3 worldPosition = vec3(u_Transform * vec4(a_Position, 1.0));
    v_EntityID = a_EntityID;
    gl_Position = u_ViewProjection * vec4(worldPosition, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int o_EntityID;

layout(location = 0) in flat int v_EntityID;

// Movendo o uniforme para um bloco uniforme adequado
layout(std140, binding = 3) uniform Material
{
    vec4 u_Color;
};

void main()
{
    o_Color = u_Color;
    o_EntityID = v_EntityID;
}