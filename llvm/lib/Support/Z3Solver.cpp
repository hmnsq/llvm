//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/ScopeExit.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Config/config.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/NativeFormatting.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/SMTAPI.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

#ਜੇ LLVM_WITH_Z3

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SmallString.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringMap.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Twine.h"

#ਅਨ੍ਤਰ੍ਭਾਵ <set>

#ਅਨ੍ਤਰ੍ਭਾਵ <z3.h>

ਨਾਮਾਕਾਸ਼ {

/// Configuration class for Z3
ਵਰਗ Z3Config {
  ਮਿੱਤਰ ਵਰਗ Z3Context;

  Z3_config Config = Z3_mk_config();

ਜਨਤਕ:
  Z3Config() = ਮੂਲ;
  Z3Config(ਅਚਲ Z3Config &) = ਮਿਟਾਓ;
  Z3Config(Z3Config &&) = ਮੂਲ;
  Z3Config &ਸੰਚਾਲਕ=(Z3Config &) = ਮਿਟਾਓ;
  Z3Config &ਸੰਚਾਲਕ=(Z3Config &&) = ਮੂਲ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
ਖਾਲੀ Z3ErrorHandler(Z3_context Context, Z3_error_ਕੋਡ Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_ਸੁਨੇਹਾ(Context, Error)));
}

/// Wrapper for Z3 context
ਵਰਗ Z3Context {
ਜਨਤਕ:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_ਸੈੱਟ_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ਅਚਲ Z3Context &) = ਮਿਟਾਓ;
  Z3Context(Z3Context &&) = ਮੂਲ;
  Z3Context &ਸੰਚਾਲਕ=(Z3Context &) = ਮਿਟਾਓ;
  Z3Context &ਸੰਚਾਲਕ=(Z3Context &&) = ਮੂਲ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = ਖਾਲੀ_ਸੂਚਕ;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
ਵਰਗ Z3_ਛਾਂਟੋ : ਜਨਤਕ SMTSort {
  ਮਿੱਤਰ ਵਰਗ Z3Solver;

  Z3Context &Context;

  Z3_ਛਾਂਟੋ_2 ਛਾਂਟੋ_2;

ਜਨਤਕ:
  /// Default constructor, mainly used by make_shared
  Z3_ਛਾਂਟੋ(Z3Context &C, Z3_ਛਾਂਟੋ_2 ZS) : Context(C), ਛਾਂਟੋ_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ਛਾਂਟੋ_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ਛਾਂਟੋ(ਅਚਲ Z3_ਛਾਂਟੋ &Other) : Context(Other.Context), ਛਾਂਟੋ_2(Other.ਛਾਂਟੋ_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ਛਾਂਟੋ_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ਛਾਂਟੋ &ਸੰਚਾਲਕ=(ਅਚਲ Z3_ਛਾਂਟੋ &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.ਛਾਂਟੋ_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ਛਾਂਟੋ_2));
    ਛਾਂਟੋ_2 = Other.ਛਾਂਟੋ_2;
    ਵਾਪਸ *ਇਹ;
  }

  Z3_ਛਾਂਟੋ(Z3_ਛਾਂਟੋ &&Other) = ਮਿਟਾਓ;
  Z3_ਛਾਂਟੋ &ਸੰਚਾਲਕ=(Z3_ਛਾਂਟੋ &&Other) = ਮਿਟਾਓ;

  ~Z3_ਛਾਂਟੋ() {
    ਜੇ (ਛਾਂਟੋ_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ਛਾਂਟੋ_2));
  }

  ਖਾਲੀ Profile(llvm::Folding_ਸੈੱਟ_ਗੰਢ_ID &ID) ਅਚਲ override {
    ID.ਜੋੜੋ_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(ਛਾਂਟੋ_2)));
  }

  ਤਾਰਕਿਕ is_Bitvector_ਛਾਂਟੋ_Impl() ਅਚਲ override {
    ਵਾਪਸ (Z3_get_ਛਾਂਟੋ_ਕਿਸਮ(Context.Context, ਛਾਂਟੋ_2) == Z3_BV_ਛਾਂਟੋ);
  }

  ਤਾਰਕਿਕ is_Float_ਛਾਂਟੋ_Impl() ਅਚਲ override {
    ਵਾਪਸ (Z3_get_ਛਾਂਟੋ_ਕਿਸਮ(Context.Context, ਛਾਂਟੋ_2) == Z3_FLOATING_ਬਿੰਦੂ_ਛਾਂਟੋ);
  }

  ਤਾਰਕਿਕ is_Boolean_ਛਾਂਟੋ_Impl() ਅਚਲ override {
    ਵਾਪਸ (Z3_get_ਛਾਂਟੋ_ਕਿਸਮ(Context.Context, ਛਾਂਟੋ_2) == Z3_BOOL_ਛਾਂਟੋ);
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ get_Bitvector_ਛਾਂਟੋ_ਮਾਪ_Impl() ਅਚਲ override {
    ਵਾਪਸ Z3_get_bv_ਛਾਂਟੋ_ਮਾਪ(Context.Context, ਛਾਂਟੋ_2);
  }

  ਬਿਨਾਨਿਸ਼ਾਨੀ get_Float_ਛਾਂਟੋ_ਮਾਪ_Impl() ਅਚਲ override {
    ਵਾਪਸ Z3_fpa_get_ebits(Context.Context, ਛਾਂਟੋ_2) +
           Z3_fpa_get_sbits(Context.Context, ਛਾਂਟੋ_2);
  }

  ਤਾਰਕਿਕ equal_ਮੰਜ਼ਿਲ(SMTSort ਅਚਲ &Other) ਅਚਲ override {
    ਵਾਪਸ Z3_is_eq_ਛਾਂਟੋ(Context.Context, ਛਾਂਟੋ_2,
                         static_cast<ਅਚਲ Z3_ਛਾਂਟੋ &>(Other).ਛਾਂਟੋ_2);
  }

  ਖਾਲੀ print(raw_ostream &OS) ਅਚਲ override {
    OS << Z3_ਛਾਂਟੋ_ਮੰਜ਼ਿਲ_string(Context.Context, ਛਾਂਟੋ_2);
  }
}; // end class Z3Sort

