cmake_minimum_required(VERSION 3.16)
project(ForgeEngine LANGUAGES CXX)

# ===========================================
# GLOBAL SETTINGS
# ===========================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build optimizations
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


# ===========================================
# OPENGL SYSTEM LIBRARIES
# ===========================================
find_package(OpenGL REQUIRED)

# Enable compilation cache if available
if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER)
    find_program(CCACHE_FOUND ccache)
    if(CCACHE_FOUND)
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_FOUND})
        message(STATUS "Using ccache for faster compilation")
    endif()
endif()

# ===========================================
# LINKER OPTIMIZATIONS
# ===========================================
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Try to use faster linkers
    if(NOT CMAKE_EXE_LINKER_FLAGS MATCHES "-fuse-ld")
        find_program(MOLD_FOUND mold)
        find_program(LLD_FOUND ld.lld)
        find_program(GOLD_FOUND ld.gold)

        if(MOLD_FOUND)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=mold")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=mold")
            message(STATUS "Using mold linker for faster linking")
        elseif(LLD_FOUND)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=lld")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=lld")
            message(STATUS "Using lld linker for faster linking")
        elseif(GOLD_FOUND)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold")
            set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold")
            message(STATUS "Using gold linker for faster linking")
        endif()
    endif()

    # Compiler optimizations
    add_compile_options(
            -Wall -Wextra -Wpedantic
            $<$<CONFIG:Release>:-O3>
            $<$<CONFIG:Release>:-march=native>
            $<$<CONFIG:Debug>:-O0>
            $<$<CONFIG:Debug>:-g3>
    )
endif()

# ===========================================
# PLATFORM-SPECIFIC DEFINITIONS
# ===========================================
if(WIN32)
    # Define M_PI for Windows
    add_compile_definitions(_USE_MATH_DEFINES)
    add_compile_definitions(NOMINMAX)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
endif()

# ===========================================
# THIRD PARTY CMAKE VARIABLES
# ===========================================
# Disable all installations and tests for third party libraries
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CMAKE_SKIP_INSTALL_RULES ON CACHE BOOL "" FORCE)

# GLFW settings
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Shaderc settings
set(SHADERC_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_EXAMPLES ON CACHE BOOL "" FORCE)
set(SHADERC_SKIP_COPYRIGHT_CHECK ON CACHE BOOL "" FORCE)
#set(SHADERC_ENABLE_SHARED_CRT ON CACHE BOOL "" FORCE)

# SPIRV-Cross settings
set(SPIRV_CROSS_CLI OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_GLSL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_HLSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_MSL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_REFLECT OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_C_API OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_ENABLE_UTIL OFF CACHE BOOL "" FORCE)
set(SPIRV_CROSS_SKIP_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_CROSS_FORCE_PIC ON CACHE BOOL "" FORCE)

# SPIRV-Tools settings (required by glslang)
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "" FORCE)
set(SPIRV_SKIP_TESTS ON CACHE BOOL "" FORCE)
set(SKIP_SPIRV_TOOLS_INSTALL ON CACHE BOOL "" FORCE)
set(SPIRV_WERROR OFF CACHE BOOL "" FORCE)

# glslang settings
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "" FORCE)
set(ENABLE_CTEST OFF CACHE BOOL "" FORCE)
set(ENABLE_HLSL ON CACHE BOOL "" FORCE)
set(BUILD_EXTERNAL OFF CACHE BOOL "" FORCE)

# ===========================================
# THIRD PARTY DIRECTORIES
# ===========================================
set(FORGE_THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty)
set(glfw_DIR ${FORGE_THIRD_PARTY_DIR}/glfw)
set(glad_DIR ${FORGE_THIRD_PARTY_DIR}/glad)
set(glm_DIR ${FORGE_THIRD_PARTY_DIR}/glm)
set(spdlog_DIR ${FORGE_THIRD_PARTY_DIR}/spdlog)
set(imgui_DIR ${FORGE_THIRD_PARTY_DIR}/imgui)
set(ImGuizmo_DIR ${FORGE_THIRD_PARTY_DIR}/ImGuizmo)
set(shaderc_DIR ${FORGE_THIRD_PARTY_DIR}/shaderc)
set(spirv_DIR ${FORGE_THIRD_PARTY_DIR}/SPIRV-Cross)

