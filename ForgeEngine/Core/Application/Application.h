#pragma once
#include "FEPCH.h"
#include "Core/Window/Window.h"
#include "Core/Event/WindowApplicationEvent.h"
#include "Core/Event/KeyEvent.h"
#include "Core/Imgui/ImguiLayer.h"
#include "Core/Layer/Layer.h"
#include "Core/Layer/LayerStack.h"


using namespace std;

namespace ForgeEngine {
  class MainUI;

  struct ApplicationCommandLineArgs {
    int Count = 0;
    char **Args = nullptr;

    const char *operator[](int index) const {
      FENGINE_CORE_ASSERT(index < Count);
      return Args[index];
    }
  };

  struct ApplicationSpecification {
    string Name = "Forge Engine Application";
    string WorkingDirectory;
    uint32_t WindowWidth = 1280;
    uint32_t WindowHeight = 720;
    ApplicationCommandLineArgs CommandLineArgs;
  };

  class Application {
  public:
    Application(const ApplicationSpecification &specification);

    virtual ~Application();

    void OnEvent(Event &e);

    void PushLayer(Layer *layer);

    void PushOverlay(Layer *layer);

    Window &GetWindow() const { return *window_; }

    void Close();

    ImGuiLayer *GetImGuiLayer() { return imgui_layer_; }

    static Application &Get() { return *instance_; }

    const ApplicationSpecification &GetSpecification() const { return specification_; }

    void SubmitToMainThread(const function<void()> &function);

  public:
    void Run();

    bool OnWindowClose(WindowCloseEvent &e);

    bool OnWindowResize(WindowResizeEvent &e);

    bool OnKeyPressed(KeyPressedEvent& e);

    void ExecuteMainThreadQueue();

  private:
    ApplicationSpecification specification_;
    Scope<Window> window_;
    ImGuiLayer *imgui_layer_;
    MainUI *main_ui_layer_;
    bool running_ = true;
    bool minimized_ = false;
    LayerStack layer_stack_;
    float last_frame_time_ = 0.0f;

    vector<function<void()> > main_thread_queue_;
    mutex main_thread_queue_mutex_;

  private:
    static Application *instance_;
  };

  Application* CreateApplication(ApplicationCommandLineArgs args);
} // BEngine
