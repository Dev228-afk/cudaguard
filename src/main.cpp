#include <iostream>
#include <string>

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Tooling/Tooling.h"
#include "clang/Tooling/CompilationDatabase.h"

#include "cudaguard/BuildWrapper.h"
#include "cudaguard/CompileDatabaseLoader.h"
#include "cudaguard/Diagnostics.h"
#include "cudaguard/RuleRegistry.h"
#include "cudaguard/ToolConfig.h"

using namespace cudaguard;

static void printHelp() {
    std::cout << "CudaGuard - CUDA C++ Static Analysis & Build Diagnostics Tool\n"
              << "Version 1.0.0\n\n"
              << "Usage:\n"
              << "  cudaguard --file <source.cu> [-- <extra-clang-args>]\n"
              << "  cudaguard --compile-db <compile_commands.json>\n"
              << "  cudaguard --wrap-nvcc -- <nvcc-command>\n\n"
              << "Options:\n"
              << "  --file <path>          Analyze a single CUDA source file\n"
              << "  --compile-db <path>    Load compilation database (compile_commands.json)\n"
              << "  --enable <rules>       Enable only specified rules (comma-separated, e.g. CG001,CG003)\n"
              << "  --disable <rules>      Disable specified rules (comma-separated)\n"
              << "  --warnings-as-errors   Treat all warnings as errors\n"
              << "  --json                 Output diagnostics in JSON format\n"
              << "  --wrap-nvcc            Wrap nvcc: run checks before invoking compiler\n"
              << "  --help, -h             Show this help message\n"
              << "  --version              Show version information\n\n"
              << "Supported Rules:\n"
              << "  CG001  Kernel launch not followed by CUDA error check\n"
              << "  CG002  Suspicious kernel launch configuration (zero/oversized dimensions)\n"
              << "  CG003  __device__ function calls host-only function\n"
              << "  CG004  Kernel uses extern __shared__ but launch omits shared-memory size\n"
              << "  CG005  cudaMemcpy direction mismatch (heuristic)\n\n"
              << "Examples:\n"
              << "  cudaguard --file kernel.cu -- -x cuda --cuda-gpu-arch=sm_75\n"
              << "  cudaguard --compile-db build/compile_commands.json --enable CG001,CG003\n"
              << "  cudaguard --wrap-nvcc -- nvcc -arch=sm_75 kernel.cu -o kernel\n";
}

static void printVersion() {
    std::cout << "CudaGuard version 1.0.0\n"
              << "Built with LLVM/Clang LibTooling\n";
}

static int runAnalysis(const ToolConfig& config) {
    DiagnosticReporter reporter;
    reporter.setJsonOutput(config.jsonOutput);
    reporter.setWarningsAsErrors(config.warningsAsErrors);

    RuleRegistry registry(reporter);
    registry.registerAllRules();

    if (!config.enabledRules.empty()) {
        registry.enableOnly(config.enabledRules);
    }
    if (!config.disabledRules.empty()) {
        registry.disableRules(config.disabledRules);
    }

    clang::ast_matchers::MatchFinder finder;
    registry.registerMatchers(finder);

    std::unique_ptr<clang::tooling::CompilationDatabase> db;
    std::vector<std::string> sourceFiles;
    std::string errorMsg;

    if (config.compileDatabasePath.has_value()) {
        db = CompileDatabaseLoader::loadFromFile(
            config.compileDatabasePath.value(), errorMsg);
        if (!db) {
            std::cerr << "Error: failed to load compilation database: "
                      << errorMsg << "\n";
            return 1;
        }
        sourceFiles = CompileDatabaseLoader::getSourceFiles(*db);
    } else if (config.singleFile.has_value()) {
        std::vector<std::string> args = config.extraArgs;
        if (args.empty()) {
            args.push_back("-x");
            args.push_back("cuda");
            args.push_back("--cuda-gpu-arch=sm_75");
        }
        db = CompileDatabaseLoader::createFromArgs(
            config.singleFile.value(), args, errorMsg);
        if (!db) {
            std::cerr << "Error: failed to create compilation database: "
                      << errorMsg << "\n";
            return 1;
        }
        sourceFiles.push_back(config.singleFile.value());
    } else {
        std::cerr << "Error: specify --file or --compile-db\n";
        return 1;
    }

    clang::tooling::ClangTool tool(*db, sourceFiles);
    int toolResult = tool.run(
        clang::tooling::newFrontendActionFactory(&finder).get());

    if (toolResult != 0 && reporter.diagnostics().empty()) {
        std::cerr << "Error: Clang failed to parse source files. "
                  << "Check that CUDA headers are available and compilation flags are correct.\n";
        return 1;
    }

    std::cout << "CudaGuard analyzed " << sourceFiles.size() << " file(s)\n";

    if (config.jsonOutput) {
        reporter.printJson();
    } else {
        reporter.printHumanReadable();
    }

    reporter.printSummary();

    return reporter.errorCount() > 0 ? 1 : 0;
}

