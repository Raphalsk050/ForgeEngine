#pragma once
#include "Core/Layer/Layer.h"
#include <glm.hpp>

#include "Core/Camera/Camera3DController.h"
#include "Core/Renderer/Mesh.h"

namespace ForgeEngine
{
    class NidavellirLayer : public Layer
    {
    public:
        NidavellirLayer();
        virtual ~NidavellirLayer() = default;
        void OnAttach() override;
        void OnDetach() override;
        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& event) override;

    private:
        Camera3DController camera_controller_ = Camera3DController(1);
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnKeyReleased(KeyReleasedEvent& e);
        glm::vec4 square_color_ = { 1.0, 1.0, 1.0, 1.0f };
        glm::vec3 quad_position_ = { 0.0f, 0.0f, 0.0f };
        glm::vec3 quad_size_ = { 2.0f, 2.0f, 2.0f };
        float* quad_vertices_ = nullptr;
        float alpha_ = 1.0f;
        float threshold_ = 0.001f;
        glm::vec4 color_ = glm::vec4{1.0f, 0.0f, 0.0f, 1.0f};

        int count_ = 0;
        int debug_frame_ = 0;
        float time_ = 0.0f;
        float previous_clicked_time_ = 0.0f;
        float new_color_[4] = {};
        bool camera_movement_enabled_ = true;
        std::unordered_map<KeyCode, bool> last_key_state_map_;

        // debug controls
        bool wireframe_enabled_ = false;
        bool render_debug_ui_enabled_ = false;
        bool render_debug_options_ui_enabled_ = false;

        Ref<Mesh> mesh_;


        // Controle de instancing automático
        bool auto_instancing_enabled_ = true;
        uint32_t instancing_threshold_ = 3; // Mínimo de objetos para usar instancing
        float previous_instancing_toggle_time_ = 0.0f;

        // Configurações de densidade de objetos para teste
        int grid_size_ = 4;          // Grid de cilindros (grid_size x grid_size)
        int cube_count_ = 20;        // Número de cubos orbitantes
        int sphere_count_ = 15;      // Número de esferas flutuantes
        int max_entities_ = 1000;


        // Configurações avançadas de teste
        struct PerformanceTestConfig
        {
            bool enable_stress_test = false;
            int stress_test_multiplier = 1;
            bool enable_culling_test = true;
            bool enable_material_variation = false;
            float animation_speed = 1.0f;

            // Configurações de densidade por tipo de objeto
            struct ObjectDensity
            {
                int cylinders = 64;      // Grid de cilindros
                int cubes = 50;         // Cubos orbitantes
                int spheres = 30;       // Esferas flutuantes
                int total() const { return cylinders + cubes + spheres; }
            } density;

        } perf_config_;


        struct FrameMetrics
        {
            float frame_time = 0.0f;
            float avg_frame_time = 0.0f;
            uint32_t fps = 0;
            uint32_t avg_fps = 0;

            // Histórico para média móvel
            static constexpr size_t HISTORY_SIZE = 60;
            float frame_history[HISTORY_SIZE] = {};
            size_t history_index = 0;
            bool history_filled = false;

            void UpdateFrameTime(float delta_time)
            {
                frame_time = delta_time;
                fps = delta_time > 0.0f ? (uint32_t)(1.0f / delta_time) : 0;

                // Atualizar histórico
                frame_history[history_index] = delta_time;
                history_index = (history_index + 1) % HISTORY_SIZE;
                if (history_index == 0) history_filled = true;

                // Calcular média
                size_t count = history_filled ? HISTORY_SIZE : history_index;
                float sum = 0.0f;
                for (size_t i = 0; i < count; i++)
                {
                    sum += frame_history[i];
                }
                avg_frame_time = sum / count;
                avg_fps = avg_frame_time > 0.0f ? (uint32_t)(1.0f / avg_frame_time) : 0;
            }
        } frame_metrics_;

        void UpdatePerformanceMetrics(float deltaTime);
        void RenderPerformancePanel();
        void RenderInstancingControlPanel();
        void RenderObjectDensityPanel();
        void RenderHelperUI();
    };
}