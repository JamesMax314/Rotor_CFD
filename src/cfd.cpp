#include "cfd.h"

std::vector<float> init_velocities(size_t gridsize, float vx, float vy, float vz) {
    std::vector<float> velocities(gridsize * gridsize * gridsize * 3);
    for (size_t i = 0; i < velocities.size(); i += 3) {
        velocities[i] = vx;
        velocities[i + 1] = vy;
        velocities[i + 2] = vz;
    }
    return velocities;
}

std::vector<float> init_scalars(size_t gridsize, float base_val) {
    std::vector<float> scalars(gridsize * gridsize * gridsize);
    for (size_t i = 0; i < scalars.size(); i += 1) {
        scalars[i] = base_val;
    }
    return scalars;
}

std::vector<float> init_vels(size_t gridsize, float base_val) {
    std::vector<float> scalars((gridsize+1) * gridsize * gridsize);
    for (size_t i = 0; i < scalars.size(); i += 1) {
        uint x = i % gridsize;
        uint y = (i / gridsize) % gridsize;
        uint z = i / (gridsize * gridsize);
        scalars[i] = base_val;
        // if (sqrt(pow(x - (gridsize-1)/2, 2) + pow(y - (gridsize-1)/2, 2)) < 10) {
        //     scalars[i] = 0.0;
        // }
    }
    return scalars;
}

std::vector<float> init_cylinder(float base_val, int sizeX, int sizeY, int sizeZ, int radius) {
    std::vector<float> scalars(sizeX * sizeY * sizeZ);
    for (int i = 0; i < scalars.size(); i += 1) {
        int x = i % sizeX;
        int y = (i / sizeX) % sizeY;
        int z = i / (sizeX * sizeY);
        if (pow(x - (sizeX-1)/2, 2) + pow(y - (sizeY-1)/2, 2) < radius * radius) {
            scalars[i] = 0.0;
        } else {
            scalars[i] = base_val;
        }
    }
    return scalars;
}

std::vector<float> init_wall(float base_val, int sizeX, int sizeY, int sizeZ) {
    std::vector<float> scalars(sizeX * sizeY * sizeZ);
    for (int i = 0; i < scalars.size(); i += 1) {
        int x = i % sizeX;
        int y = (i / sizeX) % sizeY;
        int z = i / (sizeX * sizeY);
        if (x == 0 || x == sizeX-1 && 0 < y - sizeY/4.0 && y - sizeY/4.0 < sizeY/2.0) {
            scalars[i] = base_val;
        } else {
            scalars[i] = 0.0;
        }
    }
    return scalars;
}

std::vector<float> init_boundaries(int gridSize) {
    std::vector<float> scalars(gridSize * gridSize * gridSize);
    for (int i = 0; i < scalars.size(); i += 1) {
        int x = i % gridSize;
        int y = (i / gridSize) % gridSize;
        int z = i / (gridSize * gridSize);
        if (x % (gridSize) == 0 || y % (gridSize) == 0 || z % (gridSize) == 0) {
            scalars[i] = 0.0;
        } else {
            scalars[i] = 1.0;
        }
    }
    return scalars;
}

void add_boundary_cylinder(std::vector<float>& boundaries, int rad, int posX, int posY, int gridsize) {
    for (int i = 0; i < boundaries.size(); i += 1) {
        int x = i % gridsize;
        int y = (i / gridsize) % gridsize;
        int z = i / (gridsize * gridsize);

        if (pow(x-1-posX - (gridsize-1)/2, 2) + pow(y-1-posY - (gridsize-1)/2, 2) < rad*rad) {
            boundaries[i] = 0.0;
        }
    }
}

// void init_cfd(Cfd& cfd, int gridSize) {
//     cfd.gridSize = gridSize;
//     const uint local_work_size = 32;

//     const int bufferSize = gridSize * gridSize * gridSize * sizeof(float);
//     const int velBufferSize = (gridSize+1) * gridSize * gridSize * sizeof(float);
//     const int boarderBufferSize = (gridSize+2) * (gridSize+2) * (gridSize+2) * sizeof(float);
//     const int nThreads = (gridSize * gridSize * gridSize + local_work_size - 1) / local_work_size;
//     const int nThreadsVel = ((gridSize+1) * gridSize * gridSize + local_work_size - 1) / local_work_size;


//     cfd.boundaries = create_compute_buffer(init, boarderBufferSize);

//     cfd.vx = create_compute_buffer(init, velBufferSize);
//     cfd.vy = create_compute_buffer(init, velBufferSize);
//     cfd.vz = create_compute_buffer(init, velBufferSize);

