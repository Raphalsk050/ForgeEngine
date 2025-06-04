cmake_minimum_required(VERSION 3.16)
project(ForgeApplication)

set(CMAKE_CXX_STANDARD 20)
add_subdirectory(ForgeEngine)
add_subdirectory(Nidavellir)

set(NUM_CORES 30)
set(CMAKE_BUILD_PARALLEL_LEVEL ${NUM_CORES} CACHE STRING "Number of parallel jobs" FORCE)

set_property(GLOBAL PROPERTY JOB_POOLS link_pool=${NUM_CORES})
set_property(TARGET ForgeEngine PROPERTY JOB_POOL_COMPILE link_pool)
set_property(TARGET ForgeEngine PROPERTY JOB_POOL_LINK link_pool)
set_property(TARGET Nidavellir PROPERTY JOB_POOL_COMPILE link_pool)
set_property(TARGET Nidavellir PROPERTY JOB_POOL_LINK link_pool)