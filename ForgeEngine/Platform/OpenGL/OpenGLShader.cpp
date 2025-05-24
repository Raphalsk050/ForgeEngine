#include "Platform/OpenGL/OpenGLShader.h"

#include <glad/glad.h>

#include <fstream>
#include <gtc/type_ptr.hpp>
#include <shaderc/shaderc.hpp>
#include <spirv_cross.hpp>
#include <spirv_glsl.hpp>
#include "Core/Time.h"
#include "FEPCH.h"
#include "Config.h"

namespace ForgeEngine {

namespace Utils {

static GLenum ShaderTypeFromString(const std::string& type) {
  if (type == "vertex") return GL_VERTEX_SHADER;
  if (type == "fragment" || type == "pixel") return GL_FRAGMENT_SHADER;

  FENGINE_CORE_ASSERT(false, "Unknown shader type!");
  return 0;
}

static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage) {
  switch (stage) {
    case GL_VERTEX_SHADER:
      return shaderc_glsl_vertex_shader;
    case GL_FRAGMENT_SHADER:
      return shaderc_glsl_fragment_shader;
  }
  FENGINE_CORE_ASSERT(false);
  return (shaderc_shader_kind)0;
}

static const char* GLShaderStageToString(GLenum stage) {
  switch (stage) {
    case GL_VERTEX_SHADER:
      return "GL_VERTEX_SHADER";
    case GL_FRAGMENT_SHADER:
      return "GL_FRAGMENT_SHADER";
  }
  FENGINE_CORE_ASSERT(false);
  return nullptr;
}

static const char* GetCacheDirectory() {
  // TODO: make sure the assets directory is valid
  return "assets/cache/shader/opengl";
}

static void CreateCacheDirectoryIfNeeded() {
  std::string cacheDirectory = GetCacheDirectory();
  if (!std::filesystem::exists(cacheDirectory))
    std::filesystem::create_directories(cacheDirectory);
}

static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage) {
  switch (stage) {
    case GL_VERTEX_SHADER:
      return ".cached_opengl.vert";
    case GL_FRAGMENT_SHADER:
      return ".cached_opengl.frag";
  }
  FENGINE_CORE_ASSERT(false);
  return "";
}

static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage) {
  switch (stage) {
    case GL_VERTEX_SHADER:
      return ".cached_vulkan.vert";
    case GL_FRAGMENT_SHADER:
      return ".cached_vulkan.frag";
  }
  FENGINE_CORE_ASSERT(false);
  return "";
}

}  // namespace Utils

OpenGLShader::OpenGLShader(const std::string& filepath) : m_FilePath(filepath) {
  FENGINE_PROFILE_FUNCTION();

  Utils::CreateCacheDirectoryIfNeeded();

  std::string source = ReadFile(filepath);
  auto shaderSources = PreProcess(source);

  // TENTAR COMPILAÇÃO DIRETA PRIMEIRO
  FENGINE_CORE_INFO("Attempting direct OpenGL compilation for: {}", filepath);

  try {
    CreateDirectOpenGLProgram(shaderSources);

    if (m_RendererID != 0) {
      FENGINE_CORE_INFO("Direct compilation successful!");

      // Extract name from filepath
      auto lastSlash = filepath.find_last_of("/\\");
      lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
      auto lastDot = filepath.rfind('.');
      auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
      m_Name = filepath.substr(lastSlash, count);
      return;
    }
  } catch (const std::exception& e) {
    FENGINE_CORE_CRITICAL("Direct compilation failed: {}", e.what());
  }

  // Fallback para metodo SPIR-V original
  FENGINE_CORE_WARN("Falling back to SPIR-V compilation");

  {
    Time timer;
    CompileOrGetVulkanBinaries(shaderSources);
    CompileOrGetOpenGLBinaries();
    CreateProgram();
    FENGINE_CORE_WARN("SPIR-V shader creation took {0} ms", timer.ElapsedMillis());
  }

  // Extract name from filepath
  auto lastSlash = filepath.find_last_of("/\\");
  lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
  auto lastDot = filepath.rfind('.');
  auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
  m_Name = filepath.substr(lastSlash, count);
}

