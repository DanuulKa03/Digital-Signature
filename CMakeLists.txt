cmake_minimum_required(VERSION 3.28)
project(digital_signature)

set(CXX_STANDARD 23)
set(CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_PREFIX_PATH "C:/Qt/6.9.3/mingw_64/lib/cmake")
find_package(Qt6 REQUIRED COMPONENTS Core)

add_executable(digital_signature
        src/main.cpp
        src/controller/controller.cpp
        src/model/model.cpp
        src/view/view.cpp
        src/controller/controller.h
        src/model/model.h
        src/view/view.h

)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
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