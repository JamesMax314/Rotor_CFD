#include "vk_engine.h"

#include <SDL.h>
#include <SDL_vulkan.h>

void VulkanEngine::init()
{
    // We initialize SDL and create a window with it. 
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);
	
    //create blank SDL window for our application
	_window = SDL_CreateWindow(
		"Vulkan Engine", //window title
		SDL_WINDOWPOS_UNDEFINED, //window position x (don't care)
		SDL_WINDOWPOS_UNDEFINED, //window position y (don't care)
		_windowExtent.width,  //window width in pixels
		_windowExtent.height, //window height in pixels
		window_flags 
	);
	
    //load the core Vulkan structures
	init_vulkan();

	printf("init vulkan complete\n");

	//create the swapchain
	init_swapchain();

	printf("init swapchain complete\n");

    init_commands();

	printf("init commands complete\n");

	init_allocator();

	printf("init allocator complete\n");

    init_default_renderpass();

	printf("init default renderpass complete\n");

	init_offscreen_render_pass();

	printf("init offscreen renderpass complete\n");

	init_render_images();

	printf("init depth image \n");

	init_framebuffers();

	printf("init framebuffers complete\n");

    init_sync_structures();

	printf("init sync structures complete\n");

	init_cfd();

	printf("init CFD complete \n");

	initSSBOs();

	printf("init SSBOs complete\n");

    initKernels();

	init_terrain_rendering();

	load_terrain_model("Data/out_data.txt"); 
	// load_terrain_model("Data/test_data.txt"); 

	// load_model();

	//everything went fine
	_isInitialized = true;

	printf("Vulkan Engine initialized\n");
}

void VulkanEngine::initSSBOs() {
	VkCommandBuffer cmd = vkinit::beginSingleTimeCommands(_device, _commandPool);

	vkinit::transitionImageLayout(
		cmd,
		_cfd.get_texture_bindings().at(0).image,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_GENERAL,
		0,    // srcStage (instead of 0)
		VK_ACCESS_SHADER_WRITE_BIT, // dstStage
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,                                    // srcAccessMask
		VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT            // dstAccessMask
	);


	vkinit::endSingleTimeCommands(_device, _commandPool, _graphicsQueue, cmd);

	vkinit::initMesh(_quadMesh, _device, _commandPool, _graphicsQueue, _allocator, _quadVertices, _quadIndices);

	printf("Buffers and textures created\n");
}

void VulkanEngine::init_cfd()
{
	_cfd.init_cfd(_device, _allocator, _res);
	_cfd.load_default_state(_commandPool, _graphicsQueue);
}

void VulkanEngine::init_camera()
{
	_camData.pos[0] = -1.0f; _camData.pos[1] = -1.0f; _camData.pos[2] = -1.0f;
	_camData.camUp[0] = 0.0f; _camData.camUp[1] = 1.0f; _camData.camUp[2] = 0.0f;
	_camData.lookAt[0] = 0.0f; _camData.lookAt[1] = 0.0f; _camData.lookAt[2] = 0.0f;

	SDL_ShowCursor(SDL_ENABLE);
	SDL_SetRelativeMouseMode(SDL_FALSE);
}

void VulkanEngine::initKernels() {
	VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(CamData);
	std::vector<VkPushConstantRange> pushConstants = { pushConstantRange };

	std::vector<ResourceBinding> subsetResources = {_cfd.get_texture_bindings()[0], _depthImage, _rasterColourImage};
	for (int i=0; i<subsetResources.size(); i++) {
		subsetResources[i].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}

	std::vector<VkDescriptorSetLayoutBinding> subsetLayoutBindings = {
		{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
		{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
		{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr}};

	vkhelp::transitionImageLayout(
		_device, _commandPool, _graphicsQueue,
		subsetResources[1],
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT
	);

	vkhelp::transitionImageLayout(
		_device, _commandPool, _graphicsQueue,
		subsetResources[2],
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT
	);
	
	_rp = vkinit::initKernel(_device, KernelType::Graphics, 
		{"build/shaders/triangle.vert.spv", "build/shaders/rayTrace.frag.spv"}, 
		subsetLayoutBindings, pushConstants, _renderPass, _windowExtent);

	vkinit::updateKernelDescriptors(_device, _rp, subsetResources);

	printf("Ray trace kernel initialized\n");

}

// Simplify
void VulkanEngine::init_3D_texture()
{
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_3D;
    imageInfo.extent = { _res, _res, _res };
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R32G32B32A32_SFLOAT; // or whatever fits your data
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    vkCreateImage(_device, &imageInfo, nullptr, &_3DTexture);

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_device, _3DTexture, &memRequirements);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = vkinit::findMemoryType(memRequirements.memoryTypeBits,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            _chosenGPU);

    if (vkAllocateMemory(_device, &allocInfo, nullptr, &_3DTextureMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(_device, _3DTexture, _3DTextureMemory, 0);

    _3DTextureView = vkinit::createImageView3D(_device, _3DTexture, VK_FORMAT_R32G32B32A32_SFLOAT);
}

void VulkanEngine::init_sync_structures()
{
	VkFenceCreateInfo fenceCreateInfo = vkinit::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);

	VK_CHECK(vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence));

    //enqueue the destruction of the fence
    _mainDeletionQueue.push_function([=]() {
        vkDestroyFence(_device, _renderFence, nullptr);
    });

	VkSemaphoreCreateInfo semaphoreCreateInfo = vkinit::semaphore_create_info();

	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore));
	VK_CHECK(vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore));

    //enqueue the destruction of semaphores
    _mainDeletionQueue.push_function([=]() {
        vkDestroySemaphore(_device, _presentSemaphore, nullptr);
        vkDestroySemaphore(_device, _renderSemaphore, nullptr);
    });
}

