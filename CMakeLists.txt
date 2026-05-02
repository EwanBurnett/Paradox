cmake_minimum_required(VERSION 3.14)

project("Paradox")

# Install Dependencies
find_package(Vulkan REQUIRED)

# Find GLSLC for shader compilation
find_program(glslc_EXECUTABLE NAMES glslc HINTS Vulkan::glslc)
if(NOT glslc_EXECUTABLE)
    message(FATAL_ERROR "glslc executable was not found!")
endif()


add_executable(${PROJECT_NAME} 
    main.cpp
)

# Link Library Dependencies
target_link_libraries(${PROJECT_NAME}
    Vulkan::Vulkan
)


target_compile_definitions(${PROJECT_NAME} 
    # Target C++ 20
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED true
)



