#include "Shader.h"

#include "FEPCH.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Renderer.h"

namespace ForgeEngine {

Ref<Shader> Shader::Create(const std::string& filepath) {
  switch (Renderer::GetAPI()) {
    case RendererAPI::API::None:
      FENGINE_CORE_ASSERT(false,
                          "RendererAPI::None is currently not supported!");
      return nullptr;
    case RendererAPI::API::OpenGL:
      return CreateRef<OpenGLShader>(filepath);
  }

  FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
  return nullptr;
}

Ref<Shader> Shader::Create(const std::string& name,
                           const std::string& vertexSrc,
                           const std::string& fragmentSrc) {
  switch (Renderer::GetAPI()) {
    case RendererAPI::API::None:
      FENGINE_CORE_ASSERT(false,
                          "RendererAPI::None is currently not supported!");
      return nullptr;
    case RendererAPI::API::OpenGL:
      return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
  }

  FENGINE_CORE_ASSERT(false, "Unknown RendererAPI!");
  return nullptr;
}

void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader) {
  FENGINE_CORE_ASSERT(!Exists(name), "Shader already exists!");
  m_Shaders[name] = shader;
}

void ShaderLibrary::Add(const Ref<Shader>& shader) {
  auto& name = shader->GetName();
  Add(name, shader);
}

Ref<Shader> ShaderLibrary::Load(const std::string& filepath) {
  auto shader = Shader::Create(filepath);
  Add(shader);
  return shader;
}

Ref<Shader> ShaderLibrary::Load(const std::string& name,
                                const std::string& filepath) {
  auto shader = Shader::Create(filepath);
  Add(name, shader);
  return shader;
}

Ref<Shader> ShaderLibrary::Get(const std::string& name) {
  FENGINE_CORE_ASSERT(Exists(name), "Shader not found!");
  FENGINE_CORE_CRITICAL("Shader not found!");
  return m_Shaders[name];
}

bool ShaderLibrary::Exists(const std::string& name) const {
  return m_Shaders.find(name) != m_Shaders.end();
}

}  // namespace BEngine