static int runWrapperMode(const ToolConfig& config) {
    if (config.nvccCommand.empty()) {
        std::cerr << "Error: --wrap-nvcc requires a compiler command after --\n";
        return 1;
    }

    auto cudaFiles = BuildWrapper::extractCudaFiles(config.nvccCommand);
    if (cudaFiles.empty()) {
        std::cerr << "Warning: no .cu files found in nvcc command, "
                  << "invoking compiler directly\n";
        auto result = BuildWrapper::invokeCompiler(config.nvccCommand);
        return result.exitCode;
    }

    ToolConfig analysisConfig = config;
    analysisConfig.wrapNvcc = false;
    analysisConfig.singleFile = cudaFiles[0];
    analysisConfig.extraArgs = {"-x", "cuda", "--cuda-gpu-arch=sm_75"};

    std::cout << "CudaGuard: running pre-compilation checks on "
              << cudaFiles.size() << " file(s)...\n";

    DiagnosticReporter reporter;
    reporter.setJsonOutput(config.jsonOutput);
    reporter.setWarningsAsErrors(config.warningsAsErrors);

    RuleRegistry registry(reporter);
    registry.registerAllRules();

    if (!config.enabledRules.empty()) {
        registry.enableOnly(config.enabledRules);
    }
    if (!config.disabledRules.empty()) {
        registry.disableRules(config.disabledRules);
    }

    clang::ast_matchers::MatchFinder finder;
    registry.registerMatchers(finder);

    std::string errorMsg;
    auto db = CompileDatabaseLoader::createFromArgs(
        cudaFiles[0], analysisConfig.extraArgs, errorMsg);

    if (db) {
        clang::tooling::ClangTool tool(*db, cudaFiles);
        tool.run(clang::tooling::newFrontendActionFactory(&finder).get());
    }

    if (config.jsonOutput) {
        reporter.printJson();
    } else {
        reporter.printHumanReadable();
    }

    if (reporter.errorCount() > 0) {
        std::cerr << "CudaGuard: " << reporter.errorCount()
                  << " error(s) found. Compilation aborted.\n";
        reporter.printSummary();
        return 1;
    }

    if (reporter.warningCount() > 0) {
        reporter.printSummary();
    }

    std::cout << "CudaGuard: invoking compiler...\n";
    auto result = BuildWrapper::invokeCompiler(config.nvccCommand);

    if (!result.stdoutOutput.empty()) {
        std::cout << result.stdoutOutput;
    }

    std::string artifactDir = ".cudaguard/artifacts";
    auto logPath = BuildWrapper::saveArtifactLog(result, artifactDir);
    std::cout << "CudaGuard: build log saved to " << logPath << "\n";

    return result.exitCode;
}

int main(int argc, const char* argv[]) {
    ToolConfig config = parseCommandLine(argc, argv);

    if (config.showHelp) {
        printHelp();
        return 0;
    }

    if (config.showVersion) {
        printVersion();
        return 0;
    }

    if (config.wrapNvcc) {
        return runWrapperMode(config);
    }

    return runAnalysis(config);
}
