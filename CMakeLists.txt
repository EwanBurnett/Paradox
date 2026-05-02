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

target_compile_definitions(${PROJECT_NAME} PUBLIC
    # Target C++ 20
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)


# Shader Compilation 
function(CompileShader SOURCE_PATH DST_PATH)
    message(STATUS "${SOURCE_PATH} ${DST_PATH}!\n")
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD 
        COMMAND ${glslc_EXECUTABLE} ${SOURCE_PATH} -o "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources/Shaders/${DST_PATH}" --target-env=vulkan1.2
        COMMENT "Compiling Shader ${SOURCE_PATH} -> ${DST_PATH}"
    )
endfunction()

file(GLOB SHADERS "${CMAKE_SOURCE_DIR}/Resources/Shaders/*")

foreach(SHADER ${SHADERS})
    string(REGEX REPLACE "(\\.[^.]+)$" ".spv" OUT_SHADER ${SHADER})
    string(REGEX REPLACE "(.*\/)" "" OUT_SHADER ${OUT_SHADER})

    CompileShader(${SHADER} ${OUT_SHADER}) 
endforeach()


# Copy Resources to the output directory
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/Resources/"
)
