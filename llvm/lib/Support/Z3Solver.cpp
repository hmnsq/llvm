//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/ScopeExit.h"
#অন্তর্ভাব "llvm/Config/config.h"
#অন্তর্ভাব "llvm/Support/NativeFormatting.h"
#অন্তর্ভাব "llvm/Support/SMTAPI.h"

using নামাকাশ llvm;

#যদি LLVM_WITH_Z3

#অন্তর্ভাব "llvm/ADT/SmallString.h"
#অন্তর্ভাব "llvm/ADT/StringMap.h"
#অন্তর্ভাব "llvm/ADT/Twine.h"

#অন্তর্ভাব <set>

#অন্তর্ভাব <z3.h>

নামাকাশ {

/// Configuration class for Z3
বর্গ Z3Config {
  friend বর্গ Z3Context;

  Z3_config Config = Z3_mk_config();

সার্বজনিক:
  Z3Config() = স্বভাব;
  Z3Config(ধ্রুব Z3Config &) = delete;
  Z3Config(Z3Config &&) = স্বভাব;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = স্বভাব;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
শূন্য Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_বার্তা(Context, Error)));
}

/// Wrapper for Z3 context
বর্গ Z3Context {
সার্বজনিক:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ধ্রুব Z3Context &) = delete;
  Z3Context(Z3Context &&) = স্বভাব;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = স্বভাব;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
