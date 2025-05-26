#pragma once

#include "Core/Camera/Camera.h"
#include "Core/Camera/Camera3D.h"
#include "Core/Camera/Camera3DController.h"
#include "Core/Renderer/Texture.h"
#include "Core/Renderer/Mesh.h"
#include "Core/Renderer/Material.h"
#include "Core/Scene/Components.h"

#include <glm.hpp>

#include "glad/glad.h"

namespace ForgeEngine
{
    class EarlyDepthTestManager
    {
    public:

        static void Initialize()
        {
            glEnable(GL_DEPTH_TEST);

            glDepthFunc(GL_LESS);

            glDepthMask(GL_TRUE);

            glEnable(GL_CULL_FACE);

            glCullFace(GL_FRONT);

            glDepthRange(0.0, 1.0);

            glClearDepth(1.0);

            FENGINE_CORE_INFO("Early Depth Test initialized");
        }

        // ========================================================================
        // CONFIGURAÇÃO POR FRAME
        // ========================================================================
        static void BeginFrame()
        {
            // Limpar depth buffer no início do frame
            glClear(GL_DEPTH_BUFFER_BIT);

            // Garantir que depth test está habilitado
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
        }

        // ========================================================================
        // RENDERIZAÇÃO EM DUAS PASSADAS (OPCIONAL - MÁXIMA EFICIÊNCIA)
        // ========================================================================
        static void BeginDepthPrePass()
        {
            // PRIMEIRA PASSADA: Só escrever depth, sem fragment shader complexo

            // Desabilitar escrita de cor
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

            // Habilitar escrita de depth
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);

            FENGINE_CORE_TRACE("Starting depth pre-pass");
        }

        static void EndDepthPrePass()
        {
            // Re-habilitar escrita de cor
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

            FENGINE_CORE_TRACE("Depth pre-pass complete");
        }

        static void BeginColorPass()
        {
            // SEGUNDA PASSADA: Renderizar cor com depth test, mas sem escrever depth

            // Manter depth test ativo, mas não escrever
            glDepthMask(GL_FALSE); // Não escrever depth
            glDepthFunc(GL_LEQUAL); // Aceitar pixels com depth <= existing

            FENGINE_CORE_TRACE("Starting color pass");
        }

        static void EndColorPass()
        {
            // Restaurar configurações padrão
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);

