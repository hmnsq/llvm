//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/ScopeExit.h"
#અન્તર્ભાવ "llvm/Config/config.h"
#અન્તર્ભાવ "llvm/Support/NativeFormatting.h"
#અન્તર્ભાવ "llvm/Support/SMTAPI.h"

using નામાકાશ llvm;

#જો LLVM_WITH_Z3

#અન્તર્ભાવ "llvm/ADT/SmallString.h"
#અન્તર્ભાવ "llvm/ADT/StringMap.h"
#અન્તર્ભાવ "llvm/ADT/Twine.h"

#અન્તર્ભાવ <set>

#અન્તર્ભાવ <z3.h>

નામાકાશ {

/// Configuration class for Z3
વર્ગ Z3Config {
  friend વર્ગ Z3Context;

  Z3_config Config = Z3_mk_config();

જાહેર:
  Z3Config() = મૂળભૂત;
  Z3Config(અચલ Z3Config &) = delete;
  Z3Config(Z3Config &&) = મૂળભૂત;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = મૂળભૂત;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
શૂન્ય Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_સંદેશ(Context, Error)));
}

/// Wrapper for Z3 context
વર્ગ Z3Context {
જાહેર:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(અચલ Z3Context &) = delete;
  Z3Context(Z3Context &&) = મૂળભૂત;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = મૂળભૂત;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
વર્ગ Z3Sort : જાહેર SMTSort {
  friend વર્ગ Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

જાહેર:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(અચલ Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(અચલ Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    પાછા *આ;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    જો (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  શૂન્ય Profile(llvm::Folding_Set_ગાંઠ_ID &ID) અચલ override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  તાર્કિક isBitvectorSortImpl() અચલ override {
    પાછા (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  તાર્કિક isFloatSortImpl() અચલ override {
    પાછા (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  તાર્કિક isBooleanSortImpl() અચલ override {
    પાછા (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() અચલ override {
    પાછા Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() અચલ override {
    પાછા Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  તાર્કિક equal_to(SMTSort અચલ &Other) અચલ override {
    પાછા Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<અચલ Z3Sort &>(Other).Sort);
  }

  શૂન્ય print(raw_ostream &OS) અચલ override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

સ્થિર અચલ Z3Sort &toZ3Sort(અચલ SMTSort &S) {
  પાછા static_cast<અચલ Z3Sort &>(S);
}

વર્ગ Z3_પદાવલિ : જાહેર SMTExpr {
  friend વર્ગ Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

જાહેર:
  Z3_પદાવલિ(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_પદાવલિ(અચલ Z3_પદાવલિ &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_પદાવલિ &operator=(અચલ Z3_પદાવલિ &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    પાછા *આ;
  }

  Z3_પદાવલિ(Z3_પદાવલિ &&Other) = delete;
  Z3_પદાવલિ &operator=(Z3_પદાવલિ &&Other) = delete;

  ~Z3_પદાવલિ() {
    જો (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  શૂન્ય Profile(llvm::Folding_Set_ગાંઠ_ID &ID) અચલ override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  તાર્કિક equal_to(SMTExpr અચલ &Other) અચલ override {
    પ્રતિજ્ઞા(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<અચલ Z3_પદાવલિ &>(Other).AST)) &&
           "AST's must have the same sort");
    પાછા Z3_is_eq_ast(Context.Context, AST,
                        static_cast<અચલ Z3_પદાવલિ &>(Other).AST);
  }

  શૂન્ય print(raw_ostream &OS) અચલ override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

સ્થિર અચલ Z3_પદાવલિ &to_Z3_પદાવલિ(અચલ SMTExpr &E) {
  પાછા static_cast<અચલ Z3_પદાવલિ &>(E);
}

વર્ગ Z3Model {
  friend વર્ગ Z3Solver;

  Z3Context &Context;

  Z3_model Model;

જાહેર:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(અચલ Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    જો (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  શૂન્ય print(raw_ostream &OS) અચલ {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD શૂન્ય dump() અચલ { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
સ્થિર અચલ llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  પસંદગી (BitWidth) {
  મૂળભૂત:
    llvm_unreachable("Unsupported floating-point semantics!");
    તોડો;
  સ્થિતિ 16:
    પાછા llvm::APFloat::IEEEhalf();
  સ્થિતિ 32:
    પાછા llvm::APFloat::IEEEsingle();
  સ્થિતિ 64:
    પાછા llvm::APFloat::IEEEdouble();
  સ્થિતિ 128:
    પાછા llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
સ્થિર તાર્કિક areEquivalent(અચલ llvm::fltSemantics &LHS,
                          અચલ llvm::fltSemantics &RHS) {
  પાછા (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

વર્ગ Z3Solver : જાહેર SMTSolver {
  friend વર્ગ Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [આ] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    પાછા S;
  }();

  Z3_params Params = [આ] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    પાછા P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_પદાવલિ> CachedExprs;

જાહેર:
  Z3Solver() = મૂળભૂત;
  Z3Solver(અચલ Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  શૂન્ય addConstraint(અચલ SMTExprRef &Exp) અચલ override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_પદાવલિ(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(અચલ SMTSort &Sort) {
    auto It = CachedSorts.ઉમેરો(toZ3Sort(Sort));
    પાછા &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_પદાવલિ_Ref(અચલ SMTExpr &Exp) {
    auto It = CachedExprs.ઉમેરો(to_Z3_પદાવલિ(Exp));
    પાછા &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    પાછા newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    પાછા newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(અચલ SMTExprRef &Exp) override {
    પાછા newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    પાછા newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    પાછા newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    પાછા newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    પાછા newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvneg(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkBVNot(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvnot(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkNot(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_not(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvadd(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSub(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvsub(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVMul(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvmul(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvsrem(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVURem(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvurem(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvudiv(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVShl(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvshl(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvashr(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvlshr(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVXor(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvxor(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVOr(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvor(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                   to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvand(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvult(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvslt(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvugt(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvsgt(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVUle(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvule(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSle(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvsle(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVUge(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvuge(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVSge(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_bvsge(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                    to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkAnd(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_પદાવલિ(*LHS).AST, to_Z3_પદાવલિ(*RHS).AST};
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_પદાવલિ(*LHS).AST, to_Z3_પદાવલિ(*RHS).AST};
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_eq(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                 to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkFPMul(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                             to_Z3_પદાવલિ(*LHS).AST, to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                             to_Z3_પદાવલિ(*LHS).AST, to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPRem(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                      to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                             to_Z3_પદાવલિ(*LHS).AST, to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPSub(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                             to_Z3_પદાવલિ(*LHS).AST, to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPLt(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPGt(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPLe(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                      to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPGe(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                      to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkIte(અચલ SMTExprRef &Cond, અચલ SMTExprRef &T,
                   અચલ SMTExprRef &F) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_ite(Context.Context, to_Z3_પદાવલિ(*Cond).AST,
                                  to_Z3_પદાવલિ(*T).AST, to_Z3_પદાવલિ(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_પદાવલિ(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned ઉપલું_2, unsigned નીચલું_2,
                         અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(Context, Z3_mk_extract(Context.Context, ઉપલું_2, નીચલું_2,
                                                    to_Z3_પદાવલિ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS,
                               તાર્કિક isSigned) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                         to_Z3_પદાવલિ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(અચલ SMTExprRef &LHS,
                                અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                          to_Z3_પદાવલિ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(અચલ SMTExprRef &LHS,
                               અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                         to_Z3_પદાવલિ(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS,
                                તાર્કિક isSigned) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                          to_Z3_પદાવલિ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(અચલ SMTExprRef &LHS,
                                અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                          to_Z3_પદાવલિ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(અચલ SMTExprRef &Exp) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_પદાવલિ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS,
                               તાર્કિક isSigned) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                         to_Z3_પદાવલિ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(અચલ SMTExprRef &LHS,
                                અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                          to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(અચલ SMTExprRef &LHS, અચલ SMTExprRef &RHS) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_concat(Context.Context, to_Z3_પદાવલિ(*LHS).AST,
                                     to_Z3_પદાવલિ(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(અચલ SMTExprRef &From, અચલ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                              to_Z3_પદાવલિ(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(અચલ SMTExprRef &From, અચલ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                               to_Z3_પદાવલિ(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(અચલ SMTExprRef &From, અચલ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                                 to_Z3_પદાવલિ(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(અચલ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                                  to_Z3_પદાવલિ(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(અચલ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_પદાવલિ(*RoundingMode).AST,
                                  to_Z3_પદાવલિ(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(અચલ તાર્કિક b) override {
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(અચલ llvm::APSInt Int, unsigned BitWidth) override {
    અચલ Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    જો (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    અચલ int64_t BitReprAsSigned = Int.get_Ext_કિંમત();
    અચલ uint64_t BitReprAsUnsigned =
        reinterpret_cast<અચલ uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(Context, Literal));
  }

  SMTExprRef mkFloat(અચલ llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), ખોટું);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_પદાવલિ(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_પ્રતીક(અચલ અક્ષર *Name, SMTSortRef Sort) override {
    પાછા new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_પ્રતીક(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(અચલ SMTExprRef &Exp, unsigned BitWidth,
                            તાર્કિક isUnsigned) override {
    પાછા llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_પદાવલિ(*Exp).AST),
                    10),
        isUnsigned);
  }

  તાર્કિક getBoolean(અચલ SMTExprRef &Exp) override {
    પાછા Z3_get_bool_કિંમત(Context.Context, to_Z3_પદાવલિ(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    પાછા new_પદાવલિ_Ref(Z3_પદાવલિ(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  તાર્કિક toAPFloat(અચલ SMTSortRef &Sort, અચલ SMTExprRef &AST,
                 llvm::APFloat &Float, તાર્કિક useSemantics) {
    પ્રતિજ્ઞા(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), સાચું);
    અચલ llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    જો (!toAPSInt(BVSort, AST, Int, સાચું)) {
      પાછા ખોટું;
    }

    જો (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      પ્રતિજ્ઞા(ખોટું && "Floating-point types don't match!");
      પાછા ખોટું;
    }

    Float = llvm::APFloat(Semantics, Int);
    પાછા સાચું;
  }

  તાર્કિક toAPSInt(અચલ SMTSortRef &Sort, અચલ SMTExprRef &AST,
                llvm::APSInt &Int, તાર્કિક useSemantics) {
    જો (Sort->isBitvectorSort()) {
      જો (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        પ્રતિજ્ઞા(ખોટું && "Bitvector types don't match!");
        પાછા ખોટું;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      પાછા સાચું;
    }

    જો (Sort->isBooleanSort()) {
      જો (useSemantics && Int.getBitWidth() < 1) {
        પ્રતિજ્ઞા(ખોટું && "Boolean type doesn't match!");
        પાછા ખોટું;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      પાછા સાચું;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  તાર્કિક getInterpretation(અચલ SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_પદાવલિ(*Exp).AST));
    જો (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      પાછા ખોટું;

    SMTExprRef Assign = new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    પાછા toAPSInt(Sort, Assign, Int, સાચું);
  }

  તાર્કિક getInterpretation(અચલ SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_પદાવલિ(*Exp).AST));
    જો (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      પાછા ખોટું;

    SMTExprRef Assign = new_પદાવલિ_Ref(
        Z3_પદાવલિ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    પાછા toAPFloat(Sort, Assign, Float, સાચું);
  }

  std::optional<તાર્કિક> check() અચલ override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    જો (res == Z3_L_TRUE)
      પાછા સાચું;

    જો (res == Z3_L_FALSE)
      પાછા ખોટું;

    પાછા std::nullopt;
  }

  શૂન્ય ધકેલો() override { પાછા Z3_solver_ધકેલો(Context.Context, Solver); }

  શૂન્ય કાઢો(unsigned અંક_States = 1) override {
    પ્રતિજ્ઞા(Z3_solver_get_અંક_scopes(Context.Context, Solver) >= અંક_States);
    પાછા Z3_solver_કાઢો(Context.Context, Solver, અંક_States);
  }

  તાર્કિક isFPSupported() override { પાછા સાચું; }

  /// Reset the solver and remove all constraints.
  શૂન્ય reset() override { Z3_solver_reset(Context.Context, Solver); }

  શૂન્ય print(raw_ostream &OS) અચલ override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  શૂન્ય setUnsignedParam(StringRef ચાવી, unsigned કિંમત) override {
    Z3_પ્રતીક Sym = Z3_mk_string_પ્રતીક(Context.Context, ચાવી.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, કિંમત);
  }

  શૂન્ય setBoolParam(StringRef ચાવી, તાર્કિક કિંમત) override {
    Z3_પ્રતીક Sym = Z3_mk_string_પ્રતીક(Context.Context, ચાવી.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, કિંમત);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() અચલ override;
}; // end class Z3Solver

વર્ગ Z3Statistics final : જાહેર SMTSolverStatistics {
જાહેર:
  દ્વિગુણ getDouble(StringRef ચાવી) અચલ override {
    auto It = Double_કિંમતો.find(ચાવી.str());
    પ્રતિજ્ઞા(It != Double_કિંમતો.end());
    પાછા It->second;
  };
  unsigned getUnsigned(StringRef ચાવી) અચલ override {
    auto It = Unsigned_કિંમતો.find(ચાવી.str());
    જો (It == Unsigned_કિંમતો.end())
      પાછા 0;
    પાછા It->second;
  };

  શૂન્ય print(raw_ostream &OS) અચલ override {
    માટે (અચલ auto &E : Unsigned_કિંમતો)
      OS << E.first() << ": " << E.second << '\n';
    માટે (અચલ auto &E : Double_કિંમતો) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

ખાનગી:
  friend વર્ગ Z3Solver;
  StringMap<unsigned> Unsigned_કિંમતો;
  StringMap<દ્વિગુણ> Double_કિંમતો;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() અચલ {
  auto અચલ &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned અંક_ચાવીઓ = Z3_stats_size(C, S);
  માટે (unsigned Idx = 0; Idx < અંક_ચાવીઓ; ++Idx) {
    અચલ અક્ષર *ચાવી = Z3_stats_get_ચાવી(C, S, Idx);
    જો (Z3_stats_is_uint(C, S, Idx)) {
      auto કિંમત = Z3_stats_get_uint_કિંમત(C, S, Idx);
      Result.Unsigned_કિંમતો.try_emplace(ચાવી, કિંમત);
    } નહીંતર {
      પ્રતિજ્ઞા(Z3_stats_is_double(C, S, Idx));
      auto કિંમત = Z3_stats_get_double_કિંમત(C, S, Idx);
      Result.Double_કિંમતો.try_emplace(ચાવી, કિંમત);
    }
  }
  પાછા std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#જો LLVM_WITH_Z3
  પાછા std::make_unique<Z3Solver>();
#નહીંતર
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           ખોટું);
  પાછા nullptr;
#endif
}

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય SMTSort::dump() અચલ { print(llvm::errs()); }
LLVM_DUMP_METHOD શૂન્ય SMTExpr::dump() અચલ { print(llvm::errs()); }
LLVM_DUMP_METHOD શૂન્ય SMTSolver::dump() અચલ { print(llvm::errs()); }
LLVM_DUMP_METHOD શૂન્ય SMTSolverStatistics::dump() અચલ { print(llvm::errs()); }
#endif
