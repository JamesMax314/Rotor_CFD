#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <iostream>

#include "vk_types.h"
#include "vk_initializers.h"

namespace vkhelp
{
    void copy_to_buffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, ResourceBinding &buf, void *data, size_t size);
    void copy_from_buffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue, ResourceBinding &buf, void *data, size_t size);

    void transitionImageLayout(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    uint32_t mipLevels = 1,
    uint32_t layerCount = 1);
} // namespace name
