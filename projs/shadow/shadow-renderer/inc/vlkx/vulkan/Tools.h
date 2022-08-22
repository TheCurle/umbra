#pragma once

#include <stdexcept>
#include <functional>

#include <vulkan/vulkan.h>
#include <vlkx/vulkan/VulkanManager.h>

#include <vulkan/vk_mem_alloc.h>

namespace VkTools {
    extern VmaAllocator             g_allocator;
    extern VkInstance               g_Instance;
    extern VkPhysicalDevice         g_PhysicalDevice;
    extern VkDevice                 g_Device;
    extern uint32_t                 g_QueueFamily;
    extern VkQueue                  g_Queue;
    extern VkDebugReportCallbackEXT g_DebugReport;

    struct ManagedImage {
        VkImage image;
        VmaAllocation allocation;
    };

    struct ManagedBuffer {
        VkBuffer buffer;
        VmaAllocation allocation;
    };

    ManagedImage createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent);
    VkSampler createSampler(VkFilter filters, VkSamplerAddressMode mode, uint32_t mipping);
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipping, uint32_t layers, VkDevice device);
    ManagedBuffer createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, bool hostVisible = true);
    uint32_t findMemoryIndex(uint32_t type, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
    VkCommandBuffer createTempCommandBuffer(VkCommandPool pool, VkDevice logical);
    void executeAndDeleteTempBuffer(VkCommandBuffer buffer, VkCommandPool pool, VkQueue queue, VkDevice logicalDevice);
    void immediateExecute(const std::function<void(VkCommandBuffer&)>& execute, VulkanDevice* dev);
    void copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VulkanDevice* dev);


#define VKTOOLS_IMPLEMENTATION
    #ifdef VKTOOLS_IMPLEMENTATION
    ManagedImage createImage(VkFormat format, VkImageUsageFlags flags, VkExtent3D extent) {
        // Set up image metadata
        VkImageCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        info.pNext = nullptr;
        info.format = format;
        info.extent = extent;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.tiling = VK_IMAGE_TILING_OPTIMAL;
        info.usage = flags;

        // Prepare the managed image
        ManagedImage image {};

        // Set up image allocation
        VmaAllocationCreateInfo allocateInfo = {};
        allocateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        // Allocate + create the image
        vmaCreateImage(g_allocator, &info, &allocateInfo, &image.image, &image.allocation, nullptr);

        return image;
    }

    VkSampler createSampler(VkFilter filters, VkSamplerAddressMode mode, uint32_t mipping) {
        VkSamplerCreateInfo info = {
                VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                nullptr, {}, filters, filters,
                VK_SAMPLER_MIPMAP_MODE_LINEAR, mode, mode, mode,
                0, VK_TRUE, 16, VK_FALSE,
                VK_COMPARE_OP_ALWAYS, 0, static_cast<float>(mipping),
                VK_BORDER_COLOR_INT_OPAQUE_BLACK, VK_FALSE
        };

        VkSampler sampler;
        vkCreateSampler(g_Device, &info, nullptr, &sampler);

        return sampler;
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags flags, uint32_t mipping, uint32_t layers, VkDevice device) {
        // Raw information about the image
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = layers == 1 ? VK_IMAGE_VIEW_TYPE_2D : VK_IMAGE_VIEW_TYPE_CUBE;
        viewInfo.format = format;

        // Information about the things we want to create - size, mip levels.
        viewInfo.subresourceRange.aspectMask = flags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipping;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = layers;

        VkImageView imageView;
        if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
            throw std::runtime_error("Failed to create texture image view.");

        return imageView;
    }

    ManagedBuffer createGPUBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDevice logicalDevice, VkPhysicalDevice physicalDevice, bool hostVisible) {
        // Prepare for creation of a buffer
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        ManagedBuffer buffer;

        VmaAllocationCreateInfo vmaInfo = {};
        vmaInfo.usage = hostVisible ? VMA_MEMORY_USAGE_CPU_ONLY : VMA_MEMORY_USAGE_GPU_ONLY;
        vmaInfo.requiredFlags = properties;

        // Create the buffer.
        if (VkResult status = vmaCreateBuffer(g_allocator, &bufferInfo, &vmaInfo, &buffer.buffer, &buffer.allocation, nullptr); status != VK_SUCCESS)
            throw std::runtime_error("Unable to create GPU buffer: " + std::to_string(status));

        return buffer;
    }

    uint32_t findMemoryIndex(uint32_t type, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
        // Get the physical properties of the device.
        VkPhysicalDeviceMemoryProperties physProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physProperties);

        // Iterate the device and search for a suitable index
        for (uint32_t i = 0; i < physProperties.memoryTypeCount; i++)
            // If the type matches, and the properties are what we desire, then ship it.
            if ((type & (1 << i)) && ((physProperties.memoryTypes[i].propertyFlags & properties) == properties))
                return i;

        throw std::runtime_error("Unable to find a suitable memory type on the physical device.");
    }

    VkCommandBuffer createTempCommandBuffer(VkCommandPool pool, VkDevice logical) {
        // Prepare to allocate a command buffer
        VkCommandBufferAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocateInfo.commandPool = pool;
        allocateInfo.commandBufferCount = 1;

        // Allocate the buffer
        VkCommandBuffer buffer;
        vkAllocateCommandBuffers(logical, &allocateInfo, &buffer);

        // Prepare to begin the new buffer.
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        // Begin listening on the new buffer.
        vkBeginCommandBuffer(buffer, &beginInfo);

        return buffer;
    }

    void executeAndDeleteTempBuffer(VkCommandBuffer buffer, VkCommandPool pool, VkQueue queue, VkDevice logicalDevice) {
        // Stop listening on the buffer
        vkEndCommandBuffer(buffer);

        // Prepare to execute the commands in the buffer
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &buffer;

        // Submit the commands to be executed
        vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);

        // Wait for the GPU to finish executing
        vkQueueWaitIdle(queue);

        // Delete the now unusable buffers
        vkFreeCommandBuffers(logicalDevice, pool, 1, &buffer);
    }

    void immediateExecute(const std::function<void(VkCommandBuffer&)>& execute, VulkanDevice* dev) {
        VkCommandPool pool;
        VkCommandPoolCreateInfo poolCreateInfo = {};
        poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCreateInfo.queueFamilyIndex = dev->getQueues().graphics;
        poolCreateInfo.flags = 0;

        // Create the pool
        if (vkCreateCommandPool(dev->logical, &poolCreateInfo, nullptr, &pool) != VK_SUCCESS)
            throw std::runtime_error("Unable to allocate a temporary command pool");

        VkCommandBuffer buffer = VkTools::createTempCommandBuffer(pool, dev->logical);

        execute(buffer);

        VkTools::executeAndDeleteTempBuffer(buffer, pool, dev->graphicsQueue, dev->logical);

        vkDestroyCommandPool(dev->logical, pool, nullptr);

    }

    void copyGPUBuffer(VkBuffer source, VkBuffer dest, VkDeviceSize length, VulkanDevice* dev) {
        immediateExecute([&](VkCommandBuffer& commands) {
            // Prepare to copy the data between buffers
            VkBufferCopy copyInfo = {};
            copyInfo.srcOffset = 0;
            copyInfo.dstOffset = 0;
            copyInfo.size = length;

            // Copy the data.
            vkCmdCopyBuffer(commands, source, dest, 1, &copyInfo);

        }, dev);
    }

    #endif
}