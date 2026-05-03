#include "Gpu.h"
#include "Logger.h"
#include <vulkan/vk_enum_string_helper.h>

#include <unordered_map>
#include <vector>

//Extension Functions
PFN_vkCreateDebugUtilsMessengerEXT Paradox::Gpu::vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT Paradox::Gpu::vkDestroyDebugUtilsMessengerEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT Paradox::Gpu::vkSetDebugUtilsObjectNameEXT = nullptr;

#define LOAD_VULKAN_FUNCTION(x) { \
    auto fn = (PFN_##x)vkGetInstanceProcAddr(m_Instance, #x);\
    if(fn != nullptr){ Paradox::Gpu::x = fn; Paradox::Log::Debug("Loaded Vulkan Instance Function " #x " -> <0x%08x>.\n", fn); }\
    else CheckVkResult(VK_ERROR_EXTENSION_NOT_PRESENT, "[Vulkan]\tUnable to load Function " #x " !\n"); \
}\

Paradox::Gpu::Gpu()
{
    m_Instance = VK_NULL_HANDLE;
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;

    m_VmaAllocator = VK_NULL_HANDLE;
    m_pAllocationCallbacks = nullptr;

    m_DebugMessenger = VK_NULL_HANDLE;
}

Paradox::ParadoxError Paradox::Gpu::Init(const GpuInitInfo* pInitInfo)
{
    Log::Message("[Paradox]\tInitializing GPU...\n");
    ParadoxError err = ParadoxError::Success;

    CreateInstance(pInitInfo);
    LoadInstanceFunctions(pInitInfo);

    if (pInitInfo->createDebug) {
        CreateDebugMessenger();
    }

    AcquirePhyicalDevice();


    return err;
}

Paradox::ParadoxError Paradox::Gpu::Shutdown()
{
    Log::Message("[Paradox]\tShutting Down GPU...\n");

    DestroyDebugMessenger();
    DestroyInstance();

    return ParadoxError::NotImplemented;
}

VkResult Paradox::Gpu::CheckVkResult(const VkResult res, const std::string& msg)
{
    if (res < VK_SUCCESS) {
        Log::Warning("VkResult Failed! [%s]\t%s\n", string_VkResult(res), msg.c_str());
    }
    return res;
}


VkResult Paradox::Gpu::LoadInstanceFunctions(const GpuInitInfo* pInitInfo) {
    Log::Print(ELogColour::Magenta, "[Vulkan]\tLoading Instance Functions.\n");
    if (m_Instance == VK_NULL_HANDLE) {
        return CheckVkResult(VK_ERROR_DEVICE_LOST, "Invalid Vulkan Instance!\n");
    }

    if (pInitInfo->createDebug) {
        LOAD_VULKAN_FUNCTION(vkCreateDebugUtilsMessengerEXT);
        LOAD_VULKAN_FUNCTION(vkDestroyDebugUtilsMessengerEXT);
        LOAD_VULKAN_FUNCTION(vkSetDebugUtilsObjectNameEXT);
    }
    return VK_SUCCESS;
}

VkResult Paradox::Gpu::CreateInstance(const GpuInitInfo* pInitInfo)
{
    Log::Print(ELogColour::Magenta, "[Vulkan]\tCreating VkInstance...\n");

    //Check Default init info.
    const GpuInitInfo defaultInitInfo = {
        .applicationName = "Paradox-Application",
        .applicationVersion = PackVersion(0, 0, 0),
        .createDebug = true,
    };

    if (!pInitInfo) {
        pInitInfo = &defaultInitInfo;
    }

    //Enumerate required instance layers / extensions
    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;

    const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = DebugMessengerCallback,
        .pUserData = this
    };

    if (pInitInfo->createDebug == true) {
        Log::Message("[Paradox]\tEnabling Debug Layers.\n");
        instanceLayers.push_back("VK_LAYER_KHRONOS_validation");
        instanceExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    //Populate Application Info 
    const VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = pInitInfo->applicationName.c_str(),
        .applicationVersion = pInitInfo->applicationVersion,
        .pEngineName = "Paradox",
        .apiVersion = VK_API_VERSION_1_2,
    };

    //Create the instance. 
    const VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = pInitInfo->createDebug ? &debugMessengerCreateInfo : nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = static_cast<uint32_t>(instanceLayers.size()),
        .ppEnabledLayerNames = instanceLayers.data(),
        .enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size()),
        .ppEnabledExtensionNames = instanceExtensions.data(),
    };

    VkResult res = vkCreateInstance(&createInfo, m_pAllocationCallbacks, &m_Instance);

    Log::Debug("vkCreateInstance(...) -> <0x%08x>\n", m_Instance);
    return res;
}

void Paradox::Gpu::DestroyInstance()
{
    Log::Print(ELogColour::Magenta, "[Vulkan]\tDestroying VkInstance...\n");

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, m_pAllocationCallbacks);
        m_Instance = nullptr;
    }
}

