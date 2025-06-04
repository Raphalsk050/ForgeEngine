#include "Core/Renderer/Framebuffer.h"
#include "Core/Renderer/RendererAPI.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace ForgeEngine {

Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
{
    switch (RendererAPI::GetAPI())
    {
    case RendererAPI::API::OpenGL:
        return CreateRef<OpenGLFramebuffer>(spec);
    }

    FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
    return nullptr;
}

}
