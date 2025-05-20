#include <ForgeEngine.h>

#include "NidavellirLayer.h"
#include "Core/Event/KeyEvent.h"
#include "Core/Input/Input.h"
#include "Core/Renderer/RenderCommand.h"

namespace ForgeEngine {
    class Nidavellir : public Application {
    public:
        explicit Nidavellir(const ApplicationSpecification &spec)
          : Application(spec) {
            PushLayer(new NidavellirLayer());
        }
    };

    Application *CreateApplication(ApplicationCommandLineArgs args) {
        ApplicationSpecification spec;
        spec.Name = "Nidavellir";
        spec.CommandLineArgs = args;

        return new Nidavellir(spec);
    }
}
