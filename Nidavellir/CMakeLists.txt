add_executable(Nidavellir
        NidavellirLayer.h
        NidavellirLayer.cpp
        Nidavellir.cpp
)


target_link_libraries(Nidavellir PUBLIC ForgeEngine ImGui)
target_include_directories(Nidavellir PUBLIC ${PROJECT_SOURCE_DIR}/ForgeEngine ${PROJECT_SOURCE_DIR}/ForgeEngine/ThirdParty/imgui)