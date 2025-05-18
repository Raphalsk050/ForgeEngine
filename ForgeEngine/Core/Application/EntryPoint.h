#pragma once
#include "Core/Assert/Assert.h"
#include "Core/Application/Application.h"

extern ForgeEngine::Application *ForgeEngine::CreateApplication(ApplicationCommandLineArgs args);

int main(int argc, char **argv) {
  ForgeEngine::Felog::Init();

  FENGINE_PROFILE_BEGIN_SESSION("Startup", "Profile-Startup.json");
  auto app = ForgeEngine::CreateApplication({argc, argv});
  FENGINE_PROFILE_END_SESSION();

  FENGINE_PROFILE_BEGIN_SESSION("Runtime", "Profile-Runtime.json");
  app->Run();
  FENGINE_PROFILE_END_SESSION();

  FENGINE_PROFILE_BEGIN_SESSION("Shutdown", "Profile-Shutdown.json");
  delete app;
  FENGINE_PROFILE_END_SESSION();
  return 0;
}
