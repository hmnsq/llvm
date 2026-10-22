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

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;

#ಆದರೆ LLVM_WITH_Z3

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SmallString.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/StringMap.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Twine.h"

#ಅನ್ತರ್ಭಾವ <set>

#ಅನ್ತರ್ಭಾವ <z3.h>

ನಾಮಾಕಾಶ {

/// Configuration class for Z3
ವರ್ಗ Z3Config {
  ಮಿತ್ರ ವರ್ಗ Z3Context;

  Z3_config Config = Z3_mk_config();

ಸಾರ್ವಜನಿಕ:
  Z3Config() = ಪೂರ್ವನಿಯೋಜಿತ;
  Z3Config(ಸ್ಥಿರಾಂಕ Z3Config &) = ಅಳಿಸು;
  Z3Config(Z3Config &&) = ಪೂರ್ವನಿಯೋಜಿತ;
  Z3Config &ಕ್ರಿಯಾಕಾರಕ=(Z3Config &) = ಅಳಿಸು;
  Z3Config &ಕ್ರಿಯಾಕಾರಕ=(Z3Config &&) = ಪೂರ್ವನಿಯೋಜಿತ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
ಶೂನ್ಯ Z3ErrorHandler(Z3_context Context, Z3_error_ಸಂಕೇತ Error) {
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
    Z3_ಗಣ_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ಸ್ಥಿರಾಂಕ Z3Context &) = ಅಳಿಸು;
  Z3Context(Z3Context &&) = ಪೂರ್ವನಿಯೋಜಿತ;
  Z3Context &ಕ್ರಿಯಾಕಾರಕ=(Z3Context &) = ಅಳಿಸು;
  Z3Context &ಕ್ರಿಯಾಕಾರಕ=(Z3Context &&) = ಪೂರ್ವನಿಯೋಜಿತ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = ಶೂನ್ಯಸೂಚಕ;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
ವರ್ಗ Z3_ವಿಂಗಡಿಸು : ಸಾರ್ವಜನಿಕ SMTSort {
  ಮಿತ್ರ ವರ್ಗ Z3Solver;

  Z3Context &Context;

  Z3_ವಿಂಗಡಿಸು_2 ವಿಂಗಡಿಸು_2;

ಸಾರ್ವಜನಿಕ:
  /// Default constructor, mainly used by make_shared
  Z3_ವಿಂಗಡಿಸು(Z3Context &C, Z3_ವಿಂಗಡಿಸು_2 ZS) : Context(C), ವಿಂಗಡಿಸು_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ವಿಂಗಡಿಸು_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ವಿಂಗಡಿಸು(ಸ್ಥಿರಾಂಕ Z3_ವಿಂಗಡಿಸು &Other) : Context(Other.Context), ವಿಂಗಡಿಸು_2(Other.ವಿಂಗಡಿಸು_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ವಿಂಗಡಿಸು_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ವಿಂಗಡಿಸು &ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ Z3_ವಿಂಗಡಿಸು &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.ವಿಂಗಡಿಸು_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ವಿಂಗಡಿಸು_2));
    ವಿಂಗಡಿಸು_2 = Other.ವಿಂಗಡಿಸು_2;
    ಹಿಂತಿರುಗಿ *ಇದು;
  }

  Z3_ವಿಂಗಡಿಸು(Z3_ವಿಂಗಡಿಸು &&Other) = ಅಳಿಸು;
  Z3_ವಿಂಗಡಿಸು &ಕ್ರಿಯಾಕಾರಕ=(Z3_ವಿಂಗಡಿಸು &&Other) = ಅಳಿಸು;

  ~Z3_ವಿಂಗಡಿಸು() {
    ಆದರೆ (ವಿಂಗಡಿಸು_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ವಿಂಗಡಿಸು_2));
  }

  ಶೂನ್ಯ Profile(llvm::Folding_ಗಣ_ಗಂಟು_ID &ID) ಸ್ಥಿರಾಂಕ override {
    ID.ಕೂಡಿಸು_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(ವಿಂಗಡಿಸು_2)));
  }

  ತಾರ್ಕಿಕ is_Bitvector_ವಿಂಗಡಿಸು_Impl() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ (Z3_get_ವಿಂಗಡಿಸು_ಬಗೆ(Context.Context, ವಿಂಗಡಿಸು_2) == Z3_BV_ವಿಂಗಡಿಸು);
  }

  ತಾರ್ಕಿಕ is_Float_ವಿಂಗಡಿಸು_Impl() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ (Z3_get_ವಿಂಗಡಿಸು_ಬಗೆ(Context.Context, ವಿಂಗಡಿಸು_2) == Z3_FLOATING_ಬಿಂದು_ವಿಂಗಡಿಸು);
  }

  ತಾರ್ಕಿಕ is_Boolean_ವಿಂಗಡಿಸು_Impl() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ (Z3_get_ವಿಂಗಡಿಸು_ಬಗೆ(Context.Context, ವಿಂಗಡಿಸು_2) == Z3_BOOL_ವಿಂಗಡಿಸು);
  }

  ಚಿಹ್ನೆರಹಿತ get_Bitvector_ವಿಂಗಡಿಸು_ಗಾತ್ರ_Impl() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ Z3_get_bv_ವಿಂಗಡಿಸು_ಗಾತ್ರ(Context.Context, ವಿಂಗಡಿಸು_2);
  }

  ಚಿಹ್ನೆರಹಿತ get_Float_ವಿಂಗಡಿಸು_ಗಾತ್ರ_Impl() ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ Z3_fpa_get_ebits(Context.Context, ವಿಂಗಡಿಸು_2) +
           Z3_fpa_get_sbits(Context.Context, ವಿಂಗಡಿಸು_2);
  }

  ತಾರ್ಕಿಕ equal_ಗಮ್ಯ(SMTSort ಸ್ಥಿರಾಂಕ &Other) ಸ್ಥಿರಾಂಕ override {
    ಹಿಂತಿರುಗಿ Z3_is_eq_ವಿಂಗಡಿಸು(Context.Context, ವಿಂಗಡಿಸು_2,
                         static_cast<ಸ್ಥಿರಾಂಕ Z3_ವಿಂಗಡಿಸು &>(Other).ವಿಂಗಡಿಸು_2);
  }

  ಶೂನ್ಯ print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ override {
    OS << Z3_ವಿಂಗಡಿಸು_ಗಮ್ಯ_string(Context.Context, ವಿಂಗಡಿಸು_2);
  }
}; // end class Z3Sort

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Z3_ವಿಂಗಡಿಸು &ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(ಸ್ಥಿರಾಂಕ SMTSort &S) {
  ಹಿಂತಿರುಗಿ static_cast<ಸ್ಥಿರಾಂಕ Z3_ವಿಂಗಡಿಸು &>(S);
}

