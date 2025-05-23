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
        auto window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        Layer::OnAttach();
    }

    void NidavellirLayer::OnDetach()
    {
        Layer::OnDetach();
    }

    void NidavellirLayer::OnUpdate(Timestep ts)
    {
        camera_controller_.OnUpdate(ts);

        Renderer3D::ResetStats();
        {
            RenderCommand::SetClearColor({0.2f, 0.2f, 0.2f, 1});
            RenderCommand::Clear();
        }

        Renderer3D::BeginScene(camera_controller_.GetCamera());
        // Draw a red line
        Renderer3D::DrawLine3D({-0.5f, 0.0f, -1.0f}, {0.5f, 0.0f, -1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, -1);
        // Draw a green cube
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), {0.0f, 0.0f, -1.0f});
        Renderer3D::DrawBox(transform, {0.0f, 1.0f, 0.0f, 1.0f}, -1);
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
