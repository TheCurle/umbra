#include <vlkx/render/framebuffer/RenderPass.h>
#include <vlkx/vulkan/VulkanModule.h>

RenderPass::RenderPass() {}
RenderPass::~RenderPass() {}


void RenderPass::createVertexRenderPass(VkFormat format) {
	// Set up color metadata
	VkAttachmentDescription color = {};
	color.format = format;
	color.samples = VK_SAMPLE_COUNT_1_BIT;
	color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Set up subattachments
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;

	// Prepare the Render Pass for creation
    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 1> attachments = { color };
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &dependency;
	createInfo.pSubpasses = &subpass;

	// Create the Render Pass
	if (vkCreateRenderPass(VulkanModule::getInstance()->getDevice()->logical, &createInfo, nullptr, &pass))
		throw std::runtime_error("Unable to create Render Pass 1");
}

void RenderPass::beginRenderPass(std::vector<VkClearValue> clearValues, VkCommandBuffer commands, VkFramebuffer framebuffer, VkExtent2D extent) {
	// Prepare the Render Pass for start
	VkRenderPassBeginInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	info.renderPass = pass;
	info.framebuffer = framebuffer;

	info.renderArea.offset = { 0, 0 };
	info.renderArea.extent = extent;

	info.pClearValues = clearValues.data();
	info.clearValueCount = static_cast<uint32_t>(clearValues.size());

	// Attempt to begin the pass
	vkCmdBeginRenderPass(commands, &info, VK_SUBPASS_CONTENTS_INLINE);
}

void RenderPass::endRenderPass(VkCommandBuffer commands) {
	vkCmdEndRenderPass(commands);
}


void RenderPass::destroy() {
	vkDestroyRenderPass(VulkanModule::getInstance()->getDevice()->logical, pass, nullptr);
}