# Add third party subdirectories
add_subdirectory(${glad_DIR})
add_subdirectory(${glfw_DIR})
add_subdirectory(${glm_DIR})
add_subdirectory(${spdlog_DIR})
add_subdirectory(${imgui_DIR})
add_subdirectory(${ImGuizmo_DIR})

# Add shader compiler dependencies with error handling
if(EXISTS ${spirv_DIR}/CMakeLists.txt)
    add_subdirectory(${spirv_DIR})
else()
    message(WARNING "SPIRV-Cross not found at ${spirv_DIR}")
endif()

if(EXISTS ${shaderc_DIR}/CMakeLists.txt)
    #add_subdirectory(${shaderc_DIR})
else()
    message(WARNING "Shaderc not found at ${shaderc_DIR}")
endif()

# ===========================================
# PLATFORM DETECTION
# ===========================================
if(WIN32)
    set(FORGE_PLATFORM_DEFINE FENGINE_PLATFORM_WINDOWS)
elseif(APPLE)
    set(FORGE_PLATFORM_DEFINE FENGINE_PLATFORM_APPLE)
elseif(UNIX)
    set(FORGE_PLATFORM_DEFINE FENGINE_PLATFORM_LINUX)
endif()

# ===========================================
# HELPER FUNCTION FOR LIBRARY CREATION
# ===========================================
function(forge_add_library target_name)
    add_library(${target_name} STATIC ${ARGN})

    target_compile_definitions(${target_name}
            PUBLIC
            ${FORGE_PLATFORM_DEFINE}
            $<$<CONFIG:Debug>:DEBUG>
            $<$<CONFIG:Release>:NDEBUG>
    )

    target_include_directories(${target_name}
            PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_features(${target_name} PUBLIC cxx_std_20)

    # Enable unity build for faster compilation
    set_target_properties(${target_name} PROPERTIES
            UNITY_BUILD ON
            UNITY_BUILD_BATCH_SIZE 16
    )
endfunction()

# ===========================================
# CORE LIBRARY - Basic functionality
# ===========================================
forge_add_library(ForgeCore
        Core/Log/Felog.h
        Core/Log/Felog.cpp
        Core/Assert/Assert.h
        Core/Debug/Instrumentor.h
        Core/Time.h
        Core/TimeStep.h
)

target_link_libraries(ForgeCore PUBLIC spdlog)
target_include_directories(ForgeCore PUBLIC ${spdlog_DIR}/include)

# ===========================================
# INPUT LIBRARY - Input handling
# ===========================================
forge_add_library(ForgeInput
        Core/Input/KeyCodes.h
        Core/Input/MouseCodes.h
        Core/Input/Input.h
        Core/Input/Input.cpp
        Platform/Generic/GenericInput.cpp
)

target_link_libraries(ForgeInput PUBLIC ForgeCore glfw)
target_include_directories(ForgeInput PUBLIC ${glm_DIR})

# ===========================================
# EVENT LIBRARY - Event system
# ===========================================
forge_add_library(ForgeEvent
        Core/Event/Event.h
        Core/Event/Event.cpp
        Core/Event/KeyEvent.h
        Core/Event/MouseEvent.h
        Core/Event/WindowApplicationEvent.h
)

target_link_libraries(ForgeEvent PUBLIC ForgeCore)
target_include_directories(ForgeEvent PUBLIC ${glm_DIR})

# ===========================================
# WINDOW LIBRARY - Window management
# ===========================================
forge_add_library(ForgeWindow
        Core/Window/Window.h
        Core/Window/Window.cpp
        Core/Window/GenericGLFWWindow.h
        Core/Window/GenericGLFWWindow.cpp
)

target_link_libraries(ForgeWindow
        PUBLIC
        ForgeCore
        ForgeEvent
        glfw
        glad  # Added for OpenGL functionality
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS}
)
target_include_directories(ForgeWindow
        PUBLIC
        ${glm_DIR}
        ${glad_DIR}/include  # Added for glad headers
)

# ===========================================
# RENDERER BASE LIBRARY - Core rendering
# ===========================================
forge_add_library(ForgeRendererBase
        Core/Renderer/GraphicsContext.h
        Core/Renderer/GraphicsContext.cpp
        Core/Renderer/RendererAPI.h
        Core/Renderer/RendererAPI.cpp
        Core/Renderer/Buffer.h
        Core/Renderer/Buffer.cpp
        Core/Renderer/VertexArray.h
        Core/Renderer/VertexArray.cpp
        Core/Renderer/RenderCommand.h
        Core/Renderer/RenderCommand.cpp
        Core/Renderer/Shader.h
        Core/Renderer/Shader.cpp
        Core/Renderer/UniformBuffer.h
        Core/Renderer/UniformBuffer.cpp
        Core/Renderer/Texture.h
        Core/Renderer/Texture.cpp
)

