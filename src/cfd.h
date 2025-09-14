#pragma once

#include <iostream>
#include <sstream>
#include <fstream>

#include "vk_types.h"
#include "vk_helper.h"
#include "vk_initializers.h"

class Cfd {
private:
    unsigned int _res = 129;

    VkDevice _device;
    VmaAllocator _allocator;

    ResourceBinding _vx;
    ResourceBinding _vy;
    ResourceBinding _vz;

    ResourceBinding _density;
    ResourceBinding _pressure;

    ResourceBinding _vx2;
    ResourceBinding _vy2;
    ResourceBinding _vz2;

    ResourceBinding _density2; 
    ResourceBinding _pressure2;

    ResourceBinding _boundaries;

    ResourceBinding _densityTex;

    Kernel _gaussSidel{};
    Kernel _advect{};
    Kernel _advectSwapped{};
	Kernel _writeTexture{};
	Kernel _writeTextureSwapped{};
	Kernel _rp{};

public:
    void init_cfd(VkDevice& device, VmaAllocator& allocator, int res);
    void evolve_cfd_cmd(VkCommandBuffer commandBuffer);
    void load_default_state(VkCommandPool commandPool, VkQueue queue);
    std::vector<ResourceBinding> get_texture_bindings();
};

struct CFDPushConstants {
    int gridSize;
    int shouldRed;
};

// int create_command_buffers(Init& init, RenderData& data, std::vector<texture>& textures);

// void init_cfd(Init& init, ComputeHandler& computeHandler, Cfd& cfd, int gridSize);

// void load_terrain(Init& init, Cfd& cfd, const std::string& filename);

// void evolve_cfd(Init& init, ComputeHandler& computeHandler, Cfd& cfd);

// void cleanup(Init& init, Cfd& cfd);
