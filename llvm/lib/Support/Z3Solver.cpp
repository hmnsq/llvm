//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/ScopeExit.h"
#ಅನ್ತರ್ಭಾವ "llvm/Config/config.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/NativeFormatting.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/SMTAPI.h"

using ನಾಮಾಕಾಶ llvm;

#ಯದಿ LLVM_WITH_Z3

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"

#ಅನ್ತರ್ಭಾವ <set>

#ಅನ್ತರ್ಭಾವ <z3.h>

ನಾಮಾಕಾಶ {

/// Configuration class for Z3
ವರ್ಗ Z3Config {
  friend ವರ್ಗ Z3Context;

  Z3_config Config = Z3_mk_config();

ಸಾರ್ವಜನಿಕ:
  Z3Config() = ಸ್ವಭಾವ;
  Z3Config(ಧ್ರುವ Z3Config &) = delete;
  Z3Config(Z3Config &&) = ಸ್ವಭಾವ;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = ಸ್ವಭಾವ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
ಶೂನ್ಯ Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_ಸಂದೇಶ(Context, Error)));
}

/// Wrapper for Z3 context
ವರ್ಗ Z3Context {
ಸಾರ್ವಜನಿಕ:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ಧ್ರುವ Z3Context &) = delete;
  Z3Context(Z3Context &&) = ಸ್ವಭಾವ;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = ಸ್ವಭಾವ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
