#pragma once

#include <stdexcept>
#include <functional>

#include <vulkan/vulkan.h>
#include <vlkx/vulkan/VulkanModule.h>

#include <vulkan/vk_mem_alloc.h>

namespace VkTools {
    struct ManagedImage {
        VkImage image;
        VmaAllocation allocation;
    };

    struct ManagedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    ManagedImage createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent, VkDevice device);
    VkSampler createSampler(VkFilter filters, VkSamplerAddressMode mode);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, VkDevice device);
    ManagedBuffer createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, bool hostVisible = true);
    uint32_t findMemoryIndex(uint32_t type, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
    VkCommandBuffer createTempCommandBuffer(VkCommandPool pool, VkDevice logical);
    void executeAndDeleteTempBuffer(VkCommandBuffer buffer, VkCommandPool pool, VkQueue queue, VkDevice logicalDevice);
    void copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VkDevice logical, VkQueue graphicsQueue, uint32_t queueIndex);

}