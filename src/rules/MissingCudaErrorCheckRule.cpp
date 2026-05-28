#include "cudaguard/rules/MissingCudaErrorCheckRule.h"
#include "cudaguard/Diagnostics.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/ExprCXX.h"
#include "clang/AST/Stmt.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace cudaguard {

MissingCudaErrorCheckCallback::MissingCudaErrorCheckCallback(
    DiagnosticReporter& reporter)
    : reporter_(reporter) {}

void MissingCudaErrorCheckCallback::run(
    const MatchFinder::MatchResult& result) {
    const auto* kernelCall =
        result.Nodes.getNodeAs<CUDAKernelCallExpr>("kernelCall");
    const auto* compound =
        result.Nodes.getNodeAs<CompoundStmt>("compound");

    if (!kernelCall || !compound) return;

    auto& sm = result.Context->getSourceManager();

    if (!sm.isInMainFile(kernelCall->getBeginLoc())) return;

    bool foundAfterKernel = false;
    bool foundErrorCheck = false;
    int statementsChecked = 0;
    static constexpr int kLookahead = 5;

    for (const auto* child : compound->body()) {
        if (!foundAfterKernel) {
            if (child == kernelCall) {
                foundAfterKernel = true;
            } else if (isa<Expr>(child)) {
                const Expr* stripped = cast<Expr>(child)->IgnoreImplicit();
                if (stripped == kernelCall ||
                    child->getBeginLoc() == kernelCall->getBeginLoc()) {
                    foundAfterKernel = true;
                }
            }
            continue;
        }

        if (++statementsChecked > kLookahead) break;

        if (const auto* callExpr = dyn_cast<CallExpr>(child)) {
            if (const auto* callee = callExpr->getDirectCallee()) {
                std::string name = callee->getNameAsString();
                if (name == "cudaGetLastError" ||
                    name == "cudaPeekAtLastError" ||
                    name == "cudaDeviceSynchronize") {
                    foundErrorCheck = true;
                    break;
                }
            }
        }

        if (const auto* declStmt = dyn_cast<DeclStmt>(child)) {
            for (const auto* decl : declStmt->decls()) {
                if (const auto* varDecl = dyn_cast<VarDecl>(decl)) {
                    if (const auto* init = varDecl->getInit()) {
                        if (const auto* callExpr =
                                dyn_cast<CallExpr>(init->IgnoreImplicit())) {
                            if (const auto* callee =
                                    callExpr->getDirectCallee()) {
                                std::string name = callee->getNameAsString();
                                if (name == "cudaGetLastError" ||
                                    name == "cudaPeekAtLastError" ||
                                    name == "cudaDeviceSynchronize") {
                                    foundErrorCheck = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (foundErrorCheck) break;
        }

        if (const auto* exprStmt = dyn_cast<Expr>(child)) {
            const Expr* inner = exprStmt->IgnoreImplicit();
            if (const auto* callExpr = dyn_cast<CallExpr>(inner)) {
                if (const auto* callee = callExpr->getDirectCallee()) {
                    std::string name = callee->getNameAsString();
                    if (name == "cudaGetLastError" ||
                        name == "cudaPeekAtLastError" ||
                        name == "cudaDeviceSynchronize") {
                        foundErrorCheck = true;
                        break;
                    }
                }
            }
        }
    }

    if (!foundErrorCheck) {
        auto loc = kernelCall->getBeginLoc();
        auto presumed = sm.getPresumedLoc(loc);

        Diagnostic diag;
        diag.ruleId = "CG001";
        diag.severity = Severity::Warning;
        diag.location = {
            presumed.getFilename(),
            presumed.getLine(),
            presumed.getColumn()
        };
        diag.message = "kernel launch is not followed by cudaGetLastError, "
                       "cudaPeekAtLastError, or cudaDeviceSynchronize";
        diag.hint = "add cudaGetLastError() after the launch to catch "
                    "asynchronous launch failures";

        if (const auto* callee = kernelCall->getDirectCallee()) {
            diag.symbolName = callee->getNameAsString();
        }

        reporter_.report(std::move(diag));
    }
}

MissingCudaErrorCheckRule::MissingCudaErrorCheckRule(
    DiagnosticReporter& reporter)
    : callback_(reporter) {}

std::string MissingCudaErrorCheckRule::id() const { return "CG001"; }

std::string MissingCudaErrorCheckRule::name() const {
    return "MissingCudaErrorCheck";
}

std::string MissingCudaErrorCheckRule::description() const {
    return "Kernel launch not followed by CUDA error check";
}

void MissingCudaErrorCheckRule::registerMatchers(MatchFinder& finder) {
    finder.addMatcher(
        compoundStmt(forEach(cudaKernelCallExpr().bind("kernelCall")))
            .bind("compound"),
        &callback_);
}

} // namespace cudaguard
