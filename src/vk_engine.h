#pragma once

#include <iostream>
#include <fstream>

//bootstrap library
#include "VkBootstrap.h"

#include "vk_types.h"

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
using namespace std;

struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call the function
		}

		deletors.clear();
	}
};

class VulkanEngine {
public:

    unsigned int _res = 129;
	bool _isInitialized{ false };
	int _frameNumber {0};

	VkExtent2D _windowExtent{ 1700 , 900 };

    struct SDL_Window* _window{ nullptr };
	
	//initializes everything in the engine
	void init();

    //shuts down the engine
	void cleanup();

	//draw loop
	void draw();

    //Compute loop
    void compute();

	//run main loop
	void run();

    VkInstance _instance; // Vulkan library handle
	VkDebugUtilsMessengerEXT _debug_messenger; // Vulkan debug output handle
	VkPhysicalDevice _chosenGPU; // GPU chosen as the default device
	VkDevice _device; // Vulkan device for commands
	VkSurfaceKHR _surface; // Vulkan window surface

    VkSwapchainKHR _swapchain; // from other articles

	// image format expected by the windowing system
	VkFormat _swapchainImageFormat;

	//array of images from the swapchain
	std::vector<VkImage> _swapchainImages;

	//array of image-views from the swapchain
	std::vector<VkImageView> _swapchainImageViews;

    VkQueue _graphicsQueue; //queue we will submit to
    uint32_t _graphicsQueueFamily; //family of that queue

    VkCommandPool _commandPool; //the command pool for our commands
    VkCommandBuffer _mainCommandBuffer; //the buffer we will record into

    VkRenderPass _renderPass;

	std::vector<VkFramebuffer> _framebuffers;

    VkSemaphore _presentSemaphore, _renderSemaphore;
	VkFence _renderFence;
	std::vector<VkDescriptorSetLayout> _renderDescriptorSetLayouts;

    VkPipelineLayout _trianglePipelineLayout;

    VkPipeline _trianglePipeline;

    DeletionQueue _mainDeletionQueue;

    VmaAllocator _allocator;

    VkImage _3DTexture;
    VkImageView _3DTextureView;
    VkDeviceMemory _3DTextureMemory;

    Kernel _gaussSidel{};
    Kernel _advect{};
    Kernel _advectSwapped{};
	Kernel _writeTexture{};
	Kernel _writeTextureSwapped{};
	Kernel _rp{};

    VkDeviceSize bufferSize = _res * _res * _res * sizeof(float);
    std::vector<ResourceBinding> _resourceBindings = {
        {0, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}, {1, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
		{2, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}, {3, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
		{4, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}, {5, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
		{6, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}, {7, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
		{8, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}, {9, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {10, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}, {11, bufferSize, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE}
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

	std::vector<Vertex> _quadVertices = {
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}}, // bottom left
		{{ 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f}}, // bottom right
		{{ 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f}}, // top right
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f}}, // top left
	};

	std::vector<uint16_t> _quadIndices = { 0, 1, 2, 2, 3, 0 };

	Mesh _quadMesh;

private:

	void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_default_renderpass();
	void init_framebuffers();
    void init_sync_structures();
    void init_pipelines();
    bool load_shader_module(const char* filePath, VkShaderModule* outShaderModule);
    void init_3D_texture();
    void initKernels();
    void initSSBOs();
};