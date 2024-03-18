# TODO 
- Investigate why the fuck Vulkan is using llvmpipe even when layers are off 

    > I concluded that I had to reset my pc for whatever reason?

- Perform proper cleanup on *everything* (rn validation layers are screaming at me lol)

  So far I am getting
```
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0xa000000000a, type = VK_OBJECT_TYPE_SHADER_MODULE; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkShaderModule 0xa000000000a[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0xb000000000b, type = VK_OBJECT_TYPE_SHADER_MODULE; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkShaderModule 0xb000000000b[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0xc000000000c, type = VK_OBJECT_TYPE_PIPELINE_LAYOUT; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkPipelineLayout 0xc000000000c[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0xe000000000e, type = VK_OBJECT_TYPE_PIPELINE; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkPipeline 0xe000000000e[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0xd000000000d, type = VK_OBJECT_TYPE_RENDER_PASS; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkRenderPass 0xd000000000d[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0x100000000010, type = VK_OBJECT_TYPE_FRAMEBUFFER; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkFramebuffer 0x100000000010[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
validation layer: Validation Error: [ VUID-vkDestroyDevice-device-00378 ] Object 0: handle = 0x6367d72a4310, type = VK_OBJECT_TYPE_DEVICE; Object 1: handle = 0xf000000000f, type = VK_OBJECT_TYPE_FRAMEBUFFER; | MessageID = 0x71500fba | OBJ ERROR : For VkDevice 0x6367d72a4310[], VkFramebuffer 0xf000000000f[] has not been destroyed. The Vulkan spec states: All child objects created on device must have been destroyed prior to destroying device (https://www.khronos.org/registry/vulkan/specs/1.3-extensions/html/vkspec.html#VUID-vkDestroyDevice-device-00378)
```

  all of which *will* be fixed with proper scope, for now just consider it normal even though It **should not happen**
 

- Proper *something* that will handle:

    - Synchronization
    - Handle all draw calls made by user while still maintaining a semblance of performance 
    - All the different shaders

*Shoutout to https://developer.nvidia.com/blog/vulkan-dos-donts/ gotta be of one my favorite genders*
