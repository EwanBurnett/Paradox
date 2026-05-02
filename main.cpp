#include <cstdio> 
#include <vulkan/vulkan.h>

#include "Paradox/Logger.h"

int main() {
    for (size_t i = 0; i < (size_t)Paradox::ELogColour::ELogColour_MAX; ++i) {
        Paradox::Log::Print((Paradox::ELogColour)i, "Hello, RT!\t[%d]\n", i);
    }

    VkInstanceCreateInfo i = {};

    return 0;     
}