#include <cstdio> 
#include <vulkan/vulkan.h>

#include "Paradox/Logger.h"

int main() {
    for (size_t i = 0; i < (size_t)Paradox::ELogColour::ELogColour_MAX; ++i) {
        Paradox::Log::Print((Paradox::ELogColour)i, "Hello, RT!\t[%d]\n", i);
    }

    Paradox::Log::Warning("A warning!\n"); 
    Paradox::Log::Error(__FILE__, __LINE__, __PRETTY_FUNCTION__, "Something went wrong!\n"); 

    VkInstanceCreateInfo i = {};

    return 0;     
}