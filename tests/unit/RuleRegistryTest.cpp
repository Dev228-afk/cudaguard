#include <gtest/gtest.h>
#include "cudaguard/Diagnostics.h"
#include "cudaguard/RuleRegistry.h"

using namespace cudaguard;

TEST(RuleRegistryTest, RegisterAllRules) {
    DiagnosticReporter reporter;
    RuleRegistry registry(reporter);
    registry.registerAllRules();

    EXPECT_EQ(registry.rules().size(), 5u);
}

TEST(RuleRegistryTest, EnabledRuleIds) {
    DiagnosticReporter reporter;
    RuleRegistry registry(reporter);
    registry.registerAllRules();

    auto ids = registry.enabledRuleIds();
    EXPECT_EQ(ids.size(), 5u);

    bool hasCG001 = false, hasCG002 = false, hasCG003 = false;
    bool hasCG004 = false, hasCG005 = false;
    for (const auto& id : ids) {
        if (id == "CG001") hasCG001 = true;
        if (id == "CG002") hasCG002 = true;
        if (id == "CG003") hasCG003 = true;
        if (id == "CG004") hasCG004 = true;
        if (id == "CG005") hasCG005 = true;
    }
    EXPECT_TRUE(hasCG001);
    EXPECT_TRUE(hasCG002);
    EXPECT_TRUE(hasCG003);
    EXPECT_TRUE(hasCG004);
    EXPECT_TRUE(hasCG005);
}

TEST(RuleRegistryTest, EnableOnly) {
    DiagnosticReporter reporter;
    RuleRegistry registry(reporter);
    registry.registerAllRules();
    registry.enableOnly({"CG001", "CG003"});

    auto ids = registry.enabledRuleIds();
    EXPECT_EQ(ids.size(), 2u);

    bool hasCG001 = false, hasCG003 = false;
    for (const auto& id : ids) {
        if (id == "CG001") hasCG001 = true;
        if (id == "CG003") hasCG003 = true;
    }
    EXPECT_TRUE(hasCG001);
    EXPECT_TRUE(hasCG003);
}

TEST(RuleRegistryTest, DisableRules) {
    DiagnosticReporter reporter;
    RuleRegistry registry(reporter);
    registry.registerAllRules();
    registry.disableRules({"CG002", "CG005"});

    auto ids = registry.enabledRuleIds();
    EXPECT_EQ(ids.size(), 3u);

    for (const auto& id : ids) {
        EXPECT_NE(id, "CG002");
        EXPECT_NE(id, "CG005");
    }
}

TEST(RuleRegistryTest, RuleProperties) {
    DiagnosticReporter reporter;
    RuleRegistry registry(reporter);
    registry.registerAllRules();

    for (const auto& rule : registry.rules()) {
        EXPECT_FALSE(rule->id().empty());
        EXPECT_FALSE(rule->name().empty());
        EXPECT_FALSE(rule->description().empty());
    }
}
