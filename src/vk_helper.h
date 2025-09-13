#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <iostream>

#include "vk_types.h"

namespace vkhelp
{
    void copy_to_buffer(VkDevice &device, VmaAllocator &allocator, ResourceBinding &buf, void *data, size_t size);
    void copy_from_buffer(VmaAllocator &allocator, ResourceBinding &buf, void *outData, size_t size);

} // namespace name
