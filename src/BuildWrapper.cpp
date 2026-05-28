#include "cudaguard/BuildWrapper.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace cudaguard {

std::vector<std::string>
BuildWrapper::extractCudaFiles(const std::vector<std::string>& nvccArgs) {
    std::vector<std::string> cudaFiles;
    for (const auto& arg : nvccArgs) {
        if (arg.size() > 3 && arg.substr(arg.size() - 3) == ".cu") {
            cudaFiles.push_back(arg);
        }
    }
    return cudaFiles;
}

BuildWrapper::WrapResult
BuildWrapper::invokeCompiler(const std::vector<std::string>& nvccCommand) {
    WrapResult result;

    std::string cmd;
    for (size_t i = 0; i < nvccCommand.size(); ++i) {
        if (i > 0) cmd += " ";
        cmd += nvccCommand[i];
    }
    cmd += " 2>&1";

    std::array<char, 4096> buffer;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) {
        result.exitCode = -1;
        result.stderrOutput = "Failed to invoke compiler command";
        return result;
    }

    std::ostringstream output;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output << buffer.data();
    }

    result.exitCode = pclose(pipe);
    result.stdoutOutput = output.str();
    return result;
}

std::string
BuildWrapper::saveArtifactLog(const WrapResult& result, const std::string& artifactDir) {
    namespace fs = std::filesystem;

    fs::create_directories(artifactDir);

    auto now = std::chrono::system_clock::now();
    auto epoch = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();

    std::string logFile = artifactDir + "/build_" + std::to_string(millis) + ".log";

    std::ofstream ofs(logFile);
    if (ofs.is_open()) {
        ofs << "Exit code: " << result.exitCode << "\n";
        ofs << "--- stdout ---\n" << result.stdoutOutput << "\n";
        if (!result.stderrOutput.empty()) {
            ofs << "--- stderr ---\n" << result.stderrOutput << "\n";
        }
        ofs.close();
    }

    return logFile;
}

} // namespace cudaguard
