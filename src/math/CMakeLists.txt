add_library(math_ntru STATIC
        src/hash.cpp
        src/polynomials.cpp
        src/arithmetic.cpp

        src/ntru/keys.cpp
        src/ntru/ntru.cpp
)

set_target_properties(math_ntru PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(math_ntru PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_options(math_ntru PRIVATE -Wall -Wextra -Wpedantic)
set_target_properties(math_ntru PROPERTIES POSITION_INDEPENDENT_CODE ON)
