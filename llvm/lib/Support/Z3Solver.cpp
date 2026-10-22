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

استعمال نامخلا llvm;

#اگر LLVM_WITH_Z3

#شامل "llvm/ADT/SmallString.h"
#شامل "llvm/ADT/StringMap.h"
#شامل "llvm/ADT/Twine.h"

#شامل <set>

#شامل <z3.h>

نامخلا {

/// Configuration class for Z3
جماعت Z3Config {
  دوست جماعت Z3Context;

  Z3_config Config = Z3_mk_config();

عوامی:
  Z3Config() = طےشدہ;
  Z3Config(ثابت Z3Config &) = حذف;
  Z3Config(Z3Config &&) = طےشدہ;
  Z3Config &عامل=(Z3Config &) = حذف;
  Z3Config &عامل=(Z3Config &&) = طےشدہ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
خالی Z3ErrorHandler(Z3_context Context, Z3_error_رمز Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_پیغام(Context, Error)));
}

/// Wrapper for Z3 context
جماعت Z3Context {
عوامی:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_سیٹ_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = حذف;
  Z3Context(Z3Context &&) = طےشدہ;
  Z3Context &عامل=(Z3Context &) = حذف;
  Z3Context &عامل=(Z3Context &&) = طےشدہ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = خالی_اشاریہ;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
جماعت Z3_ترتیب_دو : عوامی SMTSort {
  دوست جماعت Z3Solver;

  Z3Context &Context;

  Z3_ترتیب_دو_2 ترتیب_دو_2;

عوامی:
  /// Default constructor, mainly used by make_shared
  Z3_ترتیب_دو(Z3Context &C, Z3_ترتیب_دو_2 ZS) : Context(C), ترتیب_دو_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دو_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ترتیب_دو(ثابت Z3_ترتیب_دو &Other) : Context(Other.Context), ترتیب_دو_2(Other.ترتیب_دو_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دو_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ترتیب_دو &عامل=(ثابت Z3_ترتیب_دو &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.ترتیب_دو_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دو_2));
    ترتیب_دو_2 = Other.ترتیب_دو_2;
    واپس *یہ;
  }

  Z3_ترتیب_دو(Z3_ترتیب_دو &&Other) = حذف;
  Z3_ترتیب_دو &عامل=(Z3_ترتیب_دو &&Other) = حذف;

  ~Z3_ترتیب_دو() {
    اگر (ترتیب_دو_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دو_2));
  }

  خالی Profile(llvm::Folding_سیٹ_گرہ_ID &ID) ثابت override {
    ID.جوڑو_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دو_2)));
  }

  منطقی is_Bitvector_ترتیب_دو_Impl() ثابت override {
    واپس (Z3_get_ترتیب_دو_قسم(Context.Context, ترتیب_دو_2) == Z3_BV_ترتیب_دو);
  }

  منطقی is_Float_ترتیب_دو_Impl() ثابت override {
    واپس (Z3_get_ترتیب_دو_قسم(Context.Context, ترتیب_دو_2) == Z3_FLOATING_نقطہ_ترتیب_دو);
  }

  منطقی is_Boolean_ترتیب_دو_Impl() ثابت override {
    واپس (Z3_get_ترتیب_دو_قسم(Context.Context, ترتیب_دو_2) == Z3_BOOL_ترتیب_دو);
  }

  غیرعلامتی get_Bitvector_ترتیب_دو_سائز_Impl() ثابت override {
    واپس Z3_get_bv_ترتیب_دو_سائز(Context.Context, ترتیب_دو_2);
  }

  غیرعلامتی get_Float_ترتیب_دو_سائز_Impl() ثابت override {
    واپس Z3_fpa_get_ebits(Context.Context, ترتیب_دو_2) +
           Z3_fpa_get_sbits(Context.Context, ترتیب_دو_2);
  }

  منطقی equal_مقصد(SMTSort ثابت &Other) ثابت override {
    واپس Z3_is_eq_ترتیب_دو(Context.Context, ترتیب_دو_2,
                         static_cast<ثابت Z3_ترتیب_دو &>(Other).ترتیب_دو_2);
  }

  خالی print(raw_ostream &OS) ثابت override {
    OS << Z3_ترتیب_دو_مقصد_string(Context.Context, ترتیب_دو_2);
  }
}; // end class Z3Sort

ساکن ثابت Z3_ترتیب_دو &مقصد_Z3_ترتیب_دو(ثابت SMTSort &S) {
  واپس static_cast<ثابت Z3_ترتیب_دو &>(S);
}

