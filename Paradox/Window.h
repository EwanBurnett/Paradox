#ifndef __WINDOW_H
#define __WINDOW_H
 
#include <GLFW/glfw3.h>
#include <string> 
#include <filesystem> 
#include "Utility.h"

namespace Paradox {

    class Window {
    public: 
        Window(); 

        ParadoxError Create(const uint16_t width, const uint16_t height, const std::string& title); 
        ParadoxError Destroy(); 

        bool PollEvents(); 

        void SetWidth(const uint16_t width);
        const uint16_t GetWidth() const;
        void SetHeight(const uint16_t height);
        const uint16_t GetHeight() const;
        void SetTitle(const std::string& title);
        const std::string& GetTitle() const;
         
        void SetIcon(const std::filesystem::path& path);

        GLFWwindow* GetGLFWHandle() const;

    private: 
        GLFWwindow* m_Handle; 

        uint16_t m_Width; 
        uint16_t m_Height; 

        std::string m_Title; 

    };
}

#endif// __WINDOW_H