cmake_minimum_required(VERSION 3.28)
project(digital_signature)

option(FLAG_GUI "Запус приложения в с GUI" OFF)

set(CMAKE_CXX_STANDARD 23)

if (FLAG_GUI)
    find_package(Qt5 COMPONENTS Core Gui Qml Sql Network REQUIRED)
endif ()

add_subdirectory(math)

add_executable(digital_signature
        main.cpp
        console/utils.cpp
        console/utils.hpp
        console/settings.cpp
        console/settings.hpp
)

target_link_libraries(digital_signature PRIVATE math_ntru)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    math_ntru
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
target_compile_options(digital_signature PRIVATE -Wall -Wextra -Wpedantic)

target_include_directories(digital_signature PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/math/include)
