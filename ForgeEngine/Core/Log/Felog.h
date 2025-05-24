#pragma once

#include <memory>
#include <spdlog/spdlog.h>

namespace ForgeEngine {
  class Felog {
  public:
    // Initialize the Logger (call once at startup)
    static void Init(const std::string &engineName = "FENGINE");

    // Retrieve the core logger
    inline static std::shared_ptr<spdlog::logger> &GetCoreLogger() { return s_CoreLogger; }

    // (Optional) Retrieve a client/user logger
    inline static std::shared_ptr<spdlog::logger> &GetClientLogger() { return s_ClientLogger; }

  private:
    static std::shared_ptr<spdlog::logger> s_CoreLogger;
    static std::shared_ptr<spdlog::logger> s_ClientLogger;
  };
}
//#define DEBUG
#ifdef DEBUG

#define FENGINE_CORE_TRACE(...)    ::ForgeEngine::Felog::GetCoreLogger()->trace(__VA_ARGS__)
#define FENGINE_CORE_INFO(...)     ::ForgeEngine::Felog::GetCoreLogger()->info(__VA_ARGS__)
#define FENGINE_CORE_WARN(...)     ::ForgeEngine::Felog::GetCoreLogger()->warn(__VA_ARGS__)
#define FENGINE_CORE_ERROR(...)    ::ForgeEngine::Felog::GetCoreLogger()->error(__VA_ARGS__)
#define FENGINE_CORE_CRITICAL(...) ::ForgeEngine::Felog::GetCoreLogger()->critical(__VA_ARGS__)

#else

#define FENGINE_CORE_TRACE(...)    do {} while(false)
#define FENGINE_CORE_INFO(...)     do {} while(false)
#define FENGINE_CORE_WARN(...)     do {} while(false)
#define FENGINE_CORE_ERROR(...)    do {} while(false)
#define FENGINE_CORE_CRITICAL(...) do {} while(false)

#endif
