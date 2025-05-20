#pragma once
#include "Core/Layer/Layer.h"
#include <glm.hpp>

namespace ForgeEngine
{
    class NidavellirLayer : public Layer
    {
    public:
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& event) override;

        private:
        float alpha_ = 1.0f;
        float threshold_ = 0.001f;
        int count_ = 0;
        float new_color_[4] = {};
        glm::vec4 color_ = glm::vec4{1.0f,0.0f,0.0f,1.0f};
    };
}
