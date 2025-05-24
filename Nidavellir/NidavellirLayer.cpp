#include "NidavellirLayer.h"
#include "imgui.h"
#include "Core/Application/Application.h"
#include "Core/Renderer/RenderCommand.h"
#include "Core/Renderer/Renderer3D.h"
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
        camera_controller_ = Camera3DController(width/height);
    }


    void NidavellirLayer::OnAttach()
    {
        Layer::OnAttach();
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glEnable(GL_DEPTH_TEST);
        Renderer3D::EnableWireframe(false);
    }

    void NidavellirLayer::OnDetach()
    {
        Layer::OnDetach();
    }

    void NidavellirLayer::OnUpdate(Timestep ts)
    {
        camera_controller_.OnUpdate(ts);
        time_ += ts;
        Renderer3D::ResetStats();
        {
            RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1});
            RenderCommand::Clear();
        }

        Renderer3D::BeginScene(camera_controller_.GetCamera());
        auto x = sin(time_);
        auto z = cos(time_);
        auto position = glm::vec3(x,0.0f,z);
        //Renderer3D::SetAmbientLight(glm::vec4(x,0.0f,z,1.0f), 10.0);
        Renderer3D::SetPointLightPosition(position);

        // Renderer3D::DrawLine3D(position, {0.5f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f});
        Renderer3D::DrawSphere(glm::vec3(0.0f), 1.0f, glm::vec4(1.0));
        Renderer3D::DrawSphere(position, 0.1f, glm::vec4(1.0));
        // Renderer3D::DrawSphere(glm::vec3(0.0f), 10.0f, glm::vec4(1.0));
        // Renderer3D::DrawCube(glm::vec3(0.0f),glm::vec3(1.0f),glm::vec4(1.0));

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
        ImGui::End();
    }

    void NidavellirLayer::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
        camera_controller_.OnEvent(event);
    }
}
