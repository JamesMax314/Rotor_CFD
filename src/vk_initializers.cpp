#include "vk_initializers.h"
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

VkCommandPoolCreateInfo vkinit::command_pool_create_info(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/)
{
	VkCommandPoolCreateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	info.pNext = nullptr;

	info.queueFamilyIndex = queueFamilyIndex;
	info.flags = flags;
	return info;
}

VkCommandBufferAllocateInfo vkinit::command_buffer_allocate_info(VkCommandPool pool, uint32_t count /*= 1*/, VkCommandBufferLevel level /*= VK_COMMAND_BUFFER_LEVEL_PRIMARY*/)
{
	VkCommandBufferAllocateInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	info.pNext = nullptr;

	info.commandPool = pool;
	info.commandBufferCount = count;
	info.level = level;
	return info;
}

VkPipelineShaderStageCreateInfo vkinit::pipeline_shader_stage_create_info(VkShaderStageFlagBits stage, VkShaderModule shaderModule) {

    VkPipelineShaderStageCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    info.pNext = nullptr;

    //shader stage
    info.stage = stage;
    //module containing the code for this shader stage
    info.module = shaderModule;
    //the entry point of the shader
    info.pName = "main";
    return info;
}

VkPipelineVertexInputStateCreateInfo vkinit::vertex_input_state_create_info() {
    VkPipelineVertexInputStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    info.pNext = nullptr;

    //no vertex bindings or attributes
    info.vertexBindingDescriptionCount = 0;
    info.vertexAttributeDescriptionCount = 0;
    return info;
}

VkPipelineInputAssemblyStateCreateInfo vkinit::input_assembly_create_info(VkPrimitiveTopology topology) {
    VkPipelineInputAssemblyStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.topology = topology;
    //we are not going to use primitive restart on the entire tutorial so leave it on false
    info.primitiveRestartEnable = VK_TRUE;
    return info;
}

VkPipelineRasterizationStateCreateInfo vkinit::rasterization_state_create_info(VkPolygonMode polygonMode){
    VkPipelineRasterizationStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.depthClampEnable = VK_FALSE;
    //discards all primitives before the rasterization stage if enabled which we don't want
    info.rasterizerDiscardEnable = VK_FALSE;

    info.polygonMode = polygonMode;
    info.lineWidth = 1.0f;
    //no backface cull
    info.cullMode = VK_CULL_MODE_NONE;
    info.frontFace = VK_FRONT_FACE_CLOCKWISE;
    //no depth bias
    info.depthBiasEnable = VK_FALSE;
    info.depthBiasConstantFactor = 0.0f;
    info.depthBiasClamp = 0.0f;
    info.depthBiasSlopeFactor = 0.0f;

    return info;
}

VkPipelineMultisampleStateCreateInfo vkinit::multisampling_state_create_info(){
    VkPipelineMultisampleStateCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    info.pNext = nullptr;

    info.sampleShadingEnable = VK_FALSE;
    //multisampling defaulted to no multisampling (1 sample per pixel)
    info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    info.minSampleShading = 1.0f;
    info.pSampleMask = nullptr;
    info.alphaToCoverageEnable = VK_FALSE;
    info.alphaToOneEnable = VK_FALSE;
    return info;
}

VkPipelineColorBlendAttachmentState vkinit::color_blend_attachment_state() {
    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    return colorBlendAttachment;
}

VkPipelineLayoutCreateInfo vkinit::pipeline_layout_create_info() {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;

    //empty defaults
    info.flags = 0;
    info.setLayoutCount = 0;
    info.pSetLayouts = nullptr;
    info.pushConstantRangeCount = 0;
    info.pPushConstantRanges = nullptr;
    return info;
}