OpenGLShader::OpenGLShader(const std::string& name,
                           const std::string& vertexSrc,
                           const std::string& fragmentSrc)
    : m_Name(name) {
  FENGINE_PROFILE_FUNCTION();

  std::unordered_map<GLenum, std::string> sources;
  sources[GL_VERTEX_SHADER] = vertexSrc;
  sources[GL_FRAGMENT_SHADER] = fragmentSrc;

  CompileOrGetVulkanBinaries(sources);
  CompileOrGetOpenGLBinaries();
  CreateProgram();
}

OpenGLShader::~OpenGLShader() {
  FENGINE_PROFILE_FUNCTION();
  glDeleteProgram(m_RendererID);
}

std::string OpenGLShader::ReadFile(const std::string& filepath) {
  FENGINE_PROFILE_FUNCTION();

  std::string result;
  std::ifstream in(
      filepath,
      std::ios::in | std::ios::binary);  // ifstream closes itself due to RAII
  if (in) {
    in.seekg(0, std::ios::end);
    size_t size = in.tellg();
    if (size != -1) {
      result.resize(size);
      in.seekg(0, std::ios::beg);
      in.read(&result[0], size);
    } else {
      FENGINE_CORE_ERROR("Could not read from file '{0}'", filepath);
    }
  } else {
    FENGINE_CORE_ERROR("Could not open file '{0}'", filepath);
  }

  return result;
}

std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(
    const std::string& source) {
  FENGINE_PROFILE_FUNCTION();

  std::unordered_map<GLenum, std::string> shaderSources;

  const char* typeToken = "#type";
  size_t typeTokenLength = strlen(typeToken);
  size_t pos =
      source.find(typeToken, 0);  // Start of shader type declaration line
  while (pos != std::string::npos) {
    size_t eol = source.find_first_of(
        "\r\n", pos);  // End of shader type declaration line
    FENGINE_CORE_ASSERT(eol != std::string::npos, "Syntax error");
    size_t begin = pos + typeTokenLength +
                   1;  // Start of shader type name (after "#type " keyword)
    std::string type = source.substr(begin, eol - begin);
    FENGINE_CORE_ASSERT(Utils::ShaderTypeFromString(type),
                        "Invalid shader type specified");

    size_t nextLinePos = source.find_first_not_of(
        "\r\n",
        eol);  // Start of shader code after shader type declaration line
    FENGINE_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
    pos = source.find(
        typeToken, nextLinePos);  // Start of next shader type declaration line

    shaderSources[Utils::ShaderTypeFromString(type)] =
        (pos == std::string::npos)
            ? source.substr(nextLinePos)
            : source.substr(nextLinePos, pos - nextLinePos);
  }

  return shaderSources;
}

void OpenGLShader::CompileOrGetVulkanBinaries(
    const std::unordered_map<GLenum, std::string>& shaderSources) {
  GLuint program = glCreateProgram();

  shaderc::Compiler compiler;
  shaderc::CompileOptions options;
  options.SetTargetEnvironment(shaderc_target_env_vulkan,
                               shaderc_env_version_vulkan_1_2);
  const bool optimize = true;
  if (optimize)
    options.SetOptimizationLevel(shaderc_optimization_level_performance);

  std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

  auto& shaderData = m_VulkanSPIRV;
  shaderData.clear();
  for (auto&& [stage, source] : shaderSources) {
    std::filesystem::path shaderFilePath = m_FilePath;
    std::filesystem::path cachedPath =
        cacheDirectory / (shaderFilePath.filename().string() +
                          Utils::GLShaderStageCachedVulkanFileExtension(stage));

    std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
    if (in.is_open()) {
      in.seekg(0, std::ios::end);
      auto size = in.tellg();
      in.seekg(0, std::ios::beg);

      auto& data = shaderData[stage];
      data.resize(size / sizeof(uint32_t));
      in.read((char*)data.data(), size);
    } else {
      shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
          source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(),
          options);
      if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
        FENGINE_CORE_ERROR(module.GetErrorMessage());
        FENGINE_CORE_ASSERT(false);
      }

      shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

      std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
      if (out.is_open()) {
        auto& data = shaderData[stage];
        out.write((char*)data.data(), data.size() * sizeof(uint32_t));
        out.flush();
        out.close();
      }
    }
  }

  for (auto&& [stage, data] : shaderData) Reflect(stage, data);
}

