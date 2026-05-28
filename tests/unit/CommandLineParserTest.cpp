#include <gtest/gtest.h>
#include "cudaguard/ToolConfig.h"

using namespace cudaguard;

TEST(CommandLineParserTest, ShowHelp) {
    const char* argv[] = {"cudaguard", "--help"};
    auto config = parseCommandLine(2, argv);
    EXPECT_TRUE(config.showHelp);
}

TEST(CommandLineParserTest, ShowHelpShort) {
    const char* argv[] = {"cudaguard", "-h"};
    auto config = parseCommandLine(2, argv);
    EXPECT_TRUE(config.showHelp);
}

TEST(CommandLineParserTest, ShowVersion) {
    const char* argv[] = {"cudaguard", "--version"};
    auto config = parseCommandLine(2, argv);
    EXPECT_TRUE(config.showVersion);
}

TEST(CommandLineParserTest, SingleFile) {
    const char* argv[] = {"cudaguard", "--file", "test.cu"};
    auto config = parseCommandLine(3, argv);
    ASSERT_TRUE(config.singleFile.has_value());
    EXPECT_EQ(config.singleFile.value(), "test.cu");
}

TEST(CommandLineParserTest, CompileDb) {
    const char* argv[] = {"cudaguard", "--compile-db", "build/compile_commands.json"};
    auto config = parseCommandLine(3, argv);
    ASSERT_TRUE(config.compileDatabasePath.has_value());
    EXPECT_EQ(config.compileDatabasePath.value(), "build/compile_commands.json");
}

TEST(CommandLineParserTest, EnableRules) {
    const char* argv[] = {"cudaguard", "--file", "test.cu", "--enable", "CG001,CG003"};
    auto config = parseCommandLine(5, argv);
    EXPECT_EQ(config.enabledRules.size(), 2u);
    EXPECT_TRUE(config.enabledRules.count("CG001"));
    EXPECT_TRUE(config.enabledRules.count("CG003"));
}

TEST(CommandLineParserTest, DisableRules) {
    const char* argv[] = {"cudaguard", "--file", "test.cu", "--disable", "CG002"};
    auto config = parseCommandLine(5, argv);
    EXPECT_EQ(config.disabledRules.size(), 1u);
    EXPECT_TRUE(config.disabledRules.count("CG002"));
}

TEST(CommandLineParserTest, WarningsAsErrors) {
    const char* argv[] = {"cudaguard", "--file", "test.cu", "--warnings-as-errors"};
    auto config = parseCommandLine(4, argv);
    EXPECT_TRUE(config.warningsAsErrors);
}

TEST(CommandLineParserTest, JsonOutput) {
    const char* argv[] = {"cudaguard", "--json", "--file", "test.cu"};
    auto config = parseCommandLine(4, argv);
    EXPECT_TRUE(config.jsonOutput);
}

TEST(CommandLineParserTest, WrapNvcc) {
    const char* argv[] = {"cudaguard", "--wrap-nvcc", "--",
                          "nvcc", "-arch=sm_75", "kernel.cu", "-o", "kernel"};
    auto config = parseCommandLine(8, argv);
    EXPECT_TRUE(config.wrapNvcc);
    ASSERT_EQ(config.nvccCommand.size(), 5u);
    EXPECT_EQ(config.nvccCommand[0], "nvcc");
    EXPECT_EQ(config.nvccCommand[1], "-arch=sm_75");
    EXPECT_EQ(config.nvccCommand[2], "kernel.cu");
    EXPECT_EQ(config.nvccCommand[3], "-o");
    EXPECT_EQ(config.nvccCommand[4], "kernel");
}

TEST(CommandLineParserTest, ExtraArgs) {
    const char* argv[] = {"cudaguard", "--file", "test.cu", "--",
                          "-x", "cuda", "--cuda-gpu-arch=sm_75"};
    auto config = parseCommandLine(7, argv);
    ASSERT_TRUE(config.singleFile.has_value());
    EXPECT_EQ(config.extraArgs.size(), 3u);
    EXPECT_EQ(config.extraArgs[0], "-x");
    EXPECT_EQ(config.extraArgs[1], "cuda");
    EXPECT_EQ(config.extraArgs[2], "--cuda-gpu-arch=sm_75");
}

TEST(CommandLineParserTest, DefaultValues) {
    const char* argv[] = {"cudaguard"};
    auto config = parseCommandLine(1, argv);
    EXPECT_FALSE(config.showHelp);
    EXPECT_FALSE(config.showVersion);
    EXPECT_FALSE(config.singleFile.has_value());
    EXPECT_FALSE(config.compileDatabasePath.has_value());
    EXPECT_FALSE(config.warningsAsErrors);
    EXPECT_FALSE(config.jsonOutput);
    EXPECT_FALSE(config.wrapNvcc);
    EXPECT_TRUE(config.enabledRules.empty());
    EXPECT_TRUE(config.disabledRules.empty());
    EXPECT_TRUE(config.extraArgs.empty());
    EXPECT_TRUE(config.nvccCommand.empty());
}
