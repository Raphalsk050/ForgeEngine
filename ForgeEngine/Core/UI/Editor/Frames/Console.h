#pragma once
#include "imgui.h"
#include "Core/Layer/Layer.h"
#include "Core/Log/Felog.h"
#include "spdlog/sinks/ringbuffer_sink.h"

namespace ForgeEngine
{
    class Console : public Layer
    {
    public:
        Console();
        ~Console() override;
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& event) override;

        void OpenConsole();
        void CloseConsole();

    private:
        bool open_;
        std::shared_ptr<spdlog::sinks::ringbuffer_sink_mt> log_sink_;
        ImGuiWindowFlags window_flags_ = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoTitleBar;
    };
}
