set(FIRST_RUN_STAMP "${CMAKE_BINARY_DIR}/first_run.stamp")

if(NOT EXISTS ${FIRST_RUN_STAMP})
    message(STATUS "Performing first run setup...")

    if(NOT DEFINED CMAKE_CXX_COMPILER)
        find_program(FOUND_CLANGXX clang++)
        find_program(FOUND_GPP g++)
        if(FOUND_CLANGXX)
            set(CMAKE_CXX_COMPILER ${FOUND_CLANGXX} CACHE FILEPATH "" FORCE)
            message(STATUS "Using ${FOUND_CLANGXX} as C++ compiler")
        elseif(FOUND_GPP)
            set(CMAKE_CXX_COMPILER ${FOUND_GPP} CACHE FILEPATH "" FORCE)
            message(STATUS "Using ${FOUND_GPP} as C++ compiler")
        else()
            message(FATAL_ERROR "No suitable C++ compiler found. Install clang++ or g++")
        endif()
    endif()

    if(NOT DEFINED CMAKE_C_COMPILER)
        find_program(FOUND_CLANG clang)
        find_program(FOUND_GCC gcc)
        if(FOUND_CLANG)
            set(CMAKE_C_COMPILER ${FOUND_CLANG} CACHE FILEPATH "" FORCE)
            message(STATUS "Using ${FOUND_CLANG} as C compiler")
        elseif(FOUND_GCC)
            set(CMAKE_C_COMPILER ${FOUND_GCC} CACHE FILEPATH "" FORCE)
            message(STATUS "Using ${FOUND_GCC} as C compiler")
        else()
            message(FATAL_ERROR "No suitable C compiler found. Install clang or gcc")
        endif()
    endif()

    set(DEP_DIR "${CMAKE_SOURCE_DIR}/ForgeEngine/ThirdParty")
    set(REQUIRED_DIRS
        glfw
        glad
        glm
        entt
        spdlog
        imgui
        ImGuizmo
        shaderc
        SPIRV-Cross
        stbimage
    )
    foreach(dir IN LISTS REQUIRED_DIRS)
        if(NOT EXISTS "${DEP_DIR}/${dir}")
            message(FATAL_ERROR "Missing dependency directory: ${DEP_DIR}/${dir}")
        endif()
    endforeach()

    file(WRITE ${FIRST_RUN_STAMP} "first run complete")
    message(STATUS "First run setup complete.")
endif()