void OpenGLShader::CompileOrGetOpenGLBinaries() {
  try {
    auto& shaderData = m_OpenGLSPIRV;

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;
    options.SetTargetEnvironment(shaderc_target_env_opengl,
                                shaderc_env_version_opengl_4_5);
    const bool optimize = false;
    if (optimize)
      options.SetOptimizationLevel(shaderc_optimization_level_performance);

    std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

    shaderData.clear();
    m_OpenGLSourceCode.clear();

    for (auto&& [stage, spirv] : m_VulkanSPIRV) {
      std::filesystem::path shaderFilePath = m_FilePath;
      std::filesystem::path cachedPath =
          cacheDirectory / (shaderFilePath.filename().string() +
                          Utils::GLShaderStageCachedOpenGLFileExtension(stage));

      // Try to load from cache first
      std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
      bool loadedFromCache = false;

      if (in.is_open()) {
        try {
          in.seekg(0, std::ios::end);
          auto size = in.tellg();
          in.seekg(0, std::ios::beg);

          if (size > 0) {
            auto& data = shaderData[stage];
            data.resize(size / sizeof(uint32_t));
            in.read((char*)data.data(), size);
            loadedFromCache = true;

            // Ensure we also generate GLSL source
            spirv_cross::CompilerGLSL glslCompiler(spirv);
            m_OpenGLSourceCode[stage] = glslCompiler.compile();
            FENGINE_CORE_TRACE("Loaded shader from cache and generated GLSL for stage {0}", Utils::GLShaderStageToString(stage));
          }
        } catch (const std::exception& e) {
          FENGINE_CORE_ERROR("Error loading shader from cache: {0}", e.what());
          loadedFromCache = false;
        }
      }

      // If not loaded from cache, compile it
      if (!loadedFromCache) {
        try {
          // Generate GLSL source from SPIR-V
          spirv_cross::CompilerGLSL glslCompiler(spirv);
          m_OpenGLSourceCode[stage] = glslCompiler.compile();
          auto& source = m_OpenGLSourceCode[stage];

          FENGINE_CORE_TRACE("Generated GLSL source for {0}, size: {1} bytes",
                            Utils::GLShaderStageToString(stage), source.size());

          // Compile GLSL back to SPIR-V (for OpenGL)
          shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(
              source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);

          if (module.GetCompilationStatus() != shaderc_compilation_status_success) {
            FENGINE_CORE_ERROR("Error compiling GLSL to SPIR-V: {0}", module.GetErrorMessage());
            // Even if SPIR-V compilation fails, we still have the GLSL source for fallback
          } else {
            // Store the SPIR-V binary
            shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

            // Cache the binary
            std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
            if (out.is_open()) {
              auto& data = shaderData[stage];
              out.write((char*)data.data(), data.size() * sizeof(uint32_t));
              out.flush();
              out.close();
            }
          }
        } catch (const std::exception& e) {
          FENGINE_CORE_ERROR("Error generating GLSL or compiling to SPIR-V: {0}", e.what());
        }
      }

      // Double-check that we have GLSL source for this stage
      if (m_OpenGLSourceCode.find(stage) == m_OpenGLSourceCode.end() ||
          m_OpenGLSourceCode[stage].empty()) {
        FENGINE_CORE_ERROR("Failed to generate GLSL source for {0}", Utils::GLShaderStageToString(stage));

        // Emergency fallback - create a simple pass-through shader
        if (stage == GL_VERTEX_SHADER) {
          m_OpenGLSourceCode[stage] =
              "#version 450 core\n"
              "layout(location = 0) in vec3 a_Position;\n"
              "layout(location = 1) in vec4 a_Color;\n"
              "layout(location = 0) out vec4 v_Color;\n"
              "void main() {\n"
              "    v_Color = a_Color;\n"
              "    gl_Position = vec4(a_Position, 1.0);\n"
              "}\n";
        } else if (stage == GL_FRAGMENT_SHADER) {
          m_OpenGLSourceCode[stage] =
              "#version 450 core\n"
              "layout(location = 0) in vec4 v_Color;\n"
              "layout(location = 0) out vec4 o_Color;\n"
              "void main() {\n"
              "    o_Color = v_Color;\n"
              "}\n";
        }
      }
    }
  } catch (const std::exception& e) {
    FENGINE_CORE_ERROR("Exception in CompileOrGetOpenGLBinaries: {0}", e.what());
  } catch (...) {
    FENGINE_CORE_ERROR("Unknown exception in CompileOrGetOpenGLBinaries");
  }
}