void VulkanEngine::init_render_framebuffers()
{
    // Grab how many images we have in the swapchain
    const uint32_t swapchain_imagecount = _swapchainImages.size();
    _framebuffers.resize(swapchain_imagecount);

    for (uint32_t i = 0; i < swapchain_imagecount; i++) {
        // Two attachments: color (swapchain) + depth
        std::array<VkImageView, 1> attachments = {
            _swapchainImageViews[i],   // Color
            // _depthImage.imageView      // Depth (same for all framebuffers)
        };

        VkFramebufferCreateInfo fb_info{};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = nullptr;
        fb_info.renderPass = _renderPass;
        fb_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        fb_info.pAttachments = attachments.data();
        fb_info.width = _windowExtent.width;
        fb_info.height = _windowExtent.height;
        fb_info.layers = 1;

        VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_framebuffers[i]));
    }

    // Cleanup for all framebuffers
    _mainDeletionQueue.push_function([=]() {
        for (auto fb : _framebuffers) {
            vkDestroyFramebuffer(_device, fb, nullptr);
        }
    });
}

void VulkanEngine::init_offscreen_framebuffer()
{
    // Grab how many images we have in the swapchain

	{
        // Two attachments: color (swapchain) + depth
        std::array<VkImageView, 2> attachments = {
            _rasterColourImage.imageView,   // Color
            _depthImage.imageView      // Depth (same for all framebuffers)
        };

        VkFramebufferCreateInfo fb_info{};
        fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fb_info.pNext = nullptr;
        fb_info.renderPass = _rasterRenderPass;
        fb_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        fb_info.pAttachments = attachments.data();
        fb_info.width = _windowExtent.width;
        fb_info.height = _windowExtent.height;
        fb_info.layers = 1;

        VK_CHECK(vkCreateFramebuffer(_device, &fb_info, nullptr, &_offscreenFrameBuffer));
    }

    // Cleanup for all framebuffers
    _mainDeletionQueue.push_function([=]() {
		vkDestroyFramebuffer(_device, _offscreenFrameBuffer, nullptr);
    });
}


void VulkanEngine::init_default_renderpass()
{
	// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};
	//the attachment will have the format needed by the swapchain
	color_attachment.format = _swapchainImageFormat;
	//1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	//we don't care about stencil
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	//we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	//after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// color_attachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;

	VkAttachmentReference color_attachment_ref = {};
	//attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	    // --- Depth attachment ---
    // VkAttachmentDescription depth_attachment{};
    // depth_attachment.format = VK_FORMAT_D32_SFLOAT; // must match the format of your depth image
    // depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    // depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;   // clear depth at start of frame
    // depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // we don’t need depth after render
    // depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    // depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // VkAttachmentReference depth_attachment_ref{};
    // depth_attachment_ref.attachment = 1;
    // depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;
	// subpass.pDepthStencilAttachment = &depth_attachment_ref;

	// --- Dependencies ---
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 1> attachments = { color_attachment };

    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;


	VK_CHECK(vkCreateRenderPass(_device, &render_pass_info, nullptr, &_renderPass));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyRenderPass(_device, _renderPass, nullptr);
    });
}

