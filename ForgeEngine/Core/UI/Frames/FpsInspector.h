#pragma once
#include "Core/Layer/Layer.h"

namespace ForgeEngine
{
    class FpsInspector : public Layer
    {
    public:
        FpsInspector();
        ~FpsInspector() = default;

        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& event) override;
    };
}
