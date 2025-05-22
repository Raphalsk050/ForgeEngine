#pragma once

#include "Core/camera/camera.h"
#include "Core/camera/Camera3DController.h"
#include "Core/Renderer/material.h"
#include "Core/Renderer/Mesh.h"
#include "Core/Renderer/Texture.h"
#include "Core/scene/Components.h"

namespace ForgeEngine
{
    class Renderer3D
    {
    public:
        static void Init();
        static void Shutdown();

        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const Camera3D& camera);
        static void BeginScene(const Camera3DController& cameraController);
        static void EndScene();
        static void Flush();

        static bool IsPointVisible(const glm::vec3& point);
        static bool IsSphereVisible(const glm::vec3& center, float radius);
        static bool IsAABBVisible(const glm::vec3& min, const glm::vec3& max);
        static bool IsEntityVisible(int entityID, const glm::mat4& transform,
                                    float boundingSphereRadius);

        // Primitives
        static void DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                             const glm::vec4& color, int entityID = -1);
        static void DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                             Ref<Material> material, int entityID = -1);
        static void DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                             const glm::vec3& rotation, Ref<Mesh> mesh,
                             const glm::vec4& color, int entityID = -1);
        static void DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                             const glm::vec3& rotation, Ref<Mesh> mesh,
                             Ref<Material> material, int entityID = -1);

        // Basic primitives
        static void DrawCube(const glm::vec3& position, const glm::vec3& size,
                             const glm::vec4& color, int entityID = -1);
        static void DrawCube(const glm::vec3& position, const glm::vec3& size,
                             Ref<Material> material, int entityID = -1);
        static void DrawCube(const glm::mat4& transform, const glm::vec4& color,
                             int entityID = -1);
        static void DrawCube(const glm::mat4& transform, Ref<Material> material,
                             int entityID = -1);

        static void DrawSphere(const glm::vec3& position, float radius,
                               const glm::vec4& color, int entityID = -1);
        static void DrawSphere(const glm::vec3& position, float radius,
                               Ref<Material> material, int entityID = -1);
        static void DrawSphere(const glm::mat4& transform, const glm::vec4& color,
                               int entityID = -1);
        static void DrawSphere(const glm::mat4& transform, Ref<Material> material,
                               int entityID = -1);

        // Debug
        static void DrawLine3D(const glm::vec3& p0, const glm::vec3& p1,
                               const glm::vec4& color, int entityID = -1);
        static void DrawBox(const glm::vec3& position, const glm::vec3& size,
                            const glm::vec4& color, int entityID = -1);
        static void DrawBox(const glm::mat4& transform, const glm::vec4& color,
                            int entityID = -1);

        // Components
        static void DrawModel(const glm::mat4& transform, ModelRendererComponent& src,
                              int entityID);

        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t MeshCount = 0;
            uint32_t VisibleMeshCount = 0;
            uint32_t CulledMeshCount = 0;
            uint32_t VertexCount = 0;
            uint32_t IndexCount = 0;

            // Culling efficiency as a percentage
            float GetCullingEfficiency() const
            {
                if (MeshCount == 0) return 0.0f;
                return (float)CulledMeshCount / (float)MeshCount * 100.0f;
            }
        };

        static void ResetStats();
        static Statistics GetStats();

        // Settings
        static void SetPointLightPosition(const glm::vec3& position);
        static void SetAmbientLight(const glm::vec3& color, float intensity = 0.1f);
        static void EnableWireframe(bool enable);
        static bool IsWireframeEnabled();

    private:
        static void StartBatch();
        static void NextBatch();
        static void PreparePrimitives();
    };
} // namespace ForgeEngine
