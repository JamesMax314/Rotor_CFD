#include "vk_helper.h"

void vkhelp::copy_to_buffer(VkDevice& device, VmaAllocator& allocator, ResourceBinding& buf, void* data, size_t size) {
    void* mapped = nullptr;
    vmaMapMemory(allocator, buf.bufferAllocation, &mapped);
    memcpy(mapped, data, size);
    vmaUnmapMemory(allocator, buf.bufferAllocation);
}

void vkhelp::copy_from_buffer(VmaAllocator& allocator, ResourceBinding& buf, void* outData, size_t size) {
    void* mapped = nullptr;
    vmaMapMemory(allocator, buf.bufferAllocation, &mapped);
    memcpy(outData, mapped, size);
    vmaUnmapMemory(allocator, buf.bufferAllocation);
}