target_link_libraries(ForgeRendererBase
        PUBLIC
        ForgeCore
        ForgeWindow
        glm
        glad
        ForgeOpenGL  # Ensure OpenGL implementations are linked
)
target_include_directories(ForgeRendererBase
        PUBLIC
        ${glm_DIR}
        ${glad_DIR}/include
)

# ===========================================
# RENDERER ADVANCED LIBRARY - 3D rendering
# ===========================================
forge_add_library(ForgeRenderer
        Core/Renderer/Renderer.h
        Core/Renderer/Renderer.cpp
        Core/Renderer/Renderer3D.h
        Core/Renderer/Renderer3D.cpp
        Core/Renderer/Mesh.h
        Core/Renderer/Mesh.cpp
        Core/Renderer/TestMesh.cpp
        Core/Renderer/TestMesh.h
        Core/Renderer/InstancedRenderer.h
        Core/Renderer/InstancedRenderer.cpp
)

set(FORGE_SHADER_LIBS "")
if(TARGET spirv-cross-core)
    list(APPEND FORGE_SHADER_LIBS spirv-cross-core)
endif()
if(TARGET spirv-cross-glsl)
    list(APPEND FORGE_SHADER_LIBS spirv-cross-glsl)
endif()
if(TARGET shaderc)
    list(APPEND FORGE_SHADER_LIBS shaderc)
elseif(TARGET shaderc_shared)
    list(APPEND FORGE_SHADER_LIBS shaderc_shared)
endif()

target_link_libraries(ForgeRenderer
        PUBLIC
        ForgeRendererBase
        ${FORGE_SHADER_LIBS}
)

if(EXISTS ${spirv_DIR})
    target_include_directories(ForgeRenderer PUBLIC ${spirv_DIR})
endif()
if(EXISTS ${shaderc_DIR}/libshaderc/include)
    target_include_directories(ForgeRenderer PUBLIC ${shaderc_DIR}/libshaderc/include)
endif()

# ===========================================
# OPENGL LIBRARY - OpenGL implementation
# ===========================================
forge_add_library(ForgeOpenGL
        Platform/OpenGL/OpenGLRendererAPI.h
        Platform/OpenGL/OpenGLRendererAPI.cpp
        Platform/OpenGL/OpenGLContext.h
        Platform/OpenGL/OpenGLContext.cpp
        Platform/OpenGL/OpenGLBuffer.h
        Platform/OpenGL/OpenGLBuffer.cpp
        Platform/OpenGL/OpenGLUniformBuffer.h
        Platform/OpenGL/OpenGLUniformBuffer.cpp
        Platform/OpenGL/OpenGLShader.h
        Platform/OpenGL/OpenGLShader.cpp
        Platform/OpenGL/OpenGLTexture2D.h
        Platform/OpenGL/OpenGLTexture2D.cpp
        Platform/OpenGL/OpenGLVertexArray.h
        Platform/OpenGL/OpenGLVertexArray.cpp
)

target_link_libraries(ForgeOpenGL
        PUBLIC
        ForgeRenderer
        glad
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS}
)
target_include_directories(ForgeOpenGL
        PUBLIC
        ${glad_DIR}/include
)
# Disable Unity build to avoid potential symbol resolution issues
set_target_properties(ForgeOpenGL PROPERTIES UNITY_BUILD OFF)

# ===========================================
# CAMERA LIBRARY - Camera system
# ===========================================
forge_add_library(ForgeCamera
        Core/Camera/Camera.h
        Core/Camera/Camera3D.h
        Core/Camera/Camera3D.cpp
        Core/Camera/Camera3DController.h
        Core/Camera/Camera3DController.cpp
)

target_link_libraries(ForgeCamera
        PUBLIC
        ForgeCore
        ForgeInput
        ForgeEvent
        glm
)

# ===========================================
# SCENE LIBRARY - Scene management
# ===========================================
forge_add_library(ForgeScene
        Core/Scene/Components.h
        Core/Scene/SceneCamera.h
        Core/Scene/SceneCamera.cpp
)

target_link_libraries(ForgeScene
        PUBLIC
        ForgeCamera
        ForgeRenderer
)

