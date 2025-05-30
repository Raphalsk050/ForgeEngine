#include "NidavellirLayer.h"
#include "imgui.h"
#include "Core/Application/Application.h"
#include "Core/Input/Input.h"
#include "Core/Renderer/RenderCommand.h"
#include "Core/Renderer/Renderer3D.h"
#include "Core/Renderer/TestMesh.h"
#include "GLFW/glfw3.h"
#include "glad/glad.h"

#include "ext/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "Platform/OpenGL/OpenGLBuffer.h"
#include "Platform/OpenGL/OpenGLShader.h"

namespace ForgeEngine
{
    NidavellirLayer::NidavellirLayer(): Layer("GenericLayer"),
                                        square_color_({0.2f, 0.3f, 0.8f, 1.0f})
    {
        auto height = Application::Get().GetWindow().GetHeight();
        auto width = Application::Get().GetWindow().GetWidth();
        camera_controller_ = Camera3DController(width / height);
    }

    void NidavellirLayer::OnAttach()
    {
        Layer::OnAttach();
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);

        mesh_ = TestMesh::CreateCylinder();
        camera_controller_.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        camera_controller_.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));

        // ========================================================================
        // CONFIGURAÇÃO DO SISTEMA DE INSTANCING
        // ========================================================================

        // Configurar threshold de instancing (quantos objetos mínimos para usar instancing)
        Renderer3D::SetInstancingThreshold(instancing_threshold_);

        // Habilitar instancing automático
        Renderer3D::EnableAutoInstancing(auto_instancing_enabled_);

        FENGINE_CORE_INFO("NidavellirLayer initialized with instancing system");
        FENGINE_CORE_INFO("Instancing threshold: {}", instancing_threshold_);
        FENGINE_CORE_INFO("Auto instancing: {}", auto_instancing_enabled_ ? "ENABLED" : "DISABLED");
    }

    void NidavellirLayer::OnDetach()
    {
        Layer::OnDetach();
    }

    void NidavellirLayer::OnUpdate(Timestep ts)
    {
        camera_controller_.OnUpdate(ts);
        debug_frame_++;
        time_ += ts;

        auto x = sin(time_);
        auto z = cos(time_);
        auto max_value = 90.0f;
        auto t = (sin(time_) + 1.0f) / 2.0f;
        auto fov = glm::mix(80.0, 90.0, glm::smoothstep(0.0f, 1.0f, t));
        glm::vec3 position = glm::vec3(x, 2.0f, z);

        Renderer3D::EnableWireframe(pressed_);
        position = glm::vec3(x, 0.0f, z);

        if (Input::IsKeyPressed(Key::Space) && time_ >= previous_clicked_time_ + 0.5)
        {
            previous_clicked_time_ = time_;
            pressed_ = !pressed_;
        }

        // Toggle instancing com 'I'
        if (Input::IsKeyPressed(Key::I) && time_ >= previous_instancing_toggle_time_ + 0.5)
        {
            previous_instancing_toggle_time_ = time_;
            auto_instancing_enabled_ = !auto_instancing_enabled_;
            Renderer3D::EnableAutoInstancing(auto_instancing_enabled_);
            FENGINE_CORE_INFO("Auto instancing toggled: {}", auto_instancing_enabled_ ? "ENABLED" : "DISABLED");
        }

        Renderer3D::ResetStats();
        {
            RenderCommand::SetClearColor({0.01, 0.01, 0.01, 1.0f});
            RenderCommand::Clear();
        }

        Renderer3D::BeginScene(camera_controller_.GetCamera());
        Renderer3D::SetAmbientLight(glm::vec3(1.0f), 0.2);
        Renderer3D::SetPointLightPosition(position);

        // Renderizar luz como esfera pequena
        Renderer3D::DrawSphere(position, 0.2f, glm::vec4(1.0), 0);

        // Renderizar plano do chão
        Renderer3D::DrawMesh(glm::vec3(0.0f,-2.0,0.0),glm::vec3(100.0f),glm::vec3(0.0f),Mesh::CreatePlane(),glm::vec4(1.0), 1);

        EarlyDepthTestManager::DebugDepthBuffer();

        // ========================================================================
        // TESTE INTENSIVO DE INSTANCING - MUITOS OBJETOS IDÊNTICOS
        // ========================================================================

        int amount = grid_size_;
        glEnable(GL_DEPTH_TEST);

        for (int i = 0; i < amount; i++)
        {
            for (int j = 0; j < amount; j++)
            {
                auto size = glm::vec3(1.0);
                float height = sin(i + time_) + cos(j + time_);

                position = glm::vec3(size.x * i, height, size.z * j);

                // Usar a mesma mesh (cilindro) para demonstrar instancing
                glm::vec4 color = glm::vec4(
                    abs(sin(i + time_)),
                    abs(cos(j + time_)),
                    abs(sin(i + j + time_)),
                    1.0f
                );

                Renderer3D::DrawMesh(position, glm::vec3(1.0f), glm::vec3(0.0f),
                                   mesh_, color, i * amount + j);
            }
        }

        // ========================================================================
        // TESTE COM CUBOS PRIMITIVOS (também deve usar instancing)
        // ========================================================================

        for (int i = 0; i < cube_count_; i++)
        {
            float angle = (float)i / (float)cube_count_ * 2.0f * 3.14159f;
            float radius = 10.0f;
            glm::vec3 cubePos = glm::vec3(
                cos(angle + time_) * radius,
                sin(time_ * 2.0f + i) * 2.0f,
                sin(angle + time_) * radius
            );

            glm::vec4 cubeColor = glm::vec4(
                abs(sin(angle + time_)),
                0.5f,
                abs(cos(angle + time_)),
                1.0f
            );

            Renderer3D::DrawCube(cubePos, glm::vec3(0.5f), cubeColor, 1000 + i);
        }

        // ========================================================================
        // TESTE COM ESFERAS (mais instâncias da mesma geometria)
        // ========================================================================

        for (int i = 0; i < sphere_count_; i++)
        {
            float angle = (float)i / (float)sphere_count_ * 2.0f * 3.14159f;
            float radius = 15.0f;
            glm::vec3 spherePos = glm::vec3(
                cos(angle - time_) * radius,
                sin(time_ + i) * 3.0f + 5.0f,
                sin(angle - time_) * radius
            );

            glm::vec4 sphereColor = glm::vec4(
                0.8f,
                abs(sin(angle - time_)),
                abs(cos(angle - time_)),
                1.0f
            );

            float sphereRadius = 0.3f + sin(time_ + i) * 0.2f;
            Renderer3D::DrawSphere(spherePos, sphereRadius, sphereColor, 2000 + i);
        }

        Renderer3D::EndScene();
    }

    void NidavellirLayer::OnImGuiRender()
    {
        Layer::OnImGuiRender();

        // ========================================================================
        // PAINEL PRINCIPAL DE CONFIGURAÇÃO
        // ========================================================================
        ImGui::Begin("Nidavellir Layer - Instancing Demo");

        ImGui::Text("Nidavellir Layer with Automatic Instancing");
        ImGui::Separator();

        // Controles da aplicação
        ImGui::DragFloat4("Color", glm::value_ptr(square_color_), 0.0, 1.0);

        ImGui::Separator();

        // ========================================================================
        // CONTROLES DE INSTANCING
        // ========================================================================
        ImGui::Text("Instancing Controls");

        if (ImGui::Checkbox("Auto Instancing Enabled", &auto_instancing_enabled_))
        {
            Renderer3D::EnableAutoInstancing(auto_instancing_enabled_);
            FENGINE_CORE_INFO("Auto instancing set to: {}", auto_instancing_enabled_);
        }

        if (ImGui::SliderInt("Instancing Threshold", reinterpret_cast<int*>(&instancing_threshold_), 1, 10))
        {
            Renderer3D::SetInstancingThreshold(instancing_threshold_);
            FENGINE_CORE_INFO("Instancing threshold set to: {}", instancing_threshold_);
        }

        ImGui::Separator();

        // ========================================================================
        // CONTROLES DE DENSIDADE DE OBJETOS
        // ========================================================================
        ImGui::Text("Object Density Controls");

        ImGui::SliderInt("Grid Size", &grid_size_, 2, 20);
        ImGui::SliderInt("Cube Count", &cube_count_, 0, 100);
        ImGui::SliderInt("Sphere Count", &sphere_count_, 0, 100);

        ImGui::Text("Total Objects: %d", (grid_size_ * grid_size_) + cube_count_ + sphere_count_ + 2);

        ImGui::Separator();

        // ========================================================================
        // INFORMAÇÕES DE CONTROLE
        // ========================================================================
        ImGui::Text("Controls:");
        ImGui::BulletText("Space: Toggle Wireframe");
        ImGui::BulletText("I: Toggle Auto Instancing");
        ImGui::BulletText("ESC: Exit Application");

        ImGui::End();

        // ========================================================================
        // PAINEL DE ESTATÍSTICAS EXPANDIDO
        // ========================================================================
        ImGui::Begin("Rendering Statistics");

        auto stats = Renderer3D::GetStats();

        // Estatísticas gerais
        ImGui::Text("=== General Stats ===");
        ImGui::Text("Total Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Vertex Count: %d", stats.VertexCount);
        ImGui::Text("Index Count: %d", stats.IndexCount);
        ImGui::Text("Mesh Count: %d", stats.MeshCount);

        ImGui::Separator();

        // Estatísticas de culling
        ImGui::Text("=== Culling Stats ===");
        ImGui::Text("Total Meshes: %d", Renderer3D::GetTotalMeshCount());
        ImGui::Text("Visible Meshes: %d", stats.VisibleMeshCount);
        ImGui::Text("Culled Meshes: %d", stats.CulledMeshCount);
        ImGui::Text("Culling Efficiency: %.2f%%", Renderer3D::GetCullingEfficiency());

        ImGui::Separator();

        // ========================================================================
        // ESTATÍSTICAS DE INSTANCING (NOVAS)
        // ========================================================================
        ImGui::Text("=== Instancing Stats ===");
        ImGui::Text("Instanced Draw Calls: %d", stats.InstancedDrawCalls);
        ImGui::Text("Individual Draw Calls: %d", stats.IndividualDrawCalls);
        ImGui::Text("Total Instances: %d", stats.TotalInstances);
        ImGui::Text("Instanced Objects: %d", stats.InstancedObjects);
        ImGui::Text("Individual Objects: %d", stats.IndividualObjects);
        ImGui::Text("Instancing Efficiency: %.2f%%", stats.InstancingEfficiency);

        ImGui::Separator();

        // Análise de performance
        uint32_t totalObjects = stats.InstancedObjects + stats.IndividualObjects;
        uint32_t totalDrawCalls = stats.InstancedDrawCalls + stats.IndividualDrawCalls;
        uint32_t drawCallsWithoutInstancing = totalObjects;

        ImGui::Text("=== Performance Analysis ===");
        ImGui::Text("Objects Rendered: %d", totalObjects);
        ImGui::Text("Actual Draw Calls: %d", totalDrawCalls);
        ImGui::Text("Draw Calls Without Instancing: %d", drawCallsWithoutInstancing);

        if (drawCallsWithoutInstancing > 0)
        {
            float reduction = (float)(drawCallsWithoutInstancing - totalDrawCalls) / (float)drawCallsWithoutInstancing * 100.0f;
            ImGui::Text("Draw Call Reduction: %.2f%%", reduction);

            // Colorir baseado na eficiência
            if (reduction > 50.0f)
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Excellent Performance!");
            else if (reduction > 25.0f)
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Good Performance");
            else if (reduction > 0.0f)
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Some Improvement");
            else
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Instancing Used");
        }

        ImGui::End();

        // Debug (chame ocasionalmente, não a cada frame)
        if (debug_frame_ % 120 == 0) // A cada 120 frames (menos frequente)
        {
            Renderer3D::DebugCulling();
            Renderer3D::DebugInstancing(); // Nova função de debug
            camera_controller_.GetCamera().DebugFrustum();
        }
    }

    void NidavellirLayer::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
        camera_controller_.OnEvent(event);
    }
}