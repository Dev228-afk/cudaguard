#include "cudaguard/ToolConfig.h"

#include <algorithm>
#include <iostream>
#include <sstream>

namespace cudaguard {

namespace {

std::unordered_set<std::string> parseCommaSeparated(const std::string& input) {
    std::unordered_set<std::string> result;
    std::istringstream stream(input);
    std::string token;
    while (std::getline(stream, token, ',')) {
        if (!token.empty()) {
            result.insert(token);
        }
    }
    return result;
}

} // anonymous namespace

ToolConfig parseCommandLine(int argc, const char* argv[]) {
    ToolConfig config;
    bool afterDoubleDash = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "--") {
            afterDoubleDash = true;
            if (config.wrapNvcc) {
                for (int j = i + 1; j < argc; ++j) {
                    config.nvccCommand.emplace_back(argv[j]);
                }
                break;
            }
            continue;
        }

        if (afterDoubleDash && !config.wrapNvcc) {
            config.extraArgs.push_back(arg);
            continue;
        }

        if (arg == "--help" || arg == "-h") {
            config.showHelp = true;
        } else if (arg == "--version") {
            config.showVersion = true;
        } else if (arg == "--file" && i + 1 < argc) {
            config.singleFile = argv[++i];
        } else if (arg == "--compile-db" && i + 1 < argc) {
            config.compileDatabasePath = argv[++i];
        } else if (arg == "--enable" && i + 1 < argc) {
            config.enabledRules = parseCommaSeparated(argv[++i]);
        } else if (arg == "--disable" && i + 1 < argc) {
            config.disabledRules = parseCommaSeparated(argv[++i]);
        } else if (arg == "--warnings-as-errors") {
            config.warningsAsErrors = true;
        } else if (arg == "--json") {
            config.jsonOutput = true;
        } else if (arg == "--wrap-nvcc") {
            config.wrapNvcc = true;
        }
    }

    return config;
}

} // namespace cudaguard
