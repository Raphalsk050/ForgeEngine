#include "FpsInspector.h"

#include "imgui.h"
#include "Core/Time.h"
#include "Core/Application/Application.h"

namespace ForgeEngine
{
    FpsInspector::FpsInspector(): opened_(false)
    {
        frames_ = std::vector<float>(max_frame_values_amount_window_);
        debug_name_ = "FpsInspector";
    }

    FpsInspector::~FpsInspector() = default;

    void FpsInspector::OnAttach()
    {
        Layer::OnAttach();
    }

    void FpsInspector::OnDetach()
    {
        Layer::OnDetach();
    }

    void FpsInspector::OnUpdate(Timestep ts)
    {
        Layer::OnUpdate(ts);
        // only updates after update_tick_ms_
        int app_time_ms = Time::GetTime() * 1000.0f;
        if (app_time_ms % update_tick_ms_ != 0) return;

        float frame = round(1.0f / ts.GetSeconds());
        float avgFPS = (last_frame_amount_ + frame) / 2.0f;
        last_frame_amount_ = frame;
        UpdateFPSHistory(avgFPS);
    }

    void FpsInspector::OnImGuiRender()
    {
        Layer::OnImGuiRender();

        PlotGraph();
    }

    void FpsInspector::OnEvent(Event& event)
    {
        Layer::OnEvent(event);
    }

    void FpsInspector::Open()
    {
        opened_ = true;
    }

    void FpsInspector::Close()
    {
        opened_ = false;
    }

    void FpsInspector::UpdateFPSHistory(const float frame)
    {
        if (frames_.size() > max_frame_values_amount_window_)
            frames_.erase(frames_.begin());
        frames_.push_back(frame);
    }

    void FpsInspector::PlotGraph()
    {
        ImGui::SetNextWindowSizeConstraints(window_size_, window_size_);
        ImGui::Begin("FPSHistory", nullptr, window_flags);

        if (!frames_.empty())
        {
            std::vector<float> fps_smooth = SmoothValues(frames_, 1.0f);
            ImGui::BeginGroup();
            int last_fps_amount = static_cast<int>(fps_smooth.back());
            int first_fps_amount = static_cast<int>(fps_smooth.front());

            const char* avg_fps_text = std::to_string((last_fps_amount + first_fps_amount)/2).c_str();

            ImGui::PlotLines("##", fps_smooth.data(), max_frame_values_amount_window_, 0, nullptr, 0.0f, 2000.0f, plot_size_);

            ImGui::SetNextItemWidth(plot_size_.x);
            ImGui::SliderInt("##", &max_frame_values_amount_window_, 30, 50);

            ImGui::BeginGroup();

            ImGui::Text("0");
            ImGui::SameLine(plot_size_.x - ImGui::CalcTextSize(avg_fps_text).x);
            ImGui::Text(avg_fps_text);

            ImGui::EndGroup();
            ImGui::EndGroup();
        }

        ImGui::End();
    }

    std::vector<float> FpsInspector::SmoothValues(const std::vector<float>& values, float alpha)
    {
        std::vector<float> smooth_values(values.size());

        if (values.empty())
            return smooth_values;

        smooth_values[0] = values[0];

        for (size_t i = 1; i < values.size(); ++i)
        {
            smooth_values[i] = smooth_values[i - 1] + alpha * (values[i] - smooth_values[i - 1]);
        }

        return smooth_values;
    }
}