VkPipelineLayoutCreateInfo vkinit::pipeline_layout_create_info(VkDevice device,
    const std::vector<VkDescriptorSetLayoutBinding>& bindings,
    const std::vector<VkPushConstantRange>& pushConstants,
    std::vector<VkDescriptorSetLayout>& outSetLayouts // we return layouts here so caller can own/destroy them
) {
    VkPipelineLayoutCreateInfo info{};
    info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    info.pNext = nullptr;
    info.flags = 0;

    // ---- build descriptor set layout(s) from bindings ----
    outSetLayouts.clear();
    if (!bindings.empty()) {
        VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
        setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        setLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        setLayoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout setLayout;
        if (vkCreateDescriptorSetLayout(device, &setLayoutInfo, nullptr, &setLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout");
        }

        outSetLayouts.push_back(setLayout);

        info.setLayoutCount = static_cast<uint32_t>(outSetLayouts.size());
        info.pSetLayouts = outSetLayouts.data();
    } else {
        info.setLayoutCount = 0;
        info.pSetLayouts = nullptr;
    }

    // ---- push constants ----
    if (!pushConstants.empty()) {
        info.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        info.pPushConstantRanges = pushConstants.data();
    } else {
        info.pushConstantRangeCount = 0;
        info.pPushConstantRanges = nullptr;
    }

    return info;
}


VkFenceCreateInfo vkinit::fence_create_info(VkFenceCreateFlags flags)
{
    VkFenceCreateInfo fenceCreateInfo = {};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.flags = flags;
    return fenceCreateInfo;
}

VkSemaphoreCreateInfo vkinit::semaphore_create_info(VkSemaphoreCreateFlags flags)
{
    VkSemaphoreCreateInfo semCreateInfo = {};
    semCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semCreateInfo.pNext = nullptr;
    semCreateInfo.flags = flags;
    return semCreateInfo;
}

VkImageView vkinit::createImageView3D(VkDevice device, VkImage image, VkFormat format) {
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;  // 1D, 2D, 3D, CUBE, etc.
    viewInfo.format = format;

    // What parts of the image are accessible
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // or DEPTH/DEPTH_STENCIL
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;  // how many mip levels
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;  // how many array layers

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        std::cout << "failed to create image view!" << std::endl;
    }

    return imageView;
}

uint32_t vkinit::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties, VkPhysicalDevice physicalDevice) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    return 0;
}

bool vkinit::load_shader_module(VkDevice &device ,const char* filePath, VkShaderModule* outShaderModule)
{
	//open the file. With cursor at the end
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
        std::cout << "Failed to open file: " << filePath << std::endl;
		return false;
	}

    //find what the size of the file is by looking up the location of the cursor
    //because the cursor is at the end, it gives the size directly in bytes
    size_t fileSize = (size_t)file.tellg();

    //spirv expects the buffer to be on uint32, so make sure to reserve an int vector big enough for the entire file
    std::vector<uint32_t> buffer(fileSize / sizeof(uint32_t));

    //put file cursor at beginning
    file.seekg(0);

    //load the entire file into the buffer
    file.read((char*)buffer.data(), fileSize);

    //now that the file is loaded into the buffer, we can close it
    file.close();

    //create a new shader module, using the buffer we loaded
    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pNext = nullptr;

    //codeSize has to be in bytes, so multiply the ints in the buffer by size of int to know the real size of the buffer
    createInfo.codeSize = buffer.size() * sizeof(uint32_t);
    createInfo.pCode = buffer.data();

    //check that the creation goes well.
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        return false;
    }
    *outShaderModule = shaderModule;
    return true;
}

VkShaderModule vkinit::createShaderModule(VkDevice &device, const std::string& filename) {
    VkShaderModule shaderModule;
    if (!vkinit::load_shader_module(device, filename.c_str(), &shaderModule)) {
        throw std::runtime_error("Failed to load shader: " + filename);
    }
    return shaderModule;
}

