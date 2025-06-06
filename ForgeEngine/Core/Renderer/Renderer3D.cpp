#include "Core/Renderer/Renderer3D.h"
#include "Config.h"
#include "Core/Renderer/InstancedRenderer.h"
#include "Core/Renderer/RenderCommand.h"
#include "Core/Renderer/Shader.h"
#include "Core/Renderer/UniformBuffer.h"
#include "Core/Renderer/VertexArray.h"
#include "glad/glad.h"
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <unordered_map>

namespace ForgeEngine
{
    struct MeshVertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec3 Tangent;
        glm::vec2 TexCoord;
        glm::vec4 Color;
        int EntityID;
    };

    struct LineVertex3D
    {
        glm::vec3 Position;
        glm::vec4 Color;
        int EntityID;
    };

    struct Renderer3DData
    {
        static constexpr uint32_t MaxVertices = 100000;
        static constexpr uint32_t MaxIndices = 200000;
        static constexpr uint32_t MaxTextureSlots = 32;

        // Instancing configuration
        uint32_t InstancingThreshold
            = 3; // Minimum objects to enable instancing
        bool AutoInstancingEnabled = true;

        // Collection of render items for batching
        std::vector<Renderer3D::RenderItem> RenderQueue;

        // Mapping mesh+material -> items for efficient batching
        std::unordered_map<std::string, std::vector<Renderer3D::RenderItem>>
        MeshBatches;

        // Reference to the InstancedRenderer
        std::unique_ptr<InstancedRenderer> InstanceRenderer;

        // Active camera for frustum culling
        const Camera3D* ActiveCamera = nullptr;

        // Entity culling info (stores bounding volumes)
        struct EntityCullingData
        {
            glm::vec3 BoundingBoxMin;
            glm::vec3 BoundingBoxMax;
            float BoundingSphereRadius;
            bool WasVisible = true;
        };

        std::unordered_map<int, EntityCullingData> EntityCullingInfo;

        // Tracks visible and total entities for culling stats
        uint32_t VisibleMeshCount = 0;
        uint32_t TotalMeshCount = 0;

        // Statistics of the last frame
        Renderer3D::Statistics LastFrameStats;

        // Meshes
        Ref<Shader> MeshShader;
        Ref<Shader> WireframeShader;

        // Debug rendering
        Ref<VertexArray> LineVertexArray;
        Ref<VertexBuffer> LineVertexBuffer;
        Ref<Shader> LineShader;
        uint32_t LineVertexCount = 0;
        LineVertex3D* LineVertexBufferBase = nullptr;
        LineVertex3D* LineVertexBufferPtr = nullptr;
        float LineWidth = 2.0f;

        // Primitive meshes (basic geometry)
        Ref<Mesh> CubeMesh;
        Ref<Mesh> SphereMesh;

        // Default material (used when mesh/material is missing)
        Ref<Material> DefaultMaterial;

        // Texture slots
        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture
        Ref<Texture2D> WhiteTexture;

        // Lighting parameters
        glm::vec3 PointLightPosition = {1.0f, 1.0f, 0.0f};
        glm::vec3 AmbientLightColor = {1.0f, 1.0f, 1.0f};
        float AmbientLightIntensity = 0.0f;

        // Renderer settings
        bool WireframeMode = false;

        // Camera uniform buffer data
        struct CameraData
        {
            glm::mat4 ViewProjection;
            glm::vec3 CameraPosition;
            float _padding1;
        };

        CameraData CameraBuffer{};
        Ref<UniformBuffer> CameraUniformBuffer;

        // Lighting uniform buffer data
        struct LightData
        {
            glm::vec3 PointLightPosition;
            float PointLightIntensity = 1.0f;
            glm::vec3 AmbientLightColor;
            float AmbientLightIntensity;
        };

        LightData LightBuffer{};
        Ref<UniformBuffer> LightUniformBuffer;

        // Frame statistics (for profiling/monitoring)
        Renderer3D::Statistics Stats;
    };

    static Renderer3DData s_Data;

    // Creates a transform matrix from position, scale and Euler rotation (XYZ
    // order)
    glm::mat4 CreateTransformMatrix(const glm::vec3& position,
                                    const glm::vec3& scale,
                                    const glm::vec3& rotation)
    {
        glm::mat4 transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::translate(transformMatrix, position);
        transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.z),
                                      glm::vec3(0.0f, 0.0f, 1.0f));
        transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.y),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
        transformMatrix = glm::rotate(transformMatrix, glm::radians(rotation.x),
                                      glm::vec3(1.0f, 0.0f, 0.0f));
        transformMatrix = glm::scale(transformMatrix, scale);
        return transformMatrix;
    }

    // Performs frustum culling for an entity based on its transform and
    // bounding sphere
    bool PerformCulling(int entityID, const glm::mat4& transform,
                        float* outBoundingRadius = nullptr)
    {
        if (!s_Data.ActiveCamera || entityID < 0)
        {
            if (s_Data.ActiveCamera == nullptr)
            {
                FENGINE_CORE_ASSERT(false, "No Active Camera!")
                FENGINE_CORE_ERROR("No Active Camera!");
            }
            if (entityID < 0)
            {
                FENGINE_CORE_ASSERT(false, "EntityID is invalid!")
                FENGINE_CORE_ERROR("EntityID is invalid!");
            }
            return true;
        }

        auto& cullingData = s_Data.EntityCullingInfo[entityID];

        // Calculate bounding sphere radius if not set yet
        if (cullingData.BoundingSphereRadius == 0.0f)
        {
            glm::vec3 scale;
            scale.x = glm::length(glm::vec3(transform[0]));
            scale.y = glm::length(glm::vec3(transform[1]));
            scale.z = glm::length(glm::vec3(transform[2]));
            float maxScale = glm::max(glm::max(scale.x, scale.y), scale.z);
            cullingData.BoundingSphereRadius
                = maxScale * 0.866f; // ~sqrt(3)/2 for a cube
        }

        s_Data.TotalMeshCount++;

        bool isVisible = Renderer3D::IsEntityVisible(
            entityID, transform, cullingData.BoundingSphereRadius);

        if (isVisible)
        {
            s_Data.VisibleMeshCount++;
            cullingData.WasVisible = true;
        }
        else
        {
            cullingData.WasVisible = false;
        }

        if (outBoundingRadius)
        {
            *outBoundingRadius = cullingData.BoundingSphereRadius;
        }

        return isVisible;
    }

    // Internal function to handle mesh/material binding and draw call
    void DrawMeshInternal(const glm::mat4& transform, Ref<Mesh> mesh,
                          Ref<Material> material, int entityID)
    {
        // Bind material textures, fallback to white texture if missing
        if (material->GetAlbedoMap())
            material->GetAlbedoMap()->Bind(0);
        else
            s_Data.WhiteTexture->Bind(0);

        if (material->GetNormalMap())
            material->GetNormalMap()->Bind(1);
        else
            s_Data.WhiteTexture->Bind(1);

        if (material->GetMetallicMap())
            material->GetMetallicMap()->Bind(2);
        else
            s_Data.WhiteTexture->Bind(2);

        if (material->GetRoughnessMap())
            material->GetRoughnessMap()->Bind(3);
        else
            s_Data.WhiteTexture->Bind(3);

        // Switch between wireframe and standard shader
        if (s_Data.WireframeMode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            s_Data.WireframeShader->Bind();
            s_Data.WireframeShader->SetMat4("u_Transform", transform);
            s_Data.WireframeShader->SetFloat4("u_Color",
                                              material->GetAlbedoColor());
            s_Data.WireframeShader->SetInt("u_EntityID", entityID);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            s_Data.MeshShader->Bind();
            s_Data.MeshShader->SetMat4("u_Transform", transform);
            s_Data.MeshShader->SetFloat4("u_MaterialAlbedoColor",
                                         material->GetAlbedoColor());
            s_Data.MeshShader->SetFloat("u_MaterialMetallic",
                                        material->GetMetallic());
            s_Data.MeshShader->SetFloat("u_MaterialRoughness",
                                        material->GetRoughness());
            s_Data.MeshShader->SetInt("u_AlbedoMap", 0);
            s_Data.MeshShader->SetFloat4("u_NormalMap",
                                         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            s_Data.MeshShader->SetFloat4("u_MetallicMap",
                                         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            s_Data.MeshShader->SetFloat4("u_RoughnessMap",
                                         glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            s_Data.MeshShader->SetInt("u_EntityID", entityID);
        }

        // Bind VAO and issue draw call
        mesh->GetVertexArray()->Bind();
        RenderCommand::DrawIndexed(mesh->GetVertexArray(),
                                   mesh->GetIndexCount());

        s_Data.Stats.DrawCalls++;
        s_Data.Stats.IndividualDrawCalls++;
        s_Data.Stats.VertexCount += mesh->GetVertexCount();
        s_Data.Stats.IndexCount += mesh->GetIndexCount();
    }

    void Renderer3D::Init()
    {
        FENGINE_PROFILE_FUNCTION();
        EarlyDepthTestManager::Initialize();

        s_Data.MeshShader = Shader::Create(
            "../ForgeEngine/Assets/Shaders/Renderer3D_Mesh.glsl");
        s_Data.WireframeShader = Shader::Create(
            "../ForgeEngine/Assets/Shaders/Renderer3D_Wireframe.glsl");
        s_Data.LineShader = Shader::Create(
            "../ForgeEngine/Assets/Shaders/Renderer3D_Line.glsl");

        if (s_Data.MeshShader == nullptr)
            FENGINE_CORE_CRITICAL("Mesh shader not found");

        // Initialize InstancedRenderer
        s_Data.InstanceRenderer = std::make_unique<InstancedRenderer>();
        s_Data.InstanceRenderer->Init();

        FENGINE_CORE_INFO(
            "Instanced rendering system initialized with threshold: {}",
            s_Data.InstancingThreshold);

        // Create line rendering resources
        s_Data.LineVertexArray = VertexArray::Create();
        s_Data.LineVertexBuffer = VertexBuffer::Create(
            Renderer3DData::MaxVertices * sizeof(LineVertex3D));
        s_Data.LineVertexBuffer->SetLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Int, "a_EntityID"},
        });
        s_Data.LineVertexArray->AddVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase
            = new LineVertex3D[Renderer3DData::MaxVertices];

        // Create white texture
        s_Data.WhiteTexture = Texture2D::Create(TextureSpecification());
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        // Create primitive meshes
        s_Data.CubeMesh = Mesh::CreateCube(1.0f);
        s_Data.SphereMesh = Mesh::CreateSphere(0.5f, 16, 16);

        FENGINE_CORE_INFO("Primitive meshes created:");
        FENGINE_CORE_INFO("  Cube: {} vertices, {} indices",
                          s_Data.CubeMesh->GetVertexCount(),
                          s_Data.CubeMesh->GetIndexCount());
        FENGINE_CORE_INFO("  Sphere: {} vertices, {} indices",
                          s_Data.SphereMesh->GetVertexCount(),
                          s_Data.SphereMesh->GetIndexCount());

        // Create default material
        s_Data.DefaultMaterial = CreateRef<Material>();
        s_Data.DefaultMaterial->SetAlbedoMap(s_Data.WhiteTexture);
        s_Data.DefaultMaterial->SetRoughness(0.5f);
        s_Data.DefaultMaterial->SetMetallic(0.0f);

        // Create uniform buffers
        s_Data.CameraUniformBuffer
            = UniformBuffer::Create(sizeof(Renderer3DData::CameraData), 0);
        s_Data.LightUniformBuffer
            = UniformBuffer::Create(sizeof(Renderer3DData::LightData), 1);

        // Set default lighting data
        s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
        s_Data.LightBuffer.PointLightIntensity = 1.0f;
        s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
        s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
        s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                           sizeof(Renderer3DData::LightData));

        FENGINE_CORE_INFO("Renderer3D initialized successfully with hybrid "
            "instancing system");
    }

    void Renderer3D::Shutdown()
    {
        FENGINE_PROFILE_FUNCTION();

        // Shutdown instanced renderer
        if (s_Data.InstanceRenderer)
        {
            s_Data.InstanceRenderer->Shutdown();
            s_Data.InstanceRenderer.reset();
        }

        delete[] s_Data.LineVertexBufferBase;
    }

    void Renderer3D::BeginScene(const Camera& camera,
                                const glm::mat4& transform)
    {
        FENGINE_PROFILE_FUNCTION();
        EarlyDepthTestManager::BeginFrame();

        s_Data.CameraBuffer.ViewProjection
            = camera.GetProjection() * glm::inverse(transform);
        s_Data.CameraBuffer.CameraPosition
            = glm::vec3(glm::inverse(transform)[3]);
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer,
                                            sizeof(Renderer3DData::CameraData));

        // Update lighting data
        s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
        s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
        s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
        s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                           sizeof(Renderer3DData::LightData));

        s_Data.ActiveCamera = nullptr;
        StartBatch();
    }

    void Renderer3D::BeginScene(const Camera3D& camera)
    {
        FENGINE_PROFILE_FUNCTION();
        EarlyDepthTestManager::BeginFrame();

        s_Data.CameraBuffer.ViewProjection = camera.GetViewProjection();
        s_Data.CameraBuffer.CameraPosition = camera.GetPosition();
        s_Data.CameraUniformBuffer->SetData(&s_Data.CameraBuffer,
                                            sizeof(Renderer3DData::CameraData));

        // Update lighting data
        s_Data.LightBuffer.PointLightPosition = s_Data.PointLightPosition;
        s_Data.LightBuffer.AmbientLightColor = s_Data.AmbientLightColor;
        s_Data.LightBuffer.AmbientLightIntensity = s_Data.AmbientLightIntensity;
        s_Data.LightUniformBuffer->SetData(&s_Data.LightBuffer,
                                           sizeof(Renderer3DData::LightData));

        s_Data.ActiveCamera = &camera;
        StartBatch();
    }

    void Renderer3D::BeginScene(const Camera3DController& cameraController)
    {
        BeginScene(cameraController.GetCamera());
    }

    void Renderer3D::EndScene()
    {
        FENGINE_PROFILE_FUNCTION();

        s_Data.Stats.MeshCount = s_Data.TotalMeshCount;
        s_Data.Stats.VisibleMeshCount = s_Data.VisibleMeshCount;
        s_Data.Stats.CulledMeshCount
            = s_Data.TotalMeshCount - s_Data.VisibleMeshCount;

        ProcessBatches();

        Flush();

        if (s_Data.Stats.TotalInstances > 0)
        {
            uint32_t drawCallsWithoutInstancing
                = s_Data.Stats.IndividualDrawCalls
                + s_Data.Stats.TotalInstances;
            uint32_t actualDrawCalls = s_Data.Stats.IndividualDrawCalls
                + s_Data.Stats.InstancedDrawCalls;
            s_Data.Stats.InstancingEfficiency
                = (float)(drawCallsWithoutInstancing - actualDrawCalls)
                / (float)drawCallsWithoutInstancing * 100.0f;
        }
    }

    void Renderer3D::StartBatch()
    {
        // Clear previous frame's data
        s_Data.RenderQueue.clear();
        s_Data.MeshBatches.clear();

        // Reset line rendering
        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;
        s_Data.TextureSlotIndex = 1;

        // Reset counters
        s_Data.VisibleMeshCount = 0;
        s_Data.TotalMeshCount = 0;

        // Reset instanced renderer stats
        if (s_Data.InstanceRenderer) { s_Data.InstanceRenderer->ResetStats(); }
    }

    void Renderer3D::ProcessBatches()
    {
        FENGINE_PROFILE_FUNCTION();

        if (!s_Data.AutoInstancingEnabled)
        {
            // If auto instancing is disabled, render everything individually
            for (const auto& item : s_Data.RenderQueue)
            {
                RenderIndividualItem(item);
            }
            return;
        }

        // Group items by mesh and material
        for (const auto& item : s_Data.RenderQueue)
        {
            std::string meshKey = GetMeshKey(item.MeshPtr, item.MaterialPtr);
            s_Data.MeshBatches[meshKey].push_back(item);
        }

        // Process each group
        for (const auto& [meshKey, items] : s_Data.MeshBatches)
        {
            if (ShouldUseInstancing(items)) { RenderInstancedBatch(items); }
            else
            {
                for (const auto& item : items) { RenderIndividualItem(item); }
            }
        }
    }

    void Renderer3D::RenderInstancedBatch(const std::vector<RenderItem>& items)
    {
        FENGINE_PROFILE_FUNCTION();

        if (items.empty() || !s_Data.InstanceRenderer) return;

        // Prepare data for instancing
        std::vector<glm::mat4> transforms;
        std::vector<glm::vec4> colors;
        std::vector<int> entityIDs;

        transforms.reserve(items.size());
        colors.reserve(items.size());
        entityIDs.reserve(items.size());

        for (const auto& item : items)
        {
            transforms.push_back(item.Transform);
            colors.push_back(item.Color);
            entityIDs.push_back(item.EntityID);
        }

        // Use InstancedRenderer
        s_Data.InstanceRenderer->DrawInstancedMesh(transforms, items[0].MeshPtr,
                                                   colors, entityIDs);

        // Update statistics
        s_Data.Stats.InstancedDrawCalls++;
        s_Data.Stats.TotalInstances += items.size();
        s_Data.Stats.InstancedObjects += items.size();

#ifdef FENGINE_SHADER_DEBUG
        FENGINE_CORE_TRACE(
            "Rendered {} instances of mesh {} in single draw call",
            items.size(),
            GetMeshKey(items[0].MeshPtr, items[0].MaterialPtr));
#endif

    }

    void Renderer3D::RenderIndividualItem(const RenderItem& item)
    {
        FENGINE_PROFILE_FUNCTION();

        Ref<Material> material = item.MaterialPtr;
        if (!material)
        {
            // Use default material with specified color
            s_Data.DefaultMaterial->SetAlbedoColor(item.Color);
            material = s_Data.DefaultMaterial;
        }

        DrawMeshInternal(item.Transform, item.MeshPtr, material, item.EntityID);
        s_Data.Stats.IndividualObjects++;
    }

    bool Renderer3D::ShouldUseInstancing(const std::vector<RenderItem>& items)
    {
        return s_Data.AutoInstancingEnabled
            && items.size() >= s_Data.InstancingThreshold
            && items.size() <= s_Data.InstanceRenderer->GetMaxInstances();
    }

    std::string Renderer3D::GetMeshKey(Ref<Mesh> mesh, Ref<Material> material)
    {
        // Use mesh and material pointers as unique key
        uintptr_t meshPtr = reinterpret_cast<uintptr_t>(mesh.get());
        uintptr_t matPtr = reinterpret_cast<uintptr_t>(material.get());
        return std::to_string(meshPtr) + "_" + std::to_string(matPtr);
    }

    void Renderer3D::SubmitRenderItem(const RenderItem& item)
    {
        s_Data.RenderQueue.push_back(item);
    }

    void Renderer3D::DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                              const glm::vec4& color, int entityID)
    {
        FENGINE_PROFILE_FUNCTION();

        if (!PerformCulling(entityID, transform)) { return; }

        RenderItem item;
        item.Transform = transform;
        item.MeshPtr = mesh;
        item.MaterialPtr = nullptr; // Use color directly
        item.Color = color;
        item.EntityID = entityID;
        item.ItemType = RenderItem::Type::Mesh;

        SubmitRenderItem(item);
    }

    void Renderer3D::DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                              Ref<Material> material, int entityID)
    {
        FENGINE_PROFILE_FUNCTION();

        if (!PerformCulling(entityID, transform)) { return; }

        RenderItem item;
        item.Transform = transform;
        item.MeshPtr = mesh;
        item.MaterialPtr = material;
        item.Color = material ? material->GetAlbedoColor() : glm::vec4(1.0f);
        item.EntityID = entityID;
        item.ItemType = RenderItem::Type::Mesh;

        SubmitRenderItem(item);
    }

    void Renderer3D::DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                              const glm::vec3& rotation, Ref<Mesh> mesh,
                              const glm::vec4& color, int entityID)
    {
        glm::mat4 transform = CreateTransformMatrix(position, scale, rotation);
        DrawMesh(transform, mesh, color, entityID);
    }

    void Renderer3D::DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                              const glm::vec3& rotation, Ref<Mesh> mesh,
                              Ref<Material> material, int entityID)
    {
        glm::mat4 transform = CreateTransformMatrix(position, scale, rotation);
        DrawMesh(transform, mesh, material, entityID);
    }

    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size,
                              const glm::vec4& color, int entityID)
    {
        glm::mat4 transform
            = CreateTransformMatrix(position, size, glm::vec3(0.0f));
        DrawCube(transform, color, entityID);
    }

    void Renderer3D::DrawCube(const glm::vec3& position, const glm::vec3& size,
                              Ref<Material> material, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), size);
        DrawCube(transform, material, entityID);
    }

    void Renderer3D::DrawCube(const glm::mat4& transform,
                              const glm::vec4& color, int entityID)
    {
        DrawMesh(transform, s_Data.CubeMesh, color, entityID);
    }

    void Renderer3D::DrawCube(const glm::mat4& transform,
                              Ref<Material> material, int entityID)
    {
        DrawMesh(transform, s_Data.CubeMesh, material, entityID);
    }

    void Renderer3D::DrawSphere(const glm::vec3& position, float radius,
                                const glm::vec4& color, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
        DrawSphere(transform, color, entityID);
    }

    void Renderer3D::DrawSphere(const glm::vec3& position, float radius,
                                Ref<Material> material, int entityID)
    {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), glm::vec3(radius));
        DrawSphere(transform, material, entityID);
    }

    void Renderer3D::DrawSphere(const glm::mat4& transform,
                                const glm::vec4& color, int entityID)
    {
        DrawMesh(transform, s_Data.SphereMesh, color, entityID);
    }

    void Renderer3D::DrawSphere(const glm::mat4& transform,
                                Ref<Material> material, int entityID)
    {
        DrawMesh(transform, s_Data.SphereMesh, material, entityID);
    }

    void Renderer3D::DrawLine3D(const glm::vec3& p0, const glm::vec3& p1,
                                const glm::vec4& color, int entityID)
    {
        s_Data.LineVertexBufferPtr->Position = p0;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr->EntityID = entityID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexBufferPtr->Position = p1;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr->EntityID = entityID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexCount += 2;
    }

    void Renderer3D::DrawBox(const glm::vec3& position, const glm::vec3& size,
                             const glm::vec4& color, int entityID)
    {
        glm::vec3 halfSize = size * 0.5f;

        glm::vec3 p0
            = position + glm::vec3(-halfSize.x, -halfSize.y, -halfSize.z);
        glm::vec3 p1
            = position + glm::vec3(halfSize.x, -halfSize.y, -halfSize.z);
        glm::vec3 p2
            = position + glm::vec3(halfSize.x, -halfSize.y, halfSize.z);
        glm::vec3 p3
            = position + glm::vec3(-halfSize.x, -halfSize.y, halfSize.z);
        glm::vec3 p4
            = position + glm::vec3(-halfSize.x, halfSize.y, -halfSize.z);
        glm::vec3 p5
            = position + glm::vec3(halfSize.x, halfSize.y, -halfSize.z);
        glm::vec3 p6 = position + glm::vec3(halfSize.x, halfSize.y, halfSize.z);
        glm::vec3 p7
            = position + glm::vec3(-halfSize.x, halfSize.y, halfSize.z);

        // Bottom face
        DrawLine3D(p0, p1, color, entityID);
        DrawLine3D(p1, p2, color, entityID);
        DrawLine3D(p2, p3, color, entityID);
        DrawLine3D(p3, p0, color, entityID);

        // Top face
        DrawLine3D(p4, p5, color, entityID);
        DrawLine3D(p5, p6, color, entityID);
        DrawLine3D(p6, p7, color, entityID);
        DrawLine3D(p7, p4, color, entityID);

        // Connecting edges
        DrawLine3D(p0, p4, color, entityID);
        DrawLine3D(p1, p5, color, entityID);
        DrawLine3D(p2, p6, color, entityID);
        DrawLine3D(p3, p7, color, entityID);
    }

    void Renderer3D::DrawBox(const glm::mat4& transform, const glm::vec4& color,
                             int entityID)
    {
        glm::vec3 p0 = glm::vec3(-0.5f, -0.5f, -0.5f);
        glm::vec3 p1 = glm::vec3(0.5f, -0.5f, -0.5f);
        glm::vec3 p2 = glm::vec3(0.5f, -0.5f, 0.5f);
        glm::vec3 p3 = glm::vec3(-0.5f, -0.5f, 0.5f);
        glm::vec3 p4 = glm::vec3(-0.5f, 0.5f, -0.5f);
        glm::vec3 p5 = glm::vec3(0.5f, 0.5f, -0.5f);
        glm::vec3 p6 = glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec3 p7 = glm::vec3(-0.5f, 0.5f, 0.5f);

        p0 = transform * glm::vec4(p0, 1.0f);
        p1 = transform * glm::vec4(p1, 1.0f);
        p2 = transform * glm::vec4(p2, 1.0f);
        p3 = transform * glm::vec4(p3, 1.0f);
        p4 = transform * glm::vec4(p4, 1.0f);
        p5 = transform * glm::vec4(p5, 1.0f);
        p6 = transform * glm::vec4(p6, 1.0f);
        p7 = transform * glm::vec4(p7, 1.0f);

        // Bottom face
        DrawLine3D(p0, p1, color, entityID);
        DrawLine3D(p1, p2, color, entityID);
        DrawLine3D(p2, p3, color, entityID);
        DrawLine3D(p3, p0, color, entityID);

        // Top face
        DrawLine3D(p4, p5, color, entityID);
        DrawLine3D(p5, p6, color, entityID);
        DrawLine3D(p6, p7, color, entityID);
        DrawLine3D(p7, p4, color, entityID);

        // Connecting edges
        DrawLine3D(p0, p4, color, entityID);
        DrawLine3D(p1, p5, color, entityID);
        DrawLine3D(p2, p6, color, entityID);
        DrawLine3D(p3, p7, color, entityID);
    }

    void Renderer3D::DrawModel(const glm::mat4& transform,
                               ModelRendererComponent& src, int entityID)
    {
        if (!src.Model) return;

        for (const auto& mesh : src.Model->GetMeshes())
        {
            if (src.OverrideMaterial)
                DrawMesh(transform, mesh, src.OverrideMaterial, entityID);
            else if (mesh->GetMaterial())
                DrawMesh(transform, mesh, mesh->GetMaterial(), entityID);
            else
                DrawMesh(transform, mesh, s_Data.DefaultMaterial, entityID);
        }
    }

    void Renderer3D::Flush()
    {
        FENGINE_PROFILE_FUNCTION();

        if (s_Data.LineVertexCount)
        {
            uint32_t dataSize
                = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr
                    - (uint8_t*)s_Data.LineVertexBufferBase);
            s_Data.LineVertexBuffer->SetData(s_Data.LineVertexBufferBase,
                                             dataSize);

            s_Data.LineShader->Bind();
            RenderCommand::SetLineWidth(s_Data.LineWidth);
            RenderCommand::DrawLines(s_Data.LineVertexArray,
                                     s_Data.LineVertexCount);
            s_Data.Stats.DrawCalls++;
        }
    }

    void Renderer3D::NextBatch()
    {
        Flush();
        StartBatch();
    }

    void Renderer3D::PreparePrimitives()
    {
        s_Data.CubeMesh = Mesh::CreateCube();
        s_Data.SphereMesh = Mesh::CreateSphere();
    }

    void Renderer3D::SetInstancingThreshold(uint32_t threshold)
    {
        s_Data.InstancingThreshold = threshold;
        FENGINE_CORE_INFO("Instancing threshold set to: {}", threshold);
    }

    uint32_t Renderer3D::GetInstancingThreshold()
    {
        return s_Data.InstancingThreshold;
    }

    void Renderer3D::EnableAutoInstancing(bool enable)
    {
        s_Data.AutoInstancingEnabled = enable;
        FENGINE_CORE_INFO("Auto instancing {}",
                          enable ? "enabled" : "disabled");
    }

    bool Renderer3D::IsAutoInstancingEnabled()
    {
        return s_Data.AutoInstancingEnabled;
    }

    bool Renderer3D::IsPointVisible(const glm::vec3& point)
    {
        if (s_Data.ActiveCamera)
        {
            return s_Data.ActiveCamera->PointInFrustum(point);
        }
        return true;
    }

    bool Renderer3D::IsSphereVisible(const glm::vec3& center, float radius)
    {
        if (s_Data.ActiveCamera)
        {
            return s_Data.ActiveCamera->SphereInFrustum(center, radius);
        }
        return true;
    }

    bool Renderer3D::IsAABBVisible(const glm::vec3& min, const glm::vec3& max)
    {
        if (s_Data.ActiveCamera)
        {
            return s_Data.ActiveCamera->AABBInFrustum(min, max);
        }
        return true;
    }

    bool Renderer3D::IsEntityVisible(int entityID, const glm::mat4& transform,
                                     float boundingSphereRadius)
    {
        glm::vec3 position(transform[3][0], transform[3][1], transform[3][2]);

        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transform[0]));
        scale.y = glm::length(glm::vec3(transform[1]));
        scale.z = glm::length(glm::vec3(transform[2]));

        float maxScale = glm::max(glm::max(scale.x, scale.y), scale.z);
        float adjustedRadius = boundingSphereRadius * maxScale;

        return IsSphereVisible(position, adjustedRadius);
    }

    void Renderer3D::SetPointLightPosition(const glm::vec3& position)
    {
        s_Data.PointLightPosition = position;
    }

    void Renderer3D::SetAmbientLight(const glm::vec3& color, float intensity)
    {
        s_Data.AmbientLightColor = color;
        s_Data.AmbientLightIntensity = intensity;
    }

    void Renderer3D::EnableWireframe(bool enable)
    {
        s_Data.WireframeMode = enable;
    }

    bool Renderer3D::IsWireframeEnabled()
    {
        return s_Data.WireframeMode;
    }

    void Renderer3D::ResetStats()
    {
        s_Data.LastFrameStats = s_Data.Stats;

        memset(&s_Data.Stats, 0, sizeof(s_Data.Stats));
    }

    Renderer3D::Statistics Renderer3D::GetStats()
    {
        return s_Data.Stats;
    }

    uint32_t Renderer3D::GetTotalMeshCount()
    {
        return s_Data.TotalMeshCount;
    }

    uint32_t Renderer3D::GetVisibleMeshCount()
    {
        return s_Data.VisibleMeshCount;
    }

    uint32_t Renderer3D::GetCulledMeshCount()
    {
        return s_Data.TotalMeshCount - s_Data.VisibleMeshCount;
    }

    float Renderer3D::GetCullingEfficiency()
    {
        if (s_Data.TotalMeshCount == 0) return 0.0f;
        return (float)(s_Data.TotalMeshCount - s_Data.VisibleMeshCount)
            / (float)s_Data.TotalMeshCount * 100.0f;
    }

    float Renderer3D::GetInstancingEfficiency()
    {
        return s_Data.Stats.InstancingEfficiency;
    }

    uint32_t Renderer3D::GetInstancedObjectCount()
    {
        return s_Data.Stats.InstancedObjects;
    }

    uint32_t Renderer3D::GetIndividualObjectCount()
    {
        return s_Data.Stats.IndividualObjects;
    }

    void Renderer3D::DebugCulling()
    {
#ifdef FENGINE_CULLING_DEBUG
        FENGINE_CORE_INFO("=== CULLING DEBUG ===");
        FENGINE_CORE_INFO("Total meshes: {}", s_Data.TotalMeshCount);
        FENGINE_CORE_INFO("Visible meshes: {}", s_Data.VisibleMeshCount);
        FENGINE_CORE_INFO("Culled meshes: {}",
                          s_Data.TotalMeshCount - s_Data.VisibleMeshCount);
        FENGINE_CORE_INFO("Active camera: {}",
                          s_Data.ActiveCamera ? "YES" : "NO");

        if (s_Data.ActiveCamera) {
            glm::vec3 camPos = s_Data.ActiveCamera->GetPosition();
            FENGINE_CORE_INFO("Camera position: ({:.2f}, {:.2f}, {:.2f})",
                              camPos.x, camPos.y, camPos.z);
        }

        int count = 0;
        for (const auto& [entityID, cullingData]: s_Data.EntityCullingInfo) {
            if (count++ > 5) break;
            FENGINE_CORE_INFO("Entity {}: radius={:.2f}, visible={}", entityID,
                              cullingData.BoundingSphereRadius,
                              cullingData.WasVisible ? "YES" : "NO");
        }
#endif
    }

    void Renderer3D::DebugInstancing()
    {
#ifdef FENGINE_CULLING_DEBUG
        FENGINE_CORE_INFO("=== INSTANCING DEBUG ===");
        FENGINE_CORE_INFO("Auto instancing: {}", s_Data.AutoInstancingEnabled
                          ? "ENABLED"
                          : "DISABLED");
        FENGINE_CORE_INFO("Instancing threshold: {}",
                          s_Data.InstancingThreshold);
        FENGINE_CORE_INFO("Render queue size: {}", s_Data.RenderQueue.size());
        FENGINE_CORE_INFO("Mesh batches: {}", s_Data.MeshBatches.size());
        FENGINE_CORE_INFO("Instanced draw calls: {}",
                          s_Data.Stats.InstancedDrawCalls);
        FENGINE_CORE_INFO("Individual draw calls: {}",
                          s_Data.Stats.IndividualDrawCalls);
        FENGINE_CORE_INFO("Total instances: {}", s_Data.Stats.TotalInstances);
        FENGINE_CORE_INFO("Instancing efficiency: {:.2f}%",
                          s_Data.Stats.InstancingEfficiency);
#endif
    }

    void Renderer3D::RecalculateEntityBounds(int entityID)
    {
        auto it = s_Data.EntityCullingInfo.find(entityID);
        if (it != s_Data.EntityCullingInfo.end())
        {
            it->second.BoundingSphereRadius = 0.0f;
        }
    }

    void Renderer3D::ClearCullingData()
    {
        s_Data.EntityCullingInfo.clear();
    }
} // namespace ForgeEngine