void VulkanEngine::init_offscreen_render_pass() {
    // --- Color attachment ---
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_R32G32B32A32_SFLOAT;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;        // clear at start
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;      // store result
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;   // no need to preserve
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // for sampling in next pass

    // --- Depth attachment ---
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // optional, we only need it as shader read
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // --- References ---
    VkAttachmentReference colorRef{};
    colorRef.attachment = 0; // index in the attachment array
    colorRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef{};
    depthRef.attachment = 1;
    depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // --- Subpass ---
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorRef;
    subpass.pDepthStencilAttachment = &depthRef;

    // --- Render pass ---
    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_rasterRenderPass));
}


void VulkanEngine::init_framebuffers()
{
	init_render_framebuffers();
	init_offscreen_framebuffer();
}

void VulkanEngine::init_swapchain()
{
	vkb::SwapchainBuilder swapchainBuilder{_chosenGPU,_device,_surface };

	vkb::Swapchain vkbSwapchain = swapchainBuilder
		.use_default_format_selection()
		//use vsync present mode
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(_windowExtent.width, _windowExtent.height)
		.build()
		.value();

	//store swapchain and its related images
	_swapchain = vkbSwapchain.swapchain;
	_swapchainImages = vkbSwapchain.get_images().value();
	_swapchainImageViews = vkbSwapchain.get_image_views().value();

	_swapchainImageFormat = vkbSwapchain.image_format;

	_mainDeletionQueue.push_function([=]() {
		vkDestroySwapchainKHR(_device, _swapchain, nullptr);
	});

}

void VulkanEngine::init_vulkan()
{
    vkb::InstanceBuilder builder;

	//make the Vulkan instance, with basic debug features
	auto inst_ret = builder.set_app_name("Example Vulkan Application")
		.request_validation_layers(true)
		.require_api_version(1, 1, 0)
		.use_default_debug_messenger()
		.build();

	vkb::Instance vkb_inst = inst_ret.value();

	//store the instance
	_instance = vkb_inst.instance;
	//store the debug messenger
	_debug_messenger = vkb_inst.debug_messenger;

    // get the surface of the window we opened with SDL
	SDL_Vulkan_CreateSurface(_window, _instance, &_surface);

	//use vkbootstrap to select a GPU.
	//We want a GPU that can write to the SDL surface and supports Vulkan 1.1
	vkb::PhysicalDeviceSelector selector{ vkb_inst };
	vkb::PhysicalDevice physicalDevice = selector
		.set_minimum_version(1, 1)
		.set_surface(_surface)
		.select()
		.value();

	//create the final Vulkan device
	vkb::DeviceBuilder deviceBuilder{ physicalDevice };

	vkb::Device vkbDevice = deviceBuilder.build().value();

	// Get the VkDevice handle used in the rest of a Vulkan application
	_device = vkbDevice.device;
	_chosenGPU = physicalDevice.physical_device;

	// use vkbootstrap to get a Graphics queue
	_graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
	_graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::init_render_images()
{
 	_depthImage = {1, bufferSize, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, DEPTH_IMAGE};
 	_rasterColourImage = {2, bufferSize, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, COLOR_IMAGE};

    vkinit::createResource(_device, _allocator, _depthImage, {_windowExtent.width, _windowExtent.height, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 2);
    vkinit::createResource(_device, _allocator, _rasterColourImage, {_windowExtent.width, _windowExtent.height, 1}, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 2);

	vkhelp::transitionImageLayout(
		_device, _commandPool, _graphicsQueue,
		_depthImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT
	);

	vkhelp::transitionImageLayout(
		_device, _commandPool, _graphicsQueue,
		_rasterColourImage,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_COLOR_BIT
	);
}

void VulkanEngine::init_commands()
{
    //create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo commandPoolInfo = vkinit::command_pool_create_info(_graphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

	VK_CHECK(vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool));

	//allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = vkinit::command_buffer_allocate_info(_commandPool, 1);

	VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer));

	_mainDeletionQueue.push_function([=]() {
		vkDestroyCommandPool(_device, _commandPool, nullptr);
	});
}

void VulkanEngine::init_allocator()
{
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _chosenGPU;  // your VkPhysicalDevice
	allocatorInfo.device = _device;                  // your VkDevice
	allocatorInfo.instance = _instance;              // your VkInstance
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_2; // or whatever you're using

	if (vmaCreateAllocator(&allocatorInfo, &_allocator) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create VMA allocator!");
	}
}

void VulkanEngine::cleanup()
{
	if (_isInitialized) {

		//make sure the GPU has stopped doing its things
		vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);

		_mainDeletionQueue.flush();

		vkDestroyDevice(_device, nullptr);
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
		vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
		vkDestroyInstance(_instance, nullptr);
		SDL_DestroyWindow(_window);
	}
}