Kernel vkinit::initKernel(
    VkDevice device,
    KernelType type, 
    const std::vector<std::string>& shaderPaths, // compute: 1 file, graphics: vert+frag
    const std::vector<VkDescriptorSetLayoutBinding>& bindings,
    const std::vector<VkPushConstantRange>& pushConstants,
    VkRenderPass renderPass, // only used for graphics
    VkExtent2D _windowExtent // only needed for graphics pipelines
) {
    Kernel k{};
    k.type = type;
    k.pushConstants = pushConstants;

    // ---- 1. Load shader modules ----
    std::vector<VkShaderModule> shaderModules;
    for (auto& path : shaderPaths) {
        VkShaderModule mod;
        if (!load_shader_module(device, path.c_str(), &mod)) {
            throw std::runtime_error("Failed to load shader module: " + path);
        }
        shaderModules.push_back(mod);
    }

    // ---- 2. Descriptor set layout ----
    VkDescriptorSetLayoutCreateInfo setLayoutInfo{};
    setLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    setLayoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    setLayoutInfo.pBindings = bindings.data();

    VK_CHECK(vkCreateDescriptorSetLayout(device, &setLayoutInfo, nullptr, &k.descriptorSetLayout));

    // ---- 3. Pipeline layout ----
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &k.descriptorSetLayout;

    if (!pushConstants.empty()) {
        pipelineLayoutInfo.pushConstantRangeCount = static_cast<uint32_t>(pushConstants.size());
        pipelineLayoutInfo.pPushConstantRanges = pushConstants.data();
    }

    VK_CHECK(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &k.pipelineLayout));

    // ---- 4. Create pipeline ----
    if (type == KernelType::Compute) {
        VkComputePipelineCreateInfo computeInfo{};
        computeInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        computeInfo.layout = k.pipelineLayout;
        computeInfo.stage = vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_COMPUTE_BIT, shaderModules[0]);

        VK_CHECK(vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &computeInfo, nullptr, &k.pipeline));
    } else { // Graphics
        PipelineBuilder builder;
        builder._pipelineLayout = k.pipelineLayout;

        // Vertex and fragment stages
        builder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_VERTEX_BIT, shaderModules[0]));
        builder._shaderStages.push_back(vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_FRAGMENT_BIT, shaderModules[1]));

        // Fill in defaults: vertex input, input assembly, viewport/scissor, rasterizer, multisample, blend
        builder._vertexInputInfo = vkinit::vertex_input_state_create_info();
        builder._inputAssembly = vkinit::input_assembly_create_info(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP);
        builder._viewport.x = 0.0f;
        builder._viewport.y = 0.0f;
        builder._viewport.width = (float)_windowExtent.width;
        builder._viewport.height = (float)_windowExtent.height;
        builder._viewport.minDepth = 0.0f;
        builder._viewport.maxDepth = 1.0f;
        builder._scissor.offset = { 0, 0 };
        builder._scissor.extent = _windowExtent;
        builder._rasterizer = vkinit::rasterization_state_create_info(VK_POLYGON_MODE_FILL);
        builder._multisampling = vkinit::multisampling_state_create_info();
        builder._colorBlendAttachment = vkinit::color_blend_attachment_state();

        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        auto bindingDesc = Vertex::getBindingDescription();
        auto attrDescs = Vertex::getAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.pVertexBindingDescriptions = &bindingDesc;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attrDescs.size());
        vertexInputInfo.pVertexAttributeDescriptions = attrDescs.data();

        builder._vertexInputInfo = vertexInputInfo;


        k.pipeline = builder.build_pipeline(device, renderPass);
    }

    // ---- 5. Destroy shader modules (pipeline keeps a reference) ----
    for (auto mod : shaderModules) vkDestroyShaderModule(device, mod, nullptr);

    // ---- 6. Create descriptor pool ----
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto& b : bindings) {
        VkDescriptorPoolSize ps{};
        ps.type = b.descriptorType;
        ps.descriptorCount = b.descriptorCount;
        poolSizes.push_back(ps);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    VK_CHECK(vkCreateDescriptorPool(device, &poolInfo, nullptr, &k.descriptorPool));

    // ---- 7. Allocate descriptor set ----
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = k.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &k.descriptorSetLayout;

    VK_CHECK(vkAllocateDescriptorSets(device, &allocInfo, &k.descriptorSet));

    return k;
}

