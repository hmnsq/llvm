//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/ADT/ScopeExit.h"
#অন্তর্ভাৱ "llvm/Config/config.h"
#অন্তর্ভাৱ "llvm/Support/NativeFormatting.h"
#অন্তর্ভাৱ "llvm/Support/SMTAPI.h"

using নামাকাশ llvm;

#যদি LLVM_WITH_Z3

#অন্তর্ভাৱ "llvm/ADT/SmallString.h"
#অন্তর্ভাৱ "llvm/ADT/StringMap.h"
#অন্তর্ভাৱ "llvm/ADT/Twine.h"

#অন্তর্ভাৱ <set>

#অন্তর্ভাৱ <z3.h>

নামাকাশ {

/// Configuration class for Z3
শ্ৰেণী Z3Config {
  friend শ্ৰেণী Z3Context;

  Z3_config Config = Z3_mk_config();

সার্ৱজনিক:
  Z3Config() = স্ৱভাৱ;
  Z3Config(ধ্রুৱ Z3Config &) = delete;
  Z3Config(Z3Config &&) = স্ৱভাৱ;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = স্ৱভাৱ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
শূন্য Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_বাৰ্তা(Context, Error)));
}

/// Wrapper for Z3 context
শ্ৰেণী Z3Context {
সার্ৱজনিক:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ধ্রুৱ Z3Context &) = delete;
  Z3Context(Z3Context &&) = স্ৱভাৱ;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = স্ৱভাৱ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
