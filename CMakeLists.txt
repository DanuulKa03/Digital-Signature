cmake_minimum_required(VERSION 3.28)
project(digital_signature)

set(CMAKE_CXX_STANDARD 23)

add_library(math_ntru STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/math/arithmetic.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/common.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/polynomials.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/hash.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/math/ntru/keys.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/math/ntru/ntru.hpp
)

set_target_properties(math_ntru PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(math_ntru PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(digital_signature main.cpp)

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