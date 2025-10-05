add_library(math_ntru STATIC
        src/hash.cpp
        src/polynomials.cpp
        src/arithmetic.cpp

        src/ntru/keys.cpp
        src/ntru/ntru.cpp
)

set_target_properties(math_ntru PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(math_ntru PUBLIC
        include
)