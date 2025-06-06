#pragma once
#include <memory>

// this flag serves to show the shader's possible compilation errors and warnings
//#define FENGINE_SHADER_DEBUG

// to controls the render debug
//#define FENGINE_RENDER_DEBUG

// to enable the culling debug
//#define FENGINE_CULLING_DEBUG

// to debug the frustum information
//#define FENGINE_DEBUG_FRUSTUM

// to debug key input
//#define FENGINE_DEBUG_INPUT_KEYS

#ifdef DEBUG
#if defined(FENGINE_PLATFORM_WINDOWS)
    #define FENGINE_DEBUGBREAK() __debugbreak()
#elif defined(FENGINE_PLATFORM_LINUX)
    #include <signal.h>
    #define FENGINE_DEBUGBREAK() raise(SIGTRAP)
#elif defined(FENGINE_PLATFORM_APPLE)
#include <signal.h>
#define FENGINE_DEBUGBREAK() raise(SIGTRAP)
#else
#error "Platform doesn't support debugbreak yet!"
#endif

#else
#define FENGINE_DEBUGBREAK()
#endif

#define FENGINE_EXPAND_MACRO(x) x
#define FENGINE_STRINGIFY_MACRO(x) #x

#define BIT(x) (1 << x)

#define FENGINE_BIND_EVENT_FN(fn) [this](auto&&... args) -> decltype(auto) { return this->fn(std::forward<decltype(args)>(args)...); }

namespace ForgeEngine
{
    template <typename T>
    using Scope = std::unique_ptr<T>;

    template <typename T, typename... Args>
    constexpr Scope<T> CreateScope(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename T>
    using Ref = std::shared_ptr<T>;

    template <typename T, typename... Args>
    constexpr Ref<T> CreateRef(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }
}

#include "Core/Log/Felog.h"
#include "Core/Assert/Assert.h"