# ===========================================
# LAYER LIBRARY - Layer system
# ===========================================
forge_add_library(ForgeLayer
        Core/Layer/Layer.h
        Core/Layer/Layer.cpp
        Core/Layer/LayerStack.h
        Core/Layer/LayerStack.cpp
)

target_link_libraries(ForgeLayer
        PUBLIC
        ForgeCore
        ForgeEvent
)
target_include_directories(ForgeLayer PUBLIC ${glm_DIR})

# ===========================================
# IMGUI LIBRARY - ImGui integration
# ===========================================
forge_add_library(ForgeImGui
        Core/Imgui/ImguiLayer.h
        Core/Imgui/ImguiLayer.cpp  # Ensure correct case for filename
)

target_compile_definitions(ImGui PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_link_libraries(ForgeImGui
        PUBLIC
        ForgeLayer
        ForgeEvent
        ForgeRenderer
        ImGui
        ImGuizmo
)
target_include_directories(ForgeImGui
        PUBLIC
        ${imgui_DIR}
        ${ImGuizmo_DIR}
)
# Disable Unity build to avoid potential symbol resolution issues
set_target_properties(ForgeImGui PROPERTIES UNITY_BUILD OFF)

# Configure ImGui dependencies
target_include_directories(ImGui PUBLIC
        ${glad_DIR}/include
        ${glfw_DIR}/include
)

target_link_libraries(ImGui
        PUBLIC
        glfw
        glad
        ${OPENGL_LIBRARIES}
        ${CMAKE_DL_LIBS}
)

target_include_directories(ImGuizmo PUBLIC
        ${imgui_DIR}
)

# ===========================================
# APPLICATION LIBRARY - Application framework
# ===========================================
forge_add_library(ForgeApplication
        Core/Application/EntryPoint.h
        Core/Application/Application.h
        Core/Application/Application.cpp
)

target_link_libraries(ForgeApplication
        PUBLIC
        ForgeCore
        ForgeWindow
        ForgeEvent
        ForgeLayer
        ForgeRenderer
        ForgeImGui  # Explicitly link to ensure ImGuiLayer symbols
)

# ===========================================
# UTILITIES LIBRARY - Third party utilities
# ===========================================
forge_add_library(ForgeUtilities
        ThirdParty/stbimage/stb_image.h
        ThirdParty/stbimage/stb_image.cpp
)

target_link_libraries(ForgeUtilities
        spdlog
)

target_include_directories(ForgeUtilities
        PUBLIC
        ${glm_DIR}
        ${spdlog_DIR}/include
)
set_target_properties(ForgeUtilities PROPERTIES UNITY_BUILD OFF)

# ===========================================
# MAIN ENGINE INTERFACE - Combines all libraries
# ===========================================
add_library(ForgeEngine INTERFACE)

target_link_libraries(ForgeEngine INTERFACE
        ForgeCore
        ForgeInput
        ForgeEvent
        ForgeWindow
        ForgeRendererBase
        ForgeRenderer
        ForgeOpenGL
        ForgeCamera
        ForgeScene
        ForgeLayer
        ForgeImGui
        ForgeApplication
        ForgeUtilities
)

target_sources(ForgeEngine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/ForgeEngine.h)


add_custom_command(TARGET ForgeApplication POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/ForgeEngine/Assets/Shaders
        $<TARGET_FILE_DIR:ForgeApplication>/Assets/Shaders
)

# ===========================================
# BUILD CONFIGURATION OUTPUT
# ===========================================
message(STATUS "====================================")
message(STATUS "ForgeEngine Build Configuration:")
message(STATUS "  Platform:        ${FORGE_PLATFORM_DEFINE}")
message(STATUS "  C++ Standard:    ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:      ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler:        ${CMAKE_CXX_COMPILER_ID}")
if(CMAKE_CXX_COMPILER_LAUNCHER)
    message(STATUS "  Using ccache:    YES")
else()
    message(STATUS "  Using ccache:    NO")
endif()
message(STATUS "====================================")

# ===========================================
# EXAMPLE USAGE (for your game/application)
# ===========================================
# add_executable(MyGame main.cpp)
# target_link_libraries(MyGame PRIVATE ForgeEngine)
# Or link only specific libraries:
# target_link_libraries(MyGame PRIVATE
#     ForgeCore
#     ForgeRenderer
#     ForgeOpenGL
# )