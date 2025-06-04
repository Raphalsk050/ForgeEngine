# ForgeEngine

This engine was tested with the clang compiler. Please, to avoid errors, use clang.

### Quick Setup

Execute the `setup.sh` script to install dependencies, configure and
build the project:

```bash
./setup.sh
```

The script attempts to install all required packages using `apt` or `dnf`
and then configures the build directory and compiles the engine using
Clang and CMake. If package installation fails (e.g. on systems without
network access), the script continues and attempts to build with whatever
dependencies are already available.

![image](https://github.com/user-attachments/assets/34628e2b-0908-41e6-b0da-bc227d9a0bf8)


