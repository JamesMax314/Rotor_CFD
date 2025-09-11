#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>

struct KernelOld {
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    VkShaderModule shaderModule{}; // optional, usually destroyed after pipeline creation
};

enum class KernelType { Compute, Graphics };

struct Kernel {
    KernelType type;
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    std::vector<VkPushConstantRange> pushConstants;
};

struct BufferBinding {
    uint32_t binding;     // descriptor set binding
    VkDeviceSize size;    // buffer size in bytes
    VkBuffer buffer;      // filled by function
    VmaAllocation allocation; // filled by function
};

struct CamData {
    float pos[3];
    float padding1; // padding to make size multiple of 16 bytes
    float lookAt[3];
    float padding2; // padding to make size multiple of 16 bytes
};

struct ResourceBinding {
    uint32_t binding;                   // Binding index in the shader
    VkDeviceSize range = VK_WHOLE_SIZE; // For buffers, size of the buffer; for images, ignored
    VkDescriptorType type;              // e.g. STORAGE_BUFFER, COMBINED_IMAGE_SAMPLER, STORAGE_IMAGE

    // Buffer info (if this is a buffer descriptor)
    VkBuffer buffer = VK_NULL_HANDLE;
    VmaAllocation bufferAllocation = VK_NULL_HANDLE;
    VkDeviceSize offset = 0;

    // Image info (if this is an image descriptor)
    VkImage image = VK_NULL_HANDLE;
    VmaAllocation imageAllocation = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    VkImageLayout layout = VK_IMAGE_LAYOUT_UNDEFINED;
};


class PipelineBuilder {
public:

	std::vector<VkPipelineShaderStageCreateInfo> _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	VkViewport _viewport;
	VkRect2D _scissor;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;

	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
};

struct Vertex {
    glm::vec3 pos;   // NDC coordinates
    glm::vec2 uv;    // Texture coordinates

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return binding;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributes{};

        // Position (location = 0)
        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes[0].offset = offsetof(Vertex, pos);

        // UV (location = 1)
        attributes[1].binding = 0;
        attributes[1].location = 1;
        attributes[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributes[1].offset = offsetof(Vertex, uv);

        return attributes;
    }
};

struct Mesh {
    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation vertexAllocation = VK_NULL_HANDLE;

    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VmaAllocation indexAllocation = VK_NULL_HANDLE;

    uint32_t indexCount = 0;
    
    VkDevice device;
    VkCommandPool commandPool;
    VkQueue queue;

    void init(VkDevice device, VkCommandPool commandPool, VkQueue queue, VmaAllocator allocator,
              const std::vector<Vertex>& vertices,
              const std::vector<uint16_t>& indices);

    // Upload CPU data to GPU buffers
    void upload(VmaAllocator allocator,
                const std::vector<Vertex>& vertices,
                const std::vector<uint16_t>& indices);

    void draw(VkCommandBuffer cmd) const;

private:
    // Helper: copy CPU memory into GPU buffer via VMA staging
    void uploadBufferData(VmaAllocator allocator,
                          VkBuffer dstBuffer,
                          const void* data,
                          VkDeviceSize size);
};
