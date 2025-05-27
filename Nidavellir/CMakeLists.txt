add_executable(Nidavellir
        NidavellirLayer.h
        NidavellirLayer.cpp
        Nidavellir.cpp
)

#add_custom_command(TARGET Nidavellir PRE_BUILD
#        COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:Nidavellir>/Assets/Shaders
#)

target_link_libraries(Nidavellir PUBLIC ForgeEngine ImGui)
target_include_directories(Nidavellir PUBLIC ${PROJECT_SOURCE_DIR}/ForgeEngine ${PROJECT_SOURCE_DIR}/ForgeEngine/ThirdParty/imgui)