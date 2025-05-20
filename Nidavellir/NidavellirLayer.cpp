#include "NidavellirLayer.h"

#include "imgui.h"
#include "Core/Renderer/RenderCommand.h"

namespace ForgeEngine
{
    void NidavellirLayer::OnAttach()
    {
        Layer::OnAttach();
    }

    void NidavellirLayer::OnDetach()
    {
        Layer::OnDetach();
    }

    void NidavellirLayer::OnUpdate(Timestep ts)
    {

        color_[3] -= ts.GetMilliseconds() * threshold_;

        if (color_[3] <= 0.f)
        {
            color_ = {0.0f, 0.0f, 0.0f, 1.0f};
            color_[count_ % 3] = 1.0f;
            count_++;
        }

        new_color_[0] = color_[0];
        new_color_[1] = color_[1];
        new_color_[2] = color_[2];
        new_color_[3] = color_[3];
        RenderCommand::SetClearColor(color_);



        Layer::OnUpdate(ts);
    }

    void NidavellirLayer::OnImGuiRender()
    {
        Layer::OnImGuiRender();
        ImGui::Begin("Nidavellir Layer");
        ImGui::DragFloat4("Color",new_color_,0.0,1.0);
        ImGui::Text("Nidavellir Layer");
        ImGui::End();
    }

    void NidavellirLayer::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
    }
}