void VulkanEngine::compute()
{
	VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
    VK_CHECK(vkResetFences(_device, 1, &_renderFence));

	VkCommandBuffer cmd = _mainCommandBuffer;
	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;

    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	_cfd.evolve_cfd_cmd(cmd);

	vkEndCommandBuffer(cmd);

	// Submit once
	VkSubmitInfo submit{};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;
	vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence);

	// printf("Compute dispatched\n");
}

void VulkanEngine::draw()
{
	//wait until the GPU has finished rendering the last frame. Timeout of 1 second
	VK_CHECK(vkWaitForFences(_device, 1, &_renderFence, true, 1000000000));
	VK_CHECK(vkResetFences(_device, 1, &_renderFence));

	//request image from the swapchain, one second timeout
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSemaphore, nullptr, &swapchainImageIndex));

    //now that we are sure that the commands finished executing, we can safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(_mainCommandBuffer, 0));

    //naming it cmd for shorter writing
	VkCommandBuffer cmd = _mainCommandBuffer;

	//begin the command buffer recording. We will use this command buffer exactly once, so we want to let Vulkan know that
	VkCommandBufferBeginInfo cmdBeginInfo = {};
	cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBeginInfo.pNext = nullptr;

	cmdBeginInfo.pInheritanceInfo = nullptr;
	cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    //make a clear-color from frame number. This will flash with a 120*pi frame period.
	VkClearValue clearValue[2];
	clearValue[0].color = { {202.0f/255.0f, 226.0f/255.0f, 232.0f/255.0f, 1.0f} };
	clearValue[1].depthStencil = {1.0f, 0};

	//start the main renderpass.
	//We will use the clear color from above, and the framebuffer of the index the swapchain gave us
	VkRenderPassBeginInfo rasterPassInfo = {};
	rasterPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rasterPassInfo.pNext = nullptr;

	rasterPassInfo.renderPass = _rasterRenderPass;
	rasterPassInfo.renderArea.offset.x = 0;
	rasterPassInfo.renderArea.offset.y = 0;
	rasterPassInfo.renderArea.extent = _windowExtent;
	rasterPassInfo.framebuffer = _offscreenFrameBuffer;

	//connect clear values
	rasterPassInfo.clearValueCount = 2;
	rasterPassInfo.pClearValues = clearValue;


	VkRenderPassBeginInfo rayPassInfo = {};
	rayPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	rayPassInfo.pNext = nullptr;

	rayPassInfo.renderPass = _renderPass;
	rayPassInfo.renderArea.offset.x = 0;
	rayPassInfo.renderArea.offset.y = 0;
	rayPassInfo.renderArea.extent = _windowExtent;
	rayPassInfo.framebuffer = _framebuffers[swapchainImageIndex];

	//connect clear values
	rayPassInfo.clearValueCount = 1;
	rayPassInfo.pClearValues = clearValue;

	vkhelp::transitionImageLayout(_device, 
		_commandPool,
		_graphicsQueue,
		_depthImage,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);


	vkCmdBeginRenderPass(cmd, &rasterPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _terrainRender.pipeline);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		_terrainRender.pipelineLayout,
		0, // first set
		1, &_terrainRender.descriptorSet,
		0, nullptr
	);

	vkCmdPushConstants(
		cmd,
		_terrainRender.pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(CamMatrices),
		&_camMatrices
	);

	_terrainMesh.draw(cmd);

    // vkCmdDraw(cmd, 3, 1, 0, 0);

    //finalize the render pass
	vkCmdEndRenderPass(cmd);

	// Transition image layout,
	vkhelp::transitionImageBarrier(cmd,
		_depthImage,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_ASPECT_DEPTH_BIT);

	// Update kernels to take output from previous subpass


	vkCmdBeginRenderPass(cmd, &rayPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, _rp.pipeline);

	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		_rp.pipelineLayout,
		0, // first set
		1, &_rp.descriptorSet,
		0, nullptr
	);

	vkCmdPushConstants(
		cmd,
		_rp.pipelineLayout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(CamData),
		&_camData
	);

	_quadMesh.draw(cmd);


    //finalize the render pass
	vkCmdEndRenderPass(cmd);
	//finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

    //prepare the submission to the queue.
	//we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	//we will signal the _renderSemaphore, to signal that rendering has finished

	VkSubmitInfo submit = {};
	submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;
	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores = &_presentSemaphore;
	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores = &_renderSemaphore;
	submit.commandBufferCount = 1;
	submit.pCommandBuffers = &cmd;

	//submit command buffer to the queue and execute it.
	// _renderFence will now block until the graphic commands finish execution
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence));

    // this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &_renderSemaphore;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(_graphicsQueue, &presentInfo));

	//increase the number of frames drawn
	_frameNumber++;
}

