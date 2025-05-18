cmake_minimum_required(VERSION 3.31)
project(ForgeEngine)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(ForgeEngine)

add_executable(Nidavellir Nidavellir/Nidavellir.cpp ForgeEngine)

target_include_directories(Nidavellir PUBLIC
        ForgeEngine
)

target_link_libraries(Nidavellir PUBLIC
        ForgeEngine
)