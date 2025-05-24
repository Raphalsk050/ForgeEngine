#pragma once
#include "Core/Log/Felog.h"
#include "Core/Assert/Assert.h"
#include "Core/Input/KeyCodes.h"
#include "Core/Input/MouseCodes.h"
#include "Core/Event/Event.h"
#include "Core/Application/Application.h"
#include "Core/Application/EntryPoint.h"
#include "Core/Event/KeyEvent.h"
#include "Core/Input/Input.h"
#include "Core/Renderer/RenderCommand.h"
#include "Core/Camera/Camera3D.h"
#include "Core/Camera/Camera3DController.h"
#include "Core/Renderer/Mesh.h"
#include "Core/Camera/Camera3DController.h"

namespace ForgeEngine {
    class ForgeEngine {
    public:
        inline ForgeEngine() { Felog::Init(); }
    };
} // namespace ForgeEngine
