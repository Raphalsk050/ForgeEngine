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
#define FENGINE_DEBUG_FRUSTUM

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
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        mesh_ = TestMesh::CreateCylinder();
        camera_controller_.SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        camera_controller_.SetRotation(glm::vec3(0.0f, 0.0f, 0.0f));
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

        //camera_controller_.GetCamera().SetFov(fov);
        position = glm::vec3(x, 0.0f, z);

        if (Input::IsKeyPressed(Key::Space) && time_ >= previous_clicked_time_ + 0.5)
        {
            previous_clicked_time_ = time_;
            pressed_ = !pressed_;
        }

        Renderer3D::ResetStats();
        {
            RenderCommand::SetClearColor({0.01, 0.01, 0.01, 1.0f});
            RenderCommand::Clear();
        }

        Renderer3D::BeginScene(camera_controller_.GetCamera());
        Renderer3D::SetAmbientLight(glm::vec3(1.0f), 0.2);
        Renderer3D::SetPointLightPosition(position);

        // Renderer3D::DrawLine3D(position, {0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
        // Renderer3D::DrawSphere(glm::vec3(0.0f), 1.0f, glm::vec4(1.0));
        // Renderer3D::DrawSphere(position, 0.1f, glm::vec4(1.0));
        // Renderer3D::DrawCube(glm::vec3(0.0f),glm::vec3(1.0f),glm::vec4(1.0,0.0,0.0,1.0));
        Renderer3D::DrawSphere(position, 0.2f, glm::vec4(1.0), 0);
        Renderer3D::DrawMesh(glm::vec3(0.0f,-2.0,0.0),glm::vec3(100.0f),glm::vec3(0.0f),Mesh::CreatePlane(),glm::vec4(1.0), 1);


        int amount = 4;
        glEnable(GL_DEPTH_TEST);

        for (int i = -amount / 2; i < amount / 2; i++)
        {
            for (int j = -amount / 2; j < amount / 2; j++)
            {
                auto size = glm::vec3(1.0);
                float height = sin(i + time_) + cos(j + time_);

                position = glm::vec3(size.x * i, height, size.z * j);
                Renderer3D::DrawMesh(position, glm::vec3(1.0f), glm::vec3(0.0f), mesh_, glm::vec4(float(i), 1.0, float(j), 1.0f), i + amount + 1);
            }
        }

        Renderer3D::EndScene();
    }

    void NidavellirLayer::OnImGuiRender()
    {
        Layer::OnImGuiRender();
        ImGui::Begin("Nidavellir Layer");
        //ImGui::DragFloat("Velocity", &velocity_, 0.001,-1.0,1.0);
        ImGui::DragFloat4("Color", glm::value_ptr(square_color_), 0.0, 1.0);
        ImGui::DragFloat3("Color", glm::value_ptr(quad_size_), 0.0, 1.0);
        ImGui::SliderFloat3("Camera position", const_cast<float*>(glm::value_ptr(camera_controller_.GetPosition())), -1.0f, 1.0f, "%.3f");
        ImGui::Text("Nidavellir Layer");

        ImGui::Begin("Statistics");
        ImGui::Text("Draw calls: %d", Renderer3D::GetStats().DrawCalls);
        ImGui::Text("Culled Mesh Count: %d", Renderer3D::GetStats().CulledMeshCount);
        ImGui::Text("Index Count: %d", Renderer3D::GetStats().IndexCount);
        ImGui::Text("Vertex Count: %d", Renderer3D::GetStats().VertexCount);
        ImGui::Text("Mesh Count: %d", Renderer3D::GetStats().MeshCount);
        ImGui::Text("Visible Mesh Count: %d", Renderer3D::GetStats().VisibleMeshCount);
        ImGui::Text("Total Mesh Count: %d", Renderer3D::GetTotalMeshCount());
        ImGui::Text("Culling Efficiency: %f", Renderer3D::GetCullingEfficiency());
        ImGui::End();

        ImGui::End();

        // Debug (chame ocasionalmente, não a cada frame)
        if (debug_frame_ % 60 == 0)
        {
            // A cada 60 frames
            Renderer3D::DebugCulling();
            camera_controller_.GetCamera().DebugFrustum();
        }
    }

    void NidavellirLayer::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
        camera_controller_.OnEvent(event);
    }
}
