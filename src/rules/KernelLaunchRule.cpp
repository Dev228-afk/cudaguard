#include "cudaguard/rules/KernelLaunchRule.h"
#include "cudaguard/Diagnostics.h"

#include <optional>
#include <string>

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "llvm/ADT/APInt.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace cudaguard {

KernelLaunchCallback::KernelLaunchCallback(DiagnosticReporter& reporter)
    : reporter_(reporter) {}

namespace {

std::optional<llvm::APInt> getIntegerLiteralValue(const Expr* expr) {
    if (!expr) return std::nullopt;
    const Expr* stripped = expr->IgnoreImplicit()->IgnoreCasts();
    if (const auto* intLit = dyn_cast<IntegerLiteral>(stripped)) {
        return intLit->getValue();
    }
    return std::nullopt;
}

} // anonymous namespace

void KernelLaunchCallback::run(const MatchFinder::MatchResult& result) {
    const auto* kernelCall =
        result.Nodes.getNodeAs<CUDAKernelCallExpr>("kernelCall");
    if (!kernelCall) return;

    auto& sm = result.Context->getSourceManager();
    if (!sm.isInMainFile(kernelCall->getBeginLoc())) return;

    const CallExpr* config = kernelCall->getConfig();
    if (!config || config->getNumArgs() < 2) return;

    const Expr* gridArg = config->getArg(0);
    const Expr* blockArg = config->getArg(1);

    auto presumed = sm.getPresumedLoc(kernelCall->getBeginLoc());
    std::string fileName = presumed.getFilename();
    unsigned line = presumed.getLine();
    unsigned col = presumed.getColumn();

    auto gridVal = getIntegerLiteralValue(gridArg);
    if (gridVal.has_value() && gridVal->isZero()) {
        Diagnostic diag;
        diag.ruleId = "CG002";
        diag.severity = Severity::Warning;
        diag.location = {fileName, line, col};
        diag.message = "kernel launch uses a zero grid dimension";
        diag.hint = "grid dimension should be positive; check grid-size calculation";
        if (const auto* callee = kernelCall->getDirectCallee()) {
            diag.symbolName = callee->getNameAsString();
        }
        reporter_.report(std::move(diag));
    }

    auto blockVal = getIntegerLiteralValue(blockArg);
    if (blockVal.has_value()) {
        if (blockVal->isZero()) {
            Diagnostic diag;
            diag.ruleId = "CG002";
            diag.severity = Severity::Warning;
            diag.location = {fileName, line, col};
            diag.message = "kernel launch uses a zero block dimension";
            diag.hint = "block dimension should be positive; check thread-block calculation";
            if (const auto* callee = kernelCall->getDirectCallee()) {
                diag.symbolName = callee->getNameAsString();
            }
            reporter_.report(std::move(diag));
        } else if (blockVal->getZExtValue() > 1024) {
            Diagnostic diag;
            diag.ruleId = "CG002";
            diag.severity = Severity::Warning;
            diag.location = {fileName, line, col};
            diag.message = "kernel launch uses a suspicious block size (" +
                           std::to_string(blockVal->getZExtValue()) +
                           "), which exceeds the maximum of 1024 threads per block";
            diag.hint = "most CUDA devices support at most 1024 threads per block";
            if (const auto* callee = kernelCall->getDirectCallee()) {
                diag.symbolName = callee->getNameAsString();
            }
            reporter_.report(std::move(diag));
        }
    }
}

KernelLaunchRule::KernelLaunchRule(DiagnosticReporter& reporter)
    : callback_(reporter) {}

std::string KernelLaunchRule::id() const { return "CG002"; }

std::string KernelLaunchRule::name() const {
    return "SuspiciousKernelLaunchConfig";
}

std::string KernelLaunchRule::description() const {
    return "Suspicious kernel launch configuration (zero/oversized dimensions)";
}

void KernelLaunchRule::registerMatchers(MatchFinder& finder) {
    finder.addMatcher(cudaKernelCallExpr().bind("kernelCall"), &callback_);
}

} // namespace cudaguard