VkResult Paradox::Gpu::AcquirePhyicalDevice()
{
    Log::Print(ELogColour::Magenta, "[Vulkan]\tSelecting a Physical Device...\n");


    //Evaluate Physical Device feature support. 
    VkPhysicalDeviceFeatures requiredFeatures = {};

    //Enumerate existing physical devices
    std::vector<VkPhysicalDevice> physicalDevices;
    {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
        physicalDevices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.data());
    }

    //Prefer Discrete GPUs -> Integrated GPUs -> CPUs -> Software Driver
      //Require Vulkan 1.0 support. 
    VkPhysicalDevice deviceCandidate = VK_NULL_HANDLE;
    VkPhysicalDeviceType candidateType = VK_PHYSICAL_DEVICE_TYPE_MAX_ENUM;
    {
        for (const VkPhysicalDevice device : physicalDevices) {
            //Query the Physical Device Properties for the relevant information. 
            VkPhysicalDeviceProperties deviceProperties = {};
            vkGetPhysicalDeviceProperties(device, &deviceProperties);

            //Skip any devices that don't support Vulkan 1.2
            if (deviceProperties.apiVersion < VK_API_VERSION_1_2) {
                Log::Debug("Device [%s](%s) Skipped: Vulkan API Version (%d.%d.%d) was unsupported!\n", deviceProperties.deviceName, string_VkPhysicalDeviceType(deviceProperties.deviceType), VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion));
                continue;
            }

            {
                //Evaluate device feature compatibility
                VkPhysicalDeviceFeatures deviceFeatures = {};
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

                bool featuresValid = true;

                for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
                    VkBool32* a = ((VkBool32*)&requiredFeatures) + i;
                    VkBool32* b = ((VkBool32*)&deviceFeatures) + i;

                    if (*a == VK_TRUE) {
                        if (*b != VK_TRUE) {
                            featuresValid = false;
                            Log::Debug("Device [%s](%s) Skipped: Not all required features are supported!\n", deviceProperties.deviceName, string_VkPhysicalDeviceType(deviceProperties.deviceType));
                        }
                    }
                }

                if (!featuresValid) {
                    Log::Debug("Not all required Physical Device Features were available!\n");
                    continue;
                }
            }

            //Cache the physical device candidate
            {
                //Convenience Lambda
                auto selectCandidate = [&]() {
                    Log::Debug("Device [%s](%s) Is the current Best Candidate!\n\tVulkan API Version (%d.%d.%d)\n\tDriver Version (%d.%d.%d)\n", deviceProperties.deviceName, string_VkPhysicalDeviceType(deviceProperties.deviceType), VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion), VK_API_VERSION_MAJOR(deviceProperties.driverVersion), VK_API_VERSION_MINOR(deviceProperties.driverVersion), VK_API_VERSION_PATCH(deviceProperties.driverVersion));

                    deviceCandidate = device;
                    candidateType = deviceProperties.deviceType;
                    };

                //Only evaluate if there IS a candidate already present
                if (deviceCandidate != VK_NULL_HANDLE) {
                    if (candidateType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                        selectCandidate();
                    }
                    else {
                        selectCandidate();
                    }
                }
                else {
                    selectCandidate(); //This is the first candidate, so just select it anyway!
                }

            }
        }

        //Complain if there were no supported devices. 
        if (deviceCandidate == VK_NULL_HANDLE) {
            Log::Warning("No Physical Device Candidates were Valid!\n");
        }

        //Write-back the passing device candidate
        m_PhysicalDevice = deviceCandidate;
    }


    Log::Debug("Physical Device Acquired -> <0x%08x>\n", m_PhysicalDevice);


    return m_PhysicalDevice != VK_NULL_HANDLE ? VK_SUCCESS : VK_ERROR_DEVICE_LOST;
}

VkResult Paradox::Gpu::CreateDebugMessenger()
{
    Log::Print(ELogColour::Magenta, "[Vulkan]\tCreating Debug Utils Messenger...\n");
    if (m_Instance == VK_NULL_HANDLE) {
        return CheckVkResult(VK_ERROR_DEVICE_LOST, "Invalid Vulkan Instance!\n");
    }

    const VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
        .pfnUserCallback = DebugMessengerCallback,
        .pUserData = this
    };

    VkResult res = Gpu::vkCreateDebugUtilsMessengerEXT(m_Instance, &debugMessengerCreateInfo, m_pAllocationCallbacks, &m_DebugMessenger);

    Log::Debug("vkCreateDebugUtilsMessengerEXT(...) -> <0x%08x>\n", m_DebugMessenger);
    return res;
}

void Paradox::Gpu::DestroyDebugMessenger()
{
    if (m_Instance == VK_NULL_HANDLE) {
        CheckVkResult(VK_ERROR_DEVICE_LOST, "Invalid Vulkan Instance!\n");
        return;
    }

    if (m_DebugMessenger != VK_NULL_HANDLE) {
        Gpu::vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, m_pAllocationCallbacks);
        m_DebugMessenger = nullptr;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL Paradox::Gpu::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    Log::Print(ELogColour::LightMagenta, "[%d : %s]\t%s\n", pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);
    return VK_FALSE;
}
