#include "Gpu.h"
#include "Logger.h"
#include <vulkan/vk_enum_string_helper.h>

#include <unordered_map>
#include <vector>
#include <cstring>
#include <assert.h>
#include <set> 
#include "Profiler.h"

#include "Window.h"

#ifdef _MSC_VER
#define VK_LOG(message, ...) Paradox::Log::Print(Paradox::ELogColour::Magenta, "[Vulkan]\t" message, ##__VA_ARGS__)
#else
#define VK_LOG(message, ...) Paradox::Log::Print(Paradox::ELogColour::Magenta, "[Vulkan]\t" message __VA_OPT__(,) __VA_ARGS__)
#endif

//Extension Functions
PFN_vkCreateDebugUtilsMessengerEXT Paradox::Gpu::vkCreateDebugUtilsMessengerEXT = nullptr;
PFN_vkDestroyDebugUtilsMessengerEXT Paradox::Gpu::vkDestroyDebugUtilsMessengerEXT = nullptr;
PFN_vkSetDebugUtilsObjectNameEXT Paradox::Gpu::vkSetDebugUtilsObjectNameEXT = nullptr;

#define LOAD_VULKAN_FUNCTION(x) { \
    auto fn = (PFN_##x)vkGetInstanceProcAddr(m_Instance, #x);\
    if(fn != nullptr){ Paradox::Gpu::x = fn; Paradox::Log::Debug("Loaded Vulkan Instance Function " #x " -> <0x%08x>.\n", fn); }\
    else CheckVkResult(VK_ERROR_EXTENSION_NOT_PRESENT, "Unable to load Function " #x " !\n"); \
}\


//TODO: Promote this to a static const?
static std::unordered_map<Paradox::EGpuFeatureCapabilities, const char* > kGpuFeatureCapabilitiesNames = {
    {Paradox::EGpuFeatureCapabilities::Required, "Required"},
    {Paradox::EGpuFeatureCapabilities::Bindless, "Bindless"},
    {Paradox::EGpuFeatureCapabilities::Dynamic_Rendering, "Dynamic Rendering"},
    {Paradox::EGpuFeatureCapabilities::Ray_Tracing_Pipeline, "Ray Tracing Pipeline"},
    {Paradox::EGpuFeatureCapabilities::Ray_Query, "Ray Query"},
    {Paradox::EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX, "Invalid!"},
    {Paradox::EGpuFeatureCapabilities::EGpuFeatureCapabilities_COUNT, "Invalid!"},
};


struct FeatureRequirements {
    VkPhysicalDeviceFeatures features = {};
    std::vector<const char*> deviceExtensions;
};

static const std::unordered_map<Paradox::EGpuFeatureCapabilities, FeatureRequirements> kFeatureRequirements = {
    {
        Paradox::EGpuFeatureCapabilities::Required, {
            .features = {},
            .deviceExtensions = {VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME },
        }
    },
    {
        Paradox::EGpuFeatureCapabilities::Bindless, {
            .features = {},
            .deviceExtensions = {VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME },
        }
    },
    {
        Paradox::EGpuFeatureCapabilities::Ray_Tracing_Pipeline, {
            .features = {},
            .deviceExtensions = {VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME },
        }
    },
    {
        Paradox::EGpuFeatureCapabilities::Ray_Query, {
            .features = {},
            .deviceExtensions = {VK_KHR_RAY_QUERY_EXTENSION_NAME, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME, VK_KHR_SPIRV_1_4_EXTENSION_NAME, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME },
        }
    },
    {
        Paradox::EGpuFeatureCapabilities::Dynamic_Rendering, {
            .features = {},
            .deviceExtensions = {VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME},
        }
    },
};


Paradox::Gpu::Gpu()
{
    ParadoxZoneScoped;
    m_Instance = VK_NULL_HANDLE;
    m_PhysicalDevice = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;

    m_VmaAllocator = VK_NULL_HANDLE;
    m_pAllocationCallbacks = nullptr;

    m_DebugMessenger = VK_NULL_HANDLE;
    m_Capabilities = {};

    m_EnableDebugUtils = false;
}

Paradox::ParadoxError Paradox::Gpu::Init(const GpuInitInfo* pInitInfo)
{
    ParadoxZoneScoped;
    Log::Message("[Paradox]\tInitializing GPU...\n");
    ParadoxError err = ParadoxError::Success;

    //Check Default init info.
    const GpuInitInfo defaultInitInfo = {
        .applicationName = "Paradox-Application",
        .applicationVersion = PackVersion(0, 0, 0),
        .createDebug = true,
        .overridePhysicalDevice = false,
        .physicalDeviceOverrideIdx = (uint8_t)-1,
    };

    if (!pInitInfo) {
        pInitInfo = &defaultInitInfo;
    }

    CreateInstance(pInitInfo);
    LoadInstanceFunctions(pInitInfo);

    if (pInitInfo->createDebug) {
        m_EnableDebugUtils = true;
        CreateDebugMessenger();
    }

    AcquirePhyicalDevice(pInitInfo->overridePhysicalDevice, pInitInfo->physicalDeviceOverrideIdx);
    CreateDevice();

    //Set debug object names 
    {
        SetDebugObjectName(reinterpret_cast<uint64_t>(m_Instance), VK_OBJECT_TYPE_INSTANCE, "Paradox Instance");
        SetDebugObjectName(reinterpret_cast<uint64_t>(m_Device), VK_OBJECT_TYPE_DEVICE, "Paradox Device");
        //SetDebugObjectName(reinterpret_cast<uint64_t>(m_DebugMessenger), VK_OBJECT_TYPE_DEBUG_UTILS_MESSENGER_EXT, "Paradox Debug Messenger");
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(m_PhysicalDevice, &deviceProperties);
        SetDebugObjectName(reinterpret_cast<uint64_t>(m_PhysicalDevice), VK_OBJECT_TYPE_PHYSICAL_DEVICE, deviceProperties.deviceName);
    }

    return err;
}

Paradox::ParadoxError Paradox::Gpu::Shutdown()
{
    ParadoxZoneScoped;
    Log::Message("[Paradox]\tShutting Down GPU...\n");

    DestroyDevice();
    DestroyDebugMessenger();
    DestroyInstance();

    return ParadoxError::NotImplemented;
}

std::bitset<(size_t)Paradox::EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX> Paradox::Gpu::GetCapabilities() const
{
    ParadoxZoneScoped;
    return m_Capabilities;
}

bool Paradox::Gpu::GetCapabilitySupport(EGpuFeatureCapabilities capability) const
{
    return m_Capabilities.test((size_t)capability);
}

VkResult Paradox::Gpu::CheckVkResult(const VkResult res, const std::string& msg)
{
    VulkanZoneScoped;
    if (res < VK_SUCCESS) {
        PARADOX_ERROR("VkResult Failed! [%s]\t%s\n", string_VkResult(res), msg.c_str());
    }
    return res;
}


VkResult Paradox::Gpu::LoadInstanceFunctions(const GpuInitInfo* pInitInfo)
{
    VulkanZoneScoped;
    VK_LOG("Loading Instance Functions.\n");
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
    VulkanZoneScoped;
    VK_LOG("Creating VkInstance...\n");

    //Enumerate required instance layers / extensions
    std::vector<const char*> instanceLayers;
    std::vector<const char*> instanceExtensions;

#if WIN32 | __linux__
    //Get GLFW required extensions
    uint32_t count = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&count);
    for (uint32_t i = 0; i < count; ++i) {
        instanceExtensions.push_back(glfwExtensions[i]);
    }
#endif

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

    for (auto& ext : instanceExtensions) {
        Log::Debug("Enabling Instance Extension %s.\n", ext);
    }

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

    return CheckVkResult(res);
}

void Paradox::Gpu::DestroyInstance()
{
    VulkanZoneScoped;
    VK_LOG("Destroying VkInstance...\n");

    if (m_Instance != VK_NULL_HANDLE) {
        vkDestroyInstance(m_Instance, m_pAllocationCallbacks);
        m_Instance = nullptr;
    }
}

VkResult Paradox::Gpu::AcquirePhyicalDevice(const bool overridePhysicalDevice, const uint8_t overrideIndex)
{
    VulkanZoneScoped;
    VK_LOG("Selecting a Physical Device...\n");

    //Enumerate existing physical devices
    std::vector<VkPhysicalDevice> physicalDevices;
    {
        uint32_t physicalDeviceCount = 0;
        vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, nullptr);
        physicalDevices.resize(physicalDeviceCount);
        vkEnumeratePhysicalDevices(m_Instance, &physicalDeviceCount, physicalDevices.data());
    }

    if (overridePhysicalDevice) {
        if (overrideIndex >= physicalDevices.size() && overrideIndex != (uint8_t)-1) {
            Log::Warning("Invalid Physical Device override index [%d]!\nProceeding with automatic device selection.\n", overrideIndex);
        }
        else {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(physicalDevices[overrideIndex], &deviceProperties);
            Log::Warning("Overriding Physical Device: %s\n", deviceProperties.deviceName);
            Log::Debug("%s <0x%08x>\n\t%s\n\tVulkan API Version (%d.%d.%d)\n\tDriver Version (%d.%d.%d)\n", deviceProperties.deviceName, physicalDevices[overrideIndex], string_VkPhysicalDeviceType(deviceProperties.deviceType), VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion), VK_API_VERSION_MAJOR(deviceProperties.driverVersion), VK_API_VERSION_MINOR(deviceProperties.driverVersion), VK_API_VERSION_PATCH(deviceProperties.driverVersion));

            m_PhysicalDevice = physicalDevices[overrideIndex];

            //Evaluate device feature compatibility
            //Device Features
            {
                VkPhysicalDeviceFeatures deviceFeatures = {};
                vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &deviceFeatures);


                for (auto& featureSet : kFeatureRequirements) {
                    if (featureSet.first == EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX) {
                        break;
                    }

                    VkPhysicalDeviceFeatures requiredFeatures = featureSet.second.features;

                    bool featuresValid = true;
                    for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
                        VkBool32* a = ((VkBool32*)&requiredFeatures) + i;
                        VkBool32* b = ((VkBool32*)&deviceFeatures) + i;

                        if (*a == VK_TRUE) {
                            if (*b != VK_TRUE) {
                                featuresValid = false;
                                Log::Debug("Device [%s](%s) Skipped: Not all required features are supported!\n", deviceProperties.deviceName, string_VkPhysicalDeviceType(deviceProperties.deviceType));
                                break;
                            }
                        }
                    }


                    if (!featuresValid) {
                        Log::Warning("Not all required Physical Device Features were available! [%s]\n", kGpuFeatureCapabilitiesNames[featureSet.first]);
                        m_Capabilities[(size_t)featureSet.first] = 0;
                        continue;
                    }
                    else {
                        Log::Debug("%s Feature Set Supported!\n", kGpuFeatureCapabilitiesNames[featureSet.first]);
                        m_Capabilities[(size_t)featureSet.first] = 1;
                    }
                }
            }

            //Device Extensions
            {
                uint32_t extensionPropertyCount = 0u;
                vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionPropertyCount, nullptr);
                std::vector<VkExtensionProperties> extensionProperties(extensionPropertyCount);
                vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &extensionPropertyCount, extensionProperties.data());


                for (auto& featureSet : kFeatureRequirements) {
                    for (const auto& extension : featureSet.second.deviceExtensions) {

                        //Evaluate Instance Extension Support
                        bool extensionSupported = false;

                        for (const auto& property : extensionProperties) {
                            if (strcmp(property.extensionName, extension) == 0) {
                                extensionSupported = true;
                                break;
                            }
                        }

                        //Add the extension to our internal list, if supported. 
                        if (extensionSupported) {
                            Log::Debug("Device Extension <%s> Supported!\n", extension);
                            //deviceExtensions.push_back(extension);
                            m_Capabilities[(size_t)featureSet.first] = 1;
                        }
                        else {
                            Log::Debug("Extension <%s> is not supported by the current Vulkan Device!\n", extension);
                            Log::Warning("Not all required Device Extensions were available! [%s]\n", kGpuFeatureCapabilitiesNames[featureSet.first]);
                            m_Capabilities[(size_t)featureSet.first] = 0;

                            break;
                        }
                    }
                }
            }

            return CheckVkResult(m_PhysicalDevice != VK_NULL_HANDLE ? VK_SUCCESS : VK_ERROR_DEVICE_LOST);
        }
    }

    if (!physicalDevices.empty()) {
        //Score each available Device Candidate. 
        //Discrete GPU +1000
        //Integrated GPU +500 
        //CPU +100
        //+1000 for each supported feature set (extensions + device features)
        std::unordered_map<VkPhysicalDevice, uint64_t> candidates;
        for (auto& device : physicalDevices) {
            candidates.emplace(device, 0);
        }
        std::unordered_map<VkPhysicalDevice, std::bitset<(size_t)EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX>> capabilities;
        //All feature sets are supported, unless otherwise stated!
        for (auto& device : physicalDevices) {
            for (size_t i = 0; i < (size_t)EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX; ++i) {
                capabilities[device][i] = 1;
            }
        }

        //...

        for (auto& candidate : candidates) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(candidate.first, &deviceProperties);

            //Skip any devices that don't support Vulkan 1.2
            if (deviceProperties.apiVersion < VK_API_VERSION_1_2) {
                Log::Debug("Device [%s](%s) Skipped: Vulkan API Version (%d.%d.%d) was unsupported!\n", deviceProperties.deviceName, string_VkPhysicalDeviceType(deviceProperties.deviceType), VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion));
                candidate.second = 0;
                continue;
            }
            else {
                candidate.second += 1000;
            }

            //Prefer Discrete GPU > Integrated GPU > CPU
            switch (deviceProperties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                candidate.second += 1000;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                candidate.second += 500;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                candidate.second += 100;
                break;
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                candidate.second += 50;
                break;
            default:
                break;
            }


            //Evaluate device feature compatibility
            //Device Features
            {
                VkPhysicalDeviceFeatures deviceFeatures = {};
                vkGetPhysicalDeviceFeatures(candidate.first, &deviceFeatures);


                for (auto& featureSet : kFeatureRequirements) {
                    if (featureSet.first == EGpuFeatureCapabilities::EGpuFeatureCapabilities_MAX) {
                        break;
                    }

                    VkPhysicalDeviceFeatures requiredFeatures = featureSet.second.features;

                    bool featuresValid = true;
                    for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
                        VkBool32* a = ((VkBool32*)&requiredFeatures) + i;
                        VkBool32* b = ((VkBool32*)&deviceFeatures) + i;

                        if (*a == VK_TRUE) {
                            if (*b != VK_TRUE) {
                                featuresValid = false;
                                Log::Debug("Device [%s](%s) Skipped: Not all required features are supported!\n", deviceProperties.deviceName, string_VkPhysicalDeviceType(deviceProperties.deviceType));
                                break;
                            }
                        }
                    }


                    if (!featuresValid) {
                        Log::Warning("Not all required Physical Device Features were available! [%s]\n", kGpuFeatureCapabilitiesNames[featureSet.first]);
                        capabilities[candidate.first][(size_t)featureSet.first] = 0;
                        continue;
                    }
                    else {
                        Log::Debug("%s Feature Set Supported!\n", kGpuFeatureCapabilitiesNames[featureSet.first]);
                        capabilities[candidate.first][(size_t)featureSet.first] = 1;
                        candidate.second += 1000u;
                    }
                }
            }

            //Device Extensions
            {
                uint32_t extensionPropertyCount = 0u;
                vkEnumerateDeviceExtensionProperties(candidate.first, nullptr, &extensionPropertyCount, nullptr);
                std::vector<VkExtensionProperties> extensionProperties(extensionPropertyCount);
                vkEnumerateDeviceExtensionProperties(candidate.first, nullptr, &extensionPropertyCount, extensionProperties.data());


                for (auto& featureSet : kFeatureRequirements) {
                    for (const auto& extension : featureSet.second.deviceExtensions) {

                        //Evaluate Instance Extension Support
                        bool extensionSupported = false;

                        for (const auto& property : extensionProperties) {
                            if (strcmp(property.extensionName, extension) == 0) {
                                extensionSupported = true;
                                break;
                            }
                        }

                        //Add the extension to our internal list, if supported. 
                        if (extensionSupported) {
                            Log::Debug("Device Extension <%s> Supported!\n", extension);
                            //deviceExtensions.push_back(extension);
                            capabilities[candidate.first][(size_t)featureSet.first] = 1;
                            candidate.second += 1000u;
                        }
                        else {
                            Log::Debug("Extension <%s> is not supported by the current Vulkan Device!\n", extension);
                            Log::Warning("Not all required Device Extensions were available! [%s]\n", kGpuFeatureCapabilitiesNames[featureSet.first]);
                            capabilities[candidate.first][(size_t)featureSet.first] = 0;

                            break;
                        }
                    }
                }
            }
        }

        //Select the most appropriate candidate. 
        VkPhysicalDevice selectedCandidate = VK_NULL_HANDLE;
        uint64_t candidateScore = 0u;

        for (const auto& candidate : candidates) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(candidate.first, &deviceProperties);
            Log::Debug("(+%d) %s <0x%08x>\n\t%s\n\tVulkan API Version (%d.%d.%d)\n\tDriver Version (%d.%d.%d)\n", candidate.second, deviceProperties.deviceName, candidate.first, string_VkPhysicalDeviceType(deviceProperties.deviceType), VK_API_VERSION_MAJOR(deviceProperties.apiVersion), VK_API_VERSION_MINOR(deviceProperties.apiVersion), VK_API_VERSION_PATCH(deviceProperties.apiVersion), VK_API_VERSION_MAJOR(deviceProperties.driverVersion), VK_API_VERSION_MINOR(deviceProperties.driverVersion), VK_API_VERSION_PATCH(deviceProperties.driverVersion));
            for (size_t c = (size_t)EGpuFeatureCapabilities::Required; c < (size_t)EGpuFeatureCapabilities::EGpuFeatureCapabilities_COUNT - 1; ++c) {
                //Log::Debug("[%s] - %s\n", kGpuFeatureCapabilitiesNames[c], capabilities[candidate.first][c] ? "True" : "False");
                //TODO: 
            }
            if (candidate.second > candidateScore) {
                selectedCandidate = candidate.first;
            }
        }

        m_PhysicalDevice = selectedCandidate;
        m_Capabilities = capabilities[m_PhysicalDevice];
        Log::Debug("Physical Device Acquired -> <0x%08x>\n", m_PhysicalDevice);


    }
    else {  //This shouldn't *really* be possible, but just in case...
        Unreachable();
        Log::Warning("No Physical Devices Found!\n");
    }

    return CheckVkResult(m_PhysicalDevice != VK_NULL_HANDLE ? VK_SUCCESS : VK_ERROR_DEVICE_LOST);
}

