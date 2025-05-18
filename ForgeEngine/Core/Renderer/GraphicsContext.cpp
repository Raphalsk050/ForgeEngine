#include "GraphicsContext.h"
#include "FEPCH.h"
#include "renderer.h"
#include "Platform/OpenGL/OpenGLContext.h"

namespace ForgeEngine {
  Scope<GraphicsContext> GraphicsContext::Create(void* window)
  {
    switch (Renderer::GetAPI())
    {
      case RendererAPI::API::None:    FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
      case RendererAPI::API::OpenGL:  return CreateScope<OpenGLContext>(static_cast<GLFWwindow*>(window));
    }

    FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
} // BEngine