void OpenGLShader::CreateProgram() {
  try {
    GLuint program = glCreateProgram();
    if (program == 0) {
      FENGINE_CORE_ERROR("Failed to create shader program");
      return;
    }

    std::vector<GLuint> shaderIDs;

    // First check if we have both shader types
    bool hasVertexShader = (m_OpenGLSourceCode.find(GL_VERTEX_SHADER) != m_OpenGLSourceCode.end());
    bool hasFragmentShader = (m_OpenGLSourceCode.find(GL_FRAGMENT_SHADER) != m_OpenGLSourceCode.end());

    if (!hasVertexShader || !hasFragmentShader) {
      FENGINE_CORE_ERROR("Missing shader stage: Vertex({0}), Fragment({1})",
                        hasVertexShader, hasFragmentShader);
      glDeleteProgram(program);
      return;
    }

    // Process all shader stages
    for (auto&& [stage, source] : m_OpenGLSourceCode) {
      FENGINE_CORE_TRACE("Processing shader stage: {0}", Utils::GLShaderStageToString(stage));

      if (source.empty()) {
        FENGINE_CORE_ERROR("Empty source for shader stage {0}", Utils::GLShaderStageToString(stage));
        continue;
      }

      // Create shader
      GLuint shaderID = glCreateShader(stage);
      if (shaderID == 0) {
        GLenum error = glGetError();
        FENGINE_CORE_ERROR("Failed to create shader for stage {0}, GL error: {1}",
                          Utils::GLShaderStageToString(stage), error);
        continue;
      }

      shaderIDs.push_back(shaderID);

      // Use GLSL compilation
      try {
        // Clear previous errors
        while (glGetError() != GL_NO_ERROR) {}

        const char* sourceCStr = source.c_str();

        // Debug info
        FENGINE_CORE_TRACE("Shader source size: {0} bytes", source.size());
        if (source.size() > 100) {
          FENGINE_CORE_TRACE("First part of source: {0}...", source.substr(0, 100));
        } else {
          FENGINE_CORE_TRACE("Source: {0}", source);
        }

        glShaderSource(shaderID, 1, &sourceCStr, nullptr);
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
          FENGINE_CORE_ERROR("glShaderSource error: {0}", error);
          continue;
        }

        glCompileShader(shaderID);
        error = glGetError();
        if (error != GL_NO_ERROR) {
          FENGINE_CORE_ERROR("glCompileShader error: {0}", error);
        }

        // Check compilation
        GLint isCompiled = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
          GLint maxLength = 0;
          glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

          if (maxLength > 0) {
            std::vector<GLchar> infoLog(maxLength);
            glGetShaderInfoLog(shaderID, maxLength, &maxLength, infoLog.data());
            FENGINE_CORE_ERROR("GLSL compilation failed for {0}:\n{1}",
                              Utils::GLShaderStageToString(stage), infoLog.data());
          } else {
            FENGINE_CORE_ERROR("GLSL compilation failed with no info log");
          }

          continue;
        }

        FENGINE_CORE_TRACE("GLSL compilation succeeded for {0}", Utils::GLShaderStageToString(stage));
        glAttachShader(program, shaderID);
      } catch (const std::exception& e) {
        FENGINE_CORE_ERROR("Exception during GLSL compilation: {0}", e.what());
      } catch (...) {
        FENGINE_CORE_ERROR("Unknown exception during GLSL compilation");
      }
    }

    // Check if we have any shaders attached
    if (shaderIDs.empty()) {
      FENGINE_CORE_ERROR("No valid shaders compiled for program");
      glDeleteProgram(program);
      return;
    }

    // Link program
    FENGINE_CORE_TRACE("Linking shader program");
    while (glGetError() != GL_NO_ERROR) {} // Clear errors

    glLinkProgram(program);
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
      FENGINE_CORE_ERROR("glLinkProgram error: {0}", error);
    }

    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
      GLint maxLength = 0;
      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

      if (maxLength > 0) {
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
        FENGINE_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());
      } else {
        FENGINE_CORE_ERROR("Shader linking failed with no info log");
      }

      glDeleteProgram(program);

      for (auto id : shaderIDs) {
        glDeleteShader(id);
      }
      return;
    }

    FENGINE_CORE_TRACE("Shader program linked successfully");

    // Clean up shaders
    for (auto id : shaderIDs) {
      glDetachShader(program, id);
      glDeleteShader(id);
    }

    m_RendererID = program;
  } catch (const std::exception& e) {
    FENGINE_CORE_ERROR("Exception in CreateProgram: {0}", e.what());
  } catch (...) {
    FENGINE_CORE_ERROR("Unknown exception in CreateProgram");
  }
}