VkResult Paradox::Gpu::CreateDevice()
{
    VulkanZoneScoped;
    VK_LOG("Creating Device...\n");

    VkPhysicalDeviceFeatures features = {};
    std::vector<const char*> deviceExtensions;

    //Get feature set extensions + physical device features
    std::set<std::string> featureExtensions = {};
    {
        for (const auto& featureSet : kFeatureRequirements) {
            if (m_Capabilities[(size_t)featureSet.first]) {
                Log::Debug("Loading Extensions for Feature [%s]\n", kGpuFeatureCapabilitiesNames[featureSet.first]);

                for (auto& ext : featureSet.second.deviceExtensions) {
                    featureExtensions.emplace(ext);
                }
            }

            for (size_t i = 0; i < sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32); i++) {
                VkBool32* a = ((VkBool32*)&featureSet.second.features) + i;
                VkBool32* b = ((VkBool32*)&features) + i;

                //Unify the feature requirements. 
                *b |= *a;
            }

        }

        for (auto& ext : featureExtensions) {
            deviceExtensions.push_back(ext.c_str());
        }
    }

    for (auto& ext : deviceExtensions) {
        Log::Debug("\t--[%s]\n", ext);
    }

    //Configure device features. 

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .pNext = nullptr,
        .dynamicRendering = m_Capabilities[(size_t)EGpuFeatureCapabilities::Dynamic_Rendering] ? VK_TRUE : VK_FALSE,
    };

    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures = {
       .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
       .pNext = &dynamicRenderingFeatures,
       .accelerationStructure = m_Capabilities[(size_t)EGpuFeatureCapabilities::Ray_Tracing_Pipeline] || m_Capabilities[(size_t)EGpuFeatureCapabilities::Ray_Query] ? VK_TRUE : VK_FALSE,
    };

    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR,
        .pNext = &accelerationStructureFeatures,
        .rayQuery = m_Capabilities[(size_t)EGpuFeatureCapabilities::Ray_Query] ? VK_TRUE : VK_FALSE,
    };

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .pNext = &rayQueryFeatures,
        .rayTracingPipeline = m_Capabilities[(size_t)EGpuFeatureCapabilities::Ray_Tracing_Pipeline] ? VK_TRUE : VK_FALSE,
    };

    VkPhysicalDeviceVulkan12Features vulkan_1_2_features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &rayTracingPipelineFeatures,
        .descriptorIndexing = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .shaderUniformBufferArrayNonUniformIndexing = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE, //(#extension GL_EXT_nonuniform_qualifier : require)
        .shaderSampledImageArrayNonUniformIndexing = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .shaderStorageBufferArrayNonUniformIndexing = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .shaderStorageImageArrayNonUniformIndexing = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .descriptorBindingSampledImageUpdateAfterBind = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .descriptorBindingStorageImageUpdateAfterBind = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .descriptorBindingStorageBufferUpdateAfterBind = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .descriptorBindingPartiallyBound = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
        .runtimeDescriptorArray = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE, //Enable non-sized arrays
        .bufferDeviceAddress = m_Capabilities[(size_t)EGpuFeatureCapabilities::Bindless] ? VK_TRUE : VK_FALSE,
    };

    const VkPhysicalDeviceFeatures2 deviceFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &vulkan_1_2_features,
        .features = features,
    };

    //Expose all available device queues. 
    std::vector<std::vector<float>> priorities;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    {
        uint32_t propCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &propCount, nullptr);
        std::vector<VkQueueFamilyProperties> properties(propCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &propCount, properties.data());


        uint32_t index = 0;
        for (const auto& p : properties) {

            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.pNext = nullptr;
            queueCreateInfo.flags = 0;
            queueCreateInfo.queueCount = p.queueCount;
            queueCreateInfo.queueFamilyIndex = index;
            std::vector<float> queuePriorities(p.queueCount);
            for (int i = 0; i < p.queueCount; i++) {
                queuePriorities[i] = 1.0f;
            }
            priorities.push_back(queuePriorities);
            queueCreateInfo.pQueuePriorities = priorities.back().data();


            queueCreateInfos.push_back(queueCreateInfo);

            Log::Debug("Exposing Queue Family [%d] (%d)\n\tGraphics - %s\n\tCompute - %s\n\tTransfer - %s\n",
                queueCreateInfo.queueFamilyIndex,
                queueCreateInfo.queueCount,
                p.queueFlags & VK_QUEUE_GRAPHICS_BIT ? "True" : "False",
                p.queueFlags & VK_QUEUE_COMPUTE_BIT ? "True" : "False",
                p.queueFlags & VK_QUEUE_TRANSFER_BIT ? "True" : "False"
            );

            index++;
        }
    }

    //Create the device. 
    const VkDeviceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &deviceFeatures,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
    };

    VkResult res = CheckVkResult(vkCreateDevice(m_PhysicalDevice, &createInfo, m_pAllocationCallbacks, &m_Device), "Failed to Create Device!\n");

    Log::Debug("vkCreateDevice(...) -> <0x%08x>\n", m_Device);
    return res;
}

