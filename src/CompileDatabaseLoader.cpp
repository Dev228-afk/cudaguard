#include "cudaguard/CompileDatabaseLoader.h"

#include "clang/Tooling/JSONCompilationDatabase.h"

#include <filesystem>

namespace cudaguard {

std::unique_ptr<clang::tooling::CompilationDatabase>
CompileDatabaseLoader::loadFromFile(const std::string& path, std::string& errorMsg) {
    return clang::tooling::JSONCompilationDatabase::loadFromFile(
        path, errorMsg, clang::tooling::JSONCommandSyntax::AutoDetect);
}

std::unique_ptr<clang::tooling::CompilationDatabase>
CompileDatabaseLoader::createFromArgs(const std::string& sourceFile,
                                       const std::vector<std::string>& extraArgs,
                                       std::string& errorMsg) {
    // FixedCompilationDatabase takes a directory and a list of compiler flags.
    // It applies those flags to every file passed to ClangTool.
    std::string directory = std::filesystem::current_path().string();

    std::vector<std::string> args;
    for (const auto& arg : extraArgs) {
        args.push_back(arg);
    }

    return std::make_unique<clang::tooling::FixedCompilationDatabase>(
        directory, args);
}

std::vector<std::string>
CompileDatabaseLoader::getSourceFiles(const clang::tooling::CompilationDatabase& db) {
    return db.getAllFiles();
}

} // namespace cudaguard
