#include "cudaguard/Diagnostics.h"

#include <iostream>
#include <sstream>

namespace cudaguard {

void DiagnosticReporter::report(Diagnostic diag) {
    if (warningsAsErrors_ && diag.severity == Severity::Warning) {
        diag.severity = Severity::Error;
    }
    diagnostics_.push_back(std::move(diag));
}

void DiagnosticReporter::setJsonOutput(bool enabled) {
    jsonOutput_ = enabled;
}

void DiagnosticReporter::setWarningsAsErrors(bool enabled) {
    warningsAsErrors_ = enabled;
}

const std::vector<Diagnostic>& DiagnosticReporter::diagnostics() const {
    return diagnostics_;
}

uint32_t DiagnosticReporter::errorCount() const {
    uint32_t count = 0;
    for (const auto& d : diagnostics_) {
        if (d.severity == Severity::Error) ++count;
    }
    return count;
}

uint32_t DiagnosticReporter::warningCount() const {
    uint32_t count = 0;
    for (const auto& d : diagnostics_) {
        if (d.severity == Severity::Warning) ++count;
    }
    return count;
}

void DiagnosticReporter::printHumanReadable() const {
    for (const auto& diag : diagnostics_) {
        std::cout << formatDiagnosticHuman(diag) << "\n";
    }
}

void DiagnosticReporter::printJson() const {
    std::cout << formatDiagnosticsJson(diagnostics_) << "\n";
}

void DiagnosticReporter::printSummary() const {
    std::cout << errorCount() << " error(s), "
              << warningCount() << " warning(s)\n";
}

std::string severityToString(Severity sev) {
    switch (sev) {
        case Severity::Note: return "note";
        case Severity::Warning: return "warning";
        case Severity::Error: return "error";
    }
    return "unknown";
}

std::string formatDiagnosticHuman(const Diagnostic& diag) {
    std::ostringstream oss;
    oss << diag.location.file << ":"
        << diag.location.line << ":"
        << diag.location.column << ": "
        << severityToString(diag.severity) << ": "
        << diag.ruleId << ": "
        << diag.message;
    if (!diag.hint.empty()) {
        oss << "\n  hint: " << diag.hint;
    }
    return oss.str();
}

std::string formatDiagnosticsJson(const std::vector<Diagnostic>& diags) {
    std::ostringstream oss;
    oss << "[\n";
    for (size_t i = 0; i < diags.size(); ++i) {
        const auto& d = diags[i];
        oss << "  {\n"
            << "    \"file\": \"" << d.location.file << "\",\n"
            << "    \"line\": " << d.location.line << ",\n"
            << "    \"column\": " << d.location.column << ",\n"
            << "    \"severity\": \"" << severityToString(d.severity) << "\",\n"
            << "    \"rule_id\": \"" << d.ruleId << "\",\n"
            << "    \"message\": \"" << d.message << "\"";
        if (!d.hint.empty()) {
            oss << ",\n    \"hint\": \"" << d.hint << "\"";
        }
        if (d.symbolName.has_value()) {
            oss << ",\n    \"symbol\": \"" << d.symbolName.value() << "\"";
        }
        oss << "\n  }";
        if (i + 1 < diags.size()) oss << ",";
        oss << "\n";
    }
    oss << "]";
    return oss.str();
}

} // namespace cudaguard
