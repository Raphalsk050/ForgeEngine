add_executable(Nidavellir
        Nidavellir.cpp
)

target_link_libraries(Nidavellir PRIVATE ForgeEngine ImGui)
target_include_directories(Nidavellir PRIVATE ${PROJECT_SOURCE_DIR}/ForgeEngine ${PROJECT_SOURCE_DIR}/ForgeEngine/ThirdParty/imgui)