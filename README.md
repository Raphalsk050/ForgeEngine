# ForgeEngine

This engine was tested with the clang compiler. Please, to avoid errors, use clang.

### Quick Setup

Execute the `setup.sh` script to configure and build the project:

```bash
./setup.sh
```

This script uses Clang by default and will create the build directory and compile the engine using CMake.

![image](https://github.com/user-attachments/assets/34628e2b-0908-41e6-b0da-bc227d9a0bf8)

### Occlusion Culling

Renderer3D supports optional GPU occlusion culling. When enabled, a depth-only
pre-pass issues OpenGL occlusion queries for each visible entity. Entities that
fail the query are skipped during the color pass, improving performance on large
scenes. To enable this feature define `FENGINE_OCCLUSION_CULLING` in
`ForgeEngine/Config.h`.


