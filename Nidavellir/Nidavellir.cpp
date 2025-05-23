#include <ForgeEngine.h>

#include "NidavellirLayer.h"


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
        spec.WindowWidth = 1600;
        spec.WindowHeight = 900;

        return new Nidavellir(spec);
    }
}
