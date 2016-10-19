project(wintertoes)
cmake_minimum_required(VERSION 3.5)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(wintertoes main.cpp Logging.cpp)
target_link_libraries(wintertoes ${CONAN_LIBS} pthread)
target_compile_features(wintertoes PRIVATE cxx_alias_templates cxx_lambda_init_captures)