void vkinit::updateKernelDescriptors(VkDevice device, Kernel& kernel,
                             const std::vector<ResourceBinding>& resources) {
    std::vector<VkWriteDescriptorSet> writes;

    for (auto& res : resources) {
        if (res.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
            VkDescriptorBufferInfo bufInfo{res.buffer, 0, VK_WHOLE_SIZE};
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = kernel.descriptorSet;
            write.dstBinding = res.binding;
            write.descriptorType = res.type;
            write.descriptorCount = 1;
            write.pBufferInfo = &bufInfo;
            writes.push_back(write);
        } else {
            VkDescriptorImageInfo imgInfo{res.sampler, res.imageView, res.layout};
            VkWriteDescriptorSet write{};
            write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            write.dstSet = kernel.descriptorSet;
            write.dstBinding = res.binding;
            write.descriptorType = res.type;
            write.descriptorCount = 1;
            write.pImageInfo = &imgInfo;
            writes.push_back(write);
        }
    }

    vkUpdateDescriptorSets(device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}


KernelOld vkinit::initKernel(VkDevice &device, const std::string& shaderPath,
                                const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    KernelOld kernel{};

    // 1. Descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &kernel.descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

    // 2. Pipeline layout
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(CamData); // must match your shader block size

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &kernel.descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &kernel.pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline layout!");
    }

    // 3. Shader module
    kernel.shaderModule = vkinit::createShaderModule(device, shaderPath);

    printf("Shader module created from %s\n", shaderPath.c_str());

    // 4. Compute pipeline
    VkPipelineShaderStageCreateInfo shaderStageInfo =
        vkinit::pipeline_shader_stage_create_info(VK_SHADER_STAGE_COMPUTE_BIT, kernel.shaderModule);

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.stage = shaderStageInfo;
    pipelineInfo.layout = kernel.pipelineLayout;

    if (vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &kernel.pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute pipeline!");
    }

    // Shader module no longer needed after pipeline creation
    vkDestroyShaderModule(device, kernel.shaderModule, nullptr);
    kernel.shaderModule = VK_NULL_HANDLE;

    std::vector<VkDescriptorPoolSize> poolSizes = createPoolSizesFromBindings(bindings);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &kernel.descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create compute descriptor pool!");
    }

    printf("Descriptor pool created with %zu bindings\n", bindings.size());

    // 6. Allocate descriptor set
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = kernel.descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &kernel.descriptorSetLayout;

    if (vkAllocateDescriptorSets(device, &allocInfo, &kernel.descriptorSet) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor set!");
    }

    printf("Compute kernel initialized from %s\n", shaderPath.c_str());

    return kernel;
}

VkWriteDescriptorSet vkinit::writeDescriptorImage(VkDescriptorSet dstSet, uint32_t binding, VkDescriptorType type, const VkDescriptorImageInfo* imageInfo) {
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = dstSet;
    write.dstBinding = binding;
    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = imageInfo;
    return write;
}

void vkinit::updateStorageBuffers(VkDevice device, VkDescriptorSet dstSet, const std::vector<BufferBinding>& buffers) {
    std::vector<VkDescriptorBufferInfo> bufferInfos(buffers.size());
    std::vector<VkWriteDescriptorSet> writes(buffers.size());

    for (size_t i = 0; i < buffers.size(); i++) {
        bufferInfos[i].buffer = buffers[i].buffer;
        bufferInfos[i].offset = 0;
        bufferInfos[i].range  = buffers[i].size;

        writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet          = dstSet;
        writes[i].dstBinding      = buffers[i].binding;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].descriptorCount = 1;
        writes[i].pBufferInfo     = &bufferInfos[i];
    }

    vkUpdateDescriptorSets(device,
                           static_cast<uint32_t>(writes.size()), writes.data(),
                           0, nullptr);
}

void vkinit::createStorageBuffers(VkDevice device, VmaAllocator allocator, std::vector<BufferBinding>& buffers)
{
    for (auto& b : buffers) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = b.size;
        bufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &b.buffer, &b.allocation, nullptr) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create storage buffer!");
        }

    }
}

