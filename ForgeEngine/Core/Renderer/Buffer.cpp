#include "Buffer.h"
#include "renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace ForgeEngine {
  Ref<VertexBuffer> VertexBuffer::Create(uint32_t size) {
    switch (Renderer::GetAPI()) {
      case RendererAPI::API::None: FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
      case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(size);
    }

    FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }

  Ref<VertexBuffer> VertexBuffer::Create(float *vertices, uint32_t size) {
    switch (Renderer::GetAPI()) {
      case RendererAPI::API::None: FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
      case RendererAPI::API::OpenGL: return CreateRef<OpenGLVertexBuffer>(vertices, size);
    }

    FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }

  Ref<IndexBuffer> IndexBuffer::Create(uint32_t *indices, uint32_t size) {
    switch (Renderer::GetAPI()) {
      case RendererAPI::API::None: FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
        return nullptr;
      case RendererAPI::API::OpenGL: return CreateRef<OpenGLIndexBuffer>(indices, size);
    }

    FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
  }
} // BEngine