//     cfd.vx2 = create_compute_buffer(init, velBufferSize);
//     cfd.vy2 = create_compute_buffer(init, velBufferSize);
//     cfd.vz2 = create_compute_buffer(init, velBufferSize);

//     cfd.density = create_compute_buffer(init, bufferSize);
//     cfd.pressure = create_compute_buffer(init, bufferSize);

//     cfd.density2 = create_compute_buffer(init, bufferSize);
//     cfd.pressure2 = create_compute_buffer(init, bufferSize);


//     cfd.densityTex.x = gridSize;
//     cfd.densityTex.y = gridSize;
//     cfd.densityTex.z = gridSize;
//     create3DTexture(init, cfd.densityTex);
//     std::vector<texture> textures = {cfd.densityTex};


//     PushConstants pushConsts;
//     pushConsts.gridSize = gridSize;


//     std::vector<buffer> buffersGaussSiedel = {cfd.vx, cfd.vy, cfd.vz, cfd.boundaries};
//     VkShaderModule shaderGaussSiedel = createShaderModule(init, readFile(std::string(SHADER_DIR) + "/gaussSiedel.spv"));
//     cfd.kernGaussSiedel = gaussSiedelKernel(init, computeHandler, shaderGaussSiedel, buffersGaussSiedel, pushConsts, nThreads);

//     VkShaderModule shaderModule = createShaderModule(init, readFile(std::string(SHADER_DIR) + "/advect.spv"));
//     std::vector<buffer> buffers = {cfd.vx, cfd.vy, cfd.vz, cfd.density, cfd.pressure, cfd.vx2, cfd.vy2, cfd.vz2, cfd.density2, cfd.pressure2, cfd.boundaries};
//     std::vector<buffer> buffers2 = {cfd.vx2, cfd.vy2, cfd.vz2, cfd.density2, cfd.pressure2, cfd.vx, cfd.vy, cfd.vz, cfd.density, cfd.pressure, cfd.boundaries};
//     cfd.kern = build_compute_kernal(init, computeHandler, shaderModule, buffers, textures, pushConsts, nThreadsVel);
//     cfd.kern2 = build_compute_kernal(init, computeHandler, shaderModule, buffers2, textures, pushConsts, nThreadsVel);

//     VkShaderModule shaderModuleWrtieTex = createShaderModule(init, readFile(std::string(SHADER_DIR) + "/writeTexture.spv"));
//     std::vector<buffer> buffersWriteTex = {cfd.vx, cfd.vy, cfd.vz, cfd.density, cfd.pressure, cfd.density2, cfd.pressure2, cfd.boundaries};
//     std::vector<buffer> buffersWriteTex2 = {cfd.vx, cfd.vy, cfd.vz, cfd.density2, cfd.pressure2, cfd.density, cfd.pressure, cfd.boundaries};
//     cfd.kernWriteTex = build_compute_kernal(init, computeHandler, shaderModuleWrtieTex, buffersWriteTex, textures, pushConsts, nThreads);
//     cfd.kernWriteTex2 = build_compute_kernal(init, computeHandler, shaderModuleWrtieTex, buffersWriteTex2, textures, pushConsts, nThreads);

//     // Wall of x flow
//     std::vector<float> vxs = init_wall(2.0f, gridSize+1, gridSize, gridSize);
//     std::vector<float> vys = init_vels(gridSize, 0.0f);
//     std::vector<float> vzs = init_vels(gridSize, 0.0f);
//     std::vector<float> densities = init_scalars(gridSize, 0.0f);
//     std::vector<float> boundariesVec = init_boundaries(gridSize+2);

//     // Arbitrary Geometry
//     // add_boundary_cylinder(boundariesVec, 10, 0, 0, gridSize+2);
//     // add_boundary_cylinder(boundariesVec, 10, -20, -20, gridSize+2);

//     int nStreams = 10;
//     int streamSize = gridSize / nStreams;
//     for (int i=0; i<nStreams; i++) {
//         densities[gridSize*gridSize*(gridSize/2) + gridSize*i*streamSize + 0] = 2.0f;
//     }

//     for (int i=0; i<gridSize+2; i++)
//     {
//         boundariesVec[(gridSize+2)*(gridSize+2)*(gridSize/2+1) + (gridSize+2)*(i) + (0+1)] = 0.0f;
//     }

//     copy_to_buffer(init, cfd.vx, vxs.data());
//     copy_to_buffer(init, cfd.vy, vys.data());
//     copy_to_buffer(init, cfd.vz, vzs.data());
//     copy_to_buffer(init, cfd.density, densities.data());
//     copy_to_buffer(init, cfd.boundaries, boundariesVec.data());

