#include "vk_helper.h"

// void vkhelp::copy_to_buffer(VkDevice& device, VmaAllocator& allocator, ResourceBinding& buf, void* data, size_t size) {
//     void* mapped = nullptr;
//     vmaMapMemory(allocator, buf.bufferAllocation, &mapped);
//     memcpy(mapped, data, size);
//     vmaUnmapMemory(allocator, buf.bufferAllocation);
// }

void vkhelp::copy_to_buffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue,
                            ResourceBinding& buf, void* data, size_t size) 
{
    VmaAllocationInfo allocInfo{};
    vmaGetAllocationInfo(allocator, buf.bufferAllocation, &allocInfo);

    if (allocInfo.pMappedData) {
        // Already persistently mapped (common if you allocated with HOST_ACCESS flag)
        memcpy(allocInfo.pMappedData, data, size);
    } 
    else {
        // Query memory properties
        VkMemoryPropertyFlags memFlags;
        vmaGetAllocationMemoryProperties(allocator, buf.bufferAllocation, &memFlags);

        if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            // Can map directly
            void* mapped = nullptr;
            vmaMapMemory(allocator, buf.bufferAllocation, &mapped);
            memcpy(mapped, data, size);
            vmaUnmapMemory(allocator, buf.bufferAllocation);
        } 
        else {
            // GPU-only → use staging buffer
            VkBuffer stagingBuffer;
            VmaAllocation stagingAlloc;

            VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufInfo.size = size;
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

            VmaAllocationCreateInfo stagingAllocInfo{};
            stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

            vmaCreateBuffer(allocator, &bufInfo, &stagingAllocInfo, 
                            &stagingBuffer, &stagingAlloc, nullptr);

            // Map and copy
            void* mapped = nullptr;
            vmaMapMemory(allocator, stagingAlloc, &mapped);
            memcpy(mapped, data, size);
            vmaUnmapMemory(allocator, stagingAlloc);

            // Record a one-shot copy command
            VkCommandBuffer cmd = vkinit::beginSingleTimeCommands(device, commandPool);
            VkBufferCopy copyRegion{0,0,size};
            vkCmdCopyBuffer(cmd, stagingBuffer, buf.buffer, 1, &copyRegion);

            VkBufferMemoryBarrier bufBarrier{};
            bufBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
            bufBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            bufBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT; // whichever your shader needs
            bufBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            bufBarrier.buffer = buf.buffer;
            bufBarrier.offset = 0;
            bufBarrier.size = VK_WHOLE_SIZE;

            // Use TRANSFER -> COMPUTE stage if compute will read it next; adjust if fragment will read.
            vkCmdPipelineBarrier(
                cmd,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, // or FRAGMENT_SHADER_BIT if fragment will read immediately
                0,
                0, nullptr,
                1, &bufBarrier,
                0, nullptr
            );

            vkinit::endSingleTimeCommands(device, commandPool, queue, cmd);

            vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
        }
    }
}

void vkhelp::copy_from_buffer(VkDevice device, VmaAllocator allocator, VkCommandPool commandPool, VkQueue queue,
                            ResourceBinding& buf, void* data, size_t size) 
{
    VmaAllocationInfo allocInfo{};
    vmaGetAllocationInfo(allocator, buf.bufferAllocation, &allocInfo);

    if (allocInfo.pMappedData) {
        // Already persistently mapped (common if you allocated with HOST_ACCESS flag)
        memcpy(allocInfo.pMappedData, data, size);
    } 
    else {
        // Query memory properties
        VkMemoryPropertyFlags memFlags;
        vmaGetAllocationMemoryProperties(allocator, buf.bufferAllocation, &memFlags);

        if (memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            // Can map directly
            void* mapped = nullptr;
            vmaMapMemory(allocator, buf.bufferAllocation, &mapped);
            memcpy(mapped, data, size);
            vmaUnmapMemory(allocator, buf.bufferAllocation);
        } 
        else {
            // GPU-only → use staging buffer
            VkBuffer stagingBuffer;
            VmaAllocation stagingAlloc;

            VkBufferCreateInfo bufInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
            bufInfo.size = size;
            bufInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

            VmaAllocationCreateInfo stagingAllocInfo{};
            stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO;
            stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;

            vmaCreateBuffer(allocator, &bufInfo, &stagingAllocInfo, 
                            &stagingBuffer, &stagingAlloc, nullptr);

            // Record a one-shot copy command
            VkCommandBuffer cmd = vkinit::beginSingleTimeCommands(device, commandPool);
            VkBufferCopy copyRegion{0,0,size};
            vkCmdCopyBuffer(cmd, buf.buffer, stagingBuffer, 1, &copyRegion);

            vkinit::endSingleTimeCommands(device, commandPool, queue, cmd);

            // Map and copy
            void* mapped = nullptr;
            vmaMapMemory(allocator, stagingAlloc, &mapped);
            memcpy(data, mapped, size);
            vmaUnmapMemory(allocator, stagingAlloc);

            vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
        }
    }
}