void Paradox::Gpu::DestroyDevice()
{
    VulkanZoneScoped;
    VK_LOG("Destroying Device...\n");

    if (m_Device != VK_NULL_HANDLE) {
        vkDestroyDevice(m_Device, m_pAllocationCallbacks);
        m_Device = VK_NULL_HANDLE;
    }
}

VkResult Paradox::Gpu::CreateDebugMessenger()
{
    VulkanZoneScoped;
    VK_LOG("Creating Debug Utils Messenger...\n");
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
    return CheckVkResult(res);
}

void Paradox::Gpu::DestroyDebugMessenger()
{
    VulkanZoneScoped;
    if (m_Instance == VK_NULL_HANDLE) {
        CheckVkResult(VK_ERROR_DEVICE_LOST, "Invalid Vulkan Instance!\n");
        return;
    }

    if (m_DebugMessenger != VK_NULL_HANDLE) {
        Gpu::vkDestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, m_pAllocationCallbacks);
        m_DebugMessenger = nullptr;
    }
}

VkResult Paradox::Gpu::SetDebugObjectName(const uint64_t handle, const VkObjectType type, const std::string& name) const
{
    VulkanZoneScoped;
    VkResult res = VK_SUCCESS;
    if (m_EnableDebugUtils) {
        if (!name.empty()) {
            Log::Debug("Setting %s <0x%08x> name to %s.\n", string_VkObjectType(type), handle, name.c_str());

            const VkDebugUtilsObjectNameInfoEXT nameInfo = {
                .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
                .pNext = nullptr,
                .objectType = type,
                .objectHandle = handle,
                .pObjectName = name.c_str()
            };
            assert(Gpu::vkSetDebugUtilsObjectNameEXT != nullptr);
            if (Gpu::vkSetDebugUtilsObjectNameEXT == nullptr) {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
            }
            //if (m_EnableDebugUtils) {
            res = CheckVkResult(Gpu::vkSetDebugUtilsObjectNameEXT(m_Device, &nameInfo));
            //}
        }
    }
    return res;
}

