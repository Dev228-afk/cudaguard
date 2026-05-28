#include "cudaguard/rules/HostDeviceCallRule.h"
#include "cudaguard/Diagnostics.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "clang/AST/Expr.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace cudaguard {

HostDeviceCallCallback::HostDeviceCallCallback(DiagnosticReporter& reporter)
    : reporter_(reporter) {}

void HostDeviceCallCallback::run(const MatchFinder::MatchResult& result) {
    const auto* deviceFunc =
        result.Nodes.getNodeAs<FunctionDecl>("deviceFunc");
    const auto* callExpr =
        result.Nodes.getNodeAs<CallExpr>("call");

    if (!deviceFunc || !callExpr) return;

    auto& sm = result.Context->getSourceManager();
    if (!sm.isInMainFile(deviceFunc->getLocation())) return;

    const FunctionDecl* callee = callExpr->getDirectCallee();
    if (!callee) return;

    if (callee->isImplicit()) return;
    if (callee->getBuiltinID() != 0) return;

    if (callee->isDependentContext()) return;

    if (callee->hasAttr<CUDADeviceAttr>()) return;
    if (callee->hasAttr<CUDAGlobalAttr>()) return;

    if (callee->hasAttr<CUDAHostAttr>() && callee->hasAttr<CUDADeviceAttr>())
        return;

    std::string calleeName = callee->getNameAsString();
    if (calleeName.empty()) return;

    if (calleeName.find("__") == 0) return;

    auto loc = callExpr->getBeginLoc();
    if (!sm.isInMainFile(loc)) return;

    auto presumed = sm.getPresumedLoc(loc);

    Diagnostic diag;
    diag.ruleId = "CG003";
    diag.severity = Severity::Error;
    diag.location = {
        presumed.getFilename(),
        presumed.getLine(),
        presumed.getColumn()
    };
    diag.message = "__device__ function '" + deviceFunc->getNameAsString() +
                   "' calls function '" + calleeName +
                   "' that is not marked __device__ or __host__ __device__";
    diag.hint = "add __device__ or __host__ __device__ qualifier to '" +
                calleeName + "'";
    diag.symbolName = calleeName;

    reporter_.report(std::move(diag));
}

HostDeviceCallRule::HostDeviceCallRule(DiagnosticReporter& reporter)
    : callback_(reporter) {}

std::string HostDeviceCallRule::id() const { return "CG003"; }

std::string HostDeviceCallRule::name() const {
    return "HostDeviceCallMismatch";
}

std::string HostDeviceCallRule::description() const {
    return "__device__ function calls host-only function";
}

void HostDeviceCallRule::registerMatchers(MatchFinder& finder) {
    finder.addMatcher(
        functionDecl(
            anyOf(hasAttr(attr::CUDADevice), hasAttr(attr::CUDAGlobal)),
            forEachDescendant(callExpr().bind("call")))
            .bind("deviceFunc"),
        &callback_);
}

} // namespace cudaguard
