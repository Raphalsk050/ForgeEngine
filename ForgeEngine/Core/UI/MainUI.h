#pragma once
#include <imgui.h>
#include "Core/Layer/Layer.h"
#include "Frames/FpsInspector.h"

namespace ForgeEngine
{
    class Console;
}

namespace ForgeEngine
{
    class MainUI : public Layer
    {
    public:
        MainUI();
        ~MainUI() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& event) override;

    private:
        ImGuiWindowFlags window_flags_ = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiIO* io_;
        void ConfigureDockSpace();
        void RenderTopMenuOptions();
        void OpenConsole();

        void OpenFPSHistory();

        Console* console_;
        FpsInspector* fps_inspector_;

    };
}
