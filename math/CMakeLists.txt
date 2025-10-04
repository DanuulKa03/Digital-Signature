add_library(math_ntru STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/hash.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/polynomials.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/arithmetic.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/src/ntru/keys.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/ntru/ntru.cpp
        src/ntru/ntru.cpp
)

set_target_properties(math_ntru PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(math_ntru PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include/
)