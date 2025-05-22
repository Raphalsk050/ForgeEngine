#include "FEPCH.h"
#include "Core/Renderer/VertexArray.h"

#include "Core/Renderer/Renderer.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace ForgeEngine {

    Ref<VertexArray> VertexArray::Create()
    {
        switch (Renderer::GetAPI())
        {
        case RendererAPI::API::None:    FENGINE_CORE_ASSERT(false, "RendererAPI::None is currently not supported!"); return nullptr;
        case RendererAPI::API::OpenGL:  return CreateRef<OpenGLVertexArray>();
        }

        FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
        return nullptr;
    }

}