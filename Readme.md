# Paradox - GPU Accelerated Path Tracer 


## Build Instructions
```bash
git clone https://github.com/EwanBurnett/Paradox.git
cd paradox
```

```bash
mkdir build && cd build
cmake .. 
```

## Requirements

### Paradox Required Extensions
| Extension | Description | 
| - | - |
| [VK_KHR_timeline_semaphore](https://vulkan.gpuinfo.org/displayextensiondetail.php?extension=VK_KHR_timeline_semaphore) | ... |
| [VK_KHR_synchronization2](https://vulkan.gpuinfo.org/displayextensiondetail.php?extension=VK_KHR_synchronization2) | ... |
| [VK_KHR_shader_non_semantic_info](https://vulkan.gpuinfo.org/displayextensiondetail.php?extension=VK_KHR_shader_non_semantic_info) | ... |


### Unix
- Vulkan SDK 1.2 or higher
- Third-party Dependencies 
    - https://www.glfw.org/docs/latest/compile_guide.html#compile_deps

e.g. Debian
```bash
# Install the Vulkan SDK 
sudo apt install vulkan-sdk
# Install all third-party dependencies
sudo apt install libwayland-dev libxkbcommon-dev xorg-dev

```