ਸਥਿਰ ਅਚਲ Z3_ਛਾਂਟੋ &ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(ਅਚਲ SMTSort &S) {
  ਵਾਪਸ static_cast<ਅਚਲ Z3_ਛਾਂਟੋ &>(S);
}

ਵਰਗ Z3_ਵਿਅੰਜਕ : ਜਨਤਕ SMTExpr {
  ਮਿੱਤਰ ਵਰਗ Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

ਜਨਤਕ:
  Z3_ਵਿਅੰਜਕ(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ਵਿਅੰਜਕ(ਅਚਲ Z3_ਵਿਅੰਜਕ &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ਵਿਅੰਜਕ &ਸੰਚਾਲਕ=(ਅਚਲ Z3_ਵਿਅੰਜਕ &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    ਵਾਪਸ *ਇਹ;
  }

  Z3_ਵਿਅੰਜਕ(Z3_ਵਿਅੰਜਕ &&Other) = ਮਿਟਾਓ;
  Z3_ਵਿਅੰਜਕ &ਸੰਚਾਲਕ=(Z3_ਵਿਅੰਜਕ &&Other) = ਮਿਟਾਓ;

  ~Z3_ਵਿਅੰਜਕ() {
    ਜੇ (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  ਖਾਲੀ Profile(llvm::Folding_ਸੈੱਟ_ਗੰਢ_ID &ID) ਅਚਲ override {
    ID.ਜੋੜੋ_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  ਤਾਰਕਿਕ equal_ਮੰਜ਼ਿਲ(SMTExpr ਅਚਲ &Other) ਅਚਲ override {
    ਪ੍ਰਤਿਜ੍ਞਾ(Z3_is_eq_ਛਾਂਟੋ(Context.Context, Z3_get_ਛਾਂਟੋ(Context.Context, AST),
                         Z3_get_ਛਾਂਟੋ(Context.Context,
                                     static_cast<ਅਚਲ Z3_ਵਿਅੰਜਕ &>(Other).AST)) &&
           "AST's must have the same sort");
    ਵਾਪਸ Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ਅਚਲ Z3_ਵਿਅੰਜਕ &>(Other).AST);
  }

  ਖਾਲੀ print(raw_ostream &OS) ਅਚਲ override {
    OS << Z3_ast_ਮੰਜ਼ਿਲ_string(Context.Context, AST);
  }
}; // end class Z3Expr

ਸਥਿਰ ਅਚਲ Z3_ਵਿਅੰਜਕ &ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(ਅਚਲ SMTExpr &E) {
  ਵਾਪਸ static_cast<ਅਚਲ Z3_ਵਿਅੰਜਕ &>(E);
}

ਵਰਗ Z3Model {
  ਮਿੱਤਰ ਵਰਗ Z3Solver;

  Z3Context &Context;

  Z3_model Model;

ਜਨਤਕ:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ਅਚਲ Z3Model &Other) = ਮਿਟਾਓ;
  Z3Model(Z3Model &&Other) = ਮਿਟਾਓ;
  Z3Model &ਸੰਚਾਲਕ=(Z3Model &Other) = ਮਿਟਾਓ;
  Z3Model &ਸੰਚਾਲਕ=(Z3Model &&Other) = ਮਿਟਾਓ;

  ~Z3Model() {
    ਜੇ (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  ਖਾਲੀ print(raw_ostream &OS) ਅਚਲ {
    OS << Z3_model_ਮੰਜ਼ਿਲ_string(Context.Context, Model);
  }

  LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ dump() ਅਚਲ { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ਸਥਿਰ ਅਚਲ llvm::fltSemantics &getFloatSemantics(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
  ਚੋਣ (ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) {
  ਮੂਲ:
    llvm_unreachable("Unsupported floating-point semantics!");
    ਰੋਕੋ;
  ਹਾਲਤ 16:
    ਵਾਪਸ llvm::APFloat::IEEEhalf();
  ਹਾਲਤ 32:
    ਵਾਪਸ llvm::APFloat::IEEEsingle();
  ਹਾਲਤ 64:
    ਵਾਪਸ llvm::APFloat::IEEEdouble();
  ਹਾਲਤ 128:
    ਵਾਪਸ llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ਸਥਿਰ ਤਾਰਕਿਕ areEquivalent(ਅਚਲ llvm::fltSemantics &LHS,
                          ਅਚਲ llvm::fltSemantics &RHS) {
  ਵਾਪਸ (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(LHS) ==
          llvm::APFloat::semantics_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(RHS));
}

ਵਰਗ Z3Solver : ਜਨਤਕ SMTSolver {
  ਮਿੱਤਰ ਵਰਗ Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [ਇਹ] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    ਵਾਪਸ S;
  }();

  Z3_params Params = [ਇਹ] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    ਵਾਪਸ P;
  }();

  // Cache Sorts
  std::ਸੈੱਟ<Z3_ਛਾਂਟੋ> CachedSorts;

  // Cache Exprs
  std::ਸੈੱਟ<Z3_ਵਿਅੰਜਕ> CachedExprs;

ਜਨਤਕ:
  Z3Solver() = ਮੂਲ;
  Z3Solver(ਅਚਲ Z3Solver &Other) = ਮਿਟਾਓ;
  Z3Solver(Z3Solver &&Other) = ਮਿਟਾਓ;
  Z3Solver &ਸੰਚਾਲਕ=(Z3Solver &Other) = ਮਿਟਾਓ;
  Z3Solver &ਸੰਚਾਲਕ=(Z3Solver &&Other) = ਮਿਟਾਓ;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  ਖਾਲੀ ਜੋੜੋ_Constraint(ਅਚਲ SMTExprRef &Exp) ਅਚਲ override {
    Z3_solver_assert(Context.Context, Solver, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_ਛਾਂਟੋ_Ref(ਅਚਲ SMTSort &ਛਾਂਟੋ_2) {
    ਸਵੈਚਾਲਿਤ It = CachedSorts.ਪਾਓ(ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(ਛਾਂਟੋ_2));
    ਵਾਪਸ &(*It.ਪਹਿਲਾ_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_ਵਿਅੰਜਕ_Ref(ਅਚਲ SMTExpr &Exp) {
    ਸਵੈਚਾਲਿਤ It = CachedExprs.ਪਾਓ(ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(Exp));
    ਵਾਪਸ &(*It.ਪਹਿਲਾ_2);
  }

  SMTSortRef get_Bool_ਛਾਂਟੋ() override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(Z3_ਛਾਂਟੋ(Context, Z3_mk_bool_ਛਾਂਟੋ(Context.Context)));
  }

  SMTSortRef get_Bitvector_ਛਾਂਟੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(
        Z3_ਛਾਂਟੋ(Context, Z3_mk_bv_ਛਾਂਟੋ(Context.Context, ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)));
  }

  SMTSortRef get_ਛਾਂਟੋ(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(
        Z3_ਛਾਂਟੋ(Context, Z3_get_ਛਾਂਟੋ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTSortRef get_Float16_ਛਾਂਟੋ() override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(Z3_ਛਾਂਟੋ(Context, Z3_mk_fpa_ਛਾਂਟੋ_16(Context.Context)));
  }

  SMTSortRef get_Float32_ਛਾਂਟੋ() override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(Z3_ਛਾਂਟੋ(Context, Z3_mk_fpa_ਛਾਂਟੋ_32(Context.Context)));
  }

  SMTSortRef get_Float64_ਛਾਂਟੋ() override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(Z3_ਛਾਂਟੋ(Context, Z3_mk_fpa_ਛਾਂਟੋ_64(Context.Context)));
  }

  SMTSortRef get_Float128_ਛਾਂਟੋ() override {
    ਵਾਪਸ new_ਛਾਂਟੋ_Ref(Z3_ਛਾਂਟੋ(Context, Z3_mk_fpa_ਛਾਂਟੋ_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvneg(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvnot(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkNot(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_not(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvadd(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvsub(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvmul(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvsrem(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvurem(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvsdiv(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvudiv(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvshl(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvashr(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvlshr(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvxor(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvor(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                   ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvand(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvult(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvslt(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvugt(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvsgt(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvule(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvsle(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvuge(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_bvsge(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkAnd(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    Z3_ast ਦਲੀਲਾਂ[2] = {ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST};
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(Context, Z3_mk_and(Context.Context, 2, ਦਲੀਲਾਂ)));
  }

  SMTExprRef mkOr(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    Z3_ast ਦਲੀਲਾਂ[2] = {ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST};
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(Context, Z3_mk_or(Context.Context, 2, ਦਲੀਲਾਂ)));
  }

  SMTExprRef mkEqual(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_eq(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                 ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_neg(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_fpa_is_infinite(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_is_nan(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_ਸਾਧਾਰਨ(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_fpa_is_ਸਾਧਾਰਨ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_fpa_is_zero(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context,
               Z3_mk_fpa_mul(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                             ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context,
               Z3_mk_fpa_div(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                             ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_rem(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                      ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context,
               Z3_mk_fpa_ਜੋੜੋ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                             ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context,
               Z3_mk_fpa_sub(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                             ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_lt(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_gt(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_leq(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                      ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_geq(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                      ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_eq(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkIte(ਅਚਲ SMTExprRef &Cond, ਅਚਲ SMTExprRef &T,
                   ਅਚਲ SMTExprRef &F) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_ite(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Cond).AST,
                                  ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*T).AST, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*F).AST)));
  }

  SMTExprRef mkBVSignExt(ਬਿਨਾਨਿਸ਼ਾਨੀ i, ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_sign_ext(Context.Context, i, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(ਬਿਨਾਨਿਸ਼ਾਨੀ i, ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_zero_ext(Context.Context, i, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(ਬਿਨਾਨਿਸ਼ਾਨੀ ਉੱਚਾ_2, ਬਿਨਾਨਿਸ਼ਾਨੀ ਨੀਵਾਂ_2,
                         ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(Context, Z3_mk_ਕੱਢੋ(Context.Context, ਉੱਚਾ_2, ਨੀਵਾਂ_2,
                                                    ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_ਉਛਾਲ(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS,
                               ਤਾਰਕਿਕ isSigned) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvadd_no_ਉਛਾਲ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                         ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ਅਚਲ SMTExprRef &LHS,
                                ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                          ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_ਉਛਾਲ(ਅਚਲ SMTExprRef &LHS,
                               ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvsub_no_ਉਛਾਲ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                         ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS,
                                ਤਾਰਕਿਕ isSigned) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                          ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_ਉਛਾਲ(ਅਚਲ SMTExprRef &LHS,
                                ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvsdiv_no_ਉਛਾਲ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                          ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_ਉਛਾਲ(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvneg_no_ਉਛਾਲ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_ਉਛਾਲ(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS,
                               ਤਾਰਕਿਕ isSigned) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvmul_no_ਉਛਾਲ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                         ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ਅਚਲ SMTExprRef &LHS,
                                ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                          ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ਅਚਲ SMTExprRef &LHS, ਅਚਲ SMTExprRef &RHS) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_concat(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*LHS).AST,
                                     ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ਅਚਲ SMTExprRef &From, ਅਚਲ SMTSortRef &ਮੰਜ਼ਿਲ) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context,
        Z3_mk_fpa_ਮੰਜ਼ਿਲ_fp_float(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                              ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*From).AST, ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(*ਮੰਜ਼ਿਲ).ਛਾਂਟੋ_2)));
  }

  SMTExprRef mkSBVtoFP(ਅਚਲ SMTExprRef &From, ਅਚਲ SMTSortRef &ਮੰਜ਼ਿਲ) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context,
        Z3_mk_fpa_ਮੰਜ਼ਿਲ_fp_signed(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                               ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*From).AST, ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(*ਮੰਜ਼ਿਲ).ਛਾਂਟੋ_2)));
  }

  SMTExprRef mkUBVtoFP(ਅਚਲ SMTExprRef &From, ਅਚਲ SMTSortRef &ਮੰਜ਼ਿਲ) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context,
        Z3_mk_fpa_ਮੰਜ਼ਿਲ_fp_unsigned(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                                 ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*From).AST, ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(*ਮੰਜ਼ਿਲ).ਛਾਂਟੋ_2)));
  }

  SMTExprRef mkFPtoSBV(ਅਚਲ SMTExprRef &From, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੰਜ਼ਿਲ_ਚੌੜਾਈ) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_fpa_ਮੰਜ਼ਿਲ_sbv(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                                  ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*From).AST, ਮੰਜ਼ਿਲ_ਚੌੜਾਈ)));
  }

  SMTExprRef mkFPtoUBV(ਅਚਲ SMTExprRef &From, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੰਜ਼ਿਲ_ਚੌੜਾਈ) override {
    SMTExprRef Rounding_ਬਹੁਲਕ = get_Float_Rounding_ਬਹੁਲਕ();
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_fpa_ਮੰਜ਼ਿਲ_ubv(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Rounding_ਬਹੁਲਕ).AST,
                                  ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*From).AST, ਮੰਜ਼ਿਲ_ਚੌੜਾਈ)));
  }

  SMTExprRef mkBoolean(ਅਚਲ ਤਾਰਕਿਕ b) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ਅਚਲ llvm::APSInt Int, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2) override {
    ਅਚਲ Z3_ਛਾਂਟੋ_2 Z3_ਛਾਂਟੋ = ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(*get_Bitvector_ਛਾਂਟੋ(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2)).ਛਾਂਟੋ_2;

    // Slow path, when 64 bits are not enough.
    ਜੇ (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> ਬਫਰ;
      Int.ਮੰਜ਼ਿਲ_String(ਬਫਰ, 10);
      ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
          Context, Z3_mk_numeral(Context.Context, ਬਫਰ.c_str(), Z3_ਛਾਂਟੋ)));
    }

    ਅਚਲ int64_t ਦੋਆਧਾਰੀ_ਅੰਕ_Repr_As_Signed = Int.get_Ext_ਮੁੱਲ();
    ਅਚਲ uint64_t ਦੋਆਧਾਰੀ_ਅੰਕ_Repr_As_Unsigned =
        reinterpret_cast<ਅਚਲ uint64_t &>(ਦੋਆਧਾਰੀ_ਅੰਕ_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, ਦੋਆਧਾਰੀ_ਅੰਕ_Repr_As_Signed, Z3_ਛਾਂਟੋ)
            : Z3_mk_unsigned_int64(Context.Context, ਦੋਆਧਾਰੀ_ਅੰਕ_Repr_As_Unsigned, Z3_ਛਾਂਟੋ);
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(Context, Literal));
  }

  SMTExprRef mkFloat(ਅਚਲ llvm::APFloat Float) override {
    SMTSortRef ਛਾਂਟੋ_2 =
        get_Float_ਛਾਂਟੋ(llvm::APFloat::semantics_ਮਾਪ_ਵਿੱਚ_ਬਿੱਟ(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_ਮੰਜ਼ਿਲ_APInt(), ਝੂਠ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(
        Context, Z3_mk_fpa_ਮੰਜ਼ਿਲ_fp_bv(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Z3Int).AST,
                                    ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(*ਛਾਂਟੋ_2).ਛਾਂਟੋ_2)));
  }

  SMTExprRef mk_ਚਿੰਨ੍ਹ(ਅਚਲ ਅੱਖਰ *Name, SMTSortRef ਛਾਂਟੋ_2) override {
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_ਚਿੰਨ੍ਹ(Context.Context, Name),
                                    ਮੰਜ਼ਿਲ_Z3_ਛਾਂਟੋ(*ਛਾਂਟੋ_2).ਛਾਂਟੋ_2)));
  }

  llvm::APSInt getBitvector(ਅਚਲ SMTExprRef &Exp, ਬਿਨਾਨਿਸ਼ਾਨੀ ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
                            ਤਾਰਕਿਕ isUnsigned) override {
    ਵਾਪਸ llvm::APSInt(
        llvm::APInt(ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ_2,
                    Z3_get_numeral_string(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST),
                    10),
        isUnsigned);
  }

  ਤਾਰਕਿਕ getBoolean(ਅਚਲ SMTExprRef &Exp) override {
    ਵਾਪਸ Z3_get_bool_ਮੁੱਲ(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_ਬਹੁਲਕ() override {
    // TODO: Don't assume nearest ties to even rounding mode
    ਵਾਪਸ new_ਵਿਅੰਜਕ_Ref(Z3_ਵਿਅੰਜਕ(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  ਤਾਰਕਿਕ ਮੰਜ਼ਿਲ_APFloat(ਅਚਲ SMTSortRef &ਛਾਂਟੋ_2, ਅਚਲ SMTExprRef &AST,
                 llvm::APFloat &Float, ਤਾਰਕਿਕ useSemantics) {
    ਪ੍ਰਤਿਜ੍ਞਾ(ਛਾਂਟੋ_2->is_Float_ਛਾਂਟੋ() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(ਛਾਂਟੋ_2->get_Float_ਛਾਂਟੋ_ਮਾਪ(), ਸੱਚ);
    ਅਚਲ llvm::fltSemantics &Semantics =
        getFloatSemantics(ਛਾਂਟੋ_2->get_Float_ਛਾਂਟੋ_ਮਾਪ());
    SMTSortRef BVSort = get_Bitvector_ਛਾਂਟੋ(ਛਾਂਟੋ_2->get_Float_ਛਾਂਟੋ_ਮਾਪ());
    ਜੇ (!ਮੰਜ਼ਿਲ_APSInt(BVSort, AST, Int, ਸੱਚ)) {
      ਵਾਪਸ ਝੂਠ;
    }

    ਜੇ (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      ਪ੍ਰਤਿਜ੍ਞਾ(ਝੂਠ && "Floating-point types don't match!");
      ਵਾਪਸ ਝੂਠ;
    }

    Float = llvm::APFloat(Semantics, Int);
    ਵਾਪਸ ਸੱਚ;
  }

  ਤਾਰਕਿਕ ਮੰਜ਼ਿਲ_APSInt(ਅਚਲ SMTSortRef &ਛਾਂਟੋ_2, ਅਚਲ SMTExprRef &AST,
                llvm::APSInt &Int, ਤਾਰਕਿਕ useSemantics) {
    ਜੇ (ਛਾਂਟੋ_2->is_Bitvector_ਛਾਂਟੋ()) {
      ਜੇ (useSemantics && Int.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() != ਛਾਂਟੋ_2->get_Bitvector_ਛਾਂਟੋ_ਮਾਪ()) {
        ਪ੍ਰਤਿਜ੍ਞਾ(ਝੂਠ && "Bitvector types don't match!");
        ਵਾਪਸ ਝੂਠ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), Int.isUnsigned());
      ਵਾਪਸ ਸੱਚ;
    }

    ਜੇ (ਛਾਂਟੋ_2->is_Boolean_ਛਾਂਟੋ()) {
      ਜੇ (useSemantics && Int.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ() < 1) {
        ਪ੍ਰਤਿਜ੍ਞਾ(ਝੂਠ && "Boolean type doesn't match!");
        ਵਾਪਸ ਝੂਠ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), getBoolean(AST)),
                         Int.isUnsigned());
      ਵਾਪਸ ਸੱਚ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  ਤਾਰਕਿਕ getInterpretation(ਅਚਲ SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_ਮੰਜ਼ਿਲ_app(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST));
    ਜੇ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ਵਾਪਸ ਝੂਠ;

    SMTExprRef ਸੌਂਪੋ_2 = new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ਛਾਂਟੋ_2 = get_ਛਾਂਟੋ(ਸੌਂਪੋ_2);
    ਵਾਪਸ ਮੰਜ਼ਿਲ_APSInt(ਛਾਂਟੋ_2, ਸੌਂਪੋ_2, Int, ਸੱਚ);
  }

  ਤਾਰਕਿਕ getInterpretation(ਅਚਲ SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_ਮੰਜ਼ਿਲ_app(Context.Context, ਮੰਜ਼ਿਲ_Z3_ਵਿਅੰਜਕ(*Exp).AST));
    ਜੇ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      ਵਾਪਸ ਝੂਠ;

    SMTExprRef ਸੌਂਪੋ_2 = new_ਵਿਅੰਜਕ_Ref(
        Z3_ਵਿਅੰਜਕ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ਛਾਂਟੋ_2 = get_ਛਾਂਟੋ(ਸੌਂਪੋ_2);
    ਵਾਪਸ ਮੰਜ਼ਿਲ_APFloat(ਛਾਂਟੋ_2, ਸੌਂਪੋ_2, Float, ਸੱਚ);
  }

  std::optional<ਤਾਰਕਿਕ> check() ਅਚਲ override {
    Z3_solver_ਸੈੱਟ_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    ਜੇ (res == Z3_L_TRUE)
      ਵਾਪਸ ਸੱਚ;

    ਜੇ (res == Z3_L_FALSE)
      ਵਾਪਸ ਝੂਠ;

    ਵਾਪਸ std::nullopt;
  }

  ਖਾਲੀ ਧੱਕੋ() override { ਵਾਪਸ Z3_solver_ਧੱਕੋ(Context.Context, Solver); }

  ਖਾਲੀ ਕੱਢੋ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਹਾਲਤਾਂ = 1) override {
    ਪ੍ਰਤਿਜ੍ਞਾ(Z3_solver_get_ਅੰਕ_scopes(Context.Context, Solver) >= ਅੰਕ_ਹਾਲਤਾਂ);
    ਵਾਪਸ Z3_solver_ਕੱਢੋ(Context.Context, Solver, ਅੰਕ_ਹਾਲਤਾਂ);
  }

  ਤਾਰਕਿਕ isFPSupported() override { ਵਾਪਸ ਸੱਚ; }

  /// Reset the solver and remove all constraints.
  ਖਾਲੀ reset() override { Z3_solver_reset(Context.Context, Solver); }

  ਖਾਲੀ print(raw_ostream &OS) ਅਚਲ override {
    OS << Z3_solver_ਮੰਜ਼ਿਲ_string(Context.Context, Solver);
  }

  ਖਾਲੀ ਸੈੱਟ_Unsigned_Param(StringRef ਕੁੰਜੀ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਮੁੱਲ) override {
    Z3_ਚਿੰਨ੍ਹ Sym = Z3_mk_string_ਚਿੰਨ੍ਹ(Context.Context, ਕੁੰਜੀ.str().c_str());
    Z3_params_ਸੈੱਟ_uint(Context.Context, Params, Sym, ਮੁੱਲ);
  }

  ਖਾਲੀ ਸੈੱਟ_Bool_Param(StringRef ਕੁੰਜੀ, ਤਾਰਕਿਕ ਮੁੱਲ) override {
    Z3_ਚਿੰਨ੍ਹ Sym = Z3_mk_string_ਚਿੰਨ੍ਹ(Context.Context, ਕੁੰਜੀ.str().c_str());
    Z3_params_ਸੈੱਟ_bool(Context.Context, Params, Sym, ਮੁੱਲ);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ਅਚਲ override;
}; // end class Z3Solver

ਵਰਗ Z3Statistics final : ਜਨਤਕ SMTSolverStatistics {
ਜਨਤਕ:
  ਦੁੱਗਣਾ getDouble(StringRef ਕੁੰਜੀ) ਅਚਲ override {
    ਸਵੈਚਾਲਿਤ It = Double_ਮੁੱਲਾਂ.ਲੱਭੋ(ਕੁੰਜੀ.str());
    ਪ੍ਰਤਿਜ੍ਞਾ(It != Double_ਮੁੱਲਾਂ.ਸਿਰਾ());
    ਵਾਪਸ It->ਦੂਜਾ_2;
  };
  ਬਿਨਾਨਿਸ਼ਾਨੀ getUnsigned(StringRef ਕੁੰਜੀ) ਅਚਲ override {
    ਸਵੈਚਾਲਿਤ It = Unsigned_ਮੁੱਲਾਂ.ਲੱਭੋ(ਕੁੰਜੀ.str());
    ਜੇ (It == Unsigned_ਮੁੱਲਾਂ.ਸਿਰਾ())
      ਵਾਪਸ 0;
    ਵਾਪਸ It->ਦੂਜਾ_2;
  };

  ਖਾਲੀ print(raw_ostream &OS) ਅਚਲ override {
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &E : Unsigned_ਮੁੱਲਾਂ)
      OS << E.ਪਹਿਲਾ_2() << ": " << E.ਦੂਜਾ_2 << '\n';
    ਲਈ (ਅਚਲ ਸਵੈਚਾਲਿਤ &E : Double_ਮੁੱਲਾਂ) {
      write_double(OS << E.ਪਹਿਲਾ_2() << ": ", E.ਦੂਜਾ_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

ਨਿਜੀ:
  ਮਿੱਤਰ ਵਰਗ Z3Solver;
  StringMap<ਬਿਨਾਨਿਸ਼ਾਨੀ> Unsigned_ਮੁੱਲਾਂ;
  StringMap<ਦੁੱਗਣਾ> Double_ਮੁੱਲਾਂ;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ਅਚਲ {
  ਸਵੈਚਾਲਿਤ ਅਚਲ &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics ਨਤੀਜਾ;

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੰਕ_ਕੁੰਜੀਆਂ = Z3_stats_ਮਾਪ(C, S);
  ਲਈ (ਬਿਨਾਨਿਸ਼ਾਨੀ Idx = 0; Idx < ਅੰਕ_ਕੁੰਜੀਆਂ; ++Idx) {
    ਅਚਲ ਅੱਖਰ *ਕੁੰਜੀ = Z3_stats_get_ਕੁੰਜੀ(C, S, Idx);
    ਜੇ (Z3_stats_is_uint(C, S, Idx)) {
      ਸਵੈਚਾਲਿਤ ਮੁੱਲ = Z3_stats_get_uint_ਮੁੱਲ(C, S, Idx);
      ਨਤੀਜਾ.Unsigned_ਮੁੱਲਾਂ.try_emplace(ਕੁੰਜੀ, ਮੁੱਲ);
    } ਨਹੀਂਤਾਂ {
      ਪ੍ਰਤਿਜ੍ਞਾ(Z3_stats_is_double(C, S, Idx));
      ਸਵੈਚਾਲਿਤ ਮੁੱਲ = Z3_stats_get_double_ਮੁੱਲ(C, S, Idx);
      ਨਤੀਜਾ.Double_ਮੁੱਲਾਂ.try_emplace(ਕੁੰਜੀ, ਮੁੱਲ);
    }
  }
  ਵਾਪਸ std::make_unique<Z3Statistics>(std::move(ਨਤੀਜਾ));
}

} // end anonymous namespace

#ਜੇ_ਅੰਤ

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#ਜੇ LLVM_WITH_Z3
  ਵਾਪਸ std::make_unique<Z3Solver>();
#ਨਹੀਂਤਾਂ
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           ਝੂਠ);
  ਵਾਪਸ ਖਾਲੀ_ਸੂਚਕ;
#ਜੇ_ਅੰਤ
}

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ SMTSort::dump() ਅਚਲ { print(llvm::errs()); }
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ SMTExpr::dump() ਅਚਲ { print(llvm::errs()); }
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ SMTSolver::dump() ਅਚਲ { print(llvm::errs()); }
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ SMTSolverStatistics::dump() ਅਚਲ { print(llvm::errs()); }
#ਜੇ_ਅੰਤ
