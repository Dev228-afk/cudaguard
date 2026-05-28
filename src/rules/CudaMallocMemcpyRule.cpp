#include "cudaguard/rules/CudaMallocMemcpyRule.h"
#include "cudaguard/Diagnostics.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/Expr.h"
#include "clang/AST/Decl.h"
#include "clang/ASTMatchers/ASTMatchers.h"

using namespace clang;
using namespace clang::ast_matchers;

namespace cudaguard {

CudaMallocMemcpyCallback::CudaMallocMemcpyCallback(
    DiagnosticReporter& reporter)
    : reporter_(reporter) {}

void CudaMallocMemcpyCallback::run(
    const MatchFinder::MatchResult& result) {
    if (const auto* mallocCall =
            result.Nodes.getNodeAs<CallExpr>("cudaMallocCall")) {
        trackCudaMalloc(result);
        return;
    }

    if (result.Nodes.getNodeAs<CallExpr>("cudaMemcpyCall")) {
        checkMemcpy(result);
    }
}

void CudaMallocMemcpyCallback::trackCudaMalloc(
    const MatchFinder::MatchResult& result) {
    const auto* mallocCall =
        result.Nodes.getNodeAs<CallExpr>("cudaMallocCall");
    if (!mallocCall || mallocCall->getNumArgs() < 1) return;

    const Expr* firstArg = mallocCall->getArg(0)->IgnoreImplicit();

    if (const auto* unaryOp = dyn_cast<UnaryOperator>(firstArg)) {
        if (unaryOp->getOpcode() == UO_AddrOf) {
            if (const auto* declRef =
                    dyn_cast<DeclRefExpr>(unaryOp->getSubExpr()->IgnoreImplicit())) {
                pointerCategories_[declRef->getDecl()->getNameAsString()] =
                    PointerCategory::Device;
            }
        }
    }
}

void CudaMallocMemcpyCallback::checkMemcpy(
    const MatchFinder::MatchResult& result) {
    const auto* memcpyCall =
        result.Nodes.getNodeAs<CallExpr>("cudaMemcpyCall");
    if (!memcpyCall || memcpyCall->getNumArgs() < 4) return;

    auto& sm = result.Context->getSourceManager();
    if (!sm.isInMainFile(memcpyCall->getBeginLoc())) return;

    const Expr* dstExpr = memcpyCall->getArg(0)->IgnoreImplicit();
    const Expr* srcExpr = memcpyCall->getArg(1)->IgnoreImplicit();
    const Expr* kindExpr = memcpyCall->getArg(3)->IgnoreImplicit();

    std::string dstName, srcName;
    PointerCategory dstCat = PointerCategory::Unknown;
    PointerCategory srcCat = PointerCategory::Unknown;

    if (const auto* dstRef = dyn_cast<DeclRefExpr>(dstExpr)) {
        dstName = dstRef->getDecl()->getNameAsString();
        auto it = pointerCategories_.find(dstName);
        if (it != pointerCategories_.end()) dstCat = it->second;
    }

    if (const auto* srcRef = dyn_cast<DeclRefExpr>(srcExpr)) {
        srcName = srcRef->getDecl()->getNameAsString();
        auto it = pointerCategories_.find(srcName);
        if (it != pointerCategories_.end()) srcCat = it->second;
    }

    if (dstCat == PointerCategory::Unknown &&
        srcCat == PointerCategory::Unknown) {
        return;
    }

    const auto* kindRef = dyn_cast<DeclRefExpr>(kindExpr);
    if (!kindRef) return;

    std::string kindName = kindRef->getDecl()->getNameAsString();

    bool mismatch = false;
    std::string explanation;

    if (kindName == "cudaMemcpyHostToDevice") {
        if (dstCat == PointerCategory::Host) {
            mismatch = true;
            explanation = "destination '" + dstName +
                          "' appears to be a host pointer, but copy direction is HostToDevice";
        }
        if (srcCat == PointerCategory::Device) {
            mismatch = true;
            explanation = "source '" + srcName +
                          "' was allocated with cudaMalloc (device), but copy direction is HostToDevice";
        }
    } else if (kindName == "cudaMemcpyDeviceToHost") {
        if (dstCat == PointerCategory::Device) {
            mismatch = true;
            explanation = "destination '" + dstName +
                          "' was allocated with cudaMalloc (device), but copy direction is DeviceToHost";
        }
        if (srcCat == PointerCategory::Host) {
            mismatch = true;
            explanation = "source '" + srcName +
                          "' appears to be a host pointer, but copy direction is DeviceToHost";
        }
    } else if (kindName == "cudaMemcpyDeviceToDevice") {
        if (dstCat == PointerCategory::Host) {
            mismatch = true;
            explanation = "destination '" + dstName +
                          "' appears to be a host pointer, but copy direction is DeviceToDevice";
        }
        if (srcCat == PointerCategory::Host) {
            mismatch = true;
            explanation = "source '" + srcName +
                          "' appears to be a host pointer, but copy direction is DeviceToDevice";
        }
    } else if (kindName == "cudaMemcpyHostToHost") {
        if (dstCat == PointerCategory::Device) {
            mismatch = true;
            explanation = "destination '" + dstName +
                          "' was allocated with cudaMalloc (device), but copy direction is HostToHost";
        }
        if (srcCat == PointerCategory::Device) {
            mismatch = true;
            explanation = "source '" + srcName +
                          "' was allocated with cudaMalloc (device), but copy direction is HostToHost";
        }
    }

    if (mismatch) {
        auto presumed = sm.getPresumedLoc(memcpyCall->getBeginLoc());

        Diagnostic diag;
        diag.ruleId = "CG005";
        diag.severity = Severity::Warning;
        diag.location = {
            presumed.getFilename(),
            presumed.getLine(),
            presumed.getColumn()
        };
        diag.message = "cudaMemcpy direction may not match known pointer categories";
        diag.hint = explanation;

        reporter_.report(std::move(diag));
    }
}

CudaMallocMemcpyRule::CudaMallocMemcpyRule(DiagnosticReporter& reporter)
    : callback_(reporter) {}

std::string CudaMallocMemcpyRule::id() const { return "CG005"; }

std::string CudaMallocMemcpyRule::name() const {
    return "CudaMemcpyDirectionCheck";
}

std::string CudaMallocMemcpyRule::description() const {
    return "cudaMemcpy direction mismatch (heuristic)";
}

void CudaMallocMemcpyRule::registerMatchers(MatchFinder& finder) {
    finder.addMatcher(
        callExpr(callee(functionDecl(hasName("cudaMalloc"))))
            .bind("cudaMallocCall"),
        &callback_);

    finder.addMatcher(
        callExpr(callee(functionDecl(hasName("cudaMemcpy"))))
            .bind("cudaMemcpyCall"),
        &callback_);
}

} // namespace cudaguard