ವರ್ಗ Z3Sort : ಸಾರ್ವಜನಿಕ SMTSort {
  friend ವರ್ಗ Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

ಸಾರ್ವಜನಿಕ:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ಧ್ರುವ Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ಧ್ರುವ Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    ಯದಿ (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  ಶೂನ್ಯ Profile(llvm::Folding_Set_ಗಂಟು_ID &ID) ಧ್ರುವ override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  ತಾರ್ಕಿಕ isBitvectorSortImpl() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  ತಾರ್ಕಿಕ isFloatSortImpl() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  ತಾರ್ಕಿಕ isBooleanSortImpl() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  ತಾರ್ಕಿಕ equal_to(SMTSort ಧ್ರುವ &Other) ಧ್ರುವ override {
    ಪ್ರತ್ಯಾಗಮ Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ಧ್ರುವ Z3Sort &>(Other).Sort);
  }

  ಶೂನ್ಯ print(raw_ostream &OS) ಧ್ರುವ override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

ಸ್ಥಿರ ಧ್ರುವ Z3Sort &toZ3Sort(ಧ್ರುವ SMTSort &S) {
  ಪ್ರತ್ಯಾಗಮ static_cast<ಧ್ರುವ Z3Sort &>(S);
}

ವರ್ಗ Z3_ಅಭಿವ್ಯಕ್ತಿ : ಸಾರ್ವಜನಿಕ SMTExpr {
  friend ವರ್ಗ Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

ಸಾರ್ವಜನಿಕ:
  Z3_ಅಭಿವ್ಯಕ್ತಿ(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ಅಭಿವ್ಯಕ್ತಿ(ಧ್ರುವ Z3_ಅಭಿವ್ಯಕ್ತಿ &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ಅಭಿವ್ಯಕ್ತಿ &operator=(ಧ್ರುವ Z3_ಅಭಿವ್ಯಕ್ತಿ &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
  }

  Z3_ಅಭಿವ್ಯಕ್ತಿ(Z3_ಅಭಿವ್ಯಕ್ತಿ &&Other) = delete;
  Z3_ಅಭಿವ್ಯಕ್ತಿ &operator=(Z3_ಅಭಿವ್ಯಕ್ತಿ &&Other) = delete;

  ~Z3_ಅಭಿವ್ಯಕ್ತಿ() {
    ಯದಿ (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  ಶೂನ್ಯ Profile(llvm::Folding_Set_ಗಂಟು_ID &ID) ಧ್ರುವ override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  ತಾರ್ಕಿಕ equal_to(SMTExpr ಧ್ರುವ &Other) ಧ್ರುವ override {
    ಪ್ರತಿಜ್ಞಾ(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ಧ್ರುವ Z3_ಅಭಿವ್ಯಕ್ತಿ &>(Other).AST)) &&
           "AST's must have the same sort");
    ಪ್ರತ್ಯಾಗಮ Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ಧ್ರುವ Z3_ಅಭಿವ್ಯಕ್ತಿ &>(Other).AST);
  }

  ಶೂನ್ಯ print(raw_ostream &OS) ಧ್ರುವ override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

ಸ್ಥಿರ ಧ್ರುವ Z3_ಅಭಿವ್ಯಕ್ತಿ &to_Z3_ಅಭಿವ್ಯಕ್ತಿ(ಧ್ರುವ SMTExpr &E) {
  ಪ್ರತ್ಯಾಗಮ static_cast<ಧ್ರುವ Z3_ಅಭಿವ್ಯಕ್ತಿ &>(E);
}

ವರ್ಗ Z3Model {
  friend ವರ್ಗ Z3Solver;

  Z3Context &Context;

  Z3_model Model;

ಸಾರ್ವಜನಿಕ:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ಧ್ರುವ Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    ಯದಿ (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  ಶೂನ್ಯ print(raw_ostream &OS) ಧ್ರುವ {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD ಶೂನ್ಯ dump() ಧ್ರುವ { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ಸ್ಥಿರ ಧ್ರುವ llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  ವಿಕಲ್ಪ (BitWidth) {
  ಸ್ವಭಾವ:
    llvm_unreachable("Unsupported floating-point semantics!");
    ಭಙ್ಗ;
  ಪಕ್ಷ 16:
    ಪ್ರತ್ಯಾಗಮ llvm::APFloat::IEEEhalf();
  ಪಕ್ಷ 32:
    ಪ್ರತ್ಯಾಗಮ llvm::APFloat::IEEEsingle();
  ಪಕ್ಷ 64:
    ಪ್ರತ್ಯಾಗಮ llvm::APFloat::IEEEdouble();
  ಪಕ್ಷ 128:
    ಪ್ರತ್ಯಾಗಮ llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ಸ್ಥಿರ ತಾರ್ಕಿಕ areEquivalent(ಧ್ರುವ llvm::fltSemantics &LHS,
                          ಧ್ರುವ llvm::fltSemantics &RHS) {
  ಪ್ರತ್ಯಾಗಮ (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

ವರ್ಗ Z3Solver : ಸಾರ್ವಜನಿಕ SMTSolver {
  friend ವರ್ಗ Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [ಏತತ್] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    ಪ್ರತ್ಯಾಗಮ S;
  }();

  Z3_params Params = [ಏತತ್] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    ಪ್ರತ್ಯಾಗಮ P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_ಅಭಿವ್ಯಕ್ತಿ> CachedExprs;

ಸಾರ್ವಜನಿಕ:
  Z3Solver() = ಸ್ವಭಾವ;
  Z3Solver(ಧ್ರುವ Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  ಶೂನ್ಯ addConstraint(ಧ್ರುವ SMTExprRef &Exp) ಧ್ರುವ override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ಧ್ರುವ SMTSort &Sort) {
    auto It = CachedSorts.ಸೇರಿಸು(toZ3Sort(Sort));
    ಪ್ರತ್ಯಾಗಮ &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_ಅಭಿವ್ಯಕ್ತಿ_Ref(ಧ್ರುವ SMTExpr &Exp) {
    auto It = CachedExprs.ಸೇರಿಸು(to_Z3_ಅಭಿವ್ಯಕ್ತಿ(Exp));
    ಪ್ರತ್ಯಾಗಮ &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    ಪ್ರತ್ಯಾಗಮ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvneg(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvnot(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkNot(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_not(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvadd(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsub(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvmul(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsrem(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvurem(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvudiv(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvshl(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvashr(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvlshr(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvxor(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvor(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                   to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvand(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvult(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvslt(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvugt(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsgt(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvule(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsle(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvuge(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsge(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkAnd(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST};
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST};
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_eq(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                 to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                             to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                             to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                      to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                             to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                             to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                      to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                      to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkIte(ಧ್ರುವ SMTExprRef &Cond, ಧ್ರುವ SMTExprRef &T,
                   ಧ್ರುವ SMTExprRef &F) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_ite(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Cond).AST,
                                  to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*T).AST, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned ಮೇಲಿನ_2, unsigned ಕೆಳಗಿನ_2,
                         ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_extract(Context.Context, ಮೇಲಿನ_2, ಕೆಳಗಿನ_2,
                                                    to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS,
                               ತಾರ್ಕಿಕ isSigned) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                         to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ಧ್ರುವ SMTExprRef &LHS,
                                ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ಧ್ರುವ SMTExprRef &LHS,
                               ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                         to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS,
                                ತಾರ್ಕಿಕ isSigned) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ಧ್ರುವ SMTExprRef &LHS,
                                ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS,
                               ತಾರ್ಕಿಕ isSigned) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                         to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ಧ್ರುವ SMTExprRef &LHS,
                                ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ಧ್ರುವ SMTExprRef &LHS, ಧ್ರುವ SMTExprRef &RHS) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_concat(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ಧ್ರುವ SMTExprRef &From, ಧ್ರುವ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                              to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ಧ್ರುವ SMTExprRef &From, ಧ್ರುವ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                               to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ಧ್ರುವ SMTExprRef &From, ಧ್ರುವ SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                                 to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ಧ್ರುವ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                                  to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ಧ್ರುವ SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RoundingMode).AST,
                                  to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ಧ್ರುವ ತಾರ್ಕಿಕ b) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ಧ್ರುವ llvm::APSInt Int, unsigned BitWidth) override {
    ಧ್ರುವ Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    ಯದಿ (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ಧ್ರುವ int64_t BitReprAsSigned = Int.get_Ext_ಮೌಲ್ಯ();
    ಧ್ರುವ uint64_t BitReprAsUnsigned =
        reinterpret_cast<ಧ್ರುವ uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Literal));
  }

  SMTExprRef mkFloat(ಧ್ರುವ llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), ಅಸತ್ಯ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_ಚಿಹ್ನೆ(ಧ್ರುವ ವರ್ಣ *Name, SMTSortRef Sort) override {
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_ಚಿಹ್ನೆ(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ಧ್ರುವ SMTExprRef &Exp, unsigned BitWidth,
                            ತಾರ್ಕಿಕ isUnsigned) override {
    ಪ್ರತ್ಯಾಗಮ llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST),
                    10),
        isUnsigned);
  }

  ತಾರ್ಕಿಕ getBoolean(ಧ್ರುವ SMTExprRef &Exp) override {
    ಪ್ರತ್ಯಾಗಮ Z3_get_bool_ಮೌಲ್ಯ(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    ಪ್ರತ್ಯಾಗಮ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  ತಾರ್ಕಿಕ toAPFloat(ಧ್ರುವ SMTSortRef &Sort, ಧ್ರುವ SMTExprRef &AST,
                 llvm::APFloat &Float, ತಾರ್ಕಿಕ useSemantics) {
    ಪ್ರತಿಜ್ಞಾ(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), ಸತ್ಯ);
    ಧ್ರುವ llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    ಯದಿ (!toAPSInt(BVSort, AST, Int, ಸತ್ಯ)) {
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    ಯದಿ (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      ಪ್ರತಿಜ್ಞಾ(ಅಸತ್ಯ && "Floating-point types don't match!");
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
    }

    Float = llvm::APFloat(Semantics, Int);
    ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
  }

  ತಾರ್ಕಿಕ toAPSInt(ಧ್ರುವ SMTSortRef &Sort, ಧ್ರುವ SMTExprRef &AST,
                llvm::APSInt &Int, ತಾರ್ಕಿಕ useSemantics) {
    ಯದಿ (Sort->isBitvectorSort()) {
      ಯದಿ (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        ಪ್ರತಿಜ್ಞಾ(ಅಸತ್ಯ && "Bitvector types don't match!");
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    ಯದಿ (Sort->isBooleanSort()) {
      ಯದಿ (useSemantics && Int.getBitWidth() < 1) {
        ಪ್ರತಿಜ್ಞಾ(ಅಸತ್ಯ && "Boolean type doesn't match!");
        ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  ತಾರ್ಕಿಕ getInterpretation(ಧ್ರುವ SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST));
    ಯದಿ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    SMTExprRef Assign = new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ಪ್ರತ್ಯಾಗಮ toAPSInt(Sort, Assign, Int, ಸತ್ಯ);
  }

  ತಾರ್ಕಿಕ getInterpretation(ಧ್ರುವ SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST));
    ಯದಿ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    SMTExprRef Assign = new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    ಪ್ರತ್ಯಾಗಮ toAPFloat(Sort, Assign, Float, ಸತ್ಯ);
  }

  std::optional<ತಾರ್ಕಿಕ> check() ಧ್ರುವ override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    ಯದಿ (res == Z3_L_TRUE)
      ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ;

    ಯದಿ (res == Z3_L_FALSE)
      ಪ್ರತ್ಯಾಗಮ ಅಸತ್ಯ;

    ಪ್ರತ್ಯಾಗಮ std::nullopt;
  }

  ಶೂನ್ಯ ತಳ್ಳು() override { ಪ್ರತ್ಯಾಗಮ Z3_solver_ತಳ್ಳು(Context.Context, Solver); }

  ಶೂನ್ಯ ತೆಗೆ(unsigned ಅಂಕೆ_States = 1) override {
    ಪ್ರತಿಜ್ಞಾ(Z3_solver_get_ಅಂಕೆ_scopes(Context.Context, Solver) >= ಅಂಕೆ_States);
    ಪ್ರತ್ಯಾಗಮ Z3_solver_ತೆಗೆ(Context.Context, Solver, ಅಂಕೆ_States);
  }

  ತಾರ್ಕಿಕ isFPSupported() override { ಪ್ರತ್ಯಾಗಮ ಸತ್ಯ; }

  /// Reset the solver and remove all constraints.
  ಶೂನ್ಯ reset() override { Z3_solver_reset(Context.Context, Solver); }

  ಶೂನ್ಯ print(raw_ostream &OS) ಧ್ರುವ override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  ಶೂನ್ಯ setUnsignedParam(StringRef ಕೀಲಿ, unsigned ಮೌಲ್ಯ) override {
    Z3_ಚಿಹ್ನೆ Sym = Z3_mk_string_ಚಿಹ್ನೆ(Context.Context, ಕೀಲಿ.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, ಮೌಲ್ಯ);
  }

  ಶೂನ್ಯ setBoolParam(StringRef ಕೀಲಿ, ತಾರ್ಕಿಕ ಮೌಲ್ಯ) override {
    Z3_ಚಿಹ್ನೆ Sym = Z3_mk_string_ಚಿಹ್ನೆ(Context.Context, ಕೀಲಿ.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, ಮೌಲ್ಯ);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ಧ್ರುವ override;
}; // end class Z3Solver

ವರ್ಗ Z3Statistics final : ಸಾರ್ವಜನಿಕ SMTSolverStatistics {
ಸಾರ್ವಜನಿಕ:
  ದ್ವಿಗುಣ getDouble(StringRef ಕೀಲಿ) ಧ್ರುವ override {
    auto It = Double_ಮೌಲ್ಯಗಳು.find(ಕೀಲಿ.str());
    ಪ್ರತಿಜ್ಞಾ(It != Double_ಮೌಲ್ಯಗಳು.end());
    ಪ್ರತ್ಯಾಗಮ It->second;
  };
  unsigned getUnsigned(StringRef ಕೀಲಿ) ಧ್ರುವ override {
    auto It = Unsigned_ಮೌಲ್ಯಗಳು.find(ಕೀಲಿ.str());
    ಯದಿ (It == Unsigned_ಮೌಲ್ಯಗಳು.end())
      ಪ್ರತ್ಯಾಗಮ 0;
    ಪ್ರತ್ಯಾಗಮ It->second;
  };

  ಶೂನ್ಯ print(raw_ostream &OS) ಧ್ರುವ override {
    ಪ್ರತಿ (ಧ್ರುವ auto &E : Unsigned_ಮೌಲ್ಯಗಳು)
      OS << E.first() << ": " << E.second << '\n';
    ಪ್ರತಿ (ಧ್ರುವ auto &E : Double_ಮೌಲ್ಯಗಳು) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

ಗುಪ್ತ:
  friend ವರ್ಗ Z3Solver;
  StringMap<unsigned> Unsigned_ಮೌಲ್ಯಗಳು;
  StringMap<ದ್ವಿಗುಣ> Double_ಮೌಲ್ಯಗಳು;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ಧ್ರುವ {
  auto ಧ್ರುವ &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned ಅಂಕೆ_ಕೀಲಿಗಳು = Z3_stats_size(C, S);
  ಪ್ರತಿ (unsigned Idx = 0; Idx < ಅಂಕೆ_ಕೀಲಿಗಳು; ++Idx) {
    ಧ್ರುವ ವರ್ಣ *ಕೀಲಿ = Z3_stats_get_ಕೀಲಿ(C, S, Idx);
    ಯದಿ (Z3_stats_is_uint(C, S, Idx)) {
      auto ಮೌಲ್ಯ = Z3_stats_get_uint_ಮೌಲ್ಯ(C, S, Idx);
      Result.Unsigned_ಮೌಲ್ಯಗಳು.try_emplace(ಕೀಲಿ, ಮೌಲ್ಯ);
    } ಅನ್ಯಥಾ {
      ಪ್ರತಿಜ್ಞಾ(Z3_stats_is_double(C, S, Idx));
      auto ಮೌಲ್ಯ = Z3_stats_get_double_ಮೌಲ್ಯ(C, S, Idx);
      Result.Double_ಮೌಲ್ಯಗಳು.try_emplace(ಕೀಲಿ, ಮೌಲ್ಯ);
    }
  }
  ಪ್ರತ್ಯಾಗಮ std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#ಯದಿ LLVM_WITH_Z3
  ಪ್ರತ್ಯಾಗಮ std::make_unique<Z3Solver>();
#ಅನ್ಯಥಾ
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           ಅಸತ್ಯ);
  ಪ್ರತ್ಯಾಗಮ nullptr;
#endif
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ SMTSort::dump() ಧ್ರುವ { print(llvm::errs()); }
LLVM_DUMP_METHOD ಶೂನ್ಯ SMTExpr::dump() ಧ್ರುವ { print(llvm::errs()); }
LLVM_DUMP_METHOD ಶೂನ್ಯ SMTSolver::dump() ಧ್ರುವ { print(llvm::errs()); }
LLVM_DUMP_METHOD ಶೂನ್ಯ SMTSolverStatistics::dump() ಧ್ರುವ { print(llvm::errs()); }
#endif