            FENGINE_CORE_TRACE("Color pass complete");
        }

        // ========================================================================
        // CONFIGURAÇÕES PARA DIFERENTES TIPOS DE OBJETOS
        // ========================================================================
        static void ConfigureForOpaqueObjects()
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_TRUE);
            glDepthFunc(GL_LESS);
            glDisable(GL_BLEND);
        }

        static void ConfigureForTransparentObjects()
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE); // Transparentes não escrevem depth
            glDepthFunc(GL_LESS);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        }

        static void ConfigureForSkybox()
        {
            glEnable(GL_DEPTH_TEST);
            glDepthMask(GL_FALSE);
            glDepthFunc(GL_LEQUAL); // Skybox sempre no fundo
        }

        // ========================================================================
        // DEBUG E DIAGNÓSTICO
        // ========================================================================
        static void DebugDepthBuffer()
        {
            GLboolean depthTest, depthMask;
            GLint depthFunc;
            GLfloat depthRange[2];

            glGetBooleanv(GL_DEPTH_TEST, &depthTest);
            glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
            glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
            glGetFloatv(GL_DEPTH_RANGE, depthRange);

            FENGINE_CORE_INFO("=== DEPTH BUFFER STATUS ===");
            FENGINE_CORE_INFO("Depth Test: {}", depthTest ? "ENABLED" : "DISABLED");
            FENGINE_CORE_INFO("Depth Mask: {}", depthMask ? "WRITE" : "READ_ONLY");
            FENGINE_CORE_INFO("Depth Func: {}", GetDepthFuncName(depthFunc));
            FENGINE_CORE_INFO("Depth Range: [{:.3f}, {:.3f}]", depthRange[0], depthRange[1]);
        }

    private:
        static const char* GetDepthFuncName(GLint func)
        {
            switch (func)
            {
            case GL_NEVER: return "NEVER";
            case GL_LESS: return "LESS";
            case GL_EQUAL: return "EQUAL";
            case GL_LEQUAL: return "LEQUAL";
            case GL_GREATER: return "GREATER";
            case GL_NOTEQUAL: return "NOTEQUAL";
            case GL_GEQUAL: return "GEQUAL";
            case GL_ALWAYS: return "ALWAYS";
            default: return "UNKNOWN";
            }
        }
    };

    class Renderer3D
    {
    public:
        // ========================================================================
        // ESTRUTURA DE ESTATÍSTICAS
        // ========================================================================
        struct Statistics
        {
            uint32_t DrawCalls = 0;
            uint32_t MeshCount = 0;
            uint32_t VisibleMeshCount = 0;
            uint32_t CulledMeshCount = 0;
            uint32_t VertexCount = 0;
            uint32_t IndexCount = 0;
        };

        // ========================================================================
        // INICIALIZAÇÃO E CICLO DE VIDA
        // ========================================================================
        static void Init();
        static void Shutdown();

        // ========================================================================
        // CONTROLE DE CENA
        // ========================================================================
        static void BeginScene(const Camera& camera, const glm::mat4& transform);
        static void BeginScene(const Camera3D& camera);
        static void BeginScene(const Camera3DController& cameraController);
        static void EndScene();

        // ========================================================================
        // CONTROLE DE BATCH
        // ========================================================================
        static void StartBatch();
        static void Flush();
        static void NextBatch();

        // ========================================================================
        // FUNÇÕES DE CULLING
        // ========================================================================
        static bool IsPointVisible(const glm::vec3& point);
        static bool IsSphereVisible(const glm::vec3& center, float radius);
        static bool IsAABBVisible(const glm::vec3& min, const glm::vec3& max);
        static bool IsEntityVisible(int entityID, const glm::mat4& transform, float boundingSphereRadius);

        // ========================================================================
        // RENDERIZAÇÃO DE MESHES GENÉRICAS
        // ========================================================================

        // Renderização com cor
        static void DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                             const glm::vec4& color, int entityID = -1);
        static void DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                             const glm::vec3& rotation, Ref<Mesh> mesh,
                             const glm::vec4& color, int entityID = -1);

        // Renderização com material
        static void DrawMesh(const glm::mat4& transform, Ref<Mesh> mesh,
                             Ref<Material> material, int entityID = -1);
        static void DrawMesh(const glm::vec3& position, const glm::vec3& scale,
                             const glm::vec3& rotation, Ref<Mesh> mesh,
                             Ref<Material> material, int entityID = -1);

        // ========================================================================
        // RENDERIZAÇÃO DE PRIMITIVAS - CUBO
        // ========================================================================
        static void DrawCube(const glm::vec3& position, const glm::vec3& size,
                             const glm::vec4& color, int entityID = -1);
        static void DrawCube(const glm::vec3& position, const glm::vec3& size,
                             Ref<Material> material, int entityID = -1);
        static void DrawCube(const glm::mat4& transform, const glm::vec4& color,
                             int entityID = -1);
        static void DrawCube(const glm::mat4& transform, Ref<Material> material,
                             int entityID = -1);

        // ========================================================================
        // RENDERIZAÇÃO DE PRIMITIVAS - ESFERA
        // ========================================================================
        static void DrawSphere(const glm::vec3& position, float radius,
                               const glm::vec4& color, int entityID = -1);
        static void DrawSphere(const glm::vec3& position, float radius,
                               Ref<Material> material, int entityID = -1);
        static void DrawSphere(const glm::mat4& transform, const glm::vec4& color,
                               int entityID = -1);
        static void DrawSphere(const glm::mat4& transform, Ref<Material> material,
                               int entityID = -1);

        // ========================================================================
        // RENDERIZAÇÃO DE DEBUG - LINHAS E CAIXAS
        // ========================================================================
        static void DrawLine3D(const glm::vec3& p0, const glm::vec3& p1,
                               const glm::vec4& color, int entityID = -1);
        static void DrawBox(const glm::vec3& position, const glm::vec3& size,
                            const glm::vec4& color, int entityID = -1);
        static void DrawBox(const glm::mat4& transform, const glm::vec4& color,
                            int entityID = -1);

        // ========================================================================
        // RENDERIZAÇÃO DE MODELOS
        // ========================================================================
        static void DrawModel(const glm::mat4& transform, ModelRendererComponent& src,
                              int entityID = -1);

        // ========================================================================
        // CONFIGURAÇÕES DE ILUMINAÇÃO
        // ========================================================================
        static void SetPointLightPosition(const glm::vec3& position);
        static void SetAmbientLight(const glm::vec3& color, float intensity);

        // ========================================================================
        // CONFIGURAÇÕES DE RENDERIZAÇÃO
        // ========================================================================
        static void EnableWireframe(bool enable);
        static bool IsWireframeEnabled();

        // ========================================================================
        // ESTATÍSTICAS E DEBUG
        // ========================================================================
        static void ResetStats();
        static Statistics GetStats();

        // Estatísticas específicas de culling
        static uint32_t GetTotalMeshCount();
        static uint32_t GetVisibleMeshCount();
        static uint32_t GetCulledMeshCount();
        static float GetCullingEfficiency(); // Retorna % de objetos cortados

        // ========================================================================
        // FUNÇÕES DE DEBUG DO CULLING
        // ========================================================================
        static void DebugCulling();
        static void RecalculateEntityBounds(int entityID);
        static void ClearCullingData();

        // ========================================================================
        // FUNÇÕES AUXILIARES (LEGACY)
        // ========================================================================
        static void PreparePrimitives();

    private:
        // ========================================================================
        // FUNÇÕES INTERNAS
        // ========================================================================

        // Função helper para culling centralizado
        friend bool PerformCulling(int entityID, const glm::mat4& transform, float* outBoundingRadius);

        // Função interna de renderização (sem culling)
        friend void DrawMeshInternal(const glm::mat4& transform, Ref<Mesh> mesh,
                                     Ref<Material> material, int entityID);
    };
} // namespace ForgeEngine