void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData) {
  try {
    // For fragment shaders, just log basic info and return
    if (stage == GL_FRAGMENT_SHADER) {
      FENGINE_CORE_TRACE("OpenGLShader::Reflect - {0} {1} (skipping detailed reflection for fragment shaders)",
                         Utils::GLShaderStageToString(stage), m_FilePath);
      return;
    }

    // Only process vertex shaders normally
    spirv_cross::Compiler compiler(shaderData);
    spirv_cross::ShaderResources resources = compiler.get_shader_resources();

    FENGINE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}",
                       Utils::GLShaderStageToString(stage), m_FilePath);
    FENGINE_CORE_TRACE("    {0} uniform buffers", resources.uniform_buffers.size());
    FENGINE_CORE_TRACE("    {0} resources", resources.sampled_images.size());

    // Only process uniform buffers for vertex shaders
    if (!resources.uniform_buffers.empty()) {
      FENGINE_CORE_TRACE("Uniform buffers:");
      for (const auto& resource : resources.uniform_buffers) {
        const auto& bufferType = compiler.get_type(resource.base_type_id);
        uint32_t bufferSize = compiler.get_declared_struct_size(bufferType);
        uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        int memberCount = bufferType.member_types.size();

        FENGINE_CORE_TRACE("            {0}", resource.name);
        FENGINE_CORE_TRACE("  Size    = {0}", bufferSize);
        FENGINE_CORE_TRACE("  Binding = {0}", binding);
        FENGINE_CORE_TRACE("  Members = {0}", memberCount);
      }
    }
  } catch (const std::exception& e) {
    FENGINE_CORE_ERROR("Exception in shader reflection: {0}", e.what());
  } catch (...) {
    FENGINE_CORE_ERROR("Unknown exception in shader reflection");
  }
}

