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

void vkhelp::transitionImageLayout(
    VkDevice device,
    VkCommandPool commandPool,
    VkQueue queue,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageAspectFlags aspectMask,
    uint32_t mipLevels,
    uint32_t layerCount
) {
    // Begin a one-time-use command buffer
    VkCommandBuffer cmd = vkinit::beginSingleTimeCommands(device, commandPool);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = layerCount;

    // Default conservative masks
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
         newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } 
    else {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    vkinit::endSingleTimeCommands(device, commandPool, queue, cmd);
}