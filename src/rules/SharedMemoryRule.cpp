#include "cudaguard/rules/SharedMemoryRule.h"
#include "cudaguard/Diagnostics.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Attr.h"
#include "clang/AST/Decl.h"
#include "clang/AST/ExprCXX.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace cudaguard {

SharedMemoryDeclCallback::SharedMemoryDeclCallback(
    std::unordered_set<std::string>& kernelsWithSharedMem)
    : kernelsWithSharedMem_(kernelsWithSharedMem) {}

void SharedMemoryDeclCallback::run(const MatchFinder::MatchResult& result) {
    const auto* kernel =
        result.Nodes.getNodeAs<FunctionDecl>("kernel");
    const auto* sharedVar =
        result.Nodes.getNodeAs<VarDecl>("sharedVar");

    if (!kernel || !sharedVar) return;

    if (!sharedVar->hasAttr<CUDASharedAttr>()) return;
    if (!sharedVar->hasExternalStorage()) return;

    kernelsWithSharedMem_.insert(kernel->getNameAsString());
}

SharedMemoryLaunchCallback::SharedMemoryLaunchCallback(
    DiagnosticReporter& reporter,
    const std::unordered_set<std::string>& kernelsWithSharedMem)
    : reporter_(reporter), kernelsWithSharedMem_(kernelsWithSharedMem) {}

void SharedMemoryLaunchCallback::run(
    const MatchFinder::MatchResult& result) {
    const auto* kernelCall =
        result.Nodes.getNodeAs<CUDAKernelCallExpr>("kernelLaunch");
    if (!kernelCall) return;

    auto& sm = result.Context->getSourceManager();
    if (!sm.isInMainFile(kernelCall->getBeginLoc())) return;

    const FunctionDecl* callee = kernelCall->getDirectCallee();
    if (!callee) return;

    std::string kernelName = callee->getNameAsString();
    if (kernelsWithSharedMem_.find(kernelName) ==
        kernelsWithSharedMem_.end()) {
        return;
    }

    const CallExpr* config = kernelCall->getConfig();
    if (!config) return;

    bool hasSharedMemArg = false;
    if (config->getNumArgs() >= 3) {
        const Expr* sharedArg = config->getArg(2)->IgnoreImplicit();
        if (const auto* intLit = dyn_cast<IntegerLiteral>(sharedArg)) {
            hasSharedMemArg = !intLit->getValue().isZero();
        } else {
            hasSharedMemArg = true;
        }
    }

    if (!hasSharedMemArg) {
        auto presumed = sm.getPresumedLoc(kernelCall->getBeginLoc());

        Diagnostic diag;
        diag.ruleId = "CG004";
        diag.severity = Severity::Warning;
        diag.location = {
            presumed.getFilename(),
            presumed.getLine(),
            presumed.getColumn()
        };
        diag.message = "kernel '" + kernelName +
                       "' uses extern __shared__ memory, but launch does "
                       "not provide a dynamic shared-memory size";
        diag.hint = "use the third kernel launch parameter to specify "
                    "dynamic shared memory size";
        diag.symbolName = kernelName;

        reporter_.report(std::move(diag));
    }
}

SharedMemoryRule::SharedMemoryRule(DiagnosticReporter& reporter)
    : declCallback_(kernelsWithSharedMem_),
      launchCallback_(reporter, kernelsWithSharedMem_) {}

std::string SharedMemoryRule::id() const { return "CG004"; }

std::string SharedMemoryRule::name() const {
    return "UnsafeDynamicSharedMemory";
}

std::string SharedMemoryRule::description() const {
    return "Kernel uses extern __shared__ but launch omits shared-memory size";
}

void SharedMemoryRule::registerMatchers(MatchFinder& finder) {
    finder.addMatcher(
        functionDecl(
            hasAttr(attr::CUDAGlobal),
            hasDescendant(
                varDecl(hasAttr(attr::CUDAShared)).bind("sharedVar")))
            .bind("kernel"),
        &declCallback_);

    finder.addMatcher(
        cudaKernelCallExpr().bind("kernelLaunch"),
        &launchCallback_);
}

} // namespace cudaguard
