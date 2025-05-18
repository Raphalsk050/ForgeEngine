#pragma once
#include "Config.h"
#include "Core/Log/Felog.h"
#include <filesystem>

namespace ForgeEngine {
#pragma once

#ifdef FENGINE_ENABLE_ASSERTS

  // Alteratively we could use the same "default" message for both "WITH_MSG" and "NO_MSG" and
  // provide support for custom formatting by concatenating the formatting string instead of having the format inside the default message
#define FENGINE_INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { FENGINE##type##ERROR(msg, __VA_ARGS__); FENGINE_DEBUGBREAK(); } }
#define FENGINE_INTERNAL_ASSERT_WITH_MSG(type, check, ...) FENGINE_INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define FENGINE_INTERNAL_ASSERT_NO_MSG(type, check) FENGINE_INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", FENGINE_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define FENGINE_INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define FENGINE_INTERNAL_ASSERT_GET_MACRO(...) FENGINE_EXPAND_MACRO( FENGINE_INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, FENGINE_INTERNAL_ASSERT_WITH_MSG, FENGINE_INTERNAL_ASSERT_NO_MSG) )

  // Currently accepts at least the condition and one additional parameter (the message) being optional
#define FENGINE_ASSERT(...) FENGINE_EXPAND_MACRO( FENGINE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define FENGINE_CORE_ASSERT(...) FENGINE_EXPAND_MACRO( FENGINE_INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define FENGINE_ASSERT(...)
#define FENGINE_CORE_ASSERT(...)
#endif
}
