//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/NativeFormatting.h"
#ضمن "llvm/Support/SMTAPI.h"

using نطاق llvm;

#إذا LLVM_WITH_Z3

#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/ADT/Twine.h"

#ضمن <set>

#ضمن <z3.h>

نطاق {

/// Configuration class for Z3
صنف Z3Config {
  friend صنف Z3Context;

  Z3_config Config = Z3_mk_config();

عام:
  Z3Config() = افتراضي;
  Z3Config(ثابت Z3Config &) = delete;
  Z3Config(Z3Config &&) = افتراضي;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = افتراضي;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
فراغ Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_رسالة(Context, Error)));
}

/// Wrapper for Z3 context
صنف Z3Context {
عام:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = delete;
  Z3Context(Z3Context &&) = افتراضي;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = افتراضي;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
صنف Z3Sort : عام SMTSort {
  friend صنف Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

عام:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ثابت Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ثابت Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    إرجاع *هذا;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    إذا (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  فراغ Profile(llvm::Folding_Set_عقدة_ID &ID) ثابت override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  منطقي isBitvectorSortImpl() ثابت override {
    إرجاع (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  منطقي isFloatSortImpl() ثابت override {
    إرجاع (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  منطقي isBooleanSortImpl() ثابت override {
    إرجاع (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ثابت override {
    إرجاع Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ثابت override {
    إرجاع Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  منطقي equal_to(SMTSort ثابت &Other) ثابت override {
    إرجاع Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ثابت Z3Sort &>(Other).Sort);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

ساكن ثابت Z3Sort &toZ3Sort(ثابت SMTSort &S) {
  إرجاع static_cast<ثابت Z3Sort &>(S);
}

صنف Z3_تعبير : عام SMTExpr {
  friend صنف Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عام:
  Z3_تعبير(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_تعبير(ثابت Z3_تعبير &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_تعبير &operator=(ثابت Z3_تعبير &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    إرجاع *هذا;
  }

  Z3_تعبير(Z3_تعبير &&Other) = delete;
  Z3_تعبير &operator=(Z3_تعبير &&Other) = delete;

  ~Z3_تعبير() {
    إذا (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  فراغ Profile(llvm::Folding_Set_عقدة_ID &ID) ثابت override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقي equal_to(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ثابت Z3_تعبير &>(Other).AST)) &&
           "AST's must have the same sort");
    إرجاع Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_تعبير &>(Other).AST);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

ساكن ثابت Z3_تعبير &to_Z3_تعبير(ثابت SMTExpr &E) {
  إرجاع static_cast<ثابت Z3_تعبير &>(E);
}

صنف Z3Model {
  friend صنف Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عام:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    إذا (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  فراغ print(raw_ostream &OS) ثابت {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD فراغ dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ساكن ثابت llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  بدل (BitWidth) {
  افتراضي:
    llvm_unreachable("Unsupported floating-point semantics!");
    اكسر;
  حالة 16:
    إرجاع llvm::APFloat::IEEEhalf();
  حالة 32:
    إرجاع llvm::APFloat::IEEEsingle();
  حالة 64:
    إرجاع llvm::APFloat::IEEEdouble();
  حالة 128:
    إرجاع llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ساكن منطقي areEquivalent(ثابت llvm::fltSemantics &LHS,
                          ثابت llvm::fltSemantics &RHS) {
  إرجاع (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

صنف Z3Solver : عام SMTSolver {
  friend صنف Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [هذا] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    إرجاع S;
  }();

  Z3_params Params = [هذا] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    إرجاع P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_تعبير> CachedExprs;

عام:
  Z3Solver() = افتراضي;
  Z3Solver(ثابت Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  فراغ addConstraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_تعبير(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ثابت SMTSort &Sort) {
    auto It = CachedSorts.أدرج(toZ3Sort(Sort));
    إرجاع &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_تعبير_Ref(ثابت SMTExpr &Exp) {
    auto It = CachedExprs.أدرج(to_Z3_تعبير(Exp));
    إرجاع &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    إرجاع newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    إرجاع newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ثابت SMTExprRef &Exp) override {
    إرجاع newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    إرجاع newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    إرجاع newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    إرجاع newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    إرجاع newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvneg(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvnot(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_not(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvadd(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvsub(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvmul(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvsrem(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvurem(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvudiv(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvshl(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvashr(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvlshr(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvxor(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvor(Context.Context, to_Z3_تعبير(*LHS).AST,
                                   to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvand(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvult(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvslt(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvugt(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvsgt(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvule(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvsle(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvuge(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_bvsge(Context.Context, to_Z3_تعبير(*LHS).AST,
                                    to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_تعبير(*LHS).AST, to_Z3_تعبير(*RHS).AST};
    إرجاع new_تعبير_Ref(Z3_تعبير(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_تعبير(*LHS).AST, to_Z3_تعبير(*RHS).AST};
    إرجاع new_تعبير_Ref(Z3_تعبير(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_eq(Context.Context, to_Z3_تعبير(*LHS).AST,
                                 to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                             to_Z3_تعبير(*LHS).AST, to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                             to_Z3_تعبير(*LHS).AST, to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_تعبير(*LHS).AST,
                                      to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                             to_Z3_تعبير(*LHS).AST, to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                             to_Z3_تعبير(*LHS).AST, to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_تعبير(*LHS).AST,
                                      to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_تعبير(*LHS).AST,
                                      to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_ite(Context.Context, to_Z3_تعبير(*Cond).AST,
                                  to_Z3_تعبير(*T).AST, to_Z3_تعبير(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_تعبير(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned أعلى_2, unsigned أدنى_2,
                         ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(Context, Z3_mk_extract(Context.Context, أعلى_2, أدنى_2,
                                                    to_Z3_تعبير(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                         to_Z3_تعبير(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                          to_Z3_تعبير(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                         to_Z3_تعبير(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقي isSigned) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                          to_Z3_تعبير(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                          to_Z3_تعبير(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ثابت SMTExprRef &Exp) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_تعبير(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                         to_Z3_تعبير(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_تعبير(*LHS).AST,
                                          to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_concat(Context.Context, to_Z3_تعبير(*LHS).AST,
                                     to_Z3_تعبير(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                              to_Z3_تعبير(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                               to_Z3_تعبير(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                                 to_Z3_تعبير(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                                  to_Z3_تعبير(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_تعبير(*RoundingMode).AST,
                                  to_Z3_تعبير(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ثابت منطقي b) override {
    إرجاع new_تعبير_Ref(Z3_تعبير(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, unsigned BitWidth) override {
    ثابت Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    إذا (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      إرجاع new_تعبير_Ref(Z3_تعبير(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ثابت int64_t BitReprAsSigned = Int.get_Ext_قيمة();
    ثابت uint64_t BitReprAsUnsigned =
        reinterpret_cast<ثابت uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    إرجاع new_تعبير_Ref(Z3_تعبير(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), خطأ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    إرجاع new_تعبير_Ref(Z3_تعبير(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_تعبير(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_رمز(ثابت حرف *Name, SMTSortRef Sort) override {
    إرجاع new_تعبير_Ref(
        Z3_تعبير(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_رمز(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, unsigned BitWidth,
                            منطقي isUnsigned) override {
    إرجاع llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_تعبير(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقي getBoolean(ثابت SMTExprRef &Exp) override {
    إرجاع Z3_get_bool_قيمة(Context.Context, to_Z3_تعبير(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    إرجاع new_تعبير_Ref(Z3_تعبير(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقي toAPFloat(ثابت SMTSortRef &Sort, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقي useSemantics) {
    أكد(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), true);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    إذا (!toAPSInt(BVSort, AST, Int, true)) {
      إرجاع خطأ;
    }

    إذا (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      أكد(خطأ && "Floating-point types don't match!");
      إرجاع خطأ;
    }

    Float = llvm::APFloat(Semantics, Int);
    إرجاع true;
  }

  منطقي toAPSInt(ثابت SMTSortRef &Sort, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقي useSemantics) {
    إذا (Sort->isBitvectorSort()) {
      إذا (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        أكد(خطأ && "Bitvector types don't match!");
        إرجاع خطأ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      إرجاع true;
    }

    إذا (Sort->isBooleanSort()) {
      إذا (useSemantics && Int.getBitWidth() < 1) {
        أكد(خطأ && "Boolean type doesn't match!");
        إرجاع خطأ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      إرجاع true;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_تعبير(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      إرجاع خطأ;

    SMTExprRef Assign = new_تعبير_Ref(
        Z3_تعبير(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    إرجاع toAPSInt(Sort, Assign, Int, true);
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_تعبير(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      إرجاع خطأ;

    SMTExprRef Assign = new_تعبير_Ref(
        Z3_تعبير(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    إرجاع toAPFloat(Sort, Assign, Float, true);
  }

  std::optional<منطقي> check() ثابت override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    إذا (res == Z3_L_TRUE)
      إرجاع true;

    إذا (res == Z3_L_FALSE)
      إرجاع خطأ;

    إرجاع std::nullopt;
  }

  فراغ ادفع() override { إرجاع Z3_solver_ادفع(Context.Context, Solver); }

  فراغ اسحب(unsigned رقم_States = 1) override {
    أكد(Z3_solver_get_رقم_scopes(Context.Context, Solver) >= رقم_States);
    إرجاع Z3_solver_اسحب(Context.Context, Solver, رقم_States);
  }

  منطقي isFPSupported() override { إرجاع true; }

  /// Reset the solver and remove all constraints.
  فراغ reset() override { Z3_solver_reset(Context.Context, Solver); }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  فراغ setUnsignedParam(StringRef مفتاح, unsigned قيمة) override {
    Z3_رمز Sym = Z3_mk_string_رمز(Context.Context, مفتاح.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, قيمة);
  }

  فراغ setBoolParam(StringRef مفتاح, منطقي قيمة) override {
    Z3_رمز Sym = Z3_mk_string_رمز(Context.Context, مفتاح.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, قيمة);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

صنف Z3Statistics final : عام SMTSolverStatistics {
عام:
  مزدوج getDouble(StringRef مفتاح) ثابت override {
    auto It = Double_قيم.find(مفتاح.str());
    أكد(It != Double_قيم.end());
    إرجاع It->second;
  };
  unsigned getUnsigned(StringRef مفتاح) ثابت override {
    auto It = Unsigned_قيم.find(مفتاح.str());
    إذا (It == Unsigned_قيم.end())
      إرجاع 0;
    إرجاع It->second;
  };

  فراغ print(raw_ostream &OS) ثابت override {
    لكل (ثابت auto &E : Unsigned_قيم)
      OS << E.first() << ": " << E.second << '\n';
    لكل (ثابت auto &E : Double_قيم) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

خاص:
  friend صنف Z3Solver;
  StringMap<unsigned> Unsigned_قيم;
  StringMap<مزدوج> Double_قيم;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  auto ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned رقم_مفاتيح = Z3_stats_size(C, S);
  لكل (unsigned Idx = 0; Idx < رقم_مفاتيح; ++Idx) {
    ثابت حرف *مفتاح = Z3_stats_get_مفتاح(C, S, Idx);
    إذا (Z3_stats_is_uint(C, S, Idx)) {
      auto قيمة = Z3_stats_get_uint_قيمة(C, S, Idx);
      Result.Unsigned_قيم.try_emplace(مفتاح, قيمة);
    } وإلا {
      أكد(Z3_stats_is_double(C, S, Idx));
      auto قيمة = Z3_stats_get_double_قيمة(C, S, Idx);
      Result.Double_قيم.try_emplace(مفتاح, قيمة);
    }
  }
  إرجاع std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#إذا LLVM_WITH_Z3
  إرجاع std::make_unique<Z3Solver>();
#وإلا
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           خطأ);
  إرجاع nullptr;
#endif
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_METHOD فراغ SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_METHOD فراغ SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_METHOD فراغ SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#endif