void vkinit::createResources(
    VkDevice device,
    VmaAllocator allocator,
    std::vector<ResourceBinding>& resources,
    VkExtent3D defaultImageExtent,              // you can pass per-resource extent if needed
    VkFormat defaultImageFormat,
    VkImageUsageFlags imageUsage
) {
    for (auto& r : resources) {
        if (r.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
            r.type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) 
        {
            // --- Buffer creation ---
            VkBufferCreateInfo bufferInfo{};
            bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            bufferInfo.size = r.range;  // use range field
            bufferInfo.usage = (r.type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
                ? (VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT)
                : VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateBuffer(allocator, &bufferInfo, &allocInfo,
                                &r.buffer, &r.bufferAllocation, nullptr) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to create buffer resource!");
            }
        }
        else if (r.type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE ||
                 r.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) 
        {
            // --- Image creation ---
            VkImageCreateInfo imageInfo{};
            imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageInfo.imageType = VK_IMAGE_TYPE_3D;  // assume volume texture, change if needed
            imageInfo.format = defaultImageFormat;
            imageInfo.extent = defaultImageExtent;
            imageInfo.mipLevels = 1;
            imageInfo.arrayLayers = 1;
            imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
            imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
            imageInfo.usage = imageUsage;
            imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

            VmaAllocationCreateInfo allocInfo{};
            allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

            if (vmaCreateImage(allocator, &imageInfo, &allocInfo,
                               &r.image, &r.imageAllocation, nullptr) != VK_SUCCESS) 
            {
                throw std::runtime_error("Failed to create image resource!");
            }

            // Create image view
            VkImageViewCreateInfo viewInfo{};
            viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            viewInfo.image = r.image;
            viewInfo.viewType = VK_IMAGE_VIEW_TYPE_3D;
            viewInfo.format = defaultImageFormat;
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            viewInfo.subresourceRange.baseMipLevel = 0;
            viewInfo.subresourceRange.levelCount = 1;
            viewInfo.subresourceRange.baseArrayLayer = 0;
            viewInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &viewInfo, nullptr, &r.imageView) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create image view!");
            }

            // Create sampler if needed
            if (r.type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                VkSamplerCreateInfo samplerInfo{};
                samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
                samplerInfo.magFilter = VK_FILTER_LINEAR;
                samplerInfo.minFilter = VK_FILTER_LINEAR;
                samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
                samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
                samplerInfo.maxLod = 1.0f;

                if (vkCreateSampler(device, &samplerInfo, nullptr, &r.sampler) != VK_SUCCESS) {
                    throw std::runtime_error("Failed to create sampler!");
                }
                printf("Created sampler for resource binding %d\n", r.binding);
            }

            // Record layout
            r.layout = VK_IMAGE_LAYOUT_GENERAL;
        }
        else {
            throw std::runtime_error("Unsupported resource type in createResources");
        }
    }
}

std::vector<VkDescriptorPoolSize> vkinit::createPoolSizesFromBindings(const std::vector<VkDescriptorSetLayoutBinding>& bindings) {
    // Map descriptor type → count
    std::unordered_map<VkDescriptorType, uint32_t> typeCounts;

    for (const auto& b : bindings) {
        // accumulate counts
        typeCounts[b.descriptorType] += b.descriptorCount;
    }

    // Convert map to VkDescriptorPoolSize array
    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.reserve(typeCounts.size());

    for (const auto& [type, count] : typeCounts) {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = count;
        poolSizes.push_back(poolSize);
    }

    return poolSizes;
}

