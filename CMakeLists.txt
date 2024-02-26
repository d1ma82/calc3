cmake_minimum_required(VERSION 3.12)

project(calc)

file(GLOB SRC 
"./src/main.cpp"
"./src/lexer.cpp"
"./src/syntax.cpp"
#"./src/exec.cpp"
)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -S -g")

add_executable(${PROJECT_NAME} ${SRC})
set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../bin")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
endif()