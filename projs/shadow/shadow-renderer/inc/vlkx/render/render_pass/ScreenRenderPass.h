#pragma once

#include <optional>
#include <memory>
#include "vlkx/vulkan/Tools.h"
#include "GenericRenderPass.h"
#include "vlkx/vulkan/abstraction/ImageUsage.h"

namespace vlkx {

    /**
     * Internally-used metadata to track attachment images by name and position.
     * This behaviour is duplicated elsewhere.
     */
    struct Attachment {
        explicit Attachment(std::string_view image) : name(image) {}

        // Adds the image to the tracker and initializes state.
        void add(MultiImageTracker& tracker, const VkTools::VlkxImage& image);

        const std::string name;
        std::optional<int> index;
    };

    // A simple and versatile way to configure render passes.
    // Intended to be used with the SimpleRenderPass and the ScreenRenderPass.
    class RendererConfig {
    public:
        RendererConfig() {
            firstOpaquePass = 1;
        }

        RendererConfig(int passCount, std::optional<int> firstTransparent = std::nullopt, std::optional<int> firstOverlay = std::nullopt);

        RendererConfig(int passCount);

        // Get the number of passes that use the depth buffer.
        int depthPasses() const {
            return firstOpaquePass + firstTransparentPass;
        }

        // Get the total number of passes.
        int passes() const {
            return depthPasses() + firstOverlayPass;
        }

        // Get whether any passes use the depth buffer.
        bool usesDepth() const {
            return depthPasses() > 0;
        }

        // Create the render pass builder. Can be called multiple times.
        void build();

        RendererConfig(RendererConfig&) noexcept = default;
        RendererConfig(const RendererConfig&) = default;

    private:
        Attachment swapchainImage { "Swapchain" };
        Attachment multisampleImage { "Multisample" };
        Attachment stencilImage { "Depth-Stencil" };

        int firstOpaquePass = 0;
        int firstTransparentPass = 0;
        int firstOverlayPass = 0;
    };

    // Manages Render Passes that will output to the screen.
    // This is necessarily exclusively a graphical pass.
    // If necessary, a depth and stencil buffer will be maintained.
    // The color buffer is automatically assumed to be the swapchain.
    class ScreenRenderPassManager {
    public:
        explicit ScreenRenderPassManager(RendererConfig renderConfig) : config(renderConfig) {}

        ScreenRenderPassManager(const ScreenRenderPassManager&) = delete;
        ScreenRenderPassManager& operator=(const ScreenRenderPassManager&) = delete;

        // Initialize the render pass we're managing.
        void initializeRenderPass();

    private:

        // Prepare the Render Pass builder.
        void preparePassBuilder();

        const RendererConfig config;
        std::unique_ptr<VkTools::VlkxImage> depthStencilImage;
        std::unique_ptr<vlkx::RenderPassBuilder> passBuilder;
        std::unique_ptr<vlkx::RenderPass> pass;
    };
}