VkCommandBuffer vkinit::beginSingleTimeCommands(VkDevice device, VkCommandPool commandPool) {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void vkinit::endSingleTimeCommands(VkDevice device, VkCommandPool commandPool, VkQueue queue, VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(queue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void vkinit::transitionImageLayout(
    VkCommandBuffer cmd,
    VkImage image,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkAccessFlags srcAccessMask,
    VkAccessFlags dstAccessMask,
    VkPipelineStageFlags srcStage,
    VkPipelineStageFlags dstStage,
    VkImageAspectFlags aspectMask,
    uint32_t baseMipLevel,
    uint32_t levelCount,
    uint32_t baseArrayLayer,
    uint32_t layerCount
) {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = aspectMask;
    barrier.subresourceRange.baseMipLevel = baseMipLevel;
    barrier.subresourceRange.levelCount = levelCount;
    barrier.subresourceRange.baseArrayLayer = baseArrayLayer;
    barrier.subresourceRange.layerCount = layerCount;
    barrier.srcAccessMask = srcAccessMask;
    barrier.dstAccessMask = dstAccessMask;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        dstStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
}

void vkinit::initMesh(Mesh &mesh, VkDevice device, VkCommandPool commandPool, VkQueue queue, VmaAllocator allocator, const std::vector<Vertex> &vertices, const std::vector<uint16_t> &indices)
{
    mesh.init(device, commandPool, queue, allocator, vertices, indices);
}

void Mesh::init(VkDevice device, VkCommandPool commandPool, VkQueue queue, VmaAllocator allocator,
              const std::vector<Vertex>& vertices,
              const std::vector<uint16_t>& indices){

    this->device = device;
    this->commandPool = commandPool;
    this->queue = queue;
    upload(allocator, vertices, indices);
}

void Mesh::uploadBufferData(VmaAllocator allocator,
                          VkBuffer dstBuffer,
                          const void* data,
                          VkDeviceSize size) {
    // Create staging buffer
    VkBufferCreateInfo stagingInfo{};
    stagingInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingInfo.size = size;
    stagingInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo stagingAllocInfo{};
    stagingAllocInfo.usage = VMA_MEMORY_USAGE_AUTO; 
    stagingAllocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkBuffer stagingBuffer;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo allocInfo;

    VK_CHECK(vmaCreateBuffer(allocator, &stagingInfo, &stagingAllocInfo,
                                &stagingBuffer, &stagingAlloc, &allocInfo));

    // Copy data into staging
    memcpy(allocInfo.pMappedData, data, (size_t)size);

    // Submit copy command
    VkCommandBuffer cmd = vkinit::beginSingleTimeCommands(device, commandPool); // <-- you already have this helper
    VkBufferCopy copy{};
    copy.size = size;
    vkCmdCopyBuffer(cmd, stagingBuffer, dstBuffer, 1, &copy);
    vkinit::endSingleTimeCommands(device, commandPool, queue, cmd); // <-- and this one

    // Cleanup staging
    vmaDestroyBuffer(allocator, stagingBuffer, stagingAlloc);
}

void Mesh::draw(VkCommandBuffer cmd) const {
    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(cmd, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(cmd, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(cmd, indexCount, 1, 0, 0, 0);
}

void Mesh::upload(VmaAllocator allocator,
            const std::vector<Vertex>& vertices,
            const std::vector<uint16_t>& indices) {
    indexCount = static_cast<uint32_t>(indices.size());

    // --- Vertex buffer ---
    VkBufferCreateInfo vbInfo{};
    vbInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vbInfo.size = sizeof(Vertex) * vertices.size();
    vbInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VmaAllocationCreateInfo vmaAllocInfo{};
    vmaAllocInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;

    VK_CHECK(vmaCreateBuffer(allocator, &vbInfo, &vmaAllocInfo,
                            &vertexBuffer, &vertexAllocation, nullptr));

    // --- Index buffer ---
    VkBufferCreateInfo ibInfo{};
    ibInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ibInfo.size = sizeof(uint16_t) * indices.size();
    ibInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VK_CHECK(vmaCreateBuffer(allocator, &ibInfo, &vmaAllocInfo,
                            &indexBuffer, &indexAllocation, nullptr));

    // --- Upload data using staging buffer ---
    uploadBufferData(allocator, vertexBuffer, vertices.data(), vbInfo.size);
    uploadBufferData(allocator, indexBuffer, indices.data(), ibInfo.size);
}