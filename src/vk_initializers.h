#pragma once

#include <iostream>
#include <fstream>
#include <unordered_map>

#include <vk_mem_alloc.h>

#include "vk_types.h"

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			std::cout <<"Detected Vulkan error: " << err << std::endl; \
			abort();                                                \
		}                                                           \
	} while (0)

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

    Kernel initKernel(
        VkDevice device,
        KernelType type, 
        const std::vector<std::string>& shaderPaths, // compute: 1 file, graphics: vert+frag
        const std::vector<VkDescriptorSetLayoutBinding>& bindings,
        const std::vector<VkPushConstantRange>& pushConstants,
        VkRenderPass renderPass = VK_NULL_HANDLE, // only used for graphics
        VkExtent2D _windowExtent = {}); // only used for graphics

    void updateKernelDescriptors(VkDevice device, Kernel &kernel, const std::vector<ResourceBinding> &resources);

    KernelOld initKernel(VkDevice &device, const std::string &shaderPath, const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    
    VkWriteDescriptorSet writeDescriptorImage(VkDescriptorSet dstSet, uint32_t binding, VkDescriptorType type, const VkDescriptorImageInfo *imageInfo);
    void updateStorageBuffers(VkDevice device, VkDescriptorSet dstSet, const std::vector<BufferBinding> &buffers);
    void createStorageBuffers(VkDevice device, VmaAllocator allocator, std::vector<BufferBinding>& buffers);
    
    std::vector<VkDescriptorPoolSize> createPoolSizesFromBindings(const std::vector<VkDescriptorSetLayoutBinding> &bindings);
    VkCommandBuffer beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer);
    void transitionImageLayout(
        VkCommandBuffer cmd,
        VkImage image,
        VkImageLayout oldLayout,
        VkImageLayout newLayout,
        VkAccessFlags srcAccessMask,
        VkAccessFlags dstAccessMask,
        VkPipelineStageFlags srcStage,
        VkPipelineStageFlags dstStage,
        VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        uint32_t baseMipLevel = 0,
        uint32_t levelCount = 1,
        uint32_t baseArrayLayer = 0,
        uint32_t layerCount = 1);

    void createResources(
        VkDevice device,
        VmaAllocator allocator,
        std::vector<ResourceBinding> &resources,
        VkExtent3D defaultImageExtent = {}, // you can pass per-resource extent if needed
        VkFormat defaultImageFormat = VK_FORMAT_R8G8B8A8_UNORM,
        VkImageUsageFlags imageUsage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    template<typename T>
    std::vector<T> subsetVector(const std::vector<T>& vec, const std::vector<uint32_t>& indices) {
        std::vector<T> out;
        out.reserve(indices.size());
        for (uint32_t i : indices) out.push_back(vec[i]);
        return out;
    }

    void initMesh(Mesh& mesh, VkDevice device, VkCommandPool commandPool, VkQueue queue, VmaAllocator allocator,
                  const std::vector<Vertex>& vertices,
                  const std::vector<uint16_t>& indices);

}