VKAPI_ATTR VkBool32 VKAPI_CALL Paradox::Gpu::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    VulkanZoneScoped;
    Log::Print(ELogColour::LightMagenta, "%s\n", pCallbackData->pMessage);
    return VK_FALSE;
}

//--------------


Paradox::ParadoxError Paradox::Gpu::CreateQueue(VkQueue* pOutQueue, uint32_t* pOutQueueFamilyIndex, EQueueType type, const std::string& name) const
{
    static std::unordered_map<uint32_t, uint32_t> queueFamilyAllocatedCounts; 

    auto GetQueueTypeProperties = [&](EQueueType type) -> std::pair<uint32_t, uint32_t> {
        //Get this device's queue family properties. 
        uint32_t propCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &propCount, nullptr);
        std::vector<VkQueueFamilyProperties> properties(propCount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &propCount, properties.data());

        //Enumerate queue flag bits
        VkQueueFlags flagBits = {};
        {
            switch (type) {
            case EQueueType::Graphics:
                flagBits |= VK_QUEUE_GRAPHICS_BIT;
                break;
            case EQueueType::Transfer:
                flagBits |= VK_QUEUE_TRANSFER_BIT;
                break;
            case EQueueType::Compute:
                flagBits |= VK_QUEUE_COMPUTE_BIT;
                break;
            default:
                break;
            }
        }

        //Search all available queue families for queues meeting our requirements. 
        std::vector<std::pair<uint32_t, VkQueueFlags>> queueFamilyCandidates;

        for (size_t i = 0; i < properties.size(); ++i) {
            if (properties[i].queueFlags & flagBits) {
                Log::Debug("Found Queue Family Candidate: (%d)[%d / %d]\n", i, queueFamilyAllocatedCounts[i], properties[i].queueCount); 
                queueFamilyCandidates.push_back({ i, properties[i].queueFlags });
            }
        }
        //Early return if there are no viable candidates. 
        if (queueFamilyCandidates.empty()) {
            Log::Debug("No viable Queue Family Candidates were found!\n");
            return { -1, -1 };
        }

        //Sort the candidates based on flags, to prioritize dedicated queue families. 
        std::sort(queueFamilyCandidates.begin(), queueFamilyCandidates.end(), [](std::pair<uint32_t, VkQueueFlags>& a, std::pair<uint32_t, VkQueueFlags>& b) {
            return a.second < b.second;
            });

        //Return available queues. 
        for (auto& candidate : queueFamilyCandidates) {
            if (queueFamilyAllocatedCounts[candidate.first] < properties[candidate.first].queueCount) {
                Log::Debug("Allocating Queue (%d)[%d]\n", candidate.first, queueFamilyAllocatedCounts[candidate.first]); 
                return { candidate.first, queueFamilyAllocatedCounts[candidate.first]++ }; 
                break; 
            }
        }


        return { -1, -1 };
        };

    //Retrieve queue data. 
    auto queueProps = GetQueueTypeProperties(type);

    uint32_t queueFamilyIndex = queueProps.first;
    uint32_t queueIndex = queueProps.second;

    if (queueFamilyIndex == -1 || queueIndex == -1) {
        VK_LOG("Failed to Acquire Device Queue!\n");
        return ParadoxError::Failed;
    }

    vkGetDeviceQueue(m_Device, queueFamilyIndex, queueIndex, pOutQueue);
    *pOutQueueFamilyIndex = queueFamilyIndex; 

    if (*pOutQueue == VK_NULL_HANDLE) {
        VK_LOG("Failed to Acquire Device Queue!\n");
        return ParadoxError::Failed;
    }
    VK_LOG("Acquired Device Queue (%d)[%d]\n", queueFamilyIndex, queueIndex); 

    return ParadoxError::Success;
}

