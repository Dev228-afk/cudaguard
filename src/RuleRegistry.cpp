#include "cudaguard/RuleRegistry.h"
#include "cudaguard/Diagnostics.h"
#include "cudaguard/rules/MissingCudaErrorCheckRule.h"
#include "cudaguard/rules/KernelLaunchRule.h"
#include "cudaguard/rules/HostDeviceCallRule.h"
#include "cudaguard/rules/SharedMemoryRule.h"
#include "cudaguard/rules/CudaMallocMemcpyRule.h"

namespace cudaguard {

RuleRegistry::RuleRegistry(DiagnosticReporter& reporter)
    : reporter_(reporter) {}

void RuleRegistry::registerAllRules() {
    rules_.push_back(std::make_unique<MissingCudaErrorCheckRule>(reporter_));
    rules_.push_back(std::make_unique<KernelLaunchRule>(reporter_));
    rules_.push_back(std::make_unique<HostDeviceCallRule>(reporter_));
    rules_.push_back(std::make_unique<SharedMemoryRule>(reporter_));
    rules_.push_back(std::make_unique<CudaMallocMemcpyRule>(reporter_));
}

void RuleRegistry::enableOnly(const std::unordered_set<std::string>& ruleIds) {
    for (const auto& rule : rules_) {
        if (ruleIds.find(rule->id()) == ruleIds.end()) {
            disabledRules_.insert(rule->id());
        }
    }
}

void RuleRegistry::disableRules(const std::unordered_set<std::string>& ruleIds) {
    disabledRules_.insert(ruleIds.begin(), ruleIds.end());
}

void RuleRegistry::registerMatchers(clang::ast_matchers::MatchFinder& finder) {
    for (auto& rule : rules_) {
        if (disabledRules_.find(rule->id()) == disabledRules_.end()) {
            rule->registerMatchers(finder);
        }
    }
}

const std::vector<std::unique_ptr<Rule>>& RuleRegistry::rules() const {
    return rules_;
}

std::vector<std::string> RuleRegistry::enabledRuleIds() const {
    std::vector<std::string> ids;
    for (const auto& rule : rules_) {
        if (disabledRules_.find(rule->id()) == disabledRules_.end()) {
            ids.push_back(rule->id());
        }
    }
    return ids;
}

} // namespace cudaguard
