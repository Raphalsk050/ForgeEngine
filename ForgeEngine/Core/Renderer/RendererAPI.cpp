#include "RendererAPI.h"

#include "Config.h"
#include "RendererAPI.h"
#include "FEPCH.h"
#include "Platform/OpenGL/OpenGLRendererAPI.h"

namespace ForgeEngine {
  RendererAPI::API RendererAPI::api_ = API::OpenGL;

  Scope<RendererAPI> RendererAPI::Create()
  {
    switch (api_)
    {
      case RendererAPI::API::None:    FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
      case RendererAPI::API::OpenGL:  return CreateScope<OpenGLRendererAPI>();
    }

    FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
} // BEngine