#include <vlkx/render/render_pass/GPUPass.h>

namespace vlkx {

    inline bool needsSynchronization(const ImageUsage& prev, const ImageUsage& curr) {
        if (curr == prev && curr.getAccess() == ImageUsage::Access::ReadOnly)
            return false;
        return true;
    }

    void CommonPass::addUsage(std::string &&name, UsageTracker &&tracker) {
        for (const auto& pair : tracker.getUsageMap())
            validate(pair.first, name, false);

        tracker.add(getVirtualInitial(), tracker.getInitialUsage());
        if (tracker.getFinalUsage().has_value())
            tracker.add(getVirtualFinal(), tracker.getFinalUsage().value());

        usageHistory.emplace(std::move(name), std::move(tracker));
    }

    VkImageLayout CommonPass::getInitialLayout(const std::string &name) const {
        return getHistory(name).getUsageMap().begin()->second.getLayout();
    }

    VkImageLayout CommonPass::getFinalLayout(const std::string &name) const {
        return getHistory(name).getUsageMap().rbegin()->second.getLayout();
    }

    VkImageLayout CommonPass::getSubpassLayout(const std::string &name, int subpass) const {
        validate(subpass, name, false);
        return getUsage(name, subpass)->getLayout();
    }

    void CommonPass::update(const std::string &name, MultiImageTracker &tracker) const {
        tracker.update(name, getHistory(name).getUsageMap().rbegin()->second);
    }

    const UsageTracker& CommonPass::getHistory(const std::string &name) const {
        return usageHistory.at(name);
    }

    const ImageUsage* CommonPass::getUsage(const std::string &name, int pass) const {
        validate(pass, name, true);
        const UsageTracker& history = getHistory(name);
        const auto iter = history.getUsageMap().find(pass);
        return iter != history.getUsageMap().end() ? &iter->second : nullptr;
    }

    std::optional<CommonPass::Usages> CommonPass::checkForSync(const std::string &name, int pass) const {
        validate(pass, name, true);
        const UsageTracker& history = getHistory(name);
        const auto currIter = history.getUsageMap().find(pass);
        if (currIter == history.getUsageMap().end())
            return std::nullopt;
        const auto prevIter = std::prev(currIter);

        const ImageUsage& prevUsage = prevIter->second;
        const ImageUsage& currUsage = currIter->second;

        if (!needsSynchronization(prevUsage, currUsage))
            return std::nullopt;

        const int prevSubpass = prevIter->first;
        return CommonPass::Usages { prevSubpass, &prevUsage, &currUsage };
    }

    void CommonPass::validate(int pass, const std::string &image, bool includeVirtual) const {
        if (includeVirtual) {
            if (!(pass >= getVirtualInitial() && pass <= getVirtualFinal()))
                throw std::runtime_error("Subpass out of range.");
        } else {
            if (!(pass >= 0 && pass < numPasses))
                throw std::runtime_error("nv Subpass out of range.");
        }
    }

}