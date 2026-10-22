//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/ScopeExit.h"
#شامل "llvm/Config/config.h"
#شامل "llvm/Support/NativeFormatting.h"
#شامل "llvm/Support/SMTAPI.h"

using فضاینام llvm;

#اگر LLVM_WITH_Z3

#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/ADT/Twine.h"

#شامل <set>

#شامل <z3.h>

فضاینام {

/// Configuration class for Z3
رده Z3Config {
  friend رده Z3Context;

  Z3_config Config = Z3_mk_config();

عمومی:
  Z3Config() = پیشفرض;
  Z3Config(ثابت Z3Config &) = delete;
  Z3Config(Z3Config &&) = پیشفرض;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = پیشفرض;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
تهی Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_پیام(Context, Error)));
}

/// Wrapper for Z3 context
رده Z3Context {
عمومی:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = delete;
  Z3Context(Z3Context &&) = پیشفرض;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = پیشفرض;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
رده Z3Sort : عمومی SMTSort {
  friend رده Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

عمومی:
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
    بازگشت *این;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    اگر (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  تهی Profile(llvm::Folding_Set_گره_ID &ID) ثابت override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  منطقی isBitvectorSortImpl() ثابت override {
    بازگشت (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  منطقی isFloatSortImpl() ثابت override {
    بازگشت (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  منطقی isBooleanSortImpl() ثابت override {
    بازگشت (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ثابت override {
    بازگشت Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ثابت override {
    بازگشت Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  منطقی equal_to(SMTSort ثابت &Other) ثابت override {
    بازگشت Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ثابت Z3Sort &>(Other).Sort);
  }

  تهی print(raw_ostream &OS) ثابت override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

ایستا ثابت Z3Sort &toZ3Sort(ثابت SMTSort &S) {
  بازگشت static_cast<ثابت Z3Sort &>(S);
}

رده Z3_عبارت : عمومی SMTExpr {
  friend رده Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عمومی:
  Z3_عبارت(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_عبارت(ثابت Z3_عبارت &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_عبارت &operator=(ثابت Z3_عبارت &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    بازگشت *این;
  }

  Z3_عبارت(Z3_عبارت &&Other) = delete;
  Z3_عبارت &operator=(Z3_عبارت &&Other) = delete;

  ~Z3_عبارت() {
    اگر (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  تهی Profile(llvm::Folding_Set_گره_ID &ID) ثابت override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقی equal_to(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ثابت Z3_عبارت &>(Other).AST)) &&
           "AST's must have the same sort");
    بازگشت Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_عبارت &>(Other).AST);
  }

  تهی print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

ایستا ثابت Z3_عبارت &to_Z3_عبارت(ثابت SMTExpr &E) {
  بازگشت static_cast<ثابت Z3_عبارت &>(E);
}

رده Z3Model {
  friend رده Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عمومی:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    اگر (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  تهی print(raw_ostream &OS) ثابت {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD تهی dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ایستا ثابت llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  انتخاب (BitWidth) {
  پیشفرض:
    llvm_unreachable("Unsupported floating-point semantics!");
    بشکن;
  حالت 16:
    بازگشت llvm::APFloat::IEEEhalf();
  حالت 32:
    بازگشت llvm::APFloat::IEEEsingle();
  حالت 64:
    بازگشت llvm::APFloat::IEEEdouble();
  حالت 128:
    بازگشت llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ایستا منطقی areEquivalent(ثابت llvm::fltSemantics &LHS,
                          ثابت llvm::fltSemantics &RHS) {
  بازگشت (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

رده Z3Solver : عمومی SMTSolver {
  friend رده Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [این] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    بازگشت S;
  }();

  Z3_params Params = [این] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    بازگشت P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_عبارت> CachedExprs;

عمومی:
  Z3Solver() = پیشفرض;
  Z3Solver(ثابت Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  تهی addConstraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_عبارت(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ثابت SMTSort &Sort) {
    auto It = CachedSorts.درج(toZ3Sort(Sort));
    بازگشت &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_عبارت_Ref(ثابت SMTExpr &Exp) {
    auto It = CachedExprs.درج(to_Z3_عبارت(Exp));
    بازگشت &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    بازگشت newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    بازگشت newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ثابت SMTExprRef &Exp) override {
    بازگشت newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    بازگشت newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    بازگشت newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    بازگشت newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    بازگشت newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvneg(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvnot(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_not(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvadd(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsub(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvmul(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsrem(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvurem(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvudiv(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvshl(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvashr(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvlshr(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvxor(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvor(Context.Context, to_Z3_عبارت(*LHS).AST,
                                   to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvand(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvult(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvslt(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvugt(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsgt(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvule(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsle(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvuge(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsge(Context.Context, to_Z3_عبارت(*LHS).AST,
                                    to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_عبارت(*LHS).AST, to_Z3_عبارت(*RHS).AST};
    بازگشت new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_عبارت(*LHS).AST, to_Z3_عبارت(*RHS).AST};
    بازگشت new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_eq(Context.Context, to_Z3_عبارت(*LHS).AST,
                                 to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                             to_Z3_عبارت(*LHS).AST, to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                             to_Z3_عبارت(*LHS).AST, to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_عبارت(*LHS).AST,
                                      to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                             to_Z3_عبارت(*LHS).AST, to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                             to_Z3_عبارت(*LHS).AST, to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_عبارت(*LHS).AST,
                                      to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_عبارت(*LHS).AST,
                                      to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_ite(Context.Context, to_Z3_عبارت(*Cond).AST,
                                  to_Z3_عبارت(*T).AST, to_Z3_عبارت(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned بالا_2, unsigned کم_2,
                         ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_extract(Context.Context, بالا_2, کم_2,
                                                    to_Z3_عبارت(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقی isSigned) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                         to_Z3_عبارت(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                          to_Z3_عبارت(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                         to_Z3_عبارت(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقی isSigned) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                          to_Z3_عبارت(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                          to_Z3_عبارت(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ثابت SMTExprRef &Exp) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_عبارت(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقی isSigned) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                         to_Z3_عبارت(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_عبارت(*LHS).AST,
                                          to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_concat(Context.Context, to_Z3_عبارت(*LHS).AST,
                                     to_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                              to_Z3_عبارت(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                               to_Z3_عبارت(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                                 to_Z3_عبارت(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                                  to_Z3_عبارت(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_عبارت(*RoundingMode).AST,
                                  to_Z3_عبارت(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ثابت منطقی b) override {
    بازگشت new_عبارت_Ref(Z3_عبارت(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, unsigned BitWidth) override {
    ثابت Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    اگر (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      بازگشت new_عبارت_Ref(Z3_عبارت(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ثابت int64_t BitReprAsSigned = Int.get_Ext_مقدار();
    ثابت uint64_t BitReprAsUnsigned =
        reinterpret_cast<ثابت uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    بازگشت new_عبارت_Ref(Z3_عبارت(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), نادرست);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    بازگشت new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_عبارت(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_نماد(ثابت نویسه *Name, SMTSortRef Sort) override {
    بازگشت new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_نماد(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, unsigned BitWidth,
                            منطقی isUnsigned) override {
    بازگشت llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_عبارت(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقی getBoolean(ثابت SMTExprRef &Exp) override {
    بازگشت Z3_get_bool_مقدار(Context.Context, to_Z3_عبارت(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    بازگشت new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقی toAPFloat(ثابت SMTSortRef &Sort, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقی useSemantics) {
    أكد(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), درست);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    اگر (!toAPSInt(BVSort, AST, Int, درست)) {
      بازگشت نادرست;
    }

    اگر (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      أكد(نادرست && "Floating-point types don't match!");
      بازگشت نادرست;
    }

    Float = llvm::APFloat(Semantics, Int);
    بازگشت درست;
  }

  منطقی toAPSInt(ثابت SMTSortRef &Sort, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقی useSemantics) {
    اگر (Sort->isBitvectorSort()) {
      اگر (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        أكد(نادرست && "Bitvector types don't match!");
        بازگشت نادرست;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      بازگشت درست;
    }

    اگر (Sort->isBooleanSort()) {
      اگر (useSemantics && Int.getBitWidth() < 1) {
        أكد(نادرست && "Boolean type doesn't match!");
        بازگشت نادرست;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      بازگشت درست;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقی getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_عبارت(*Exp).AST));
    اگر (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      بازگشت نادرست;

    SMTExprRef Assign = new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    بازگشت toAPSInt(Sort, Assign, Int, درست);
  }

  منطقی getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_عبارت(*Exp).AST));
    اگر (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      بازگشت نادرست;

    SMTExprRef Assign = new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    بازگشت toAPFloat(Sort, Assign, Float, درست);
  }

  std::optional<منطقی> check() ثابت override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    اگر (res == Z3_L_TRUE)
      بازگشت درست;

    اگر (res == Z3_L_FALSE)
      بازگشت نادرست;

    بازگشت std::nullopt;
  }

  تهی افزودن() override { بازگشت Z3_solver_افزودن(Context.Context, Solver); }

  تهی برداشت(unsigned رقم_States = 1) override {
    أكد(Z3_solver_get_رقم_scopes(Context.Context, Solver) >= رقم_States);
    بازگشت Z3_solver_برداشت(Context.Context, Solver, رقم_States);
  }

  منطقی isFPSupported() override { بازگشت درست; }

  /// Reset the solver and remove all constraints.
  تهی reset() override { Z3_solver_reset(Context.Context, Solver); }

  تهی print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  تهی setUnsignedParam(StringRef کلید, unsigned مقدار) override {
    Z3_نماد Sym = Z3_mk_string_نماد(Context.Context, کلید.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, مقدار);
  }

  تهی setBoolParam(StringRef کلید, منطقی مقدار) override {
    Z3_نماد Sym = Z3_mk_string_نماد(Context.Context, کلید.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, مقدار);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

رده Z3Statistics final : عمومی SMTSolverStatistics {
عمومی:
  دوگانه getDouble(StringRef کلید) ثابت override {
    auto It = Double_مقادیر.find(کلید.str());
    أكد(It != Double_مقادیر.end());
    بازگشت It->second;
  };
  unsigned getUnsigned(StringRef کلید) ثابت override {
    auto It = Unsigned_مقادیر.find(کلید.str());
    اگر (It == Unsigned_مقادیر.end())
      بازگشت 0;
    بازگشت It->second;
  };

  تهی print(raw_ostream &OS) ثابت override {
    برای (ثابت auto &E : Unsigned_مقادیر)
      OS << E.first() << ": " << E.second << '\n';
    برای (ثابت auto &E : Double_مقادیر) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

خصوصی:
  friend رده Z3Solver;
  StringMap<unsigned> Unsigned_مقادیر;
  StringMap<دوگانه> Double_مقادیر;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  auto ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned رقم_کلیدها = Z3_stats_size(C, S);
  برای (unsigned Idx = 0; Idx < رقم_کلیدها; ++Idx) {
    ثابت نویسه *کلید = Z3_stats_get_کلید(C, S, Idx);
    اگر (Z3_stats_is_uint(C, S, Idx)) {
      auto مقدار = Z3_stats_get_uint_مقدار(C, S, Idx);
      Result.Unsigned_مقادیر.try_emplace(کلید, مقدار);
    } وگرنه {
      أكد(Z3_stats_is_double(C, S, Idx));
      auto مقدار = Z3_stats_get_double_مقدار(C, S, Idx);
      Result.Double_مقادیر.try_emplace(کلید, مقدار);
    }
  }
  بازگشت std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#اگر LLVM_WITH_Z3
  بازگشت std::make_unique<Z3Solver>();
#وگرنه
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           نادرست);
  بازگشت nullptr;
#endif
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_METHOD تهی SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_METHOD تهی SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_METHOD تهی SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#endif
