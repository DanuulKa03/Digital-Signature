cmake_minimum_required(VERSION 3.28)
project(digital_signature)

set(CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Core5Compat)

add_executable(digital_signature
        src/main.cpp
        src/controller/controller.cpp
        src/model/model.cpp
        src/view/view.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Core5Compat
)

#if(WIN32)
#    target_link_options(digital_signature PRIVATE -static-libstdc++ -static-libgcc)
#    # Полная статика (по необходимости, может потребовать доп. либы):
#    # target_link_options(digital_signature PRIVATE -static)
#endif()
#
#if(WIN32)
#    add_custom_command(TARGET digital_signature POST_BUILD
#            COMMAND ${CMAKE_COMMAND}
#            -DDEPS=\"$<TARGET_RUNTIME_DLLS:digital_signature>\"
#            -DDEST=\"$<TARGET_FILE_DIR:digital_signature>\"
#            -P \"${CMAKE_CURRENT_SOURCE_DIR}/cmake/deploy_runtime.cmake\"
#            COMMAND_EXPAND_LISTS
#    )
#endif()