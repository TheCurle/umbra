#pragma once

#include "renderer/Interface.h"
#include <vulkan/vulkan.h>

namespace rx {
  class VulkanInterface : public Interface {
    friend struct CommandQueue;

  protected:
    bool debug = false;
    VkInstance instance = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugUtilsMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;
    std::vector<VkQueueFamilyProperties2> queueFamilies;
    //std::vector<VkQueueFamilyVideoPropertiesKHR> queueFamiliesVideo;
    size_t graphicsFamily = VK_QUEUE_FAMILY_IGNORED;
    size_t computeFamily = VK_QUEUE_FAMILY_IGNORED;
    size_t copyFamily = VK_QUEUE_FAMILY_IGNORED;
    size_t videoFamily = VK_QUEUE_FAMILY_IGNORED;
    std::vector<size_t> families;
    VkQueue graphicsQueue = VK_NULL_HANDLE;
    VkQueue computeQueue = VK_NULL_HANDLE;
    VkQueue copyQueue = VK_NULL_HANDLE;
    VkQueue videoQueue = VK_NULL_HANDLE;
    VkPhysicalDeviceProperties2 deviceProps2 = {};
    VkPhysicalDeviceVulkan11Properties deviceProps11 = {};
    VkPhysicalDeviceVulkan12Properties deviceProps12 = {};
    VkPhysicalDeviceVulkan13Properties deviceProps13 = {};
    VkPhysicalDeviceSamplerFilterMinmaxProperties samplerMinmaxProps = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureProps = {};
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingProps = {};
    VkPhysicalDeviceFragmentShadingRatePropertiesKHR fragmentShadingRateProps = {};
    //VkPhysicalDeviceMeshShaderPropertiesEXT meshShaderProps = {};
    VkPhysicalDeviceMemoryProperties2 memoryProps = {};
    VkPhysicalDeviceDepthStencilResolveProperties depthStencilResolveProps = {};
    VkPhysicalDeviceFeatures2 deviceFeatures2 = {};
    VkPhysicalDeviceVulkan11Features deviceFeatures11 = {};
    VkPhysicalDeviceVulkan12Features deviceFeatures12 = {};
    VkPhysicalDeviceVulkan13Features deviceFeatures13 = {};
    VkPhysicalDeviceAccelerationStructurePropertiesKHR accelerationStructureFeatures = {};
    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingFeatures = {};
    VkPhysicalDeviceRayQueryFeaturesKHR rayTracingQueryFeatures = {};
    VkPhysicalDeviceFragmentShadingRateFeaturesKHR fragmentShadingRateFeatures = {};
    VkPhysicalDeviceConditionalRenderingFeaturesEXT conditionalRenderingFeatures = {};
    VkPhysicalDeviceDepthClipEnableFeaturesEXT depthClipEnableFeatures = {};
    //VkPhysicalDeviceMeshShaderFeaturesEXT meshShaderFeatures = {};
    VkVideoDecodeH264ProfileInfoKHR decodeH264Profile = {};
  };
}