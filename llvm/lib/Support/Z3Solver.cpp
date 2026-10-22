//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/ScopeExit.h"
#அந்தர்பாவ "llvm/Config/config.h"
#அந்தர்பாவ "llvm/Support/NativeFormatting.h"
#அந்தர்பாவ "llvm/Support/SMTAPI.h"

using நாமாகாஶ llvm;

#எனில் LLVM_WITH_Z3

#அந்தர்பாவ "llvm/ADT/SmallString.h"
#அந்தர்பாவ "llvm/ADT/StringMap.h"
#அந்தர்பாவ "llvm/ADT/Twine.h"

#அந்தர்பாவ <set>

#அந்தர்பாவ <z3.h>

நாமாகாஶ {

/// Configuration class for Z3
வகுப்பு Z3Config {
  friend வகுப்பு Z3Context;

  Z3_config Config = Z3_mk_config();

பொது:
  Z3Config() = இயல்பு;
  Z3Config(மாறிலி Z3Config &) = delete;
  Z3Config(Z3Config &&) = இயல்பு;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = இயல்பு;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
வெறுமை Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_செய்தி(Context, Error)));
}

/// Wrapper for Z3 context
வகுப்பு Z3Context {
பொது:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(மாறிலி Z3Context &) = delete;
  Z3Context(Z3Context &&) = இயல்பு;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = இயல்பு;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
வகுப்பு Z3Sort : பொது SMTSort {
  friend வகுப்பு Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

பொது:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(மாறிலி Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(மாறிலி Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    திரும்பு *இது;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    எனில் (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  வெறுமை Profile(llvm::Folding_Set_முனை_ID &ID) மாறிலி override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  தருக்க isBitvectorSortImpl() மாறிலி override {
    திரும்பு (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  தருக்க isFloatSortImpl() மாறிலி override {
    திரும்பு (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  தருக்க isBooleanSortImpl() மாறிலி override {
    திரும்பு (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() மாறிலி override {
    திரும்பு Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() மாறிலி override {
    திரும்பு Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  தருக்க equal_to(SMTSort மாறிலி &Other) மாறிலி override {
    திரும்பு Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<மாறிலி Z3Sort &>(Other).Sort);
  }

  வெறுமை print(raw_ostream &OS) மாறிலி override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

நிலையான மாறிலி Z3Sort &toZ3Sort(மாறிலி SMTSort &S) {
  திரும்பு static_cast<மாறிலி Z3Sort &>(S);
}

வகுப்பு Z3_கோவை : பொது SMTExpr {
  friend வகுப்பு Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

பொது:
  Z3_கோவை(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_கோவை(மாறிலி Z3_கோவை &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_கோவை &operator=(மாறிலி Z3_கோவை &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    திரும்பு *இது;
  }

  Z3_கோவை(Z3_கோவை &&Other) = delete;
  Z3_கோவை &operator=(Z3_கோவை &&Other) = delete;

  ~Z3_கோவை() {
    எனில் (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  வெறுமை Profile(llvm::Folding_Set_முனை_ID &ID) மாறிலி override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  தருக்க equal_to(SMTExpr மாறிலி &Other) மாறிலி override {
    ப்ரதிஜ்ஞா(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<மாறிலி Z3_கோவை &>(Other).AST)) &&
           "AST's must have the same sort");
    திரும்பு Z3_is_eq_ast(Context.Context, AST,
                        static_cast<மாறிலி Z3_கோவை &>(Other).AST);
  }

  வெறுமை print(raw_ostream &OS) மாறிலி override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

நிலையான மாறிலி Z3_கோவை &to_Z3_கோவை(மாறிலி SMTExpr &E) {
  திரும்பு static_cast<மாறிலி Z3_கோவை &>(E);
}

வகுப்பு Z3Model {
  friend வகுப்பு Z3Solver;

  Z3Context &Context;

  Z3_model Model;

பொது:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(மாறிலி Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    எனில் (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  வெறுமை print(raw_ostream &OS) மாறிலி {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD வெறுமை dump() மாறிலி { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
நிலையான மாறிலி llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  தேர்வு (BitWidth) {
  இயல்பு:
    llvm_unreachable("Unsupported floating-point semantics!");
    நிறுத்து;
  நிலை 16:
    திரும்பு llvm::APFloat::IEEEhalf();
  நிலை 32:
    திரும்பு llvm::APFloat::IEEEsingle();
  நிலை 64:
    திரும்பு llvm::APFloat::IEEEdouble();
  நிலை 128:
    திரும்பு llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
நிலையான தருக்க areEquivalent(மாறிலி llvm::fltSemantics &LHS,
                          மாறிலி llvm::fltSemantics &RHS) {
  திரும்பு (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

வகுப்பு Z3Solver : பொது SMTSolver {
  friend வகுப்பு Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [இது] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    திரும்பு S;
  }();

  Z3_params Params = [இது] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    திரும்பு P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_கோவை> CachedExprs;

பொது:
  Z3Solver() = இயல்பு;
  Z3Solver(மாறிலி Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  வெறுமை addConstraint(மாறிலி SMTExprRef &Exp) மாறிலி override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_கோவை(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(மாறிலி SMTSort &Sort) {
    auto It = CachedSorts.செருகு(toZ3Sort(Sort));
    திரும்பு &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_கோவை_Ref(மாறிலி SMTExpr &Exp) {
    auto It = CachedExprs.செருகு(to_Z3_கோவை(Exp));
    திரும்பு &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    திரும்பு newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    திரும்பு newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(மாறிலி SMTExprRef &Exp) override {
    திரும்பு newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    திரும்பு newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    திரும்பு newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    திரும்பு newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    திரும்பு newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvneg(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkBVNot(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvnot(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkNot(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_not(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvadd(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSub(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvsub(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVMul(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvmul(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvsrem(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVURem(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvurem(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvudiv(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVShl(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvshl(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvashr(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvlshr(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVXor(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvxor(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVOr(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvor(Context.Context, to_Z3_கோவை(*LHS).AST,
                                   to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvand(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvult(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvslt(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvugt(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvsgt(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVUle(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvule(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSle(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvsle(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVUge(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvuge(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVSge(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_bvsge(Context.Context, to_Z3_கோவை(*LHS).AST,
                                    to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkAnd(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_கோவை(*LHS).AST, to_Z3_கோவை(*RHS).AST};
    திரும்பு new_கோவை_Ref(Z3_கோவை(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_கோவை(*LHS).AST, to_Z3_கோவை(*RHS).AST};
    திரும்பு new_கோவை_Ref(Z3_கோவை(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_eq(Context.Context, to_Z3_கோவை(*LHS).AST,
                                 to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkFPMul(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                             to_Z3_கோவை(*LHS).AST, to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                             to_Z3_கோவை(*LHS).AST, to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPRem(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_கோவை(*LHS).AST,
                                      to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                             to_Z3_கோவை(*LHS).AST, to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPSub(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                             to_Z3_கோவை(*LHS).AST, to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPLt(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPGt(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPLe(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_கோவை(*LHS).AST,
                                      to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPGe(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_கோவை(*LHS).AST,
                                      to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkIte(மாறிலி SMTExprRef &Cond, மாறிலி SMTExprRef &T,
                   மாறிலி SMTExprRef &F) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_ite(Context.Context, to_Z3_கோவை(*Cond).AST,
                                  to_Z3_கோவை(*T).AST, to_Z3_கோவை(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_கோவை(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned உயர்வு_2, unsigned கீழ்_2,
                         மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(Context, Z3_mk_extract(Context.Context, உயர்வு_2, கீழ்_2,
                                                    to_Z3_கோவை(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS,
                               தருக்க isSigned) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                         to_Z3_கோவை(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(மாறிலி SMTExprRef &LHS,
                                மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                          to_Z3_கோவை(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(மாறிலி SMTExprRef &LHS,
                               மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                         to_Z3_கோவை(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS,
                                தருக்க isSigned) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                          to_Z3_கோவை(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(மாறிலி SMTExprRef &LHS,
                                மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                          to_Z3_கோவை(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(மாறிலி SMTExprRef &Exp) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_கோவை(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS,
                               தருக்க isSigned) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                         to_Z3_கோவை(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(மாறிலி SMTExprRef &LHS,
                                மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_கோவை(*LHS).AST,
                                          to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(மாறிலி SMTExprRef &LHS, மாறிலி SMTExprRef &RHS) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_concat(Context.Context, to_Z3_கோவை(*LHS).AST,
                                     to_Z3_கோவை(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(மாறிலி SMTExprRef &From, மாறிலி SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                              to_Z3_கோவை(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(மாறிலி SMTExprRef &From, மாறிலி SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                               to_Z3_கோவை(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(மாறிலி SMTExprRef &From, மாறிலி SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                                 to_Z3_கோவை(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(மாறிலி SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                                  to_Z3_கோவை(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(மாறிலி SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_கோவை(*RoundingMode).AST,
                                  to_Z3_கோவை(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(மாறிலி தருக்க b) override {
    திரும்பு new_கோவை_Ref(Z3_கோவை(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(மாறிலி llvm::APSInt Int, unsigned BitWidth) override {
    மாறிலி Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    எனில் (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      திரும்பு new_கோவை_Ref(Z3_கோவை(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    மாறிலி int64_t BitReprAsSigned = Int.get_Ext_மதிப்பு();
    மாறிலி uint64_t BitReprAsUnsigned =
        reinterpret_cast<மாறிலி uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    திரும்பு new_கோவை_Ref(Z3_கோவை(Context, Literal));
  }

  SMTExprRef mkFloat(மாறிலி llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), பொய்);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    திரும்பு new_கோவை_Ref(Z3_கோவை(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_கோவை(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_குறியீடு(மாறிலி எழுத்து *Name, SMTSortRef Sort) override {
    திரும்பு new_கோவை_Ref(
        Z3_கோவை(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_குறியீடு(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(மாறிலி SMTExprRef &Exp, unsigned BitWidth,
                            தருக்க isUnsigned) override {
    திரும்பு llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_கோவை(*Exp).AST),
                    10),
        isUnsigned);
  }

  தருக்க getBoolean(மாறிலி SMTExprRef &Exp) override {
    திரும்பு Z3_get_bool_மதிப்பு(Context.Context, to_Z3_கோவை(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    திரும்பு new_கோவை_Ref(Z3_கோவை(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  தருக்க toAPFloat(மாறிலி SMTSortRef &Sort, மாறிலி SMTExprRef &AST,
                 llvm::APFloat &Float, தருக்க useSemantics) {
    ப்ரதிஜ்ஞா(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), உண்மை);
    மாறிலி llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    எனில் (!toAPSInt(BVSort, AST, Int, உண்மை)) {
      திரும்பு பொய்;
    }

    எனில் (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      ப்ரதிஜ்ஞா(பொய் && "Floating-point types don't match!");
      திரும்பு பொய்;
    }

    Float = llvm::APFloat(Semantics, Int);
    திரும்பு உண்மை;
  }

  தருக்க toAPSInt(மாறிலி SMTSortRef &Sort, மாறிலி SMTExprRef &AST,
                llvm::APSInt &Int, தருக்க useSemantics) {
    எனில் (Sort->isBitvectorSort()) {
      எனில் (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        ப்ரதிஜ்ஞா(பொய் && "Bitvector types don't match!");
        திரும்பு பொய்;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      திரும்பு உண்மை;
    }

    எனில் (Sort->isBooleanSort()) {
      எனில் (useSemantics && Int.getBitWidth() < 1) {
        ப்ரதிஜ்ஞா(பொய் && "Boolean type doesn't match!");
        திரும்பு பொய்;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      திரும்பு உண்மை;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  தருக்க getInterpretation(மாறிலி SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_கோவை(*Exp).AST));
    எனில் (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      திரும்பு பொய்;

    SMTExprRef Assign = new_கோவை_Ref(
        Z3_கோவை(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    திரும்பு toAPSInt(Sort, Assign, Int, உண்மை);
  }

  தருக்க getInterpretation(மாறிலி SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_கோவை(*Exp).AST));
    எனில் (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      திரும்பு பொய்;

    SMTExprRef Assign = new_கோவை_Ref(
        Z3_கோவை(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    திரும்பு toAPFloat(Sort, Assign, Float, உண்மை);
  }

  std::optional<தருக்க> check() மாறிலி override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    எனில் (res == Z3_L_TRUE)
      திரும்பு உண்மை;

    எனில் (res == Z3_L_FALSE)
      திரும்பு பொய்;

    திரும்பு std::nullopt;
  }

  வெறுமை தள்ளு() override { திரும்பு Z3_solver_தள்ளு(Context.Context, Solver); }

  வெறுமை எடு(unsigned இலக்கம்_States = 1) override {
    ப்ரதிஜ்ஞா(Z3_solver_get_இலக்கம்_scopes(Context.Context, Solver) >= இலக்கம்_States);
    திரும்பு Z3_solver_எடு(Context.Context, Solver, இலக்கம்_States);
  }

  தருக்க isFPSupported() override { திரும்பு உண்மை; }

  /// Reset the solver and remove all constraints.
  வெறுமை reset() override { Z3_solver_reset(Context.Context, Solver); }

  வெறுமை print(raw_ostream &OS) மாறிலி override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  வெறுமை setUnsignedParam(StringRef சாவி, unsigned மதிப்பு) override {
    Z3_குறியீடு Sym = Z3_mk_string_குறியீடு(Context.Context, சாவி.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, மதிப்பு);
  }

  வெறுமை setBoolParam(StringRef சாவி, தருக்க மதிப்பு) override {
    Z3_குறியீடு Sym = Z3_mk_string_குறியீடு(Context.Context, சாவி.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, மதிப்பு);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() மாறிலி override;
}; // end class Z3Solver

வகுப்பு Z3Statistics final : பொது SMTSolverStatistics {
பொது:
  இரட்டை getDouble(StringRef சாவி) மாறிலி override {
    auto It = Double_மதிப்புகள்.find(சாவி.str());
    ப்ரதிஜ்ஞா(It != Double_மதிப்புகள்.end());
    திரும்பு It->second;
  };
  unsigned getUnsigned(StringRef சாவி) மாறிலி override {
    auto It = Unsigned_மதிப்புகள்.find(சாவி.str());
    எனில் (It == Unsigned_மதிப்புகள்.end())
      திரும்பு 0;
    திரும்பு It->second;
  };

  வெறுமை print(raw_ostream &OS) மாறிலி override {
    க்கு (மாறிலி auto &E : Unsigned_மதிப்புகள்)
      OS << E.first() << ": " << E.second << '\n';
    க்கு (மாறிலி auto &E : Double_மதிப்புகள்) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

தனிப்பட்ட:
  friend வகுப்பு Z3Solver;
  StringMap<unsigned> Unsigned_மதிப்புகள்;
  StringMap<இரட்டை> Double_மதிப்புகள்;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() மாறிலி {
  auto மாறிலி &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned இலக்கம்_சாவிகள் = Z3_stats_size(C, S);
  க்கு (unsigned Idx = 0; Idx < இலக்கம்_சாவிகள்; ++Idx) {
    மாறிலி எழுத்து *சாவி = Z3_stats_get_சாவி(C, S, Idx);
    எனில் (Z3_stats_is_uint(C, S, Idx)) {
      auto மதிப்பு = Z3_stats_get_uint_மதிப்பு(C, S, Idx);
      Result.Unsigned_மதிப்புகள்.try_emplace(சாவி, மதிப்பு);
    } இல்லையெனில் {
      ப்ரதிஜ்ஞா(Z3_stats_is_double(C, S, Idx));
      auto மதிப்பு = Z3_stats_get_double_மதிப்பு(C, S, Idx);
      Result.Double_மதிப்புகள்.try_emplace(சாவி, மதிப்பு);
    }
  }
  திரும்பு std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#எனில் LLVM_WITH_Z3
  திரும்பு std::make_unique<Z3Solver>();
#இல்லையெனில்
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           பொய்);
  திரும்பு nullptr;
#endif
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை SMTSort::dump() மாறிலி { print(llvm::errs()); }
LLVM_DUMP_METHOD வெறுமை SMTExpr::dump() மாறிலி { print(llvm::errs()); }
LLVM_DUMP_METHOD வெறுமை SMTSolver::dump() மாறிலி { print(llvm::errs()); }
LLVM_DUMP_METHOD வெறுமை SMTSolverStatistics::dump() மாறிலி { print(llvm::errs()); }
#endif