جماعت Z3_اظہاریہ : عوامی SMTExpr {
  دوست جماعت Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عوامی:
  Z3_اظہاریہ(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_اظہاریہ(ثابت Z3_اظہاریہ &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_اظہاریہ &عامل=(ثابت Z3_اظہاریہ &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    واپس *یہ;
  }

  Z3_اظہاریہ(Z3_اظہاریہ &&Other) = حذف;
  Z3_اظہاریہ &عامل=(Z3_اظہاریہ &&Other) = حذف;

  ~Z3_اظہاریہ() {
    اگر (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  خالی Profile(llvm::Folding_سیٹ_گرہ_ID &ID) ثابت override {
    ID.جوڑو_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقی equal_مقصد(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_ترتیب_دو(Context.Context, Z3_get_ترتیب_دو(Context.Context, AST),
                         Z3_get_ترتیب_دو(Context.Context,
                                     static_cast<ثابت Z3_اظہاریہ &>(Other).AST)) &&
           "AST's must have the same sort");
    واپس Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_اظہاریہ &>(Other).AST);
  }

  خالی print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_مقصد_string(Context.Context, AST);
  }
}; // end class Z3Expr

ساکن ثابت Z3_اظہاریہ &مقصد_Z3_اظہاریہ(ثابت SMTExpr &E) {
  واپس static_cast<ثابت Z3_اظہاریہ &>(E);
}

جماعت Z3Model {
  دوست جماعت Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عوامی:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = حذف;
  Z3Model(Z3Model &&Other) = حذف;
  Z3Model &عامل=(Z3Model &Other) = حذف;
  Z3Model &عامل=(Z3Model &&Other) = حذف;

  ~Z3Model() {
    اگر (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  خالی print(raw_ostream &OS) ثابت {
    OS << Z3_model_مقصد_string(Context.Context, Model);
  }

  LLVM_DUMP_طریقہ خالی dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ساکن ثابت llvm::fltSemantics &getFloatSemantics(غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) {
  انتخاب (ثنائی_ہندسہ_چوڑائی_2) {
  طےشدہ:
    llvm_unreachable("Unsupported floating-point semantics!");
    توڑو;
  حالت 16:
    واپس llvm::APFloat::IEEEhalf();
  حالت 32:
    واپس llvm::APFloat::IEEEsingle();
  حالت 64:
    واپس llvm::APFloat::IEEEdouble();
  حالت 128:
    واپس llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ساکن منطقی areEquivalent(ثابت llvm::fltSemantics &LHS,
                          ثابت llvm::fltSemantics &RHS) {
  واپس (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_سائز_اندر_بٹ(LHS) ==
          llvm::APFloat::semantics_سائز_اندر_بٹ(RHS));
}

جماعت Z3Solver : عوامی SMTSolver {
  دوست جماعت Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [یہ] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    واپس S;
  }();

  Z3_params Params = [یہ] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    واپس P;
  }();

  // Cache Sorts
  std::سیٹ<Z3_ترتیب_دو> CachedSorts;

  // Cache Exprs
  std::سیٹ<Z3_اظہاریہ> CachedExprs;

عوامی:
  Z3Solver() = طےشدہ;
  Z3Solver(ثابت Z3Solver &Other) = حذف;
  Z3Solver(Z3Solver &&Other) = حذف;
  Z3Solver &عامل=(Z3Solver &Other) = حذف;
  Z3Solver &عامل=(Z3Solver &&Other) = حذف;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  خالی جوڑو_Constraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, مقصد_Z3_اظہاریہ(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_ترتیب_دو_Ref(ثابت SMTSort &ترتیب_دو_2) {
    خودکار It = CachedSorts.داخل_کرو(مقصد_Z3_ترتیب_دو(ترتیب_دو_2));
    واپس &(*It.پہلا_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_اظہاریہ_Ref(ثابت SMTExpr &Exp) {
    خودکار It = CachedExprs.داخل_کرو(مقصد_Z3_اظہاریہ(Exp));
    واپس &(*It.پہلا_2);
  }

  SMTSortRef get_Bool_ترتیب_دو() override {
    واپس new_ترتیب_دو_Ref(Z3_ترتیب_دو(Context, Z3_mk_bool_ترتیب_دو(Context.Context)));
  }

  SMTSortRef get_Bitvector_ترتیب_دو(غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) override {
    واپس new_ترتیب_دو_Ref(
        Z3_ترتیب_دو(Context, Z3_mk_bv_ترتیب_دو(Context.Context, ثنائی_ہندسہ_چوڑائی_2)));
  }

  SMTSortRef get_ترتیب_دو(ثابت SMTExprRef &Exp) override {
    واپس new_ترتیب_دو_Ref(
        Z3_ترتیب_دو(Context, Z3_get_ترتیب_دو(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTSortRef get_Float16_ترتیب_دو() override {
    واپس new_ترتیب_دو_Ref(Z3_ترتیب_دو(Context, Z3_mk_fpa_ترتیب_دو_16(Context.Context)));
  }

  SMTSortRef get_Float32_ترتیب_دو() override {
    واپس new_ترتیب_دو_Ref(Z3_ترتیب_دو(Context, Z3_mk_fpa_ترتیب_دو_32(Context.Context)));
  }

  SMTSortRef get_Float64_ترتیب_دو() override {
    واپس new_ترتیب_دو_Ref(Z3_ترتیب_دو(Context, Z3_mk_fpa_ترتیب_دو_64(Context.Context)));
  }

  SMTSortRef get_Float128_ترتیب_دو() override {
    واپس new_ترتیب_دو_Ref(Z3_ترتیب_دو(Context, Z3_mk_fpa_ترتیب_دو_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvneg(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvnot(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_not(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvadd(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsub(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvmul(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsrem(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvurem(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsdiv(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvudiv(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvshl(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvashr(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvlshr(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvxor(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvor(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                   مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvand(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvult(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvslt(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvugt(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsgt(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvule(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsle(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvuge(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsge(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                    مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دلائل[2] = {مقصد_Z3_اظہاریہ(*LHS).AST, مقصد_Z3_اظہاریہ(*RHS).AST};
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_and(Context.Context, 2, دلائل)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دلائل[2] = {مقصد_Z3_اظہاریہ(*LHS).AST, مقصد_Z3_اظہاریہ(*RHS).AST};
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_or(Context.Context, 2, دلائل)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_eq(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                 مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_neg(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_is_infinite(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_is_nan(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_معمول(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_is_معمول(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_is_zero(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_mul(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                             مقصد_Z3_اظہاریہ(*LHS).AST, مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_div(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                             مقصد_Z3_اظہاریہ(*LHS).AST, مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_rem(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                      مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_جوڑو(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                             مقصد_Z3_اظہاریہ(*LHS).AST, مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_sub(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                             مقصد_Z3_اظہاریہ(*LHS).AST, مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_lt(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_gt(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_leq(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                      مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_geq(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                      مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_eq(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_ite(Context.Context, مقصد_Z3_اظہاریہ(*Cond).AST,
                                  مقصد_Z3_اظہاریہ(*T).AST, مقصد_Z3_اظہاریہ(*F).AST)));
  }

  SMTExprRef mkBVSignExt(غیرعلامتی i, ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_sign_ext(Context.Context, i, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(غیرعلامتی i, ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_zero_ext(Context.Context, i, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(غیرعلامتی بلند_2, غیرعلامتی کم_2,
                         ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_نکالو(Context.Context, بلند_2, کم_2,
                                                    مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_حد_سے_تجاوز(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقی isSigned) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvadd_no_حد_سے_تجاوز(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                         مقصد_Z3_اظہاریہ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                          مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_حد_سے_تجاوز(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvsub_no_حد_سے_تجاوز(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                         مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقی isSigned) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                          مقصد_Z3_اظہاریہ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_حد_سے_تجاوز(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvsdiv_no_حد_سے_تجاوز(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                          مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_حد_سے_تجاوز(ثابت SMTExprRef &Exp) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvneg_no_حد_سے_تجاوز(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_حد_سے_تجاوز(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقی isSigned) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvmul_no_حد_سے_تجاوز(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                         مقصد_Z3_اظہاریہ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                          مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_concat(Context.Context, مقصد_Z3_اظہاریہ(*LHS).AST,
                                     مقصد_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &مقصد) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context,
        Z3_mk_fpa_مقصد_fp_float(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                              مقصد_Z3_اظہاریہ(*From).AST, مقصد_Z3_ترتیب_دو(*مقصد).ترتیب_دو_2)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &مقصد) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context,
        Z3_mk_fpa_مقصد_fp_signed(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                               مقصد_Z3_اظہاریہ(*From).AST, مقصد_Z3_ترتیب_دو(*مقصد).ترتیب_دو_2)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &مقصد) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context,
        Z3_mk_fpa_مقصد_fp_unsigned(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                                 مقصد_Z3_اظہاریہ(*From).AST, مقصد_Z3_ترتیب_دو(*مقصد).ترتیب_دو_2)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, غیرعلامتی مقصد_چوڑائی) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_مقصد_sbv(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                                  مقصد_Z3_اظہاریہ(*From).AST, مقصد_چوڑائی)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, غیرعلامتی مقصد_چوڑائی) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_مقصد_ubv(Context.Context, مقصد_Z3_اظہاریہ(*Rounding_منوال).AST,
                                  مقصد_Z3_اظہاریہ(*From).AST, مقصد_چوڑائی)));
  }

  SMTExprRef mkBoolean(ثابت منطقی b) override {
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, غیرعلامتی ثنائی_ہندسہ_چوڑائی_2) override {
    ثابت Z3_ترتیب_دو_2 Z3_ترتیب_دو = مقصد_Z3_ترتیب_دو(*get_Bitvector_ترتیب_دو(ثنائی_ہندسہ_چوڑائی_2)).ترتیب_دو_2;

    // Slow path, when 64 bits are not enough.
    اگر (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> بفر;
      Int.مقصد_String(بفر, 10);
      واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
          Context, Z3_mk_numeral(Context.Context, بفر.c_str(), Z3_ترتیب_دو)));
    }

    ثابت int64_t ثنائی_ہندسہ_Repr_As_Signed = Int.get_Ext_قدر();
    ثابت uint64_t ثنائی_ہندسہ_Repr_As_Unsigned =
        reinterpret_cast<ثابت uint64_t &>(ثنائی_ہندسہ_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, ثنائی_ہندسہ_Repr_As_Signed, Z3_ترتیب_دو)
            : Z3_mk_unsigned_int64(Context.Context, ثنائی_ہندسہ_Repr_As_Unsigned, Z3_ترتیب_دو);
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef ترتیب_دو_2 =
        get_Float_ترتیب_دو(llvm::APFloat::semantics_سائز_اندر_بٹ(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_مقصد_APInt(), جھوٹ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_ثنائی_ہندسہ_چوڑائی());
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_مقصد_fp_bv(Context.Context, مقصد_Z3_اظہاریہ(*Z3Int).AST,
                                    مقصد_Z3_ترتیب_دو(*ترتیب_دو_2).ترتیب_دو_2)));
  }

  SMTExprRef mk_علامت(ثابت حرف *Name, SMTSortRef ترتیب_دو_2) override {
    واپس new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_علامت(Context.Context, Name),
                                    مقصد_Z3_ترتیب_دو(*ترتیب_دو_2).ترتیب_دو_2)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, غیرعلامتی ثنائی_ہندسہ_چوڑائی_2,
                            منطقی isUnsigned) override {
    واپس llvm::APSInt(
        llvm::APInt(ثنائی_ہندسہ_چوڑائی_2,
                    Z3_get_numeral_string(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقی getBoolean(ثابت SMTExprRef &Exp) override {
    واپس Z3_get_bool_قدر(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_منوال() override {
    // TODO: Don't assume nearest ties to even rounding mode
    واپس new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقی مقصد_APFloat(ثابت SMTSortRef &ترتیب_دو_2, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقی useSemantics) {
    أكد(ترتیب_دو_2->is_Float_ترتیب_دو() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(ترتیب_دو_2->get_Float_ترتیب_دو_سائز(), سچ);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(ترتیب_دو_2->get_Float_ترتیب_دو_سائز());
    SMTSortRef BVSort = get_Bitvector_ترتیب_دو(ترتیب_دو_2->get_Float_ترتیب_دو_سائز());
    اگر (!مقصد_APSInt(BVSort, AST, Int, سچ)) {
      واپس جھوٹ;
    }

    اگر (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      أكد(جھوٹ && "Floating-point types don't match!");
      واپس جھوٹ;
    }

    Float = llvm::APFloat(Semantics, Int);
    واپس سچ;
  }

  منطقی مقصد_APSInt(ثابت SMTSortRef &ترتیب_دو_2, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقی useSemantics) {
    اگر (ترتیب_دو_2->is_Bitvector_ترتیب_دو()) {
      اگر (useSemantics && Int.get_ثنائی_ہندسہ_چوڑائی() != ترتیب_دو_2->get_Bitvector_ترتیب_دو_سائز()) {
        أكد(جھوٹ && "Bitvector types don't match!");
        واپس جھوٹ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_ثنائی_ہندسہ_چوڑائی(), Int.isUnsigned());
      واپس سچ;
    }

    اگر (ترتیب_دو_2->is_Boolean_ترتیب_دو()) {
      اگر (useSemantics && Int.get_ثنائی_ہندسہ_چوڑائی() < 1) {
        أكد(جھوٹ && "Boolean type doesn't match!");
        واپس جھوٹ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_ثنائی_ہندسہ_چوڑائی(), getBoolean(AST)),
                         Int.isUnsigned());
      واپس سچ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقی getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_مقصد_app(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST));
    اگر (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      واپس جھوٹ;

    SMTExprRef تفویض_2 = new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتیب_دو_2 = get_ترتیب_دو(تفویض_2);
    واپس مقصد_APSInt(ترتیب_دو_2, تفویض_2, Int, سچ);
  }

  منطقی getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_مقصد_app(Context.Context, مقصد_Z3_اظہاریہ(*Exp).AST));
    اگر (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      واپس جھوٹ;

    SMTExprRef تفویض_2 = new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتیب_دو_2 = get_ترتیب_دو(تفویض_2);
    واپس مقصد_APFloat(ترتیب_دو_2, تفویض_2, Float, سچ);
  }

  std::optional<منطقی> check() ثابت override {
    Z3_solver_سیٹ_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    اگر (res == Z3_L_TRUE)
      واپس سچ;

    اگر (res == Z3_L_FALSE)
      واپس جھوٹ;

    واپس std::nullopt;
  }

  خالی ڈالو() override { واپس Z3_solver_ڈالو(Context.Context, Solver); }

  خالی نکالو(غیرعلامتی عدد_حالتیں = 1) override {
    أكد(Z3_solver_get_عدد_scopes(Context.Context, Solver) >= عدد_حالتیں);
    واپس Z3_solver_نکالو(Context.Context, Solver, عدد_حالتیں);
  }

  منطقی isFPSupported() override { واپس سچ; }

  /// Reset the solver and remove all constraints.
  خالی reset() override { Z3_solver_reset(Context.Context, Solver); }

  خالی print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_مقصد_string(Context.Context, Solver);
  }

  خالی سیٹ_Unsigned_Param(StringRef کلید, غیرعلامتی قدر) override {
    Z3_علامت Sym = Z3_mk_string_علامت(Context.Context, کلید.str().c_str());
    Z3_params_سیٹ_uint(Context.Context, Params, Sym, قدر);
  }

  خالی سیٹ_Bool_Param(StringRef کلید, منطقی قدر) override {
    Z3_علامت Sym = Z3_mk_string_علامت(Context.Context, کلید.str().c_str());
    Z3_params_سیٹ_bool(Context.Context, Params, Sym, قدر);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

جماعت Z3Statistics final : عوامی SMTSolverStatistics {
عوامی:
  دوہرا getDouble(StringRef کلید) ثابت override {
    خودکار It = Double_اقدار.ڈھونڈو(کلید.str());
    أكد(It != Double_اقدار.انتہا());
    واپس It->دوسرا_2;
  };
  غیرعلامتی getUnsigned(StringRef کلید) ثابت override {
    خودکار It = Unsigned_اقدار.ڈھونڈو(کلید.str());
    اگر (It == Unsigned_اقدار.انتہا())
      واپس 0;
    واپس It->دوسرا_2;
  };

  خالی print(raw_ostream &OS) ثابت override {
    برائے (ثابت خودکار &E : Unsigned_اقدار)
      OS << E.پہلا_2() << ": " << E.دوسرا_2 << '\n';
    برائے (ثابت خودکار &E : Double_اقدار) {
      write_double(OS << E.پہلا_2() << ": ", E.دوسرا_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

نجی:
  دوست جماعت Z3Solver;
  StringMap<غیرعلامتی> Unsigned_اقدار;
  StringMap<دوہرا> Double_اقدار;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  خودکار ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics نتیجہ;

  غیرعلامتی عدد_کلیدیں = Z3_stats_سائز(C, S);
  برائے (غیرعلامتی Idx = 0; Idx < عدد_کلیدیں; ++Idx) {
    ثابت حرف *کلید = Z3_stats_get_کلید(C, S, Idx);
    اگر (Z3_stats_is_uint(C, S, Idx)) {
      خودکار قدر = Z3_stats_get_uint_قدر(C, S, Idx);
      نتیجہ.Unsigned_اقدار.try_emplace(کلید, قدر);
    } ورنہ {
      أكد(Z3_stats_is_double(C, S, Idx));
      خودکار قدر = Z3_stats_get_double_قدر(C, S, Idx);
      نتیجہ.Double_اقدار.try_emplace(کلید, قدر);
    }
  }
  واپس std::make_unique<Z3Statistics>(std::move(نتیجہ));
}

} // end anonymous namespace

#اگر_ختم

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#اگر LLVM_WITH_Z3
  واپس std::make_unique<Z3Solver>();
#ورنہ
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           جھوٹ);
  واپس خالی_اشاریہ;
#اگر_ختم
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طریقہ خالی SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طریقہ خالی SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طریقہ خالی SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#اگر_ختم
