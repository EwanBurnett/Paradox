cmake_minimum_required(VERSION 3.14)

project("Paradox")

# Install Dependencies
find_package(Vulkan REQUIRED)
find_package(Threads REQUIRED)

# Find GLSLC for shader compilation
find_program(glslc_EXECUTABLE NAMES glslc HINTS Vulkan::glslc)
if(NOT glslc_EXECUTABLE)
    message(FATAL_ERROR "glslc executable was not found!")
endif()


# Third-Party Dependencies
include(FetchContent)

# Tracy 
FetchContent_Declare(
    tracy
    GIT_REPOSITORY https://github.com/wolfpld/tracy.git 
	GIT_TAG v0.13.1
    GIT_SHALLOW TRUE
)
set(TRACY_ENABLE ${MIDORI_ENABLE_PROFILING})
set(TRACY_ON_DEMAND ${MIDORI_ENABLE_PROFILING})

# GLM
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/icaven/glm
    GIT_SHALLOW TRUE
)

# GLFW
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_SHALLOW TRUE
)

# VMA 
FetchContent_Declare(
	vma
	GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
	GIT_TAG v3.3.0
    GIT_SHALLOW TRUE
)
# Disable VMA sample builds by default
set(VMA_BUILD_SAMPLES OFF)

# ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.92.7
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(tracy glm glfw vma imgui)


# Manually build ImGui as a static library. 
add_library(imgui 
	"${imgui_SOURCE_DIR}/imgui.cpp" 
	"${imgui_SOURCE_DIR}/imgui_demo.cpp"
	"${imgui_SOURCE_DIR}/imgui_draw.cpp" 
	"${imgui_SOURCE_DIR}/imgui_tables.cpp" 
	"${imgui_SOURCE_DIR}/imgui_widgets.cpp" 
	"${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
	"${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"
)
target_link_libraries(imgui PRIVATE 
    glfw Vulkan::Vulkan
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# Build the executable. 

add_executable(${PROJECT_NAME} 
    main.cpp
)

# Link Library Dependencies
target_link_libraries(${PROJECT_NAME}
    Threads::Threads
    Vulkan::Vulkan
    VulkanMemoryAllocator
    glm
    glfw
    imgui
    TracyClient
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