শ্ৰেণী Z3Sort : সার্ৱজনিক SMTSort {
  friend শ্ৰেণী Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

সার্ৱজনিক:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ধ্রুৱ Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ধ্রুৱ Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    ঘূৰাই *এতত্;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    যদি (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  শূন্য Profile(llvm::Folding_Set_গ্ৰন্থি_ID &ID) ধ্রুৱ override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  তার্কিক isBitvectorSortImpl() ধ্রুৱ override {
    ঘূৰাই (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  তার্কিক isFloatSortImpl() ধ্রুৱ override {
    ঘূৰাই (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  তার্কিক isBooleanSortImpl() ধ্রুৱ override {
    ঘূৰাই (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ধ্রুৱ override {
    ঘূৰাই Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ধ্রুৱ override {
    ঘূৰাই Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  তার্কিক equal_to(SMTSort ধ্রুৱ &Other) ধ্রুৱ override {
    ঘূৰাই Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ধ্রুৱ Z3Sort &>(Other).Sort);
  }

  শূন্য print(raw_ostream &OS) ধ্রুৱ override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

স্থির ধ্রুৱ Z3Sort &toZ3Sort(ধ্রুৱ SMTSort &S) {
  ঘূৰাই static_cast<ধ্রুৱ Z3Sort &>(S);
}

শ্ৰেণী Z3_ৰাশিমালা : সার্ৱজনিক SMTExpr {
  friend শ্ৰেণী Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

সার্ৱজনিক:
  Z3_ৰাশিমালা(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ৰাশিমালা(ধ্রুৱ Z3_ৰাশিমালা &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ৰাশিমালা &operator=(ধ্রুৱ Z3_ৰাশিমালা &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    ঘূৰাই *এতত্;
  }

  Z3_ৰাশিমালা(Z3_ৰাশিমালা &&Other) = delete;
  Z3_ৰাশিমালা &operator=(Z3_ৰাশিমালা &&Other) = delete;

  ~Z3_ৰাশিমালা() {
    যদি (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  শূন্য Profile(llvm::Folding_Set_গ্ৰন্থি_ID &ID) ধ্রুৱ override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  তার্কিক equal_to(SMTExpr ধ্রুৱ &Other) ধ্রুৱ override {
    প্রতিজ্ঞা(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ধ্রুৱ Z3_ৰাশিমালা &>(Other).AST)) &&
           "AST's must have the same sort");
    ঘূৰাই Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ধ্রুৱ Z3_ৰাশিমালা &>(Other).AST);
  }

  শূন্য print(raw_ostream &OS) ধ্রুৱ override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

স্থির ধ্রুৱ Z3_ৰাশিমালা &to_Z3_ৰাশিমালা(ধ্রুৱ SMTExpr &E) {
  ঘূৰাই static_cast<ধ্রুৱ Z3_ৰাশিমালা &>(E);
}

শ্ৰেণী Z3Model {
  friend শ্ৰেণী Z3Solver;

  Z3Context &Context;

  Z3_model Model;

সার্ৱজনিক:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ধ্রুৱ Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    যদি (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  শূন্য print(raw_ostream &OS) ধ্রুৱ {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD শূন্য dump() ধ্রুৱ { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
স্থির ধ্রুৱ llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  ৱিকল্প (BitWidth) {
  স্ৱভাৱ:
    llvm_unreachable("Unsupported floating-point semantics!");
    ভঙ্গ;
  পক্ষ 16:
    ঘূৰাই llvm::APFloat::IEEEhalf();
  পক্ষ 32:
    ঘূৰাই llvm::APFloat::IEEEsingle();
  পক্ষ 64:
    ঘূৰাই llvm::APFloat::IEEEdouble();
  পক্ষ 128:
    ঘূৰাই llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
স্থির তার্কিক areEquivalent(ধ্রুৱ llvm::fltSemantics &LHS,
                          ধ্রুৱ llvm::fltSemantics &RHS) {
  ঘূৰাই (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

শ্ৰেণী Z3Solver : সার্ৱজনিক SMTSolver {
  friend শ্ৰেণী Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [এতত্] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    ঘূৰাই S;
  }();

  Z3_params Params = [এতত্] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    ঘূৰাই P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_ৰাশিমালা> CachedExprs;

সার্ৱজনিক:
  Z3Solver() = স্ৱভাৱ;
  Z3Solver(ধ্রুৱ Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  শূন্য addConstraint(ধ্রুৱ SMTExprRef &Exp) ধ্রুৱ override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_ৰাশিমালা(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ধ্রুৱ SMTSort &Sort) {
    auto It = CachedSorts.ঢোকাও(toZ3Sort(Sort));
    ঘূৰাই &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_ৰাশিমালা_Ref(ধ্রুৱ SMTExpr &Exp) {
    auto It = CachedExprs.ঢোকাও(to_Z3_ৰাশিমালা(Exp));
    ঘূৰাই &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    ঘূৰাই newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    ঘূৰাই newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    ঘূৰাই newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    ঘূৰাই newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    ঘূৰাই newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    ঘূৰাই newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvneg(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvnot(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkNot(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_not(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvadd(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvsub(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvmul(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvsrem(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvurem(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvudiv(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvshl(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvashr(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvlshr(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvxor(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvor(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                   to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvand(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvult(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvslt(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvugt(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvsgt(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvule(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvsle(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvuge(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_bvsge(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                    to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkAnd(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_ৰাশিমালা(*LHS).AST, to_Z3_ৰাশিমালা(*RHS).AST};
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_ৰাশিমালা(*LHS).AST, to_Z3_ৰাশিমালা(*RHS).AST};
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_eq(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                 to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                             to_Z3_ৰাশিমালা(*LHS).AST, to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                             to_Z3_ৰাশিমালা(*LHS).AST, to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                      to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                             to_Z3_ৰাশিমালা(*LHS).AST, to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                             to_Z3_ৰাশিমালা(*LHS).AST, to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                      to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                      to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkIte(ধ্রুৱ SMTExprRef &Cond, ধ্রুৱ SMTExprRef &T,
                   ধ্রুৱ SMTExprRef &F) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_ite(Context.Context, to_Z3_ৰাশিমালা(*Cond).AST,
                                  to_Z3_ৰাশিমালা(*T).AST, to_Z3_ৰাশিমালা(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned উপৰেৰ_2, unsigned নিচেৰ_2,
                         ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(Context, Z3_mk_extract(Context.Context, উপৰেৰ_2, নিচেৰ_2,
                                                    to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS,
                               তার্কিক isSigned) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                         to_Z3_ৰাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ধ্রুৱ SMTExprRef &LHS,
                                ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                          to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ধ্রুৱ SMTExprRef &LHS,
                               ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                         to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS,
                                তার্কিক isSigned) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                          to_Z3_ৰাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ধ্রুৱ SMTExprRef &LHS,
                                ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                          to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS,
                               তার্কিক isSigned) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                         to_Z3_ৰাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ধ্রুৱ SMTExprRef &LHS,
                                ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                          to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ধ্রুৱ SMTExprRef &LHS, ধ্রুৱ SMTExprRef &RHS) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_concat(Context.Context, to_Z3_ৰাশিমালা(*LHS).AST,
                                     to_Z3_ৰাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ধ্রুৱ SMTExprRef &From, ধ্রুৱ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                              to_Z3_ৰাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ধ্রুৱ SMTExprRef &From, ধ্রুৱ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                               to_Z3_ৰাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ধ্রুৱ SMTExprRef &From, ধ্রুৱ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                                 to_Z3_ৰাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ধ্রুৱ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                                  to_Z3_ৰাশিমালা(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ধ্রুৱ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_ৰাশিমালা(*RoundingMode).AST,
                                  to_Z3_ৰাশিমালা(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ধ্রুৱ তার্কিক b) override {
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ধ্রুৱ llvm::APSInt Int, unsigned BitWidth) override {
    ধ্রুৱ Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    যদি (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ধ্রুৱ int64_t BitReprAsSigned = Int.get_Ext_মান();
    ধ্রুৱ uint64_t BitReprAsUnsigned =
        reinterpret_cast<ধ্রুৱ uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(Context, Literal));
  }

  SMTExprRef mkFloat(ধ্রুৱ llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), মিছা);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_ৰাশিমালা(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_প্ৰতীক(ধ্রুৱ ৱর্ণ *Name, SMTSortRef Sort) override {
    ঘূৰাই new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_প্ৰতীক(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ধ্রুৱ SMTExprRef &Exp, unsigned BitWidth,
                            তার্কিক isUnsigned) override {
    ঘূৰাই llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST),
                    10),
        isUnsigned);
  }

  তার্কিক getBoolean(ধ্রুৱ SMTExprRef &Exp) override {
    ঘূৰাই Z3_get_bool_মান(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    ঘূৰাই new_ৰাশিমালা_Ref(Z3_ৰাশিমালা(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  তার্কিক toAPFloat(ধ্রুৱ SMTSortRef &Sort, ধ্রুৱ SMTExprRef &AST,
                 llvm::APFloat &Float, তার্কিক useSemantics) {
    প্রতিজ্ঞা(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), সঁচা);
    ধ্রুৱ llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    যদি (!toAPSInt(BVSort, AST, Int, সঁচা)) {
      ঘূৰাই মিছা;
    }

    যদি (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      প্রতিজ্ঞা(মিছা && "Floating-point types don't match!");
      ঘূৰাই মিছা;
    }

    Float = llvm::APFloat(Semantics, Int);
    ঘূৰাই সঁচা;
  }

  তার্কিক toAPSInt(ধ্রুৱ SMTSortRef &Sort, ধ্রুৱ SMTExprRef &AST,
                llvm::APSInt &Int, তার্কিক useSemantics) {
    যদি (Sort->isBitvectorSort()) {
      যদি (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        প্রতিজ্ঞা(মিছা && "Bitvector types don't match!");
        ঘূৰাই মিছা;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      ঘূৰাই সঁচা;
    }

    যদি (Sort->isBooleanSort()) {
      যদি (useSemantics && Int.getBitWidth() < 1) {
        প্রতিজ্ঞা(মিছা && "Boolean type doesn't match!");
        ঘূৰাই মিছা;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      ঘূৰাই সঁচা;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  তার্কিক getInterpretation(ধ্রুৱ SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST));
    যদি (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ঘূৰাই মিছা;

    SMTExprRef Assign = new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ঘূৰাই toAPSInt(Sort, Assign, Int, সঁচা);
  }

  তার্কিক getInterpretation(ধ্রুৱ SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_ৰাশিমালা(*Exp).AST));
    যদি (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ঘূৰাই মিছা;

    SMTExprRef Assign = new_ৰাশিমালা_Ref(
        Z3_ৰাশিমালা(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ঘূৰাই toAPFloat(Sort, Assign, Float, সঁচা);
  }

  std::optional<তার্কিক> check() ধ্রুৱ override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    যদি (res == Z3_L_TRUE)
      ঘূৰাই সঁচা;

    যদি (res == Z3_L_FALSE)
      ঘূৰাই মিছা;

    ঘূৰাই std::nullopt;
  }

  শূন্য ঠেলো() override { ঘূৰাই Z3_solver_ঠেলো(Context.Context, Solver); }

  শূন্য বেৰ_কৰো(unsigned অঙ্ক_States = 1) override {
    প্রতিজ্ঞা(Z3_solver_get_অঙ্ক_scopes(Context.Context, Solver) >= অঙ্ক_States);
    ঘূৰাই Z3_solver_বেৰ_কৰো(Context.Context, Solver, অঙ্ক_States);
  }

  তার্কিক isFPSupported() override { ঘূৰাই সঁচা; }

  /// Reset the solver and remove all constraints.
  শূন্য reset() override { Z3_solver_reset(Context.Context, Solver); }

  শূন্য print(raw_ostream &OS) ধ্রুৱ override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  শূন্য setUnsignedParam(StringRef চাবি, unsigned মান) override {
    Z3_প্ৰতীক Sym = Z3_mk_string_প্ৰতীক(Context.Context, চাবি.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, মান);
  }

  শূন্য setBoolParam(StringRef চাবি, তার্কিক মান) override {
    Z3_প্ৰতীক Sym = Z3_mk_string_প্ৰতীক(Context.Context, চাবি.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, মান);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ধ্রুৱ override;
}; // end class Z3Solver

শ্ৰেণী Z3Statistics final : সার্ৱজনিক SMTSolverStatistics {
সার্ৱজনিক:
  দ্ৱিগুণ getDouble(StringRef চাবি) ধ্রুৱ override {
    auto It = Double_মানসমূহ.find(চাবি.str());
    প্রতিজ্ঞা(It != Double_মানসমূহ.end());
    ঘূৰাই It->second;
  };
  unsigned getUnsigned(StringRef চাবি) ধ্রুৱ override {
    auto It = Unsigned_মানসমূহ.find(চাবি.str());
    যদি (It == Unsigned_মানসমূহ.end())
      ঘূৰাই 0;
    ঘূৰাই It->second;
  };

  শূন্য print(raw_ostream &OS) ধ্রুৱ override {
    বাবে (ধ্রুৱ auto &E : Unsigned_মানসমূহ)
      OS << E.first() << ": " << E.second << '\n';
    বাবে (ধ্রুৱ auto &E : Double_মানসমূহ) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

গুপ্ত:
  friend শ্ৰেণী Z3Solver;
  StringMap<unsigned> Unsigned_মানসমূহ;
  StringMap<দ্ৱিগুণ> Double_মানসমূহ;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ধ্রুৱ {
  auto ধ্রুৱ &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned অঙ্ক_চাবিগুলি = Z3_stats_size(C, S);
  বাবে (unsigned Idx = 0; Idx < অঙ্ক_চাবিগুলি; ++Idx) {
    ধ্রুৱ ৱর্ণ *চাবি = Z3_stats_get_চাবি(C, S, Idx);
    যদি (Z3_stats_is_uint(C, S, Idx)) {
      auto মান = Z3_stats_get_uint_মান(C, S, Idx);
      Result.Unsigned_মানসমূহ.try_emplace(চাবি, মান);
    } অন্যথা {
      প্রতিজ্ঞা(Z3_stats_is_double(C, S, Idx));
      auto মান = Z3_stats_get_double_মান(C, S, Idx);
      Result.Double_মানসমূহ.try_emplace(চাবি, মান);
    }
  }
  ঘূৰাই std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#যদি LLVM_WITH_Z3
  ঘূৰাই std::make_unique<Z3Solver>();
#অন্যথা
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           মিছা);
  ঘূৰাই nullptr;
#endif
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য SMTSort::dump() ধ্রুৱ { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTExpr::dump() ধ্রুৱ { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTSolver::dump() ধ্রুৱ { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTSolverStatistics::dump() ধ্রুৱ { print(llvm::errs()); }
#endif
