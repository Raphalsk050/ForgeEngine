#include "OpenGLContext.h"
#include "FEPCH.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>


namespace ForgeEngine {
  OpenGLContext::OpenGLContext(GLFWwindow *windowHandle)
    : window_handle_(windowHandle) {
    FENGINE_CORE_ASSERT(windowHandle, "Window handle is null!")
  }

  void OpenGLContext::Init() {
    FENGINE_PROFILE_FUNCTION();

    glfwMakeContextCurrent(window_handle_);
    int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
    FENGINE_CORE_ASSERT(status, "Failed to initialize Glad!");

    FENGINE_CORE_INFO("OpenGL Info:");
    // FENGINE_CORE_INFO("  Vendor: {0}", *glGetString(GL_VENDOR));
    // FENGINE_CORE_INFO("  Renderer: {0}", *glGetString(GL_RENDERER));
    // FENGINE_CORE_INFO("  Version: {0}", *glGetString(GL_VERSION));

    // TODO: Fix this check
    // FENGINE_CORE_ASSERT(GLVersion.major > 4 || (GLVersion.major == 4 && GLVersion.minor >= 5),
    //                "Block engine requires at least OpenGL version 4.5!");
  }

  void OpenGLContext::SwapBuffers() {
    FENGINE_PROFILE_FUNCTION();

    glfwSwapBuffers(window_handle_);
  }
}