বর্গ Z3Sort : সার্বজনিক SMTSort {
  friend বর্গ Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

সার্বজনিক:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ধ্রুব Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ধ্রুব Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    প্রত্যাগম *এতত্;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    যদি (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  শূন্য Profile(llvm::Folding_Set_গ্রন্থি_ID &ID) ধ্রুব override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  তার্কিক isBitvectorSortImpl() ধ্রুব override {
    প্রত্যাগম (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  তার্কিক isFloatSortImpl() ধ্রুব override {
    প্রত্যাগম (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  তার্কিক isBooleanSortImpl() ধ্রুব override {
    প্রত্যাগম (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ধ্রুব override {
    প্রত্যাগম Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ধ্রুব override {
    প্রত্যাগম Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  তার্কিক equal_to(SMTSort ধ্রুব &Other) ধ্রুব override {
    প্রত্যাগম Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ধ্রুব Z3Sort &>(Other).Sort);
  }

  শূন্য print(raw_ostream &OS) ধ্রুব override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

স্থির ধ্রুব Z3Sort &toZ3Sort(ধ্রুব SMTSort &S) {
  প্রত্যাগম static_cast<ধ্রুব Z3Sort &>(S);
}

বর্গ Z3_রাশিমালা : সার্বজনিক SMTExpr {
  friend বর্গ Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

সার্বজনিক:
  Z3_রাশিমালা(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_রাশিমালা(ধ্রুব Z3_রাশিমালা &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_রাশিমালা &operator=(ধ্রুব Z3_রাশিমালা &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    প্রত্যাগম *এতত্;
  }

  Z3_রাশিমালা(Z3_রাশিমালা &&Other) = delete;
  Z3_রাশিমালা &operator=(Z3_রাশিমালা &&Other) = delete;

  ~Z3_রাশিমালা() {
    যদি (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  শূন্য Profile(llvm::Folding_Set_গ্রন্থি_ID &ID) ধ্রুব override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  তার্কিক equal_to(SMTExpr ধ্রুব &Other) ধ্রুব override {
    প্রতিজ্ঞা(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ধ্রুব Z3_রাশিমালা &>(Other).AST)) &&
           "AST's must have the same sort");
    প্রত্যাগম Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ধ্রুব Z3_রাশিমালা &>(Other).AST);
  }

  শূন্য print(raw_ostream &OS) ধ্রুব override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

স্থির ধ্রুব Z3_রাশিমালা &to_Z3_রাশিমালা(ধ্রুব SMTExpr &E) {
  প্রত্যাগম static_cast<ধ্রুব Z3_রাশিমালা &>(E);
}

বর্গ Z3Model {
  friend বর্গ Z3Solver;

  Z3Context &Context;

  Z3_model Model;

সার্বজনিক:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ধ্রুব Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    যদি (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  শূন্য print(raw_ostream &OS) ধ্রুব {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD শূন্য dump() ধ্রুব { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
স্থির ধ্রুব llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  বিকল্প (BitWidth) {
  স্বভাব:
    llvm_unreachable("Unsupported floating-point semantics!");
    ভঙ্গ;
  পক্ষ 16:
    প্রত্যাগম llvm::APFloat::IEEEhalf();
  পক্ষ 32:
    প্রত্যাগম llvm::APFloat::IEEEsingle();
  পক্ষ 64:
    প্রত্যাগম llvm::APFloat::IEEEdouble();
  পক্ষ 128:
    প্রত্যাগম llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
স্থির তার্কিক areEquivalent(ধ্রুব llvm::fltSemantics &LHS,
                          ধ্রুব llvm::fltSemantics &RHS) {
  প্রত্যাগম (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

বর্গ Z3Solver : সার্বজনিক SMTSolver {
  friend বর্গ Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [এতত্] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    প্রত্যাগম S;
  }();

  Z3_params Params = [এতত্] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    প্রত্যাগম P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_রাশিমালা> CachedExprs;

সার্বজনিক:
  Z3Solver() = স্বভাব;
  Z3Solver(ধ্রুব Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  শূন্য addConstraint(ধ্রুব SMTExprRef &Exp) ধ্রুব override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_রাশিমালা(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ধ্রুব SMTSort &Sort) {
    auto It = CachedSorts.ঢোকাও(toZ3Sort(Sort));
    প্রত্যাগম &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_রাশিমালা_Ref(ধ্রুব SMTExpr &Exp) {
    auto It = CachedExprs.ঢোকাও(to_Z3_রাশিমালা(Exp));
    প্রত্যাগম &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    প্রত্যাগম newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    প্রত্যাগম newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    প্রত্যাগম newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    প্রত্যাগম newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    প্রত্যাগম newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    প্রত্যাগম newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvneg(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvnot(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkNot(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_not(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvadd(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsub(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvmul(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsrem(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvurem(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvudiv(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvshl(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvashr(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvlshr(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvxor(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvor(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                   to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvand(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvult(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvslt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvugt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsgt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvule(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsle(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvuge(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsge(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkAnd(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST};
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST};
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_eq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                 to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                      to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                      to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                      to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkIte(ধ্রুব SMTExprRef &Cond, ধ্রুব SMTExprRef &T,
                   ধ্রুব SMTExprRef &F) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_ite(Context.Context, to_Z3_রাশিমালা(*Cond).AST,
                                  to_Z3_রাশিমালা(*T).AST, to_Z3_রাশিমালা(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned উপরের_2, unsigned নিচের_2,
                         ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_extract(Context.Context, উপরের_2, নিচের_2,
                                                    to_Z3_রাশিমালা(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS,
                               তার্কিক isSigned) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                         to_Z3_রাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ধ্রুব SMTExprRef &LHS,
                                ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ধ্রুব SMTExprRef &LHS,
                               ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                         to_Z3_রাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS,
                                তার্কিক isSigned) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ধ্রুব SMTExprRef &LHS,
                                ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS,
                               তার্কিক isSigned) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                         to_Z3_রাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ধ্রুব SMTExprRef &LHS,
                                ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ধ্রুব SMTExprRef &LHS, ধ্রুব SMTExprRef &RHS) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_concat(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ধ্রুব SMTExprRef &From, ধ্রুব SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                              to_Z3_রাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ধ্রুব SMTExprRef &From, ধ্রুব SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                               to_Z3_রাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ধ্রুব SMTExprRef &From, ধ্রুব SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                                 to_Z3_রাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ধ্রুব SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                                  to_Z3_রাশিমালা(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ধ্রুব SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                                  to_Z3_রাশিমালা(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ধ্রুব তার্কিক b) override {
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ধ্রুব llvm::APSInt Int, unsigned BitWidth) override {
    ধ্রুব Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    যদি (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ধ্রুব int64_t BitReprAsSigned = Int.get_Ext_মান();
    ধ্রুব uint64_t BitReprAsUnsigned =
        reinterpret_cast<ধ্রুব uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Literal));
  }

  SMTExprRef mkFloat(ধ্রুব llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), অসত্য);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_রাশিমালা(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_প্রতীক(ধ্রুব বর্ণ *Name, SMTSortRef Sort) override {
    প্রত্যাগম new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_প্রতীক(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ধ্রুব SMTExprRef &Exp, unsigned BitWidth,
                            তার্কিক isUnsigned) override {
    প্রত্যাগম llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_রাশিমালা(*Exp).AST),
                    10),
        isUnsigned);
  }

  তার্কিক getBoolean(ধ্রুব SMTExprRef &Exp) override {
    প্রত্যাগম Z3_get_bool_মান(Context.Context, to_Z3_রাশিমালা(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    প্রত্যাগম new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  তার্কিক toAPFloat(ধ্রুব SMTSortRef &Sort, ধ্রুব SMTExprRef &AST,
                 llvm::APFloat &Float, তার্কিক useSemantics) {
    প্রতিজ্ঞা(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), সত্য);
    ধ্রুব llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    যদি (!toAPSInt(BVSort, AST, Int, সত্য)) {
      প্রত্যাগম অসত্য;
    }

    যদি (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      প্রতিজ্ঞা(অসত্য && "Floating-point types don't match!");
      প্রত্যাগম অসত্য;
    }

    Float = llvm::APFloat(Semantics, Int);
    প্রত্যাগম সত্য;
  }

  তার্কিক toAPSInt(ধ্রুব SMTSortRef &Sort, ধ্রুব SMTExprRef &AST,
                llvm::APSInt &Int, তার্কিক useSemantics) {
    যদি (Sort->isBitvectorSort()) {
      যদি (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        প্রতিজ্ঞা(অসত্য && "Bitvector types don't match!");
        প্রত্যাগম অসত্য;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      প্রত্যাগম সত্য;
    }

    যদি (Sort->isBooleanSort()) {
      যদি (useSemantics && Int.getBitWidth() < 1) {
        প্রতিজ্ঞা(অসত্য && "Boolean type doesn't match!");
        প্রত্যাগম অসত্য;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      প্রত্যাগম সত্য;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  তার্কিক getInterpretation(ধ্রুব SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_রাশিমালা(*Exp).AST));
    যদি (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      প্রত্যাগম অসত্য;

    SMTExprRef Assign = new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    প্রত্যাগম toAPSInt(Sort, Assign, Int, সত্য);
  }

  তার্কিক getInterpretation(ধ্রুব SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_রাশিমালা(*Exp).AST));
    যদি (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      প্রত্যাগম অসত্য;

    SMTExprRef Assign = new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    প্রত্যাগম toAPFloat(Sort, Assign, Float, সত্য);
  }

  std::optional<তার্কিক> check() ধ্রুব override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    যদি (res == Z3_L_TRUE)
      প্রত্যাগম সত্য;

    যদি (res == Z3_L_FALSE)
      প্রত্যাগম অসত্য;

    প্রত্যাগম std::nullopt;
  }

  শূন্য ঠেলো() override { প্রত্যাগম Z3_solver_ঠেলো(Context.Context, Solver); }

  শূন্য বের_করো(unsigned অঙ্ক_States = 1) override {
    প্রতিজ্ঞা(Z3_solver_get_অঙ্ক_scopes(Context.Context, Solver) >= অঙ্ক_States);
    প্রত্যাগম Z3_solver_বের_করো(Context.Context, Solver, অঙ্ক_States);
  }

  তার্কিক isFPSupported() override { প্রত্যাগম সত্য; }

  /// Reset the solver and remove all constraints.
  শূন্য reset() override { Z3_solver_reset(Context.Context, Solver); }

  শূন্য print(raw_ostream &OS) ধ্রুব override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  শূন্য setUnsignedParam(StringRef চাবি, unsigned মান) override {
    Z3_প্রতীক Sym = Z3_mk_string_প্রতীক(Context.Context, চাবি.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, মান);
  }

  শূন্য setBoolParam(StringRef চাবি, তার্কিক মান) override {
    Z3_প্রতীক Sym = Z3_mk_string_প্রতীক(Context.Context, চাবি.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, মান);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ধ্রুব override;
}; // end class Z3Solver

বর্গ Z3Statistics final : সার্বজনিক SMTSolverStatistics {
সার্বজনিক:
  দ্বিগুণ getDouble(StringRef চাবি) ধ্রুব override {
    auto It = Double_মানসমূহ.find(চাবি.str());
    প্রতিজ্ঞা(It != Double_মানসমূহ.end());
    প্রত্যাগম It->second;
  };
  unsigned getUnsigned(StringRef চাবি) ধ্রুব override {
    auto It = Unsigned_মানসমূহ.find(চাবি.str());
    যদি (It == Unsigned_মানসমূহ.end())
      প্রত্যাগম 0;
    প্রত্যাগম It->second;
  };

  শূন্য print(raw_ostream &OS) ধ্রুব override {
    প্রতি (ধ্রুব auto &E : Unsigned_মানসমূহ)
      OS << E.first() << ": " << E.second << '\n';
    প্রতি (ধ্রুব auto &E : Double_মানসমূহ) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

গুপ্ত:
  friend বর্গ Z3Solver;
  StringMap<unsigned> Unsigned_মানসমূহ;
  StringMap<দ্বিগুণ> Double_মানসমূহ;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ধ্রুব {
  auto ধ্রুব &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned অঙ্ক_চাবিগুলি = Z3_stats_size(C, S);
  প্রতি (unsigned Idx = 0; Idx < অঙ্ক_চাবিগুলি; ++Idx) {
    ধ্রুব বর্ণ *চাবি = Z3_stats_get_চাবি(C, S, Idx);
    যদি (Z3_stats_is_uint(C, S, Idx)) {
      auto মান = Z3_stats_get_uint_মান(C, S, Idx);
      Result.Unsigned_মানসমূহ.try_emplace(চাবি, মান);
    } অন্যথা {
      প্রতিজ্ঞা(Z3_stats_is_double(C, S, Idx));
      auto মান = Z3_stats_get_double_মান(C, S, Idx);
      Result.Double_মানসমূহ.try_emplace(চাবি, মান);
    }
  }
  প্রত্যাগম std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#যদি LLVM_WITH_Z3
  প্রত্যাগম std::make_unique<Z3Solver>();
#অন্যথা
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           অসত্য);
  প্রত্যাগম nullptr;
#endif
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য SMTSort::dump() ধ্রুব { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTExpr::dump() ধ্রুব { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTSolver::dump() ধ্রুব { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTSolverStatistics::dump() ধ্রুব { print(llvm::errs()); }
#endif
