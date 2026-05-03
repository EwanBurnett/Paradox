#include "Window.h"
#include "Profiler.h"
#include "Logger.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Paradox::Window::Window()
{
    ParadoxZoneScoped;

    m_Width = 0; 
    m_Height = 0; 
    m_Handle = nullptr; 
    m_Title = "";
}

Paradox::ParadoxError Paradox::Window::Create(const uint16_t width, const uint16_t height, const std::string& title)
{
    ParadoxZoneScoped;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); 

    //Create the window. 
    m_Width = width; 
    m_Height = height; 
    m_Title = title; 

    m_Handle = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr); 
    if (m_Handle == nullptr) {
        PARADOX_ERROR("Failed to Create Window %s!\n", title.c_str()); 
        return ParadoxError::Failed;
    }

    glfwSetWindowUserPointer(m_Handle, this); 

    return ParadoxError::Success; 
}

Paradox::ParadoxError Paradox::Window::Destroy()
{
    ParadoxZoneScoped;
    if (m_Handle) {
        Log::Message("[Paradox]\tDestroying Window %s <0x%08x>\n", m_Title.c_str(), m_Handle);
        glfwDestroyWindow(m_Handle);
        m_Handle = nullptr; 
    }
    return ParadoxError::Success;
}

bool Paradox::Window::PollEvents()
{
    ParadoxZoneScoped;
    if (m_Handle == nullptr) {
        PARADOX_ERROR("Invalid Window Handle!\n"); 
    }

    if (!glfwWindowShouldClose(m_Handle)) {
        glfwPollEvents(); 
        return true; 
    }

    return false;
}

void Paradox::Window::SetWidth(const uint16_t width)
{
    ParadoxZoneScoped;
    m_Width = width; 
    glfwSetWindowSize(m_Handle, m_Width, m_Height); 
}
const uint16_t Paradox::Window::GetWidth() const 
{
    ParadoxZoneScoped;
    return m_Width; 
}

void Paradox::Window::SetHeight(const uint16_t height)
{
    ParadoxZoneScoped;
    m_Height = height; 
    glfwSetWindowSize(m_Handle, m_Width, m_Height); 
}

const uint16_t Paradox::Window::GetHeight() const 
{
    ParadoxZoneScoped;
    return m_Height; 
}

void Paradox::Window::SetTitle(const std::string& title)
{
    ParadoxZoneScoped;
    m_Title = title; 
    glfwSetWindowTitle(m_Handle, title.c_str()); 
}

const std::string& Paradox::Window::GetTitle() const
{
    ParadoxZoneScoped;
    return m_Title; 
}

void Paradox::Window::SetIcon(const std::filesystem::path& path)
{
    ParadoxZoneScoped; 
    if (std::filesystem::exists(path)) {

        //Load the image through STB. 
        int size_x = 0; 
        int size_y = 0; 
        int num_channels = 0; 

        uint8_t* imageData = stbi_load((const char*)path.generic_u8string().c_str(), &size_x, &size_y, &num_channels, 4); 
        if (imageData != nullptr) {
            //Bind the icon to the window. 
            GLFWimage iconImage;
            iconImage.pixels = imageData;
            iconImage.width = size_x;
            iconImage.height = size_y;

            glfwSetWindowIcon(m_Handle, 1, &iconImage);

            stbi_image_free(imageData);
        }
        else {
            Log::Warning("Failed to load file %s!\n", path.c_str()); 
        }
    }
    else {
        Log::Warning("Icon %s does not exist!\n", path.c_str()); 
    }
}

GLFWwindow* Paradox::Window::GetGLFWHandle() const
{
    ParadoxZoneScoped;
    return m_Handle;
}
