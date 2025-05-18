#include "RenderCommand.h"

namespace ForgeEngine {

  Scope<RendererAPI> RenderCommand::renderer_api_ = RendererAPI::Create();

}