//     init.disp.destroyShaderModule(shaderGaussSiedel, nullptr);
//     init.disp.destroyShaderModule(shaderModule, nullptr);
//     init.disp.destroyShaderModule(shaderModuleWrtieTex, nullptr);
// }

void loadTerrain(const std::string& filename, std::vector<float>& terrain, int& sizeX, int& sizeY) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }

    sizeX = 0;
    terrain.clear();

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        float value;
        sizeY = 0;

        while (iss >> value) {
            terrain.push_back(value);
            sizeY += 1;
        }
        sizeX += 1;
    }
    file.close();
}

// void load_terrain(Init& init, Cfd& cfd, const std::string& filename) {
//     int terrainSizeX, terrainSizeY;
//     std::vector<float> terrain;
//     loadTerrain(filename, terrain, terrainSizeX, terrainSizeY);

//     std::cout << "Terrain size: " << terrainSizeX << " x " << terrainSizeY << std::endl;

//     int gridSize = cfd.gridSize;
//     int boundarySize = gridSize + 2;
//     std::vector<float> boundariesVec = init_boundaries(boundarySize);
    
//     float terrainStepX = terrainSizeX / float(gridSize);
//     float terrainStepY = terrainSizeY / float(gridSize);

//     std::cout << "Terrain step: " << terrainStepX << " x " << terrainStepY << std::endl;

//     for (int i = 0; i < boundariesVec.size(); i += 1) {
//         int x = i % boundarySize;
//         int y = (i / boundarySize) % boundarySize;
//         int z = i / (boundarySize * boundarySize);

//         if (x > 0 && x < gridSize+1 && y > 0 && y < gridSize+1) {
//             int terrainX = (x-1) * terrainStepX;
//             int terrainY = (y-1) * terrainStepY;
    
//             float terrainHeight = terrain[terrainX + terrainY*terrainSizeX];

//             if (z >= terrainHeight*boundarySize) {
//                 boundariesVec[i] = 1.0;
//             } else {
//                 boundariesVec[i] = 0.0;
//             }
//         }
//     }

//     for (int i=0; i<gridSize+2; i++)
//     {
//         boundariesVec[(gridSize+2)*(gridSize+2)*(gridSize/2+1) + (gridSize+2)*(i) + (0+1)] = 0.0f;
//     }

//     copy_to_buffer(init, cfd.boundaries, boundariesVec.data());
// }

// void evolve_cfd(Init& init, ComputeHandler& computeHandler, Cfd& cfd) {
//     for (int i=0; i<10; i++)
//     {
//         execute_kernel(init, computeHandler, cfd.kernGaussSiedel);
//     }

//     execute_kernel(init, computeHandler, cfd.kern);
//     execute_kernel(init, computeHandler, cfd.kern2);

//     execute_kernel(init, computeHandler, cfd.kernWriteTex);
//     execute_kernel(init, computeHandler, cfd.kernWriteTex2);
// }

// void cleanup(Init &init, Cfd &cfd)
// {
//     cleanup(init, cfd.kernGaussSiedel);
//     cleanup(init, cfd.kern);
//     cleanup(init, cfd.kern2);
//     cleanup(init, cfd.kernWriteTex);
//     cleanup(init, cfd.kernWriteTex2);

//     std::vector<buffer> buffers = {cfd.vx, cfd.vy, cfd.vz, cfd.density, cfd.pressure, cfd.vx2, cfd.vy2, cfd.vz2, cfd.density2, cfd.pressure2, cfd.boundaries};
//     cleanup(init, buffers);
//     cleanup(init, cfd.densityTex);   
// }