void OpenGLShader::CreateDirectOpenGLProgram(const std::unordered_map<GLenum, std::string>& shaderSources)
{
  FENGINE_CORE_INFO("Creating OpenGL program directly from GLSL source");

    GLuint program = glCreateProgram();
    if (program == 0) {
        FENGINE_CORE_ERROR("Failed to create OpenGL program");
        return;
    }

    std::vector<GLuint> shaderIDs;

    for (auto&& [stage, source] : shaderSources) {
        FENGINE_CORE_INFO("Compiling direct shader stage: {0}", Utils::GLShaderStageToString(stage));

        GLuint shaderID = glCreateShader(stage);
        if (shaderID == 0) {
            FENGINE_CORE_ERROR("Failed to create shader object for stage: {0}", Utils::GLShaderStageToString(stage));
            continue;
        }

        const char* sourceCStr = source.c_str();
        glShaderSource(shaderID, 1, &sourceCStr, nullptr);
        glCompileShader(shaderID);

        // Check compilation
        GLint isCompiled = 0;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &isCompiled);
        if (isCompiled == GL_FALSE) {
            GLint maxLength = 0;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

            if (maxLength > 0) {
                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shaderID, maxLength, &maxLength, infoLog.data());
                FENGINE_CORE_ERROR("Direct GLSL compilation failed for {0}:\n{1}",
                                  Utils::GLShaderStageToString(stage), infoLog.data());

                // Log também o código fonte para debug
                FENGINE_CORE_ERROR("Shader source that failed:\n{0}", source);
            }

            glDeleteShader(shaderID);
            continue;
        }

        FENGINE_CORE_INFO("Successfully compiled direct {0}", Utils::GLShaderStageToString(stage));
        shaderIDs.push_back(shaderID);
        glAttachShader(program, shaderID);
    }

    if (shaderIDs.empty()) {
        FENGINE_CORE_ERROR("No shaders compiled successfully in direct mode");
        glDeleteProgram(program);
        return;
    }

    // Link program
    FENGINE_CORE_INFO("Linking direct shader program");
    glLinkProgram(program);

    GLint isLinked;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        if (maxLength > 0) {
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
            FENGINE_CORE_ERROR("Direct shader linking failed:\n{0}", infoLog.data());
        }

        glDeleteProgram(program);
        for (auto id : shaderIDs) {
            glDeleteShader(id);
        }
        return;
    }

    FENGINE_CORE_INFO("Direct shader program linked successfully");

    // Clean up individual shaders
    for (auto id : shaderIDs) {
        glDetachShader(program, id);
        glDeleteShader(id);
    }

    m_RendererID = program;
}

void OpenGLShader::Bind() const {
  FENGINE_PROFILE_FUNCTION();

  glUseProgram(m_RendererID);
}

void OpenGLShader::Unbind() const {
  FENGINE_PROFILE_FUNCTION();

  glUseProgram(0);
}

void OpenGLShader::SetInt(const std::string& name, int value) {
  FENGINE_PROFILE_FUNCTION();

  UploadUniformInt(name, value);
}

void OpenGLShader::SetIntArray(const std::string& name, int* values,
                               uint32_t count) {
  UploadUniformIntArray(name, values, count);
}

void OpenGLShader::SetFloat(const std::string& name, float value) {
  FENGINE_PROFILE_FUNCTION();

  UploadUniformFloat(name, value);
}

void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value) {
  FENGINE_PROFILE_FUNCTION();

  UploadUniformFloat2(name, value);
}

void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value) {
  FENGINE_PROFILE_FUNCTION();

  UploadUniformFloat3(name, value);
}

void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value) {
  FENGINE_PROFILE_FUNCTION();

  UploadUniformFloat4(name, value);
}

void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value) {
  FENGINE_PROFILE_FUNCTION();

  UploadUniformMat4(name, value);
}

void OpenGLShader::UploadUniformInt(const std::string& name, int value) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniform1i(location, value);
}

void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values,
                                         uint32_t count) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniform1iv(location, count, values);
}

void OpenGLShader::UploadUniformFloat(const std::string& name, float value) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniform1f(location, value);
}

void OpenGLShader::UploadUniformFloat2(const std::string& name,
                                       const glm::vec2& value) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniform2f(location, value.x, value.y);
}

void OpenGLShader::UploadUniformFloat3(const std::string& name,
                                       const glm::vec3& value) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniform3f(location, value.x, value.y, value.z);
}

void OpenGLShader::UploadUniformFloat4(const std::string& name,
                                       const glm::vec4& value) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniform4f(location, value.x, value.y, value.z, value.w);
}

void OpenGLShader::UploadUniformMat3(const std::string& name,
                                     const glm::mat3& matrix) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

void OpenGLShader::UploadUniformMat4(const std::string& name,
                                     const glm::mat4& matrix) {
  GLint location = glGetUniformLocation(m_RendererID, name.c_str());
  glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
}

}  // namespace BEngine