void VulkanEngine::run()
{
	SDL_Event e;
	bool bQuit = false;

	//main loop
	while (!bQuit)
	{
		update_camera(0.016f); // assuming ~60 FPS, so about 16ms per frame
        compute();
		draw();
	}
}

void VulkanEngine::update_camera(float dt) {
    static float yaw   = 60.0f;
    static float pitch = 30.0f;

    const float cameraSpeed = 1.0f * dt;
    const float sensitivity = 0.1f;

    // Wrap _camData.pos in glm::vec3
    glm::vec3 position = glm::make_vec3(_camData.pos);

    // Compute front from yaw + pitch
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front   = glm::normalize(front);

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 right   = glm::normalize(glm::cross(front, worldUp));
    glm::vec3 up      = glm::normalize(glm::cross(right, front));

    // Constrain movement direction to horizontal plane (XZ)
    glm::vec3 horizontalFront = glm::normalize(glm::vec3(front.x, 0.0f, front.z));
    glm::vec3 horizontalRight = glm::normalize(glm::cross(horizontalFront, worldUp));

    // Keyboard movement
	if (mouseCaptured) {
	
    const Uint8* keystate = SDL_GetKeyboardState(NULL);
		if (keystate[SDL_SCANCODE_W]) position += cameraSpeed * horizontalFront;
		if (keystate[SDL_SCANCODE_S]) position -= cameraSpeed * horizontalFront;
		if (keystate[SDL_SCANCODE_A]) position -= cameraSpeed * horizontalRight;
		if (keystate[SDL_SCANCODE_D]) position += cameraSpeed * horizontalRight;

		// Vertical movement
		if (keystate[SDL_SCANCODE_SPACE])     position -= cameraSpeed * worldUp;
		if (keystate[SDL_SCANCODE_LSHIFT])    position += cameraSpeed * worldUp;
	}

    // Mouse motion
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            exit(0);
        }
        if (event.type == SDL_MOUSEBUTTONDOWN && !mouseCaptured) {
            // First click → capture the mouse
            SDL_ShowCursor(SDL_DISABLE);         // hide cursor
            SDL_SetRelativeMouseMode(SDL_TRUE);  // enable relative mouse
            mouseCaptured = true;
        } 
        else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE && mouseCaptured) {
            // ESC key → release the mouse
            SDL_ShowCursor(SDL_ENABLE);          // show cursor
            SDL_SetRelativeMouseMode(SDL_FALSE); // disable relative mouse
            mouseCaptured = false;
        }
        if (event.type == SDL_MOUSEMOTION && mouseCaptured) {
            float xoffset = event.motion.xrel * sensitivity;
            float yoffset = event.motion.yrel * sensitivity; // invert Y

            yaw   += xoffset;
            pitch += yoffset;

            if (pitch > 89.0f)  pitch = 89.0f;
            if (pitch < -89.0f) pitch = -89.0f;
        }
    }

    // Update CamData
    glm::vec3 lookAt = position + front;

    memcpy(_camData.pos,    glm::value_ptr(position), sizeof(float) * 3);
    memcpy(_camData.camUp,  glm::value_ptr(up),       sizeof(float) * 3);
    memcpy(_camData.lookAt, glm::value_ptr(lookAt),   sizeof(float) * 3);

	_camMatrices = ConvertToMatrices(_camData);
}

void VulkanEngine::init_terrain_rendering()
{

	VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(CamMatrices);
	std::vector<VkPushConstantRange> pushConstants = { pushConstantRange };

	// If you want to use a subset of the resources and bindings, you can do so like this:
	std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};

	_terrainRender = vkinit::initKernel(_device, KernelType::Graphics, 
		{"build/shaders/model.vert.spv", "build/shaders/model.frag.spv"}, 
		layoutBindings, pushConstants, _rasterRenderPass, _windowExtent);
	
	printf("Render kernel initialized\n");
}

void VulkanEngine::load_terrain_model(const std::string &filename)
{
	float terrainScale = 0.7;
	_cfd.load_terrain(_commandPool, _graphicsQueue, filename.c_str(), terrainScale);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	MeshGen::generateMesh(vertices, indices, filename, _res, terrainScale);

	_terrainMesh.init(_device, _commandPool, _graphicsQueue, _allocator, vertices, indices);

	printf("Terrain Loaded \n");
}
