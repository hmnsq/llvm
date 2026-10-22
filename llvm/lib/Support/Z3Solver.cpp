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
শ্রেণী Z3Config {
  friend শ্রেণী Z3Context;

  Z3_config Config = Z3_mk_config();

সর্বজনীন:
  Z3Config() = পূর্বনির্ধারিত;
  Z3Config(ধ্রুবক Z3Config &) = delete;
  Z3Config(Z3Config &&) = পূর্বনির্ধারিত;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = পূর্বনির্ধারিত;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
শূন্য Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_বার্তা(Context, Error)));
}

/// Wrapper for Z3 context
শ্রেণী Z3Context {
সর্বজনীন:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ধ্রুবক Z3Context &) = delete;
  Z3Context(Z3Context &&) = পূর্বনির্ধারিত;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = পূর্বনির্ধারিত;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
শ্রেণী Z3Sort : সর্বজনীন SMTSort {
  friend শ্রেণী Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

সর্বজনীন:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ধ্রুবক Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ধ্রুবক Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    ফেরত *এই;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    যদি (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  শূন্য Profile(llvm::Folding_Set_গ্রন্থি_ID &ID) ধ্রুবক override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  যৌক্তিক isBitvectorSortImpl() ধ্রুবক override {
    ফেরত (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  যৌক্তিক isFloatSortImpl() ধ্রুবক override {
    ফেরত (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  যৌক্তিক isBooleanSortImpl() ধ্রুবক override {
    ফেরত (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned get_Bitvector_Sort_আকার_Impl() ধ্রুবক override {
    ফেরত Z3_get_bv_sort_আকার(Context.Context, Sort);
  }

  unsigned get_Float_Sort_আকার_Impl() ধ্রুবক override {
    ফেরত Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  যৌক্তিক equal_to(SMTSort ধ্রুবক &Other) ধ্রুবক override {
    ফেরত Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ধ্রুবক Z3Sort &>(Other).Sort);
  }

  শূন্য print(raw_ostream &OS) ধ্রুবক override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

স্থির ধ্রুবক Z3Sort &toZ3Sort(ধ্রুবক SMTSort &S) {
  ফেরত static_cast<ধ্রুবক Z3Sort &>(S);
}

শ্রেণী Z3_রাশিমালা : সর্বজনীন SMTExpr {
  friend শ্রেণী Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

সর্বজনীন:
  Z3_রাশিমালা(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_রাশিমালা(ধ্রুবক Z3_রাশিমালা &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_রাশিমালা &operator=(ধ্রুবক Z3_রাশিমালা &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    ফেরত *এই;
  }

  Z3_রাশিমালা(Z3_রাশিমালা &&Other) = delete;
  Z3_রাশিমালা &operator=(Z3_রাশিমালা &&Other) = delete;

  ~Z3_রাশিমালা() {
    যদি (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  শূন্য Profile(llvm::Folding_Set_গ্রন্থি_ID &ID) ধ্রুবক override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  যৌক্তিক equal_to(SMTExpr ধ্রুবক &Other) ধ্রুবক override {
    প্রতিজ্ঞা(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ধ্রুবক Z3_রাশিমালা &>(Other).AST)) &&
           "AST's must have the same sort");
    ফেরত Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ধ্রুবক Z3_রাশিমালা &>(Other).AST);
  }

  শূন্য print(raw_ostream &OS) ধ্রুবক override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

স্থির ধ্রুবক Z3_রাশিমালা &to_Z3_রাশিমালা(ধ্রুবক SMTExpr &E) {
  ফেরত static_cast<ধ্রুবক Z3_রাশিমালা &>(E);
}

শ্রেণী Z3Model {
  friend শ্রেণী Z3Solver;

  Z3Context &Context;

  Z3_model Model;

সর্বজনীন:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ধ্রুবক Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    যদি (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  শূন্য print(raw_ostream &OS) ধ্রুবক {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD শূন্য dump() ধ্রুবক { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
স্থির ধ্রুবক llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  নির্বাচন (BitWidth) {
  পূর্বনির্ধারিত:
    llvm_unreachable("Unsupported floating-point semantics!");
    বিরতি;
  ক্ষেত্র 16:
    ফেরত llvm::APFloat::IEEEhalf();
  ক্ষেত্র 32:
    ফেরত llvm::APFloat::IEEEsingle();
  ক্ষেত্র 64:
    ফেরত llvm::APFloat::IEEEdouble();
  ক্ষেত্র 128:
    ফেরত llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
স্থির যৌক্তিক areEquivalent(ধ্রুবক llvm::fltSemantics &LHS,
                          ধ্রুবক llvm::fltSemantics &RHS) {
  ফেরত (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_আকার_In_Bits(LHS) ==
          llvm::APFloat::semantics_আকার_In_Bits(RHS));
}

শ্রেণী Z3Solver : সর্বজনীন SMTSolver {
  friend শ্রেণী Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [এই] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    ফেরত S;
  }();

  Z3_params Params = [এই] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    ফেরত P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_রাশিমালা> CachedExprs;

সর্বজনীন:
  Z3Solver() = পূর্বনির্ধারিত;
  Z3Solver(ধ্রুবক Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  শূন্য addConstraint(ধ্রুবক SMTExprRef &Exp) ধ্রুবক override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_রাশিমালা(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ধ্রুবক SMTSort &Sort) {
    auto It = CachedSorts.ঢোকাও(toZ3Sort(Sort));
    ফেরত &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_রাশিমালা_Ref(ধ্রুবক SMTExpr &Exp) {
    auto It = CachedExprs.ঢোকাও(to_Z3_রাশিমালা(Exp));
    ফেরত &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    ফেরত newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    ফেরত newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    ফেরত newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    ফেরত newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    ফেরত newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    ফেরত newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvneg(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvnot(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkNot(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_not(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvadd(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsub(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvmul(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsrem(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvurem(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvudiv(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvshl(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvashr(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvlshr(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvxor(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvor(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                   to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvand(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvult(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvslt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvugt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsgt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvule(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsle(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvuge(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_bvsge(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                    to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkAnd(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST};
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST};
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_eq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                 to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                      to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                             to_Z3_রাশিমালা(*LHS).AST, to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                      to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                      to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkIte(ধ্রুবক SMTExprRef &Cond, ধ্রুবক SMTExprRef &T,
                   ধ্রুবক SMTExprRef &F) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_ite(Context.Context, to_Z3_রাশিমালা(*Cond).AST,
                                  to_Z3_রাশিমালা(*T).AST, to_Z3_রাশিমালা(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_রাশিমালা(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned উপরের_2, unsigned নিচের_2,
                         ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_extract(Context.Context, উপরের_2, নিচের_2,
                                                    to_Z3_রাশিমালা(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS,
                               যৌক্তিক isSigned) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                         to_Z3_রাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ধ্রুবক SMTExprRef &LHS,
                                ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ধ্রুবক SMTExprRef &LHS,
                               ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                         to_Z3_রাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS,
                                যৌক্তিক isSigned) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ধ্রুবক SMTExprRef &LHS,
                                ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_রাশিমালা(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS,
                               যৌক্তিক isSigned) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                         to_Z3_রাশিমালা(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ধ্রুবক SMTExprRef &LHS,
                                ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                          to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ধ্রুবক SMTExprRef &LHS, ধ্রুবক SMTExprRef &RHS) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_concat(Context.Context, to_Z3_রাশিমালা(*LHS).AST,
                                     to_Z3_রাশিমালা(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ধ্রুবক SMTExprRef &From, ধ্রুবক SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                              to_Z3_রাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ধ্রুবক SMTExprRef &From, ধ্রুবক SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                               to_Z3_রাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ধ্রুবক SMTExprRef &From, ধ্রুবক SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                                 to_Z3_রাশিমালা(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ধ্রুবক SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                                  to_Z3_রাশিমালা(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ধ্রুবক SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_রাশিমালা(*RoundingMode).AST,
                                  to_Z3_রাশিমালা(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ধ্রুবক যৌক্তিক b) override {
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ধ্রুবক llvm::APSInt Int, unsigned BitWidth) override {
    ধ্রুবক Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    যদি (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> বাফার;
      Int.toString(বাফার, 10);
      ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
          Context, Z3_mk_numeral(Context.Context, বাফার.c_str(), Z3Sort)));
    }

    ধ্রুবক int64_t BitReprAsSigned = Int.get_Ext_মান();
    ধ্রুবক uint64_t BitReprAsUnsigned =
        reinterpret_cast<ধ্রুবক uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Literal));
  }

  SMTExprRef mkFloat(ধ্রুবক llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semantics_আকার_In_Bits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), মিথ্যা);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_রাশিমালা(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_প্রতীক(ধ্রুবক অক্ষর *Name, SMTSortRef Sort) override {
    ফেরত new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_প্রতীক(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ধ্রুবক SMTExprRef &Exp, unsigned BitWidth,
                            যৌক্তিক isUnsigned) override {
    ফেরত llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_রাশিমালা(*Exp).AST),
                    10),
        isUnsigned);
  }

  যৌক্তিক getBoolean(ধ্রুবক SMTExprRef &Exp) override {
    ফেরত Z3_get_bool_মান(Context.Context, to_Z3_রাশিমালা(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    ফেরত new_রাশিমালা_Ref(Z3_রাশিমালা(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  যৌক্তিক toAPFloat(ধ্রুবক SMTSortRef &Sort, ধ্রুবক SMTExprRef &AST,
                 llvm::APFloat &Float, যৌক্তিক useSemantics) {
    প্রতিজ্ঞা(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->get_Float_Sort_আকার(), সত্য);
    ধ্রুবক llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->get_Float_Sort_আকার());
    SMTSortRef BVSort = getBitvectorSort(Sort->get_Float_Sort_আকার());
    যদি (!toAPSInt(BVSort, AST, Int, সত্য)) {
      ফেরত মিথ্যা;
    }

    যদি (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      প্রতিজ্ঞা(মিথ্যা && "Floating-point types don't match!");
      ফেরত মিথ্যা;
    }

    Float = llvm::APFloat(Semantics, Int);
    ফেরত সত্য;
  }

  যৌক্তিক toAPSInt(ধ্রুবক SMTSortRef &Sort, ধ্রুবক SMTExprRef &AST,
                llvm::APSInt &Int, যৌক্তিক useSemantics) {
    যদি (Sort->isBitvectorSort()) {
      যদি (useSemantics && Int.getBitWidth() != Sort->get_Bitvector_Sort_আকার()) {
        প্রতিজ্ঞা(মিথ্যা && "Bitvector types don't match!");
        ফেরত মিথ্যা;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      ফেরত সত্য;
    }

    যদি (Sort->isBooleanSort()) {
      যদি (useSemantics && Int.getBitWidth() < 1) {
        প্রতিজ্ঞা(মিথ্যা && "Boolean type doesn't match!");
        ফেরত মিথ্যা;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      ফেরত সত্য;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  যৌক্তিক getInterpretation(ধ্রুবক SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_রাশিমালা(*Exp).AST));
    যদি (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ফেরত মিথ্যা;

    SMTExprRef Assign = new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ফেরত toAPSInt(Sort, Assign, Int, সত্য);
  }

  যৌক্তিক getInterpretation(ধ্রুবক SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_রাশিমালা(*Exp).AST));
    যদি (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ফেরত মিথ্যা;

    SMTExprRef Assign = new_রাশিমালা_Ref(
        Z3_রাশিমালা(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ফেরত toAPFloat(Sort, Assign, Float, সত্য);
  }

  std::optional<যৌক্তিক> check() ধ্রুবক override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    যদি (res == Z3_L_TRUE)
      ফেরত সত্য;

    যদি (res == Z3_L_FALSE)
      ফেরত মিথ্যা;

    ফেরত std::nullopt;
  }

  শূন্য ঠেলো() override { ফেরত Z3_solver_ঠেলো(Context.Context, Solver); }

  শূন্য বের_করো(unsigned অঙ্ক_States = 1) override {
    প্রতিজ্ঞা(Z3_solver_get_অঙ্ক_scopes(Context.Context, Solver) >= অঙ্ক_States);
    ফেরত Z3_solver_বের_করো(Context.Context, Solver, অঙ্ক_States);
  }

  যৌক্তিক isFPSupported() override { ফেরত সত্য; }

  /// Reset the solver and remove all constraints.
  শূন্য reset() override { Z3_solver_reset(Context.Context, Solver); }

  শূন্য print(raw_ostream &OS) ধ্রুবক override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  শূন্য setUnsignedParam(StringRef চাবি, unsigned মান) override {
    Z3_প্রতীক Sym = Z3_mk_string_প্রতীক(Context.Context, চাবি.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, মান);
  }

  শূন্য setBoolParam(StringRef চাবি, যৌক্তিক মান) override {
    Z3_প্রতীক Sym = Z3_mk_string_প্রতীক(Context.Context, চাবি.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, মান);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ধ্রুবক override;
}; // end class Z3Solver

শ্রেণী Z3Statistics final : সর্বজনীন SMTSolverStatistics {
সর্বজনীন:
  দ্বিগুণ getDouble(StringRef চাবি) ধ্রুবক override {
    auto It = Double_মানসমূহ.find(চাবি.str());
    প্রতিজ্ঞা(It != Double_মানসমূহ.end());
    ফেরত It->second;
  };
  unsigned getUnsigned(StringRef চাবি) ধ্রুবক override {
    auto It = Unsigned_মানসমূহ.find(চাবি.str());
    যদি (It == Unsigned_মানসমূহ.end())
      ফেরত 0;
    ফেরত It->second;
  };

  শূন্য print(raw_ostream &OS) ধ্রুবক override {
    জন্য (ধ্রুবক auto &E : Unsigned_মানসমূহ)
      OS << E.first() << ": " << E.second << '\n';
    জন্য (ধ্রুবক auto &E : Double_মানসমূহ) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

ব্যক্তিগত:
  friend শ্রেণী Z3Solver;
  StringMap<unsigned> Unsigned_মানসমূহ;
  StringMap<দ্বিগুণ> Double_মানসমূহ;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ধ্রুবক {
  auto ধ্রুবক &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics ফলাফল;

  unsigned অঙ্ক_চাবিগুলি = Z3_stats_আকার(C, S);
  জন্য (unsigned Idx = 0; Idx < অঙ্ক_চাবিগুলি; ++Idx) {
    ধ্রুবক অক্ষর *চাবি = Z3_stats_get_চাবি(C, S, Idx);
    যদি (Z3_stats_is_uint(C, S, Idx)) {
      auto মান = Z3_stats_get_uint_মান(C, S, Idx);
      ফলাফল.Unsigned_মানসমূহ.try_emplace(চাবি, মান);
    } অন্যথা {
      প্রতিজ্ঞা(Z3_stats_is_double(C, S, Idx));
      auto মান = Z3_stats_get_double_মান(C, S, Idx);
      ফলাফল.Double_মানসমূহ.try_emplace(চাবি, মান);
    }
  }
  ফেরত std::make_unique<Z3Statistics>(std::move(ফলাফল));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#যদি LLVM_WITH_Z3
  ফেরত std::make_unique<Z3Solver>();
#অন্যথা
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           মিথ্যা);
  ফেরত nullptr;
#endif
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য SMTSort::dump() ধ্রুবক { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTExpr::dump() ধ্রুবক { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTSolver::dump() ধ্রুবক { print(llvm::errs()); }
LLVM_DUMP_METHOD শূন্য SMTSolverStatistics::dump() ধ্রুবক { print(llvm::errs()); }
#endif