void Cfd::init_cfd(VkDevice &device, VmaAllocator &allocator, int res)
{
    _device = device;
    _allocator = allocator;
    _res = res;

    const VkDeviceSize bufferSize = _res * _res * _res * sizeof(float);
    const VkDeviceSize velBufferSize = (_res+1) * _res * _res * sizeof(float);
    const VkDeviceSize boarderBufferSize = (_res+2) * (_res+2) * (_res+2) * sizeof(float);

    _vx = {0, velBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _vy = {1, velBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _vz = {2, velBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    _density = {3, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _pressure = {4, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _source = {4, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    _vx2 = {5, velBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _vy2 = {6, velBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _vz2 = {7, velBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    _density2 = {8, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _pressure2 = {9, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};
    _source2 = {9, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    _boundaries = {10, boarderBufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER};

    _densityTex = {11, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE};

    vkinit::createResource(_device, _allocator, _vx);
    vkinit::createResource(_device, _allocator, _vy);
    vkinit::createResource(_device, _allocator, _vz);
    vkinit::createResource(_device, _allocator, _density);
    vkinit::createResource(_device, _allocator, _pressure);
    vkinit::createResource(_device, _allocator, _source);

    vkinit::createResource(_device, _allocator, _vx2);
    vkinit::createResource(_device, _allocator, _vy2);
    vkinit::createResource(_device, _allocator, _vz2);
    vkinit::createResource(_device, _allocator, _density2);
    vkinit::createResource(_device, _allocator, _pressure2);
    vkinit::createResource(_device, _allocator, _source2);

    vkinit::createResource(_device, _allocator, _boundaries);

    _densityTex.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    vkinit::createResource(_device, _allocator, _densityTex, {_res, _res, _res}, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	_densityTex.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; // revert back for compute shader

    std::vector<ResourceBinding> resourceBindings = {
        _vx, _vy, _vz, _density, _pressure, _source,
        _vx2, _vy2, _vz2, _density2, _pressure2, _source2,
        _boundaries, _densityTex
    };


    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(CFDPushConstants);
	std::vector<VkPushConstantRange> pushConstants = { pushConstantRange };

	std::vector<ResourceBinding> swappedBindings = resourceBindings;
	std::swap(swappedBindings[0], swappedBindings[6]);
	std::swap(swappedBindings[1], swappedBindings[7]);
	std::swap(swappedBindings[2], swappedBindings[8]);
	std::swap(swappedBindings[3], swappedBindings[9]);
	std::swap(swappedBindings[4], swappedBindings[10]);
    std::swap(swappedBindings[5], swappedBindings[11]);

    printf("Creating CFD Kernels...\n");

	_gaussSidel = vkinit::initKernel(_device, KernelType::Compute, { "build/shaders/gaussSiedel.comp.spv" }, resourceBindings, pushConstants);
	vkinit::updateKernelDescriptors(_device, _gaussSidel, resourceBindings);

    _advect = vkinit::initKernel(_device, KernelType::Compute, { "build/shaders/advect.comp.spv" }, resourceBindings, pushConstants);
	vkinit::updateKernelDescriptors(_device, _advect, resourceBindings);

	_advectSwapped = vkinit::initKernel(_device, KernelType::Compute, { "build/shaders/advect.comp.spv" }, swappedBindings, pushConstants);
	vkinit::updateKernelDescriptors(_device, _advectSwapped, swappedBindings);

	_writeTexture = vkinit::initKernel(_device, KernelType::Compute, { "build/shaders/writeTexture.comp.spv" }, resourceBindings, pushConstants);
	vkinit::updateKernelDescriptors(_device, _writeTexture, resourceBindings);

	_writeTextureSwapped = vkinit::initKernel(_device, KernelType::Compute, { "build/shaders/writeTexture.comp.spv" }, swappedBindings, pushConstants);
	vkinit::updateKernelDescriptors(_device, _writeTextureSwapped, swappedBindings);

    printf("Initialized CFD with res %d\n", _res);
}

void Cfd::evolve_cfd_cmd(VkCommandBuffer commandBuffer)
{
    const uint local_work_size = 32;

    const VkDeviceSize nThreads = (_res * _res * _res + local_work_size - 1) / local_work_size;
    const VkDeviceSize nThreadsVel = ((_res+1) * _res * _res + local_work_size - 1) / local_work_size;

    for (int i=0; i<50; i++)
    {
        CFDPushConstants pushData;
        pushData.gridSize = _res;
        pushData.shouldRed = i % 2;
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gaussSidel.pipeline);
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _gaussSidel.pipelineLayout, 0, 1, &_gaussSidel.descriptorSet, 0, nullptr);
        vkCmdPushConstants(commandBuffer, _gaussSidel.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CFDPushConstants), &pushData);

        vkCmdDispatch(commandBuffer, nThreads, 1, 1);
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _advect.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _advect.pipelineLayout, 0, 1, &_advect.descriptorSet, 0, nullptr);
    vkCmdDispatch(commandBuffer, nThreadsVel, 1, 1);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _advectSwapped.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _advectSwapped.pipelineLayout, 0, 1, &_advectSwapped.descriptorSet, 0, nullptr);
    vkCmdDispatch(commandBuffer, nThreadsVel, 1, 1);

    CFDPushConstants pushData;
    pushData.gridSize = _res;

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _writeTexture.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _writeTexture.pipelineLayout, 0, 1, &_writeTexture.descriptorSet, 0, nullptr);
    vkCmdPushConstants(commandBuffer, _writeTexture.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CFDPushConstants), &pushData);
    vkCmdDispatch(commandBuffer, nThreads, 1, 1);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _writeTextureSwapped.pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, _writeTextureSwapped.pipelineLayout, 0, 1, &_writeTextureSwapped.descriptorSet, 0, nullptr);
    vkCmdPushConstants(commandBuffer, _writeTextureSwapped.pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(CFDPushConstants), &pushData);
    vkCmdDispatch(commandBuffer, nThreads, 1, 1);

}

void Cfd::load_default_state(VkCommandPool commandPool, VkQueue queue)
{
    // std::vector<float> vxs = init_wall(10.0f, _res+1, _res, _res);
    std::vector<float> vxs = init_vels(_res, 0.0f);
    std::vector<float> vys = init_vels(_res, 0.0f);
    std::vector<float> vzs = init_vels(_res, 0.0f);
    std::vector<float> densities = init_scalars(_res, 0.0f);
    std::vector<float> pressures = init_scalars(_res, 0.0f);
    std::vector<float> source = init_scalars(_res, 0.0f);
    std::vector<float> boundariesVec = init_boundaries(_res+2);

    // set velocity x to 1 in the first quarter
    int boarder = 40;
    for (int z=0; z<_res-2*boarder; z++) {
        for (int y=0; y<_res-2*boarder; y++) {
            for (int x=0; x<_res/4; x++) {
                vxs[(_res+1)*_res*(z+boarder) + (_res+1)*(y+boarder) + x] = 20.0f;
            }
        }
    }

    // Arbitrary Geometry
    // add_boundary_cylinder(boundariesVec, 10, 0, 0, _res+2);
    // add_boundary_cylinder(boundariesVec, 10, -20, -20, _res+2);

    // int nStreams = 20;
    // int streamSize = _res / nStreams;
    // for (int i=1; i<nStreams-1; i++) {
    //     densities[_res*_res*(_res/2) + _res*i*streamSize + 10] = 10.0f;
    //     source[_res*_res*(_res/2) + _res*i*streamSize + 10] = 1.0f;

    //     densities[_res*_res*(_res/2) + _res*i*streamSize + 80] = 10.0f;
    //     source[_res*_res*(_res/2) + _res*i*streamSize + 80] = 1.0f;
    // }

    // profuce 3d grid of 20x20x20 streams
    int nStreams3D = 10;
    int streamSize3D = _res / nStreams3D;
    for (int i=1; i<nStreams3D-1; i++) {
        for (int j=1; j<nStreams3D-1; j++) {
            for (int k=1; k<nStreams3D-1; k++) {
                densities[_res*_res*(k*streamSize3D) + _res*i*streamSize3D + j*streamSize3D] = 10.0f;
                source[_res*_res*(k*streamSize3D) + _res*i*streamSize3D + j*streamSize3D] = 1.0f;
            }
        }
    }    

    // densities[_res*_res*_res + _res*_res + _res-1] = 10.0f; // last cell is 0 density
    // source[_res*_res*_res + _res*_res + _res-1] = 10.0f; // last cell is 0 density
    // for (int i=0; i<_res+2; i++)
    // {
    //     boundariesVec[(_res+2)*(_res+2)*(_res/2+1) + (_res+2)*(i) + (0+1)] = 0.0f;
    // }

    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _vx, vxs.data(), vxs.size() * sizeof(float));
    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _vy, vys.data(), vys.size() * sizeof(float));
    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _vz, vzs.data(), vzs.size() * sizeof(float));
    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _density, densities.data(), densities.size() * sizeof(float));
    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _pressure, pressures.data(), pressures.size() * sizeof(float));
    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _source, source.data(), source.size() * sizeof(float));
    vkhelp::copy_to_buffer(_device, _allocator, commandPool, queue, _boundaries, boundariesVec.data(), boundariesVec.size() * sizeof(float));

    // Test read back
    // std::vector<float> testDensity(densities.size());
    // vkhelp::copy_from_buffer(_device, _allocator, commandPool, queue, _density, testDensity.data(), testDensity.size() * sizeof(float));
    // for (int i=0; i<10; i++) {
    //     printf("Density[%d] = %f\n", i, testDensity[i]);
    // }    
}

std::vector<ResourceBinding> Cfd::get_texture_bindings()
{
    // std::vector<uint32_t> activeBindings = {11};
	// auto subsetResources = vkinit::subsetVector(_resourceBindings, activeBindings);
    std::vector<ResourceBinding> subsetResources = {_densityTex};
    return subsetResources;
}