void Paradox::Gpu::DestroyQueue(VkQueue* pQueue) const
{
    assert(pQueue != VK_NULL_HANDLE);
    if (pQueue != VK_NULL_HANDLE) {
        //NOTE: Since Queues are Acquired and not created in vulkan, we can just reset the handle. 
        *pQueue = VK_NULL_HANDLE;
    }
    else {
        VK_LOG("Attempting to destroy an invalid VkQueue!\n");
    }
}

Paradox::ParadoxError Paradox::Gpu::CreateBinarySemaphore(VkSemaphore* pOutSemaphore, const std::string& name) const
{
    return Paradox::ParadoxError();
}

Paradox::ParadoxError Paradox::Gpu::CreateTimelineSemaphore(VkSemaphore* pOutSemaphore, const uint64_t initialValue, const std::string& name) const
{
    return Paradox::ParadoxError();
}

void Paradox::Gpu::DestroySemaphore(VkSemaphore* pSemaphore) const
{
}

void Paradox::Gpu::SignalSemaphore(VkSemaphore* pSemaphore, const uint64_t value) const
{ 
    const VkSemaphoreSignalInfo signalInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO, 
        .pNext = nullptr,
        .semaphore = *pSemaphore,
        .value = value, 
    };

    VkResult res = vkSignalSemaphore(m_Device, &signalInfo);
    CheckVkResult(res, std::format("Failed to Signal Semaphore <0x{:#08x}> at (0x{:#08x})!\n", *pSemaphore, pSemaphore));
}

Paradox::ParadoxError Paradox::Gpu::WaitSemaphore(VkSemaphore* pSemaphore, const uint64_t value, const uint64_t timeout) const
{
    const VkSemaphoreWaitInfo waitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO, 
        .pNext = nullptr, 
        .flags = 0, 
        .semaphoreCount = 1, 
        .pSemaphores = pSemaphore, 
        .pValues = &value, 
    };
    VkResult res = vkWaitSemaphores(m_Device, &waitInfo, timeout); 
    if (res == VK_TIMEOUT) {
        return ParadoxError::Timeout; 
    }
    
    if (CheckVkResult(res, std::format("Failed to Wait for Semaphore <0x{:#08x}> at (0x{:#08x})!\n", *pSemaphore, pSemaphore)) != VK_SUCCESS) {
        return ParadoxError::Failed; 
    }

    return ParadoxError::Success; 
}

Paradox::ParadoxError Paradox::Gpu::CreateFence(VkFence* pOutFence, const uint64_t initialValue, bool createSignaled, const std::string& name) const
{
    return Paradox::ParadoxError();
}

void Paradox::Gpu::DestroyFence(VkFence* pFence) const
{
}

