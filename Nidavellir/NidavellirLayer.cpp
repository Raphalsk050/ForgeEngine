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

        mesh_ = TestMesh::CreateTriangle();
        camera_controller_.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        camera_controller_.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));


        // Configurar threshold de instancing (quantos objetos mínimos para usar instancing)
        Renderer3D::SetInstancingThreshold(instancing_threshold_);

        // Habilitar instancing automático
        Renderer3D::EnableAutoInstancing(auto_instancing_enabled_);
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

        auto x = sin(time_) * 10.0f;
        auto z = cos(time_) * 10.0f;
        auto max_value = 90.0f;
        auto t = (sin(time_) + 1.0f) / 2.0f;
        auto fov = glm::mix(80.0, 90.0, glm::smoothstep(0.0f, 1.0f, t));
        glm::vec3 position = glm::vec3(x, 2.0f, z);

        Renderer3D::EnableWireframe(wireframe_enabled_);
        position = glm::vec3(x, 0.0f, z);

        Renderer3D::ResetStats();
        {
            RenderCommand::SetClearColor({0.01, 0.01, 0.01, 1.0f});
            RenderCommand::Clear();
        }

        Renderer3D::BeginScene(camera_controller_.GetCamera());
        Renderer3D::SetAmbientLight(glm::vec3(1.0f), 0.2);
        Renderer3D::SetPointLightPosition(position);

        // Sphere to show where the point light is being positioned
        Renderer3D::DrawSphere(position, 0.2f, glm::vec4(1.0), 0);

        // Floor plane
        Renderer3D::DrawMesh(glm::vec3(0.0f, -2.0, 0.0), glm::vec3(100.0f), glm::vec3(0.0f), Mesh::CreatePlane(), glm::vec4(1.0), 1);

        EarlyDepthTestManager::DebugDepthBuffer();


        for (int i = 0; i < cube_count_; i++)
        {
            float angle = (float)i / (float)cube_count_ * 2.0f * 3.14159f;
            int a = i % 10;
            float radius = 10.0f + a;
            glm::vec3 cubePos = glm::vec3(
                cos(angle) * radius,
                1.0f,
                sin(angle) * radius
            );

            glm::vec4 cubeColor = glm::vec4(
                abs(sin(angle)),
                0.5f,
                abs(cos(angle)),
                1.0f
            );

            Renderer3D::DrawCube(cubePos, glm::vec3(0.5f), cubeColor, i);
        }


        for (int i = 0; i < sphere_count_; i++)
        {
            float angle = (float)i / (float)sphere_count_ * 2.0f * 3.14159f;
            int a = i % 20;
            float radius = 3.0f + a;
            glm::vec3 spherePos = glm::vec3(
                cos(angle) * radius,
                1.0f,
                sin(angle) * radius
            );

            glm::vec4 sphereColor = glm::vec4(
                0.8f,
                abs(sin(angle)),
                abs(cos(angle)),
                1.0f
            );

            // float sphereRadius = 0.3f + sin(time_ + i) * 0.2f;
            float sphereRadius = 0.3f;
            Renderer3D::DrawSphere(spherePos, sphereRadius, sphereColor, 2000 + i);
        }

        Renderer3D::EndScene();
    }

    void NidavellirLayer::OnImGuiRender()
    {
        RenderHelperUI();

        if (render_debug_ui_enabled_)
            RenderPerformancePanel();

        if (render_debug_options_ui_enabled_)
            RenderInstancingControlPanel();

        Layer::OnImGuiRender();


        if (debug_frame_ % 120 == 0)
        {
            Renderer3D::DebugCulling();
            Renderer3D::DebugInstancing();
            camera_controller_.GetCamera().DebugFrustum();
        }
    }

    bool NidavellirLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        bool pressed = (e.IsRepeat() || !e.IsRepeat());
        auto KeyCode = e.GetKeyCode();

        if (!last_key_state_map_.contains(KeyCode))
            last_key_state_map_[KeyCode] = false;


        switch (KeyCode)
        {
        case Key::Space:
            {
                if (!last_key_state_map_[KeyCode])
                {
                    wireframe_enabled_ = !wireframe_enabled_;
                    last_key_state_map_[KeyCode] = true;
                }

                break;
            }

        case Key::Tab:
            {
                if (!last_key_state_map_[KeyCode])
                {
                    camera_movement_enabled_ = !camera_movement_enabled_;
                    last_key_state_map_[KeyCode] = true;
                }

                break;
            }

        case Key::I:
            {
                if (!last_key_state_map_[KeyCode])
                {
                    auto_instancing_enabled_ = !auto_instancing_enabled_;
                    Renderer3D::EnableAutoInstancing(auto_instancing_enabled_);
                    last_key_state_map_[KeyCode] = true;
                }

                break;
            }
        }

        return false;
    }

    bool NidavellirLayer::OnKeyReleased(KeyReleasedEvent& e)
    {
        auto KeyCode = e.GetKeyCode();

        switch (KeyCode)
        {
        case Key::Space:
            {
                last_key_state_map_[KeyCode] = false;
                break;
            }

        case Key::Tab:
            {
                last_key_state_map_[KeyCode] = false;
                break;
            }

        case Key::I:
            {
                last_key_state_map_[KeyCode] = false;
                break;
            }
        }


        return false;
    }

    void NidavellirLayer::RenderPerformancePanel()
    {
        ImGui::Begin("Rendering Statistics");

        auto stats = Renderer3D::GetStats();

        // General statistics
        ImGui::Text("=== General Stats ===");
        ImGui::Text("Total Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("Vertex Count: %d", stats.VertexCount);
        ImGui::Text("Index Count: %d", stats.IndexCount);
        ImGui::Text("Mesh Count: %d", stats.MeshCount);

        ImGui::Separator();

        // Culling statistics
        ImGui::Text("=== Culling Stats ===");
        ImGui::Text("Total Meshes: %d", Renderer3D::GetTotalMeshCount());
        ImGui::Text("Visible Meshes: %d", stats.VisibleMeshCount);
        ImGui::Text("Culled Meshes: %d", stats.CulledMeshCount);
        ImGui::Text("Culling Efficiency: %.2f%%", Renderer3D::GetCullingEfficiency());

        ImGui::Separator();

        ImGui::Text("=== Instancing Stats ===");
        ImGui::Text("Instanced Draw Calls: %d", stats.InstancedDrawCalls);
        ImGui::Text("Individual Draw Calls: %d", stats.IndividualDrawCalls);
        ImGui::Text("Total Instances: %d", stats.TotalInstances);
        ImGui::Text("Instanced Objects: %d", stats.InstancedObjects);
        ImGui::Text("Individual Objects: %d", stats.IndividualObjects);
        ImGui::Text("Instancing Efficiency: %.2f%%", stats.InstancingEfficiency);

        ImGui::Separator();

        // performance analysis
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
    }

    void NidavellirLayer::RenderInstancingControlPanel()
    {
        ImGui::Begin("Nidavellir Layer Demo");

        ImGui::Text("Nidavellir Layer with Automatic Instancing");
        ImGui::Separator();
        ImGui::Text("Instancing Controls");

        if (ImGui::Checkbox("Auto Instancing Enabled", &auto_instancing_enabled_))
        {
            Renderer3D::EnableAutoInstancing(auto_instancing_enabled_);
        }

        if (ImGui::SliderInt("Instancing Threshold", reinterpret_cast<int*>(&instancing_threshold_), 1, 10))
        {
            Renderer3D::SetInstancingThreshold(instancing_threshold_);
        }

        ImGui::Separator();

        ImGui::Text("Object Density Controls");

        ImGui::SliderInt("Grid Size", &grid_size_, 2, max_entities_);
        ImGui::SliderInt("Cube Count", &cube_count_, 0, max_entities_);
        ImGui::SliderInt("Sphere Count", &sphere_count_, 0, max_entities_);

        ImGui::Text("Total Objects: %d", (grid_size_ * grid_size_) + cube_count_ + sphere_count_ + 2);

        ImGui::Separator();

        ImGui::Text("Controls:");
        ImGui::BulletText("Space: Toggle Wireframe");
        ImGui::BulletText("I: Toggle Auto Instancing");
        ImGui::BulletText("ESC: Exit Application");

        ImGui::End();

    }

    void NidavellirLayer::RenderHelperUI()
    {
        ImGui::Begin("Helper UI");

        ImGui::Separator();
        ImGui::Text("Render Options");
        if (ImGui::Button("Enable WireFrame", {200,25}))
        {
            wireframe_enabled_ = !wireframe_enabled_;
        }
        ImGui::Separator();
        ImGui::Separator();

        ImGui::Text("Debug Options");
        if (ImGui::Button("Enable Render statics", {200,25}))
        {
            render_debug_ui_enabled_ = !render_debug_ui_enabled_;
        }

        if (ImGui::Button("Enable Render debug options", {200,25}))
        {
            render_debug_options_ui_enabled_ = !render_debug_options_ui_enabled_;
        }

        ImGui::Separator();

        ImGui::End();
    }

    void NidavellirLayer::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
        if (camera_movement_enabled_)
        {
            camera_controller_.OnEvent(event);
        }

        EventDispatcher dispatcher(event);
        dispatcher.Dispatch<KeyPressedEvent>(FENGINE_BIND_EVENT_FN(OnKeyPressed));
        dispatcher.Dispatch<KeyReleasedEvent>(FENGINE_BIND_EVENT_FN(OnKeyReleased));
    }
}
