#pragma once
#include "Core/Log/Felog.h"
#include "Core/Assert/Assert.h"
#include "Core/Input/KeyCodes.h"
#include "Core/Input/MouseCodes.h"
#include "Core/Event/Event.h"
#include "Core/Application/Application.h"
#include "Core/Application/EntryPoint.h"

namespace ForgeEngine {
    class ForgeEngine {
    public:
        inline ForgeEngine() { Felog::Init(); }
    };
} // namespace ForgeEngine
