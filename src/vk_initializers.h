#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>

#include <vk_mem_alloc.h>

#include "vk_types.h"

namespace vkinit {

	VkCommandPoolCreateInfo command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);

	VkCommandBufferAllocateInfo command_buffer_allocate_info(VkCommandPool pool, uint32_t count = 1, VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkPipelineShaderStageCreateInfo pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule);
    VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info();
    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info(VkPrimitiveTopology topology);
    VkPipelineRasterizationStateCreateInfo rasterization_state_create_info(VkPolygonMode polygonMode);
    VkPipelineMultisampleStateCreateInfo multisampling_state_create_info();
    VkPipelineColorBlendAttachmentState color_blend_attachment_state();
    VkPipelineLayoutCreateInfo pipeline_layout_create_info();

    VkPipelineLayoutCreateInfo pipeline_layout_create_info(VkDevice device, const std::vector<VkDescriptorSetLayoutBinding> &bindings, const std::vector<VkPushConstantRange> &pushConstants, std::vector<VkDescriptorSetLayout> &outSetLayouts);

    VkFenceCreateInfo fence_create_info(VkFenceCreateFlags flags = 0);
    VkSemaphoreCreateInfo semaphore_create_info(VkSemaphoreCreateFlags flags = 0);
    VkImageView createImageView3D(VkDevice device, VkImage image, VkFormat format);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice);
    bool load_shader_module(VkDevice &device, const char *filePath, VkShaderModule *outShaderModule);
    VkShaderModule createShaderModule(VkDevice &device, const std::string &filename);
    Kernel initKernel(VkDevice &device, const std::string &shaderPath, const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    VkWriteDescriptorSet writeDescriptorImage(VkDescriptorSet dstSet, uint32_t binding, VkDescriptorType type, const VkDescriptorImageInfo *imageInfo);
    void updateStorageBuffers(VkDevice device, VkDescriptorSet dstSet, const std::vector<BufferBinding> &buffers);
    void createStorageBuffers(VkDevice device, VmaAllocator allocator, std::vector<BufferBinding>& buffers);
    std::vector<VkDescriptorPoolSize> createPoolSizesFromBindings(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
}
