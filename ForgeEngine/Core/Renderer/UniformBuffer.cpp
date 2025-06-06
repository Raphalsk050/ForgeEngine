#include "FEPCH.h"
#include "Core/Renderer/UniformBuffer.h"

#include "RendererAPI.h"
#include "Core/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLUniformBuffer.h"

namespace ForgeEngine {

Ref<UniformBuffer> UniformBuffer::Create(uint32_t size, uint32_t binding)
{
  switch (Renderer::GetAPI())
  {
    case RendererAPI::API::None:    FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
    case RendererAPI::API::OpenGL:  return CreateRef<OpenGLUniformBuffer>(size, binding);
  }

  FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
  return nullptr;
}

}