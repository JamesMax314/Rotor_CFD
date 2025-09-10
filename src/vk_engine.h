#pragma once

#include <iostream>
#include <fstream>

//bootstrap library
#include "VkBootstrap.h"

#include "vk_types.h"

//we want to immediately abort when there is an error. In normal engines this would give an error message to the user, or perform a dump of state.
using namespace std;
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

    unsigned int _res = 128;
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

    Kernel _cp{};

    VkDeviceSize bufferSize = _res * _res * _res * sizeof(float);
    std::vector<BufferBinding> _advectBuffers = {
        {0, bufferSize}, {1, bufferSize}, {2, bufferSize}, {3, bufferSize}, {4, bufferSize},
        {5, bufferSize}, {6, bufferSize}, {7, bufferSize}, {8, bufferSize}, {9, bufferSize},
        {10, bufferSize}
    };

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
    void initComputeKernels();
    void initSSBOs();
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