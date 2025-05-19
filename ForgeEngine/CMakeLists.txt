cmake_minimum_required(VERSION 3.10)
project(ForgeEngine)
set(CMAKE_CXX_STANDARD 20)


### third_party #################
set(glfw_DIR ThirdParty/glfw)
set(glad_DIR ThirdParty/glad)
set(glm_DIR ThirdParty/glm)
set(spdlog_DIR ThirdParty/spdlog)
set(imgui_DIR ThirdParty/imgui)
set(ImGuizmo_DIR ThirdParty/ImGuizmo)

add_subdirectory(${glad_DIR})
add_subdirectory(${glfw_DIR})
add_subdirectory(${glm_DIR})
add_subdirectory(${spdlog_DIR})
add_subdirectory(${imgui_DIR})
add_subdirectory(${ImGuizmo_DIR})
#################################

add_library(ForgeEngine STATIC
        Core/Log/Felog.h
        Core/Log/Felog.cpp
        Core/Input/KeyCodes.h
        Core/Input/MouseCodes.h
        Core/Input/Input.h
        Core/Input/Input.cpp
        Core/Event/Event.h
        Core/Event/KeyEvent.h
        Core/Event/MouseEvent.h
        Core/Event/WindowApplicationEvent.h
        Core/Assert/Assert.h
        Core/Application/EntryPoint.h
        Core/Application/Application.cpp
        Core/Application/Application.h
        Core/Debug/Instrumentor.h
        Core/Window/Window.h
        Core/Window/Window.cpp
        Core/Window/GenericGLFWWindow.h
        Core/Window/GenericGLFWWindow.cpp
        Core/Renderer/GraphicsContext.h
        Core/Renderer/GraphicsContext.cpp
        Core/Renderer/RendererAPI.h
        Core/Renderer/RendererAPI.cpp
        Core/Renderer/Buffer.h
        Core/Renderer/Buffer.cpp
        Core/Renderer/VertexArray.h
        Core/Renderer/VertexArray.cpp
        Core/Time.h
        Core/TimeStep.h
        Core/Renderer/RenderCommand.h
        Core/Renderer/RenderCommand.cpp
        Core/Renderer/Renderer.h
        Core/Renderer/Renderer.cpp
        Core/Layer/layer.h
        Core/Layer/layer.cpp
        Core/Layer/LayerStack.h
        Core/Layer/LayerStack.cpp
        Core/Imgui/ImguiLayer.h
        Core/Imgui/ImguiLayer.cpp
        Platform/OpenGL/OpenGLRendererAPI.h
        Platform/OpenGL/OpenGLRendererAPI.cpp
        Platform/OpenGL/OpenGLContext.h
        Platform/OpenGL/OpenGLContext.cpp
        Platform/Generic/GenericInput.cpp
        #        Core/Renderer/Renderer3D.h
        #        Core/Renderer/Renderer3D.cpp
        #        Core/Renderer/shader.h
        #        Core/Renderer/shader.cpp
        #        Core/Renderer/UniformBuffer.h
        #        Core/Renderer/UniformBuffer.cpp
        #        Core/Camera/camera.h
        #        Core/Camera/Camera3D.cpp
        #        Core/Camera/Camera3D.h
        #        Core/Camera/Camera3DController.h
        #        Core/Camera/Camera3DController.cpp
        #        Platform/OpenGL/OpenGLBuffer.h
        #        Platform/OpenGL/OpenGLBuffer.cpp
        #        Platform/OpenGL/OpenGLUniformBuffer.h
        #        Platform/OpenGL/OpenGLUniformBuffer.cpp
        ForgeEngine.h
)

target_link_libraries(ForgeEngine PUBLIC
        glad
        glfw
        glm
        spdlog
        ImGui
        ImGuizmo
)

target_include_directories(ForgeEngine PUBLIC
        ${glad_DIR}/include
        ${glm_DIR}
        ${spdlog_DIR}/include
        ${imgui_DIR}
        ${ImGuizmo_DIR}
)

target_include_directories(ImGui PUBLIC
        ${glad_DIR}/include
        ${glfw_DIR}/include
)

target_include_directories(ImGuizmo PUBLIC
        ${imgui_DIR}
)

set_target_properties(ForgeEngine PROPERTIES LINKER_LANGUAGE CXX)

###############################################################
#### compile commands #########################################
###############################################################
target_compile_definitions(ForgeEngine
        PRIVATE
        $<$<CONFIG:Debug>:DEBUG>
        $<$<CONFIG:Release>:NDEBUG>
)

###############################################################
#### platform detection #######################################
###############################################################
if (WIN32)
    target_compile_definitions(ForgeEngine
            PUBLIC
            FENGINE_PLATFORM_WINDOWS
    )
elseif (APPLE)
    target_compile_definitions(ForgeEngine
            PUBLIC
            FENGINE_PLATFORM_APPLE
    )
elseif (UNIX)
    target_compile_definitions(ForgeEngine
            PUBLIC
            FENGINE_PLATFORM_LINUX
    )
endif ()