ವರ್ಗ Z3_ಅಭಿವ್ಯಕ್ತಿ : ಸಾರ್ವಜನಿಕ SMTExpr {
  ಮಿತ್ರ ವರ್ಗ Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

ಸಾರ್ವಜನಿಕ:
  Z3_ಅಭಿವ್ಯಕ್ತಿ(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ಅಭಿವ್ಯಕ್ತಿ(ಸ್ಥಿರಾಂಕ Z3_ಅಭಿವ್ಯಕ್ತಿ &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ಅಭಿವ್ಯಕ್ತಿ &ಕ್ರಿಯಾಕಾರಕ=(ಸ್ಥಿರಾಂಕ Z3_ಅಭಿವ್ಯಕ್ತಿ &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    ಹಿಂತಿರುಗಿ *ಇದು;
  }

  Z3_ಅಭಿವ್ಯಕ್ತಿ(Z3_ಅಭಿವ್ಯಕ್ತಿ &&Other) = ಅಳಿಸು;
  Z3_ಅಭಿವ್ಯಕ್ತಿ &ಕ್ರಿಯಾಕಾರಕ=(Z3_ಅಭಿವ್ಯಕ್ತಿ &&Other) = ಅಳಿಸು;

  ~Z3_ಅಭಿವ್ಯಕ್ತಿ() {
    ಆದರೆ (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  ಶೂನ್ಯ Profile(llvm::Folding_ಗಣ_ಗಂಟು_ID &ID) ಸ್ಥಿರಾಂಕ override {
    ID.ಕೂಡಿಸು_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  ತಾರ್ಕಿಕ equal_ಗಮ್ಯ(SMTExpr ಸ್ಥಿರಾಂಕ &Other) ಸ್ಥಿರಾಂಕ override {
    ಪ್ರತಿಜ್ಞಾ(Z3_is_eq_ವಿಂಗಡಿಸು(Context.Context, Z3_get_ವಿಂಗಡಿಸು(Context.Context, AST),
                         Z3_get_ವಿಂಗಡಿಸು(Context.Context,
                                     static_cast<ಸ್ಥಿರಾಂಕ Z3_ಅಭಿವ್ಯಕ್ತಿ &>(Other).AST)) &&
           "AST's must have the same sort");
    ಹಿಂತಿರುಗಿ Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ಸ್ಥಿರಾಂಕ Z3_ಅಭಿವ್ಯಕ್ತಿ &>(Other).AST);
  }

  ಶೂನ್ಯ print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ override {
    OS << Z3_ast_ಗಮ್ಯ_string(Context.Context, AST);
  }
}; // end class Z3Expr

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ Z3_ಅಭಿವ್ಯಕ್ತಿ &ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(ಸ್ಥಿರಾಂಕ SMTExpr &E) {
  ಹಿಂತಿರುಗಿ static_cast<ಸ್ಥಿರಾಂಕ Z3_ಅಭಿವ್ಯಕ್ತಿ &>(E);
}

ವರ್ಗ Z3Model {
  ಮಿತ್ರ ವರ್ಗ Z3Solver;

  Z3Context &Context;

  Z3_model Model;

ಸಾರ್ವಜನಿಕ:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ಸ್ಥಿರಾಂಕ Z3Model &Other) = ಅಳಿಸು;
  Z3Model(Z3Model &&Other) = ಅಳಿಸು;
  Z3Model &ಕ್ರಿಯಾಕಾರಕ=(Z3Model &Other) = ಅಳಿಸು;
  Z3Model &ಕ್ರಿಯಾಕಾರಕ=(Z3Model &&Other) = ಅಳಿಸು;

  ~Z3Model() {
    ಆದರೆ (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  ಶೂನ್ಯ print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ {
    OS << Z3_model_ಗಮ್ಯ_string(Context.Context, Model);
  }

  LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ dump() ಸ್ಥಿರಾಂಕ { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ llvm::fltSemantics &getFloatSemantics(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) {
  ಆಯ್ಕೆ (ಬಿಟ್_ಅಗಲ_2) {
  ಪೂರ್ವನಿಯೋಜಿತ:
    llvm_unreachable("Unsupported floating-point semantics!");
    ನಿಲ್ಲಿಸು;
  ಸ್ಥಿತಿ 16:
    ಹಿಂತಿರುಗಿ llvm::APFloat::IEEEhalf();
  ಸ್ಥಿತಿ 32:
    ಹಿಂತಿರುಗಿ llvm::APFloat::IEEEsingle();
  ಸ್ಥಿತಿ 64:
    ಹಿಂತಿರುಗಿ llvm::APFloat::IEEEdouble();
  ಸ್ಥಿತಿ 128:
    ಹಿಂತಿರುಗಿ llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ಸ್ಥಿರ ತಾರ್ಕಿಕ areEquivalent(ಸ್ಥಿರಾಂಕ llvm::fltSemantics &LHS,
                          ಸ್ಥಿರಾಂಕ llvm::fltSemantics &RHS) {
  ಹಿಂತಿರುಗಿ (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(LHS) ==
          llvm::APFloat::semantics_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(RHS));
}

ವರ್ಗ Z3Solver : ಸಾರ್ವಜನಿಕ SMTSolver {
  ಮಿತ್ರ ವರ್ಗ Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [ಇದು] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    ಹಿಂತಿರುಗಿ S;
  }();

  Z3_params Params = [ಇದು] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    ಹಿಂತಿರುಗಿ P;
  }();

  // Cache Sorts
  std::ಗಣ<Z3_ವಿಂಗಡಿಸು> CachedSorts;

  // Cache Exprs
  std::ಗಣ<Z3_ಅಭಿವ್ಯಕ್ತಿ> CachedExprs;

ಸಾರ್ವಜನಿಕ:
  Z3Solver() = ಪೂರ್ವನಿಯೋಜಿತ;
  Z3Solver(ಸ್ಥಿರಾಂಕ Z3Solver &Other) = ಅಳಿಸು;
  Z3Solver(Z3Solver &&Other) = ಅಳಿಸು;
  Z3Solver &ಕ್ರಿಯಾಕಾರಕ=(Z3Solver &Other) = ಅಳಿಸು;
  Z3Solver &ಕ್ರಿಯಾಕಾರಕ=(Z3Solver &&Other) = ಅಳಿಸು;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  ಶೂನ್ಯ ಕೂಡಿಸು_Constraint(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) ಸ್ಥಿರಾಂಕ override {
    Z3_solver_assert(Context.Context, Solver, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_ವಿಂಗಡಿಸು_Ref(ಸ್ಥಿರಾಂಕ SMTSort &ವಿಂಗಡಿಸು_2) {
    ಸ್ವಯಂಚಾಲಿತ It = CachedSorts.ಸೇರಿಸು(ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(ವಿಂಗಡಿಸು_2));
    ಹಿಂತಿರುಗಿ &(*It.ಮೊದಲ_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_ಅಭಿವ್ಯಕ್ತಿ_Ref(ಸ್ಥಿರಾಂಕ SMTExpr &Exp) {
    ಸ್ವಯಂಚಾಲಿತ It = CachedExprs.ಸೇರಿಸು(ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(Exp));
    ಹಿಂತಿರುಗಿ &(*It.ಮೊದಲ_2);
  }

  SMTSortRef get_Bool_ವಿಂಗಡಿಸು() override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(Z3_ವಿಂಗಡಿಸು(Context, Z3_mk_bool_ವಿಂಗಡಿಸು(Context.Context)));
  }

  SMTSortRef get_Bitvector_ವಿಂಗಡಿಸು(ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(
        Z3_ವಿಂಗಡಿಸು(Context, Z3_mk_bv_ವಿಂಗಡಿಸು(Context.Context, ಬಿಟ್_ಅಗಲ_2)));
  }

  SMTSortRef get_ವಿಂಗಡಿಸು(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(
        Z3_ವಿಂಗಡಿಸು(Context, Z3_get_ವಿಂಗಡಿಸು(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTSortRef get_Float16_ವಿಂಗಡಿಸು() override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(Z3_ವಿಂಗಡಿಸು(Context, Z3_mk_fpa_ವಿಂಗಡಿಸು_16(Context.Context)));
  }

  SMTSortRef get_Float32_ವಿಂಗಡಿಸು() override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(Z3_ವಿಂಗಡಿಸು(Context, Z3_mk_fpa_ವಿಂಗಡಿಸು_32(Context.Context)));
  }

  SMTSortRef get_Float64_ವಿಂಗಡಿಸು() override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(Z3_ವಿಂಗಡಿಸು(Context, Z3_mk_fpa_ವಿಂಗಡಿಸು_64(Context.Context)));
  }

  SMTSortRef get_Float128_ವಿಂಗಡಿಸು() override {
    ಹಿಂತಿರುಗಿ new_ವಿಂಗಡಿಸು_Ref(Z3_ವಿಂಗಡಿಸು(Context, Z3_mk_fpa_ವಿಂಗಡಿಸು_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvneg(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvnot(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkNot(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_not(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvadd(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsub(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvmul(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsrem(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvurem(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsdiv(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvudiv(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvshl(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvashr(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvlshr(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvxor(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvor(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                   ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvand(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvult(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvslt(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvugt(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsgt(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvule(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsle(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvuge(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_bvsge(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkAnd(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    Z3_ast ವಾದಗಳು[2] = {ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST};
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_and(Context.Context, 2, ವಾದಗಳು)));
  }

  SMTExprRef mkOr(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    Z3_ast ವಾದಗಳು[2] = {ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST};
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_or(Context.Context, 2, ವಾದಗಳು)));
  }

  SMTExprRef mkEqual(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_eq(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                 ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_neg(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_is_infinite(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_is_nan(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_ಸಾಮಾನ್ಯ(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_is_ಸಾಮಾನ್ಯ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_is_zero(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_mul(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                             ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_div(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                             ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_rem(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                      ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_ಕೂಡಿಸು(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                             ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_mk_fpa_sub(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                             ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_lt(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_gt(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_leq(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                      ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_geq(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                      ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_eq(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkIte(ಸ್ಥಿರಾಂಕ SMTExprRef &Cond, ಸ್ಥಿರಾಂಕ SMTExprRef &T,
                   ಸ್ಥಿರಾಂಕ SMTExprRef &F) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_ite(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Cond).AST,
                                  ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*T).AST, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*F).AST)));
  }

  SMTExprRef mkBVSignExt(ಚಿಹ್ನೆರಹಿತ i, ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_sign_ext(Context.Context, i, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(ಚಿಹ್ನೆರಹಿತ i, ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_zero_ext(Context.Context, i, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(ಚಿಹ್ನೆರಹಿತ ಮೇಲಿನ_2, ಚಿಹ್ನೆರಹಿತ ಕೆಳಗಿನ_2,
                         ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_ಹೊರತೆಗೆ(Context.Context, ಮೇಲಿನ_2, ಕೆಳಗಿನ_2,
                                                    ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_ಅತಿಪ್ರವಾಹ(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS,
                               ತಾರ್ಕಿಕ isSigned) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvadd_no_ಅತಿಪ್ರವಾಹ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                         ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS,
                                ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_ಅತಿಪ್ರವಾಹ(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS,
                               ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvsub_no_ಅತಿಪ್ರವಾಹ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                         ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS,
                                ತಾರ್ಕಿಕ isSigned) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_ಅತಿಪ್ರವಾಹ(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS,
                                ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvsdiv_no_ಅತಿಪ್ರವಾಹ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_ಅತಿಪ್ರವಾಹ(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvneg_no_ಅತಿಪ್ರವಾಹ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_ಅತಿಪ್ರವಾಹ(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS,
                               ತಾರ್ಕಿಕ isSigned) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvmul_no_ಅತಿಪ್ರವಾಹ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                         ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS,
                                ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                          ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ಸ್ಥಿರಾಂಕ SMTExprRef &LHS, ಸ್ಥಿರಾಂಕ SMTExprRef &RHS) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_concat(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*LHS).AST,
                                     ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ಸ್ಥಿರಾಂಕ SMTExprRef &From, ಸ್ಥಿರಾಂಕ SMTSortRef &ಗಮ್ಯ) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context,
        Z3_mk_fpa_ಗಮ್ಯ_fp_float(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                              ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(*ಗಮ್ಯ).ವಿಂಗಡಿಸು_2)));
  }

  SMTExprRef mkSBVtoFP(ಸ್ಥಿರಾಂಕ SMTExprRef &From, ಸ್ಥಿರಾಂಕ SMTSortRef &ಗಮ್ಯ) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context,
        Z3_mk_fpa_ಗಮ್ಯ_fp_signed(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                               ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(*ಗಮ್ಯ).ವಿಂಗಡಿಸು_2)));
  }

  SMTExprRef mkUBVtoFP(ಸ್ಥಿರಾಂಕ SMTExprRef &From, ಸ್ಥಿರಾಂಕ SMTSortRef &ಗಮ್ಯ) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context,
        Z3_mk_fpa_ಗಮ್ಯ_fp_unsigned(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                                 ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(*ಗಮ್ಯ).ವಿಂಗಡಿಸು_2)));
  }

  SMTExprRef mkFPtoSBV(ಸ್ಥಿರಾಂಕ SMTExprRef &From, ಚಿಹ್ನೆರಹಿತ ಗಮ್ಯ_ಅಗಲ) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_ಗಮ್ಯ_sbv(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                                  ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ಗಮ್ಯ_ಅಗಲ)));
  }

  SMTExprRef mkFPtoUBV(ಸ್ಥಿರಾಂಕ SMTExprRef &From, ಚಿಹ್ನೆರಹಿತ ಗಮ್ಯ_ಅಗಲ) override {
    SMTExprRef Rounding_ಬಹುಲಕ = get_Float_Rounding_ಬಹುಲಕ();
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_ಗಮ್ಯ_ubv(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Rounding_ಬಹುಲಕ).AST,
                                  ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*From).AST, ಗಮ್ಯ_ಅಗಲ)));
  }

  SMTExprRef mkBoolean(ಸ್ಥಿರಾಂಕ ತಾರ್ಕಿಕ b) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ಸ್ಥಿರಾಂಕ llvm::APSInt Int, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2) override {
    ಸ್ಥಿರಾಂಕ Z3_ವಿಂಗಡಿಸು_2 Z3_ವಿಂಗಡಿಸು = ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(*get_Bitvector_ವಿಂಗಡಿಸು(ಬಿಟ್_ಅಗಲ_2)).ವಿಂಗಡಿಸು_2;

    // Slow path, when 64 bits are not enough.
    ಆದರೆ (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> ಬಫರ್;
      Int.ಗಮ್ಯ_String(ಬಫರ್, 10);
      ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
          Context, Z3_mk_numeral(Context.Context, ಬಫರ್.c_str(), Z3_ವಿಂಗಡಿಸು)));
    }

    ಸ್ಥಿರಾಂಕ int64_t ಬಿಟ್_Repr_As_Signed = Int.get_Ext_ಮೌಲ್ಯ();
    ಸ್ಥಿರಾಂಕ uint64_t ಬಿಟ್_Repr_As_Unsigned =
        reinterpret_cast<ಸ್ಥಿರಾಂಕ uint64_t &>(ಬಿಟ್_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, ಬಿಟ್_Repr_As_Signed, Z3_ವಿಂಗಡಿಸು)
            : Z3_mk_unsigned_int64(Context.Context, ಬಿಟ್_Repr_As_Unsigned, Z3_ವಿಂಗಡಿಸು);
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Literal));
  }

  SMTExprRef mkFloat(ಸ್ಥಿರಾಂಕ llvm::APFloat Float) override {
    SMTSortRef ವಿಂಗಡಿಸು_2 =
        get_Float_ವಿಂಗಡಿಸು(llvm::APFloat::semantics_ಗಾತ್ರ_ಒಳ_ಬಿಟ್ಟುಗಳು(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_ಗಮ್ಯ_APInt(), ಸುಳ್ಳು);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_ಬಿಟ್_ಅಗಲ());
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(
        Context, Z3_mk_fpa_ಗಮ್ಯ_fp_bv(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Z3Int).AST,
                                    ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(*ವಿಂಗಡಿಸು_2).ವಿಂಗಡಿಸು_2)));
  }

  SMTExprRef mk_ಚಿಹ್ನೆ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *Name, SMTSortRef ವಿಂಗಡಿಸು_2) override {
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_ಚಿಹ್ನೆ(Context.Context, Name),
                                    ಗಮ್ಯ_Z3_ವಿಂಗಡಿಸು(*ವಿಂಗಡಿಸು_2).ವಿಂಗಡಿಸು_2)));
  }

  llvm::APSInt getBitvector(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp, ಚಿಹ್ನೆರಹಿತ ಬಿಟ್_ಅಗಲ_2,
                            ತಾರ್ಕಿಕ isUnsigned) override {
    ಹಿಂತಿರುಗಿ llvm::APSInt(
        llvm::APInt(ಬಿಟ್_ಅಗಲ_2,
                    Z3_get_numeral_string(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST),
                    10),
        isUnsigned);
  }

  ತಾರ್ಕಿಕ getBoolean(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp) override {
    ಹಿಂತಿರುಗಿ Z3_get_bool_ಮೌಲ್ಯ(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_ಬಹುಲಕ() override {
    // TODO: Don't assume nearest ties to even rounding mode
    ಹಿಂತಿರುಗಿ new_ಅಭಿವ್ಯಕ್ತಿ_Ref(Z3_ಅಭಿವ್ಯಕ್ತಿ(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  ತಾರ್ಕಿಕ ಗಮ್ಯ_APFloat(ಸ್ಥಿರಾಂಕ SMTSortRef &ವಿಂಗಡಿಸು_2, ಸ್ಥಿರಾಂಕ SMTExprRef &AST,
                 llvm::APFloat &Float, ತಾರ್ಕಿಕ useSemantics) {
    ಪ್ರತಿಜ್ಞಾ(ವಿಂಗಡಿಸು_2->is_Float_ವಿಂಗಡಿಸು() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(ವಿಂಗಡಿಸು_2->get_Float_ವಿಂಗಡಿಸು_ಗಾತ್ರ(), ನಿಜ);
    ಸ್ಥಿರಾಂಕ llvm::fltSemantics &Semantics =
        getFloatSemantics(ವಿಂಗಡಿಸು_2->get_Float_ವಿಂಗಡಿಸು_ಗಾತ್ರ());
    SMTSortRef BVSort = get_Bitvector_ವಿಂಗಡಿಸು(ವಿಂಗಡಿಸು_2->get_Float_ವಿಂಗಡಿಸು_ಗಾತ್ರ());
    ಆದರೆ (!ಗಮ್ಯ_APSInt(BVSort, AST, Int, ನಿಜ)) {
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }

    ಆದರೆ (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      ಪ್ರತಿಜ್ಞಾ(ಸುಳ್ಳು && "Floating-point types don't match!");
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
    }

    Float = llvm::APFloat(Semantics, Int);
    ಹಿಂತಿರುಗಿ ನಿಜ;
  }

  ತಾರ್ಕಿಕ ಗಮ್ಯ_APSInt(ಸ್ಥಿರಾಂಕ SMTSortRef &ವಿಂಗಡಿಸು_2, ಸ್ಥಿರಾಂಕ SMTExprRef &AST,
                llvm::APSInt &Int, ತಾರ್ಕಿಕ useSemantics) {
    ಆದರೆ (ವಿಂಗಡಿಸು_2->is_Bitvector_ವಿಂಗಡಿಸು()) {
      ಆದರೆ (useSemantics && Int.get_ಬಿಟ್_ಅಗಲ() != ವಿಂಗಡಿಸು_2->get_Bitvector_ವಿಂಗಡಿಸು_ಗಾತ್ರ()) {
        ಪ್ರತಿಜ್ಞಾ(ಸುಳ್ಳು && "Bitvector types don't match!");
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_ಬಿಟ್_ಅಗಲ(), Int.isUnsigned());
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    ಆದರೆ (ವಿಂಗಡಿಸು_2->is_Boolean_ವಿಂಗಡಿಸು()) {
      ಆದರೆ (useSemantics && Int.get_ಬಿಟ್_ಅಗಲ() < 1) {
        ಪ್ರತಿಜ್ಞಾ(ಸುಳ್ಳು && "Boolean type doesn't match!");
        ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_ಬಿಟ್_ಅಗಲ(), getBoolean(AST)),
                         Int.isUnsigned());
      ಹಿಂತಿರುಗಿ ನಿಜ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  ತಾರ್ಕಿಕ getInterpretation(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_ಗಮ್ಯ_app(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST));
    ಆದರೆ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    SMTExprRef ನಿಯೋಜಿಸು_2 = new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ವಿಂಗಡಿಸು_2 = get_ವಿಂಗಡಿಸು(ನಿಯೋಜಿಸು_2);
    ಹಿಂತಿರುಗಿ ಗಮ್ಯ_APSInt(ವಿಂಗಡಿಸು_2, ನಿಯೋಜಿಸು_2, Int, ನಿಜ);
  }

  ತಾರ್ಕಿಕ getInterpretation(ಸ್ಥಿರಾಂಕ SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_ಗಮ್ಯ_app(Context.Context, ಗಮ್ಯ_Z3_ಅಭಿವ್ಯಕ್ತಿ(*Exp).AST));
    ಆದರೆ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    SMTExprRef ನಿಯೋಜಿಸು_2 = new_ಅಭಿವ್ಯಕ್ತಿ_Ref(
        Z3_ಅಭಿವ್ಯಕ್ತಿ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ವಿಂಗಡಿಸು_2 = get_ವಿಂಗಡಿಸು(ನಿಯೋಜಿಸು_2);
    ಹಿಂತಿರುಗಿ ಗಮ್ಯ_APFloat(ವಿಂಗಡಿಸು_2, ನಿಯೋಜಿಸು_2, Float, ನಿಜ);
  }

  std::optional<ತಾರ್ಕಿಕ> check() ಸ್ಥಿರಾಂಕ override {
    Z3_solver_ಗಣ_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    ಆದರೆ (res == Z3_L_TRUE)
      ಹಿಂತಿರುಗಿ ನಿಜ;

    ಆದರೆ (res == Z3_L_FALSE)
      ಹಿಂತಿರುಗಿ ಸುಳ್ಳು;

    ಹಿಂತಿರುಗಿ std::nullopt;
  }

  ಶೂನ್ಯ ತಳ್ಳು() override { ಹಿಂತಿರುಗಿ Z3_solver_ತಳ್ಳು(Context.Context, Solver); }

  ಶೂನ್ಯ ತೆಗೆ(ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಸ್ಥಿತಿಗಳು = 1) override {
    ಪ್ರತಿಜ್ಞಾ(Z3_solver_get_ಅಂಕೆ_scopes(Context.Context, Solver) >= ಅಂಕೆ_ಸ್ಥಿತಿಗಳು);
    ಹಿಂತಿರುಗಿ Z3_solver_ತೆಗೆ(Context.Context, Solver, ಅಂಕೆ_ಸ್ಥಿತಿಗಳು);
  }

  ತಾರ್ಕಿಕ isFPSupported() override { ಹಿಂತಿರುಗಿ ನಿಜ; }

  /// Reset the solver and remove all constraints.
  ಶೂನ್ಯ reset() override { Z3_solver_reset(Context.Context, Solver); }

  ಶೂನ್ಯ print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ override {
    OS << Z3_solver_ಗಮ್ಯ_string(Context.Context, Solver);
  }

  ಶೂನ್ಯ ಗಣ_Unsigned_Param(StringRef ಕೀಲಿ, ಚಿಹ್ನೆರಹಿತ ಮೌಲ್ಯ) override {
    Z3_ಚಿಹ್ನೆ Sym = Z3_mk_string_ಚಿಹ್ನೆ(Context.Context, ಕೀಲಿ.str().c_str());
    Z3_params_ಗಣ_uint(Context.Context, Params, Sym, ಮೌಲ್ಯ);
  }

  ಶೂನ್ಯ ಗಣ_Bool_Param(StringRef ಕೀಲಿ, ತಾರ್ಕಿಕ ಮೌಲ್ಯ) override {
    Z3_ಚಿಹ್ನೆ Sym = Z3_mk_string_ಚಿಹ್ನೆ(Context.Context, ಕೀಲಿ.str().c_str());
    Z3_params_ಗಣ_bool(Context.Context, Params, Sym, ಮೌಲ್ಯ);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ಸ್ಥಿರಾಂಕ override;
}; // end class Z3Solver

ವರ್ಗ Z3Statistics final : ಸಾರ್ವಜನಿಕ SMTSolverStatistics {
ಸಾರ್ವಜನಿಕ:
  ದ್ವಿಗುಣ getDouble(StringRef ಕೀಲಿ) ಸ್ಥಿರಾಂಕ override {
    ಸ್ವಯಂಚಾಲಿತ It = Double_ಮೌಲ್ಯಗಳು.ಕಂಡುಹಿಡಿ(ಕೀಲಿ.str());
    ಪ್ರತಿಜ್ಞಾ(It != Double_ಮೌಲ್ಯಗಳು.ಕೊನೆ());
    ಹಿಂತಿರುಗಿ It->ಎರಡನೇ_2;
  };
  ಚಿಹ್ನೆರಹಿತ getUnsigned(StringRef ಕೀಲಿ) ಸ್ಥಿರಾಂಕ override {
    ಸ್ವಯಂಚಾಲಿತ It = Unsigned_ಮೌಲ್ಯಗಳು.ಕಂಡುಹಿಡಿ(ಕೀಲಿ.str());
    ಆದರೆ (It == Unsigned_ಮೌಲ್ಯಗಳು.ಕೊನೆ())
      ಹಿಂತಿರುಗಿ 0;
    ಹಿಂತಿರುಗಿ It->ಎರಡನೇ_2;
  };

  ಶೂನ್ಯ print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ override {
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &E : Unsigned_ಮೌಲ್ಯಗಳು)
      OS << E.ಮೊದಲ_2() << ": " << E.ಎರಡನೇ_2 << '\n';
    ಗಾಗಿ (ಸ್ಥಿರಾಂಕ ಸ್ವಯಂಚಾಲಿತ &E : Double_ಮೌಲ್ಯಗಳು) {
      write_double(OS << E.ಮೊದಲ_2() << ": ", E.ಎರಡನೇ_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

ಖಾಸಗಿ:
  ಮಿತ್ರ ವರ್ಗ Z3Solver;
  StringMap<ಚಿಹ್ನೆರಹಿತ> Unsigned_ಮೌಲ್ಯಗಳು;
  StringMap<ದ್ವಿಗುಣ> Double_ಮೌಲ್ಯಗಳು;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ಸ್ಥಿರಾಂಕ {
  ಸ್ವಯಂಚಾಲಿತ ಸ್ಥಿರಾಂಕ &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics ಫಲಿತಾಂಶ;

  ಚಿಹ್ನೆರಹಿತ ಅಂಕೆ_ಕೀಲಿಗಳು = Z3_stats_ಗಾತ್ರ(C, S);
  ಗಾಗಿ (ಚಿಹ್ನೆರಹಿತ Idx = 0; Idx < ಅಂಕೆ_ಕೀಲಿಗಳು; ++Idx) {
    ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೀಲಿ = Z3_stats_get_ಕೀಲಿ(C, S, Idx);
    ಆದರೆ (Z3_stats_is_uint(C, S, Idx)) {
      ಸ್ವಯಂಚಾಲಿತ ಮೌಲ್ಯ = Z3_stats_get_uint_ಮೌಲ್ಯ(C, S, Idx);
      ಫಲಿತಾಂಶ.Unsigned_ಮೌಲ್ಯಗಳು.try_emplace(ಕೀಲಿ, ಮೌಲ್ಯ);
    } ಇಲ್ಲದಿದ್ದರೆ {
      ಪ್ರತಿಜ್ಞಾ(Z3_stats_is_double(C, S, Idx));
      ಸ್ವಯಂಚಾಲಿತ ಮೌಲ್ಯ = Z3_stats_get_double_ಮೌಲ್ಯ(C, S, Idx);
      ಫಲಿತಾಂಶ.Double_ಮೌಲ್ಯಗಳು.try_emplace(ಕೀಲಿ, ಮೌಲ್ಯ);
    }
  }
  ಹಿಂತಿರುಗಿ std::make_unique<Z3Statistics>(std::move(ಫಲಿತಾಂಶ));
}

} // end anonymous namespace

#ಆದರೆ_ಅಂತ್ಯ

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#ಆದರೆ LLVM_WITH_Z3
  ಹಿಂತಿರುಗಿ std::make_unique<Z3Solver>();
#ಇಲ್ಲದಿದ್ದರೆ
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           ಸುಳ್ಳು);
  ಹಿಂತಿರುಗಿ ಶೂನ್ಯಸೂಚಕ;
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ SMTSort::dump() ಸ್ಥಿರಾಂಕ { print(llvm::errs()); }
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ SMTExpr::dump() ಸ್ಥಿರಾಂಕ { print(llvm::errs()); }
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ SMTSolver::dump() ಸ್ಥಿರಾಂಕ { print(llvm::errs()); }
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ SMTSolverStatistics::dump() ಸ್ಥಿರಾಂಕ { print(llvm::errs()); }
#ಆದರೆ_ಅಂತ್ಯ
