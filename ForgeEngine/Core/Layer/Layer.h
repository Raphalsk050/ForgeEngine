#pragma once

#include "Config.h"
#include "Core/Event/Event.h"
#include "Core/TimeStep.h"

namespace ForgeEngine
{
    class Layer
    {
    public:
        Layer(const std::string& name = "Layer");
        virtual ~Layer() = default;

        virtual void OnAttach()
        {
        }

        virtual void OnDetach()
        {
        }

        virtual void OnUpdate(Timestep ts)
        {
        }

        virtual void OnImGuiRender()
        {
        }

        virtual void OnEvent(Event& event)
        {
        }

        const std::string& GetName() const { return debug_name_; }

    protected:
        std::string debug_name_;
    };
} // namespace BEngine
