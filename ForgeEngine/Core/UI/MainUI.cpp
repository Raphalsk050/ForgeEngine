#include "MainUI.h"

#include <iostream>

#include "Core/Application/Application.h"
#include "Frames/Console.h"

namespace ForgeEngine
{
    MainUI::MainUI(): io_(nullptr), console_(nullptr)
    {
    }

    void MainUI::OnAttach()
    {
        Layer::OnAttach();
        io_ = &ImGui::GetIO();

        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        io_->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        // creates the main engine ui. This creates a UI docker space to attach other ui layers.
        window_flags_ |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags_ |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        window_flags_ |= ImGuiWindowFlags_NoBackground;
        window_flags_ |= ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    }

    void MainUI::OnDetach()
    {
        Layer::OnDetach();
    }

    void MainUI::OnUpdate(Timestep ts)
    {
        Layer::OnUpdate(ts);
    }

    void MainUI::OnImGuiRender()
    {
        Layer::OnImGuiRender();

        if (!io_) {
            return;
        }

        ConfigureDockSpace();
    }

    void MainUI::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
    }

    void MainUI::ConfigureDockSpace()
    {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->GetWorkPos());
        ImGui::SetNextWindowSize(viewport->GetWorkSize());
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

        bool dockspace_open_dummy = true;
        ImGui::Begin("MainEngineDockspace", &dockspace_open_dummy, window_flags_);

        ImGui::PopStyleVar(3);

        ImGuiID dockspace_id = ImGui::GetID("PrimaryDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

        RenderTopMenuOptions();

        ImGui::End();
    }

    void MainUI::RenderTopMenuOptions()
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("Window"))
            {
                if (ImGui::MenuItem("Console"))
                {
                    OpenConsole();
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }
    }

    void MainUI::OpenConsole()
    {
        console_ = new Console();
        Application::Get().PushLayer(console_);
        console_->OpenConsole();
    }
}
