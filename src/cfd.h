#pragma once

#include <iostream>
#include <sstream>
#include <fstream>

#include "vk_types.h"
#include "vk_initializers.h"

class Cfd {
private:
    unsigned int _res = 129;

    VkDevice _device;
    VmaAllocator _allocator;

    VkDeviceSize bufferSize = _res * _res * _res * sizeof(float);
    ResourceBinding _vx = {0, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    ResourceBinding _vy = {1, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    ResourceBinding _vz = {2, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    ResourceBinding _density = {3, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    ResourceBinding _pressure = {4, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    ResourceBinding _vx2 = {5, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    ResourceBinding _vy2 = {6, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    ResourceBinding _vz2 = {7, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    ResourceBinding _density2 = {8, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    ResourceBinding _pressure2 = {9, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    ResourceBinding _boundaries = {10, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    ResourceBinding _densityTex = {11, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};

        std::vector<ResourceBinding> _resourceBindings = {
        _vx, _vy, _vz, _density, _pressure,
        _vx2, _vy2, _vz2, _density2, _pressure2,
        _boundaries, _densityTex
    };

    std::vector<VkDescriptorSetLayoutBinding> _layoutBindings = {
        // Storage buffers (read/write)
        {0,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // velX
        {1,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // velY
        {2,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // velZ
        {3,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // density
        {4,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // pressure
        {5,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // velX2
        {6,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // velY2
        {7,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // velZ2
        {8,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // density2
        {9,  VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // pressure2
        {10, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // boundaries
        // Storage image (writeonly)
        {11, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT, nullptr}, // outputTexture
    };


    Kernel _gaussSidel{};
    Kernel _advect{};
    Kernel _advectSwapped{};
	Kernel _writeTexture{};
	Kernel _writeTextureSwapped{};
	Kernel _rp{};

public:
    void init_cfd(VkDevice& device, VmaAllocator& allocator, int res);
    void evolve_cfd_cmd(VkCommandBuffer commandBuffer);
    std::vector<ResourceBinding> get_texture_bindings();
};

struct PushConstants {
    int gridSize;
    int shouldRed;
};

// int create_command_buffers(Init& init, RenderData& data, std::vector<texture>& textures);

// void init_cfd(Init& init, ComputeHandler& computeHandler, Cfd& cfd, int gridSize);

// void load_terrain(Init& init, Cfd& cfd, const std::string& filename);

// void evolve_cfd(Init& init, ComputeHandler& computeHandler, Cfd& cfd);

// void cleanup(Init& init, Cfd& cfd);
