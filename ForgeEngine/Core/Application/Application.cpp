#include <filesystem>
#include <functional>
#include <mutex>

#include "FEPCH.h"
#include "Application.h"
#include "Core/Log/Felog.h"
#include "Core/TimeStep.h"
#include "Core/Time.h"
#include "Core/Debug/Instrumentor.h"
#include "Core/Input/Input.h"
#include "Core/Event/Event.h"
#include "Core/Event/WindowApplicationEvent.h"
#include "Core/Renderer/Renderer3D.h"
// #include "Core/Renderer/Renderer3D.h"

namespace ForgeEngine {
  Application *Application::instance_ = nullptr;

  Application::Application(const ApplicationSpecification &specification)
    : specification_(specification) {
    FENGINE_PROFILE_FUNCTION();

    FENGINE_CORE_ASSERT(!instance_, "Application already exists!");
    instance_ = this;

    // Set working directory here
    if (!specification_.WorkingDirectory.empty())
      std::filesystem::current_path(specification_.WorkingDirectory);

    window_ = Window::Create(WindowProps(specification_.Name,specification_.WindowWidth, specification_.WindowHeight));
    window_->SetEventCallback(FENGINE_BIND_EVENT_FN(Application::OnEvent));

    Renderer3D::Init();

    imgui_layer_ = new ImGuiLayer();
    PushOverlay(imgui_layer_);
  }

  Application::~Application() {
    FENGINE_PROFILE_FUNCTION();

    //ScriptEngine::Shutdown();
    Renderer3D::Shutdown();
  }

  void Application::PushLayer(Layer *layer) {
    FENGINE_PROFILE_FUNCTION();

    layer_stack_.PushLayer(layer);
    layer->OnAttach();
  }

  void Application::PushOverlay(Layer *layer) {
    FENGINE_PROFILE_FUNCTION();

    layer_stack_.PushOverlay(layer);
    layer->OnAttach();
  }

  void Application::Close() {
    running_ = false;
  }

  void Application::SubmitToMainThread(const std::function<void()> &function) {
    std::scoped_lock<std::mutex> lock(main_thread_queue_mutex_);

    main_thread_queue_.emplace_back(function);
  }

  void Application::OnEvent(Event &e) {
    FENGINE_PROFILE_FUNCTION();

    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<WindowCloseEvent>(FENGINE_BIND_EVENT_FN(Application::OnWindowClose));
    dispatcher.Dispatch<WindowResizeEvent>(FENGINE_BIND_EVENT_FN(Application::OnWindowResize));

    for (auto it = layer_stack_.rbegin(); it != layer_stack_.rend(); ++it) {
      if (e.Handled)
        break;
      (*it)->OnEvent(e);
    }
  }

  void Application::Run() {
    FENGINE_PROFILE_FUNCTION();

    while (running_) {
      FENGINE_PROFILE_SCOPE("RunLoop");

      float time = Time::GetTime();
      Timestep timestep = time - last_frame_time_;
      last_frame_time_ = time;

      ExecuteMainThreadQueue();

      if (!minimized_) {
        {
          FENGINE_PROFILE_SCOPE("LayerStack OnUpdate");

          for (Layer *layer: layer_stack_)
            layer->OnUpdate(timestep);
        }

        imgui_layer_->Begin(); {
          FENGINE_PROFILE_SCOPE("LayerStack OnImGuiRender");

          for (Layer *layer: layer_stack_)
            layer->OnImGuiRender();
        }
        imgui_layer_->End();
      }

      window_->OnUpdate();

      // TODO(rafael): this is a temporary feature
      if (Input::IsKeyPressed(Key::Escape))
      {
        FENGINE_CORE_INFO("Escape key was pressed!");
        Application::Close();
      }
    }
  }

  bool Application::OnWindowClose(WindowCloseEvent &e) {
    running_ = false;
    return true;
  }

  bool Application::OnWindowResize(WindowResizeEvent &e) {
    FENGINE_PROFILE_FUNCTION();

    if (e.GetWidth() == 0 || e.GetHeight() == 0) {
      minimized_ = true;
      return false;
    }

    minimized_ = false;

    return false;
  }

  void Application::ExecuteMainThreadQueue() {
    std::scoped_lock<std::mutex> lock(main_thread_queue_mutex_);

    for (auto &func: main_thread_queue_)
      func();

    main_thread_queue_.clear();
  }
}
