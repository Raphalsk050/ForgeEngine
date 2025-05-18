#pragma once

#include "Core/Renderer/RenderCommand.h"

namespace ForgeEngine {

class Renderer
{
public:
  static RendererAPI::API GetAPI() { return RendererAPI::GetAPI(); }
private:
  struct SceneData
  {
    glm::mat4 ViewProjectionMatrix;
  };

  static Scope<SceneData> s_SceneData;
};
}