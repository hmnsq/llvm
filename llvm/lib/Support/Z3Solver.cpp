//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/ScopeExit.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Config/config.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/NativeFormatting.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/SMTAPI.h"

using ନାମାକାଶ llvm;

#ଯଦି LLVM_WITH_Z3

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SmallString.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringMap.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Twine.h"

#ଅନ୍ତର୍ଭାଵ <set>

#ଅନ୍ତର୍ଭାଵ <z3.h>

ନାମାକାଶ {

/// Configuration class for Z3
ଶ୍ରେଣୀ Z3Config {
  friend ଶ୍ରେଣୀ Z3Context;

  Z3_config Config = Z3_mk_config();

ସର୍ବସାଧାରଣ:
  Z3Config() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  Z3Config(ଅଚଳ Z3Config &) = delete;
  Z3Config(Z3Config &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
ଶୂନ୍ୟ Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_ସନ୍ଦେଶ(Context, Error)));
}

/// Wrapper for Z3 context
ଶ୍ରେଣୀ Z3Context {
ସର୍ବସାଧାରଣ:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ଅଚଳ Z3Context &) = delete;
  Z3Context(Z3Context &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
ଶ୍ରେଣୀ Z3Sort : ସର୍ବସାଧାରଣ SMTSort {
  friend ଶ୍ରେଣୀ Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

ସର୍ବସାଧାରଣ:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ଅଚଳ Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ଅଚଳ Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    ଫେରନ୍ତୁ *ଏହି;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    ଯଦି (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  ଶୂନ୍ୟ Profile(llvm::Folding_Set_ଗଣ୍ଠି_ID &ID) ଅଚଳ override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  ତାର୍କିକ isBitvectorSortImpl() ଅଚଳ override {
    ଫେରନ୍ତୁ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  ତାର୍କିକ isFloatSortImpl() ଅଚଳ override {
    ଫେରନ୍ତୁ (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  ତାର୍କିକ isBooleanSortImpl() ଅଚଳ override {
    ଫେରନ୍ତୁ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ଅଚଳ override {
    ଫେରନ୍ତୁ Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ଅଚଳ override {
    ଫେରନ୍ତୁ Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  ତାର୍କିକ equal_to(SMTSort ଅଚଳ &Other) ଅଚଳ override {
    ଫେରନ୍ତୁ Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ଅଚଳ Z3Sort &>(Other).Sort);
  }

  ଶୂନ୍ୟ print(raw_ostream &OS) ଅଚଳ override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

ସ୍ଥିର ଅଚଳ Z3Sort &toZ3Sort(ଅଚଳ SMTSort &S) {
  ଫେରନ୍ତୁ static_cast<ଅଚଳ Z3Sort &>(S);
}

ଶ୍ରେଣୀ Z3_ପଦସମଷ୍ଟି : ସର୍ବସାଧାରଣ SMTExpr {
  friend ଶ୍ରେଣୀ Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

ସର୍ବସାଧାରଣ:
  Z3_ପଦସମଷ୍ଟି(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ପଦସମଷ୍ଟି(ଅଚଳ Z3_ପଦସମଷ୍ଟି &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ପଦସମଷ୍ଟି &operator=(ଅଚଳ Z3_ପଦସମଷ୍ଟି &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    ଫେରନ୍ତୁ *ଏହି;
  }

  Z3_ପଦସମଷ୍ଟି(Z3_ପଦସମଷ୍ଟି &&Other) = delete;
  Z3_ପଦସମଷ୍ଟି &operator=(Z3_ପଦସମଷ୍ଟି &&Other) = delete;

  ~Z3_ପଦସମଷ୍ଟି() {
    ଯଦି (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  ଶୂନ୍ୟ Profile(llvm::Folding_Set_ଗଣ୍ଠି_ID &ID) ଅଚଳ override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  ତାର୍କିକ equal_to(SMTExpr ଅଚଳ &Other) ଅଚଳ override {
    ପ୍ରତିଜ୍ଞା(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ଅଚଳ Z3_ପଦସମଷ୍ଟି &>(Other).AST)) &&
           "AST's must have the same sort");
    ଫେରନ୍ତୁ Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ଅଚଳ Z3_ପଦସମଷ୍ଟି &>(Other).AST);
  }

  ଶୂନ୍ୟ print(raw_ostream &OS) ଅଚଳ override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

ସ୍ଥିର ଅଚଳ Z3_ପଦସମଷ୍ଟି &to_Z3_ପଦସମଷ୍ଟି(ଅଚଳ SMTExpr &E) {
  ଫେରନ୍ତୁ static_cast<ଅଚଳ Z3_ପଦସମଷ୍ଟି &>(E);
}

ଶ୍ରେଣୀ Z3Model {
  friend ଶ୍ରେଣୀ Z3Solver;

  Z3Context &Context;

  Z3_model Model;

ସର୍ବସାଧାରଣ:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ଅଚଳ Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    ଯଦି (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  ଶୂନ୍ୟ print(raw_ostream &OS) ଅଚଳ {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD ଶୂନ୍ୟ dump() ଅଚଳ { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ସ୍ଥିର ଅଚଳ llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  ବାଛ (BitWidth) {
  ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
    llvm_unreachable("Unsupported floating-point semantics!");
    ଭାଙ୍ଗ;
  ସ୍ଥିତି 16:
    ଫେରନ୍ତୁ llvm::APFloat::IEEEhalf();
  ସ୍ଥିତି 32:
    ଫେରନ୍ତୁ llvm::APFloat::IEEEsingle();
  ସ୍ଥିତି 64:
    ଫେରନ୍ତୁ llvm::APFloat::IEEEdouble();
  ସ୍ଥିତି 128:
    ଫେରନ୍ତୁ llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ସ୍ଥିର ତାର୍କିକ areEquivalent(ଅଚଳ llvm::fltSemantics &LHS,
                          ଅଚଳ llvm::fltSemantics &RHS) {
  ଫେରନ୍ତୁ (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

ଶ୍ରେଣୀ Z3Solver : ସର୍ବସାଧାରଣ SMTSolver {
  friend ଶ୍ରେଣୀ Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [ଏହି] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    ଫେରନ୍ତୁ S;
  }();

  Z3_params Params = [ଏହି] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    ଫେରନ୍ତୁ P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_ପଦସମଷ୍ଟି> CachedExprs;

ସର୍ବସାଧାରଣ:
  Z3Solver() = ପୂର୍ବନିର୍ଦ୍ଧାରିତ;
  Z3Solver(ଅଚଳ Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  ଶୂନ୍ୟ addConstraint(ଅଚଳ SMTExprRef &Exp) ଅଚଳ override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ଅଚଳ SMTSort &Sort) {
    auto It = CachedSorts.ଭର୍ତ୍ତି(toZ3Sort(Sort));
    ଫେରନ୍ତୁ &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_ପଦସମଷ୍ଟି_Ref(ଅଚଳ SMTExpr &Exp) {
    auto It = CachedExprs.ଭର୍ତ୍ତି(to_Z3_ପଦସମଷ୍ଟି(Exp));
    ଫେରନ୍ତୁ &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    ଫେରନ୍ତୁ newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    ଫେରନ୍ତୁ newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    ଫେରନ୍ତୁ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    ଫେରନ୍ତୁ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    ଫେରନ୍ତୁ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    ଫେରନ୍ତୁ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvneg(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvnot(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkNot(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_not(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvadd(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvsub(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvmul(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvsrem(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvurem(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvudiv(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvshl(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvashr(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvlshr(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvxor(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvor(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                   to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvand(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvult(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvslt(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvugt(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvsgt(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvule(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvsle(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvuge(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_bvsge(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                    to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkAnd(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_ପଦସମଷ୍ଟି(*LHS).AST, to_Z3_ପଦସମଷ୍ଟି(*RHS).AST};
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_ପଦସମଷ୍ଟି(*LHS).AST, to_Z3_ପଦସମଷ୍ଟି(*RHS).AST};
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_eq(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                 to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                             to_Z3_ପଦସମଷ୍ଟି(*LHS).AST, to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                             to_Z3_ପଦସମଷ୍ଟି(*LHS).AST, to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                      to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                             to_Z3_ପଦସମଷ୍ଟି(*LHS).AST, to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                             to_Z3_ପଦସମଷ୍ଟି(*LHS).AST, to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                      to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                      to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkIte(ଅଚଳ SMTExprRef &Cond, ଅଚଳ SMTExprRef &T,
                   ଅଚଳ SMTExprRef &F) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_ite(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Cond).AST,
                                  to_Z3_ପଦସମଷ୍ଟି(*T).AST, to_Z3_ପଦସମଷ୍ଟି(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned ଉପର_2, unsigned ତଳ_2,
                         ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_extract(Context.Context, ଉପର_2, ତଳ_2,
                                                    to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS,
                               ତାର୍କିକ isSigned) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                         to_Z3_ପଦସମଷ୍ଟି(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ଅଚଳ SMTExprRef &LHS,
                                ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                          to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ଅଚଳ SMTExprRef &LHS,
                               ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                         to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS,
                                ତାର୍କିକ isSigned) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                          to_Z3_ପଦସମଷ୍ଟି(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ଅଚଳ SMTExprRef &LHS,
                                ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                          to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS,
                               ତାର୍କିକ isSigned) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                         to_Z3_ପଦସମଷ୍ଟି(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ଅଚଳ SMTExprRef &LHS,
                                ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                          to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ଅଚଳ SMTExprRef &LHS, ଅଚଳ SMTExprRef &RHS) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_concat(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*LHS).AST,
                                     to_Z3_ପଦସମଷ୍ଟି(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ଅଚଳ SMTExprRef &From, ଅଚଳ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                              to_Z3_ପଦସମଷ୍ଟି(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ଅଚଳ SMTExprRef &From, ଅଚଳ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                               to_Z3_ପଦସମଷ୍ଟି(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ଅଚଳ SMTExprRef &From, ଅଚଳ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                                 to_Z3_ପଦସମଷ୍ଟି(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ଅଚଳ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                                  to_Z3_ପଦସମଷ୍ଟି(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ଅଚଳ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*RoundingMode).AST,
                                  to_Z3_ପଦସମଷ୍ଟି(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ଅଚଳ ତାର୍କିକ b) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ଅଚଳ llvm::APSInt Int, unsigned BitWidth) override {
    ଅଚଳ Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    ଯଦି (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ଅଚଳ int64_t BitReprAsSigned = Int.get_Ext_ମୂଲ୍ୟ();
    ଅଚଳ uint64_t BitReprAsUnsigned =
        reinterpret_cast<ଅଚଳ uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(Context, Literal));
  }

  SMTExprRef mkFloat(ଅଚଳ llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), ମିଥ୍ୟା);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_ପ୍ରତୀକ(ଅଚଳ ଅକ୍ଷର *Name, SMTSortRef Sort) override {
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_ପ୍ରତୀକ(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ଅଚଳ SMTExprRef &Exp, unsigned BitWidth,
                            ତାର୍କିକ isUnsigned) override {
    ଫେରନ୍ତୁ llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST),
                    10),
        isUnsigned);
  }

  ତାର୍କିକ getBoolean(ଅଚଳ SMTExprRef &Exp) override {
    ଫେରନ୍ତୁ Z3_get_bool_ମୂଲ୍ୟ(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    ଫେରନ୍ତୁ new_ପଦସମଷ୍ଟି_Ref(Z3_ପଦସମଷ୍ଟି(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  ତାର୍କିକ toAPFloat(ଅଚଳ SMTSortRef &Sort, ଅଚଳ SMTExprRef &AST,
                 llvm::APFloat &Float, ତାର୍କିକ useSemantics) {
    ପ୍ରତିଜ୍ଞା(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), ସତ୍ୟ);
    ଅଚଳ llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    ଯଦି (!toAPSInt(BVSort, AST, Int, ସତ୍ୟ)) {
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    ଯଦି (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      ପ୍ରତିଜ୍ଞା(ମିଥ୍ୟା && "Floating-point types don't match!");
      ଫେରନ୍ତୁ ମିଥ୍ୟା;
    }

    Float = llvm::APFloat(Semantics, Int);
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ତାର୍କିକ toAPSInt(ଅଚଳ SMTSortRef &Sort, ଅଚଳ SMTExprRef &AST,
                llvm::APSInt &Int, ତାର୍କିକ useSemantics) {
    ଯଦି (Sort->isBitvectorSort()) {
      ଯଦି (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        ପ୍ରତିଜ୍ଞା(ମିଥ୍ୟା && "Bitvector types don't match!");
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    ଯଦି (Sort->isBooleanSort()) {
      ଯଦି (useSemantics && Int.getBitWidth() < 1) {
        ପ୍ରତିଜ୍ଞା(ମିଥ୍ୟା && "Boolean type doesn't match!");
        ଫେରନ୍ତୁ ମିଥ୍ୟା;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      ଫେରନ୍ତୁ ସତ୍ୟ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  ତାର୍କିକ getInterpretation(ଅଚଳ SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST));
    ଯଦି (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    SMTExprRef Assign = new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ଫେରନ୍ତୁ toAPSInt(Sort, Assign, Int, ସତ୍ୟ);
  }

  ତାର୍କିକ getInterpretation(ଅଚଳ SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_ପଦସମଷ୍ଟି(*Exp).AST));
    ଯଦି (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    SMTExprRef Assign = new_ପଦସମଷ୍ଟି_Ref(
        Z3_ପଦସମଷ୍ଟି(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ଫେରନ୍ତୁ toAPFloat(Sort, Assign, Float, ସତ୍ୟ);
  }

  std::optional<ତାର୍କିକ> check() ଅଚଳ override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    ଯଦି (res == Z3_L_TRUE)
      ଫେରନ୍ତୁ ସତ୍ୟ;

    ଯଦି (res == Z3_L_FALSE)
      ଫେରନ୍ତୁ ମିଥ୍ୟା;

    ଫେରନ୍ତୁ std::nullopt;
  }

  ଶୂନ୍ୟ ଠେଲ() override { ଫେରନ୍ତୁ Z3_solver_ଠେଲ(Context.Context, Solver); }

  ଶୂନ୍ୟ କାଢ଼(unsigned ଅଙ୍କ_States = 1) override {
    ପ୍ରତିଜ୍ଞା(Z3_solver_get_ଅଙ୍କ_scopes(Context.Context, Solver) >= ଅଙ୍କ_States);
    ଫେରନ୍ତୁ Z3_solver_କାଢ଼(Context.Context, Solver, ଅଙ୍କ_States);
  }

  ତାର୍କିକ isFPSupported() override { ଫେରନ୍ତୁ ସତ୍ୟ; }

  /// Reset the solver and remove all constraints.
  ଶୂନ୍ୟ reset() override { Z3_solver_reset(Context.Context, Solver); }

  ଶୂନ୍ୟ print(raw_ostream &OS) ଅଚଳ override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  ଶୂନ୍ୟ setUnsignedParam(StringRef ଚାବି, unsigned ମୂଲ୍ୟ) override {
    Z3_ପ୍ରତୀକ Sym = Z3_mk_string_ପ୍ରତୀକ(Context.Context, ଚାବି.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, ମୂଲ୍ୟ);
  }

  ଶୂନ୍ୟ setBoolParam(StringRef ଚାବି, ତାର୍କିକ ମୂଲ୍ୟ) override {
    Z3_ପ୍ରତୀକ Sym = Z3_mk_string_ପ୍ରତୀକ(Context.Context, ଚାବି.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, ମୂଲ୍ୟ);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ଅଚଳ override;
}; // end class Z3Solver

ଶ୍ରେଣୀ Z3Statistics final : ସର୍ବସାଧାରଣ SMTSolverStatistics {
ସର୍ବସାଧାରଣ:
  ଦ୍ୱିଗୁଣ getDouble(StringRef ଚାବି) ଅଚଳ override {
    auto It = Double_ମୂଲ୍ୟଗୁଡ଼ିକ.find(ଚାବି.str());
    ପ୍ରତିଜ୍ଞା(It != Double_ମୂଲ୍ୟଗୁଡ଼ିକ.end());
    ଫେରନ୍ତୁ It->second;
  };
  unsigned getUnsigned(StringRef ଚାବି) ଅଚଳ override {
    auto It = Unsigned_ମୂଲ୍ୟଗୁଡ଼ିକ.find(ଚାବି.str());
    ଯଦି (It == Unsigned_ମୂଲ୍ୟଗୁଡ଼ିକ.end())
      ଫେରନ୍ତୁ 0;
    ଫେରନ୍ତୁ It->second;
  };

  ଶୂନ୍ୟ print(raw_ostream &OS) ଅଚଳ override {
    ପାଇଁ (ଅଚଳ auto &E : Unsigned_ମୂଲ୍ୟଗୁଡ଼ିକ)
      OS << E.first() << ": " << E.second << '\n';
    ପାଇଁ (ଅଚଳ auto &E : Double_ମୂଲ୍ୟଗୁଡ଼ିକ) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

ବ୍ୟକ୍ତିଗତ:
  friend ଶ୍ରେଣୀ Z3Solver;
  StringMap<unsigned> Unsigned_ମୂଲ୍ୟଗୁଡ଼ିକ;
  StringMap<ଦ୍ୱିଗୁଣ> Double_ମୂଲ୍ୟଗୁଡ଼ିକ;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ଅଚଳ {
  auto ଅଚଳ &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned ଅଙ୍କ_ଚାବିଗୁଡ଼ିକ = Z3_stats_size(C, S);
  ପାଇଁ (unsigned Idx = 0; Idx < ଅଙ୍କ_ଚାବିଗୁଡ଼ିକ; ++Idx) {
    ଅଚଳ ଅକ୍ଷର *ଚାବି = Z3_stats_get_ଚାବି(C, S, Idx);
    ଯଦି (Z3_stats_is_uint(C, S, Idx)) {
      auto ମୂଲ୍ୟ = Z3_stats_get_uint_ମୂଲ୍ୟ(C, S, Idx);
      Result.Unsigned_ମୂଲ୍ୟଗୁଡ଼ିକ.try_emplace(ଚାବି, ମୂଲ୍ୟ);
    } ଅନ୍ୟଥା {
      ପ୍ରତିଜ୍ଞା(Z3_stats_is_double(C, S, Idx));
      auto ମୂଲ୍ୟ = Z3_stats_get_double_ମୂଲ୍ୟ(C, S, Idx);
      Result.Double_ମୂଲ୍ୟଗୁଡ଼ିକ.try_emplace(ଚାବି, ମୂଲ୍ୟ);
    }
  }
  ଫେରନ୍ତୁ std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#ଯଦି LLVM_WITH_Z3
  ଫେରନ୍ତୁ std::make_unique<Z3Solver>();
#ଅନ୍ୟଥା
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           ମିଥ୍ୟା);
  ଫେରନ୍ତୁ nullptr;
#endif
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ SMTSort::dump() ଅଚଳ { print(llvm::errs()); }
LLVM_DUMP_METHOD ଶୂନ୍ୟ SMTExpr::dump() ଅଚଳ { print(llvm::errs()); }
LLVM_DUMP_METHOD ଶୂନ୍ୟ SMTSolver::dump() ଅଚଳ { print(llvm::errs()); }
LLVM_DUMP_METHOD ଶୂନ୍ୟ SMTSolverStatistics::dump() ଅଚଳ { print(llvm::errs()); }
#endif
