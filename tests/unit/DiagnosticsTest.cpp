#include <gtest/gtest.h>
#include "cudaguard/Diagnostics.h"

using namespace cudaguard;

TEST(DiagnosticsTest, SeverityToString) {
    EXPECT_EQ(severityToString(Severity::Note), "note");
    EXPECT_EQ(severityToString(Severity::Warning), "warning");
    EXPECT_EQ(severityToString(Severity::Error), "error");
}

TEST(DiagnosticsTest, FormatHumanReadable) {
    Diagnostic diag;
    diag.ruleId = "CG001";
    diag.severity = Severity::Warning;
    diag.location = {"test.cu", 42, 5};
    diag.message = "kernel launch is not followed by a CUDA error check";
    diag.hint = "add cudaGetLastError() after the launch";

    std::string output = formatDiagnosticHuman(diag);
    EXPECT_NE(output.find("test.cu:42:5"), std::string::npos);
    EXPECT_NE(output.find("warning"), std::string::npos);
    EXPECT_NE(output.find("CG001"), std::string::npos);
    EXPECT_NE(output.find("kernel launch"), std::string::npos);
    EXPECT_NE(output.find("hint:"), std::string::npos);
}

TEST(DiagnosticsTest, FormatHumanReadableNoHint) {
    Diagnostic diag;
    diag.ruleId = "CG003";
    diag.severity = Severity::Error;
    diag.location = {"file.cu", 10, 3};
    diag.message = "device function calls host-only function";

    std::string output = formatDiagnosticHuman(diag);
    EXPECT_NE(output.find("file.cu:10:3"), std::string::npos);
    EXPECT_NE(output.find("error"), std::string::npos);
    EXPECT_EQ(output.find("hint:"), std::string::npos);
}

TEST(DiagnosticsTest, FormatJson) {
    std::vector<Diagnostic> diags;
    Diagnostic d1;
    d1.ruleId = "CG001";
    d1.severity = Severity::Warning;
    d1.location = {"test.cu", 42, 5};
    d1.message = "test message";
    d1.hint = "test hint";
    diags.push_back(d1);

    std::string json = formatDiagnosticsJson(diags);
    EXPECT_NE(json.find("\"rule_id\": \"CG001\""), std::string::npos);
    EXPECT_NE(json.find("\"line\": 42"), std::string::npos);
    EXPECT_NE(json.find("\"severity\": \"warning\""), std::string::npos);
    EXPECT_NE(json.find("\"hint\": \"test hint\""), std::string::npos);
}

TEST(DiagnosticsTest, ReporterCounts) {
    DiagnosticReporter reporter;

    Diagnostic w1;
    w1.ruleId = "CG001";
    w1.severity = Severity::Warning;
    w1.location = {"a.cu", 1, 1};
    w1.message = "warning 1";
    reporter.report(w1);

    Diagnostic e1;
    e1.ruleId = "CG003";
    e1.severity = Severity::Error;
    e1.location = {"b.cu", 2, 1};
    e1.message = "error 1";
    reporter.report(e1);

    EXPECT_EQ(reporter.warningCount(), 1u);
    EXPECT_EQ(reporter.errorCount(), 1u);
    EXPECT_EQ(reporter.diagnostics().size(), 2u);
}

TEST(DiagnosticsTest, WarningsAsErrors) {
    DiagnosticReporter reporter;
    reporter.setWarningsAsErrors(true);

    Diagnostic w1;
    w1.ruleId = "CG001";
    w1.severity = Severity::Warning;
    w1.location = {"a.cu", 1, 1};
    w1.message = "promoted warning";
    reporter.report(w1);

    EXPECT_EQ(reporter.warningCount(), 0u);
    EXPECT_EQ(reporter.errorCount(), 1u);
    EXPECT_EQ(reporter.diagnostics()[0].severity, Severity::Error);
}

TEST(DiagnosticsTest, EmptyDiagnostics) {
    DiagnosticReporter reporter;
    EXPECT_EQ(reporter.warningCount(), 0u);
    EXPECT_EQ(reporter.errorCount(), 0u);
    EXPECT_TRUE(reporter.diagnostics().empty());
}

TEST(DiagnosticsTest, JsonWithSymbolName) {
    std::vector<Diagnostic> diags;
    Diagnostic d;
    d.ruleId = "CG003";
    d.severity = Severity::Error;
    d.location = {"test.cu", 15, 9};
    d.message = "bad call";
    d.symbolName = "hostFunc";
    diags.push_back(d);

    std::string json = formatDiagnosticsJson(diags);
    EXPECT_NE(json.find("\"symbol\": \"hostFunc\""), std::string::npos);
}
