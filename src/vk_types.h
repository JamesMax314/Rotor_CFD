#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

struct Kernel {
    VkPipeline pipeline{};
    VkPipelineLayout pipelineLayout{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    VkShaderModule shaderModule{}; // optional, usually destroyed after pipeline creation
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