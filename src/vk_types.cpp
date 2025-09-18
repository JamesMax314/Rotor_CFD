#include "vk_types.h"

CamMatrices ConvertToMatrices(
    const CamData& cam,
    float fovY,
    float aspect,
    float nearPlane,
    float farPlane)
{
    glm::vec3 eye(cam.pos[0], cam.pos[1], cam.pos[2]);
    glm::vec3 center(cam.lookAt[0], cam.lookAt[1], cam.lookAt[2]);
    glm::vec3 up(cam.camUp[0], cam.camUp[1], cam.camUp[2]);

    glm::mat4 view = glm::lookAt(eye, center, up);
    // glm::mat4 proj = glm::perspective(fovY, aspect, nearPlane, farPlane);
    glm::mat4 proj = glm::perspectiveRH_ZO(fovY, aspect, nearPlane, farPlane);

    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for model

    // GLM’s perspective matrix is OpenGL-style (depth range -1..1).  
    // If using Vulkan/DirectX, apply correction if needed.
    // For Vulkan:
    proj[1][1] *= 1.0f;

    CamMatrices result;
    result.mvp = proj * view * model;
    return result;
}

VkPipeline PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass) {
    //make viewport state from our stored viewport and scissor.
    //at the moment we won't support multiple viewports or scissors
    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;

    viewportState.viewportCount = 1;
    viewportState.pViewports = &_viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &_scissor;

    //setup dummy color blending. We aren't using transparent objects yet
    //the blending is just "no blend", but we do write to the color attachment
    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;

    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &_colorBlendAttachment;

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS; // or LESS_OR_EQUAL
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;

    //build the actual pipeline
	//we now use all of the info structs we have been writing into into this one to create the pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;

	pipelineInfo.stageCount = _shaderStages.size();
	pipelineInfo.pStages = _shaderStages.data();
	pipelineInfo.pVertexInputState = &_vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &_inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &_rasterizer;
	pipelineInfo.pMultisampleState = &_multisampling;
	pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = pass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	//it's easy to error out on create graphics pipeline, so we handle it a bit better than the common VK_CHECK case
	VkPipeline newPipeline;
	if (vkCreateGraphicsPipelines(
		device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &newPipeline) != VK_SUCCESS) {
		std::cout << "failed to create pipeline\n";
		return VK_NULL_HANDLE; // failed to create graphics pipeline
	}
	else
	{
		return newPipeline;
	}
}