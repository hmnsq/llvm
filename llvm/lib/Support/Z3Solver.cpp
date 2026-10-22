//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/ScopeExit.h"
#ضمن "llvm/Config/config.h"
#ضمن "llvm/Support/NativeFormatting.h"
#ضمن "llvm/Support/SMTAPI.h"

ورتوں نطاق llvm;

#إذا LLVM_WITH_Z3

#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/ADT/Twine.h"

#ضمن <set>

#ضمن <z3.h>

نطاق {

/// Configuration class for Z3
صنف Z3Config {
  یار صنف Z3Context;

  Z3_config Config = Z3_mk_config();

عام:
  Z3Config() = افتراضي;
  Z3Config(ثابت Z3Config &) = مٹاؤ;
  Z3Config(Z3Config &&) = افتراضي;
  Z3Config &عامل=(Z3Config &) = مٹاؤ;
  Z3Config &عامل=(Z3Config &&) = افتراضي;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
فراغ Z3ErrorHandler(Z3_context Context, Z3_error_کوڈ Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_پیغام(Context, Error)));
}

/// Wrapper for Z3 context
صنف Z3Context {
عام:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_سیٹ_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = مٹاؤ;
  Z3Context(Z3Context &&) = افتراضي;
  Z3Context &عامل=(Z3Context &) = مٹاؤ;
  Z3Context &عامل=(Z3Context &&) = افتراضي;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = خالی_سوچک;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
صنف Z3_ترتیب_دیو : عام SMTSort {
  یار صنف Z3Solver;

  Z3Context &Context;

  Z3_ترتیب_دیو_2 ترتیب_دیو_2;

عام:
  /// Default constructor, mainly used by make_shared
  Z3_ترتیب_دیو(Z3Context &C, Z3_ترتیب_دیو_2 ZS) : Context(C), ترتیب_دیو_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دیو_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ترتیب_دیو(ثابت Z3_ترتیب_دیو &Other) : Context(Other.Context), ترتیب_دیو_2(Other.ترتیب_دیو_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دیو_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ترتیب_دیو &عامل=(ثابت Z3_ترتیب_دیو &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.ترتیب_دیو_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دیو_2));
    ترتیب_دیو_2 = Other.ترتیب_دیو_2;
    أرجع *هذا;
  }

  Z3_ترتیب_دیو(Z3_ترتیب_دیو &&Other) = مٹاؤ;
  Z3_ترتیب_دیو &عامل=(Z3_ترتیب_دیو &&Other) = مٹاؤ;

  ~Z3_ترتیب_دیو() {
    إذا (ترتیب_دیو_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دیو_2));
  }

  فراغ Profile(llvm::Folding_سیٹ_گرہ_ID &ID) ثابت override {
    ID.جوڑو_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(ترتیب_دیو_2)));
  }

  منطقي is_Bitvector_ترتیب_دیو_Impl() ثابت override {
    أرجع (Z3_get_ترتیب_دیو_قسم(Context.Context, ترتیب_دیو_2) == Z3_BV_ترتیب_دیو);
  }

  منطقي is_Float_ترتیب_دیو_Impl() ثابت override {
    أرجع (Z3_get_ترتیب_دیو_قسم(Context.Context, ترتیب_دیو_2) == Z3_FLOATING_نقطہ_ترتیب_دیو);
  }

  منطقي is_Boolean_ترتیب_دیو_Impl() ثابت override {
    أرجع (Z3_get_ترتیب_دیو_قسم(Context.Context, ترتیب_دیو_2) == Z3_BOOL_ترتیب_دیو);
  }

  بنانشانی get_Bitvector_ترتیب_دیو_ناپ_Impl() ثابت override {
    أرجع Z3_get_bv_ترتیب_دیو_ناپ(Context.Context, ترتیب_دیو_2);
  }

  بنانشانی get_Float_ترتیب_دیو_ناپ_Impl() ثابت override {
    أرجع Z3_fpa_get_ebits(Context.Context, ترتیب_دیو_2) +
           Z3_fpa_get_sbits(Context.Context, ترتیب_دیو_2);
  }

  منطقي equal_منزل(SMTSort ثابت &Other) ثابت override {
    أرجع Z3_is_eq_ترتیب_دیو(Context.Context, ترتیب_دیو_2,
                         static_cast<ثابت Z3_ترتیب_دیو &>(Other).ترتیب_دیو_2);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ترتیب_دیو_منزل_string(Context.Context, ترتیب_دیو_2);
  }
}; // end class Z3Sort

ساكن ثابت Z3_ترتیب_دیو &منزل_Z3_ترتیب_دیو(ثابت SMTSort &S) {
  أرجع static_cast<ثابت Z3_ترتیب_دیو &>(S);
}

صنف Z3_اظہاریہ : عام SMTExpr {
  یار صنف Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عام:
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
    أرجع *هذا;
  }

  Z3_اظہاریہ(Z3_اظہاریہ &&Other) = مٹاؤ;
  Z3_اظہاریہ &عامل=(Z3_اظہاریہ &&Other) = مٹاؤ;

  ~Z3_اظہاریہ() {
    إذا (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  فراغ Profile(llvm::Folding_سیٹ_گرہ_ID &ID) ثابت override {
    ID.جوڑو_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقي equal_منزل(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_ترتیب_دیو(Context.Context, Z3_get_ترتیب_دیو(Context.Context, AST),
                         Z3_get_ترتیب_دیو(Context.Context,
                                     static_cast<ثابت Z3_اظہاریہ &>(Other).AST)) &&
           "AST's must have the same sort");
    أرجع Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_اظہاریہ &>(Other).AST);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_منزل_string(Context.Context, AST);
  }
}; // end class Z3Expr

ساكن ثابت Z3_اظہاریہ &منزل_Z3_اظہاریہ(ثابت SMTExpr &E) {
  أرجع static_cast<ثابت Z3_اظہاریہ &>(E);
}

صنف Z3Model {
  یار صنف Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عام:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = مٹاؤ;
  Z3Model(Z3Model &&Other) = مٹاؤ;
  Z3Model &عامل=(Z3Model &Other) = مٹاؤ;
  Z3Model &عامل=(Z3Model &&Other) = مٹاؤ;

  ~Z3Model() {
    إذا (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  فراغ print(raw_ostream &OS) ثابت {
    OS << Z3_model_منزل_string(Context.Context, Model);
  }

  LLVM_DUMP_طریقہ فراغ dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ساكن ثابت llvm::fltSemantics &getFloatSemantics(بنانشانی ثنائی_ہندسہ_چوڑائی_2) {
  بدل (ثنائی_ہندسہ_چوڑائی_2) {
  افتراضي:
    llvm_unreachable("Unsupported floating-point semantics!");
    اكسر;
  حالة 16:
    أرجع llvm::APFloat::IEEEhalf();
  حالة 32:
    أرجع llvm::APFloat::IEEEsingle();
  حالة 64:
    أرجع llvm::APFloat::IEEEdouble();
  حالة 128:
    أرجع llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
ساكن منطقي areEquivalent(ثابت llvm::fltSemantics &LHS,
                          ثابت llvm::fltSemantics &RHS) {
  أرجع (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_ناپ_وچ_بٹ(LHS) ==
          llvm::APFloat::semantics_ناپ_وچ_بٹ(RHS));
}

صنف Z3Solver : عام SMTSolver {
  یار صنف Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [هذا] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    أرجع S;
  }();

  Z3_params Params = [هذا] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    أرجع P;
  }();

  // Cache Sorts
  std::سیٹ<Z3_ترتیب_دیو> CachedSorts;

  // Cache Exprs
  std::سیٹ<Z3_اظہاریہ> CachedExprs;

عام:
  Z3Solver() = افتراضي;
  Z3Solver(ثابت Z3Solver &Other) = مٹاؤ;
  Z3Solver(Z3Solver &&Other) = مٹاؤ;
  Z3Solver &عامل=(Z3Solver &Other) = مٹاؤ;
  Z3Solver &عامل=(Z3Solver &&Other) = مٹاؤ;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  فراغ جوڑو_Constraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, منزل_Z3_اظہاریہ(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_ترتیب_دیو_Ref(ثابت SMTSort &ترتیب_دیو_2) {
    خودکار It = CachedSorts.داخل_کرو(منزل_Z3_ترتیب_دیو(ترتیب_دیو_2));
    أرجع &(*It.پہلا_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_اظہاریہ_Ref(ثابت SMTExpr &Exp) {
    خودکار It = CachedExprs.داخل_کرو(منزل_Z3_اظہاریہ(Exp));
    أرجع &(*It.پہلا_2);
  }

  SMTSortRef get_Bool_ترتیب_دیو() override {
    أرجع new_ترتیب_دیو_Ref(Z3_ترتیب_دیو(Context, Z3_mk_bool_ترتیب_دیو(Context.Context)));
  }

  SMTSortRef get_Bitvector_ترتیب_دیو(بنانشانی ثنائی_ہندسہ_چوڑائی_2) override {
    أرجع new_ترتیب_دیو_Ref(
        Z3_ترتیب_دیو(Context, Z3_mk_bv_ترتیب_دیو(Context.Context, ثنائی_ہندسہ_چوڑائی_2)));
  }

  SMTSortRef get_ترتیب_دیو(ثابت SMTExprRef &Exp) override {
    أرجع new_ترتیب_دیو_Ref(
        Z3_ترتیب_دیو(Context, Z3_get_ترتیب_دیو(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTSortRef get_Float16_ترتیب_دیو() override {
    أرجع new_ترتیب_دیو_Ref(Z3_ترتیب_دیو(Context, Z3_mk_fpa_ترتیب_دیو_16(Context.Context)));
  }

  SMTSortRef get_Float32_ترتیب_دیو() override {
    أرجع new_ترتیب_دیو_Ref(Z3_ترتیب_دیو(Context, Z3_mk_fpa_ترتیب_دیو_32(Context.Context)));
  }

  SMTSortRef get_Float64_ترتیب_دیو() override {
    أرجع new_ترتیب_دیو_Ref(Z3_ترتیب_دیو(Context, Z3_mk_fpa_ترتیب_دیو_64(Context.Context)));
  }

  SMTSortRef get_Float128_ترتیب_دیو() override {
    أرجع new_ترتیب_دیو_Ref(Z3_ترتیب_دیو(Context, Z3_mk_fpa_ترتیب_دیو_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvneg(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvnot(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_not(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvadd(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsub(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvmul(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsrem(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvurem(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsdiv(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvudiv(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvshl(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvashr(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvlshr(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvxor(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvor(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                   منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvand(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvult(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvslt(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvugt(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsgt(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvule(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsle(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvuge(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_bvsge(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                    منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دلیلاں[2] = {منزل_Z3_اظہاریہ(*LHS).AST, منزل_Z3_اظہاریہ(*RHS).AST};
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_and(Context.Context, 2, دلیلاں)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دلیلاں[2] = {منزل_Z3_اظہاریہ(*LHS).AST, منزل_Z3_اظہاریہ(*RHS).AST};
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_or(Context.Context, 2, دلیلاں)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_eq(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                 منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_neg(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_is_infinite(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_is_nan(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_عام(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_is_عام(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_is_zero(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_mul(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                             منزل_Z3_اظہاریہ(*LHS).AST, منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_div(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                             منزل_Z3_اظہاریہ(*LHS).AST, منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_rem(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                      منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_جوڑو(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                             منزل_Z3_اظہاریہ(*LHS).AST, منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_mk_fpa_sub(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                             منزل_Z3_اظہاریہ(*LHS).AST, منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_lt(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_gt(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_leq(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                      منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_geq(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                      منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_fpa_eq(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_ite(Context.Context, منزل_Z3_اظہاریہ(*Cond).AST,
                                  منزل_Z3_اظہاریہ(*T).AST, منزل_Z3_اظہاریہ(*F).AST)));
  }

  SMTExprRef mkBVSignExt(بنانشانی i, ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_sign_ext(Context.Context, i, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(بنانشانی i, ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_zero_ext(Context.Context, i, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(بنانشانی بلند_2, بنانشانی کم_2,
                         ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_کڈھو(Context.Context, بلند_2, کم_2,
                                                    منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_حد_توں_ودھ(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvadd_no_حد_توں_ودھ(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                         منزل_Z3_اظہاریہ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                          منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_حد_توں_ودھ(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvsub_no_حد_توں_ودھ(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                         منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقي isSigned) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                          منزل_Z3_اظہاریہ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_حد_توں_ودھ(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvsdiv_no_حد_توں_ودھ(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                          منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_حد_توں_ودھ(ثابت SMTExprRef &Exp) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvneg_no_حد_توں_ودھ(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_حد_توں_ودھ(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvmul_no_حد_توں_ودھ(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                         منزل_Z3_اظہاریہ(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                          منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_concat(Context.Context, منزل_Z3_اظہاریہ(*LHS).AST,
                                     منزل_Z3_اظہاریہ(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context,
        Z3_mk_fpa_منزل_fp_float(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                              منزل_Z3_اظہاریہ(*From).AST, منزل_Z3_ترتیب_دیو(*منزل).ترتیب_دیو_2)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context,
        Z3_mk_fpa_منزل_fp_signed(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                               منزل_Z3_اظہاریہ(*From).AST, منزل_Z3_ترتیب_دیو(*منزل).ترتیب_دیو_2)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context,
        Z3_mk_fpa_منزل_fp_unsigned(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                                 منزل_Z3_اظہاریہ(*From).AST, منزل_Z3_ترتیب_دیو(*منزل).ترتیب_دیو_2)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, بنانشانی منزل_چوڑائی) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_منزل_sbv(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                                  منزل_Z3_اظہاریہ(*From).AST, منزل_چوڑائی)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, بنانشانی منزل_چوڑائی) override {
    SMTExprRef Rounding_منوال = get_Float_Rounding_منوال();
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_منزل_ubv(Context.Context, منزل_Z3_اظہاریہ(*Rounding_منوال).AST,
                                  منزل_Z3_اظہاریہ(*From).AST, منزل_چوڑائی)));
  }

  SMTExprRef mkBoolean(ثابت منطقي b) override {
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, بنانشانی ثنائی_ہندسہ_چوڑائی_2) override {
    ثابت Z3_ترتیب_دیو_2 Z3_ترتیب_دیو = منزل_Z3_ترتیب_دیو(*get_Bitvector_ترتیب_دیو(ثنائی_ہندسہ_چوڑائی_2)).ترتیب_دیو_2;

    // Slow path, when 64 bits are not enough.
    إذا (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> بفر;
      Int.منزل_String(بفر, 10);
      أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
          Context, Z3_mk_numeral(Context.Context, بفر.c_str(), Z3_ترتیب_دیو)));
    }

    ثابت int64_t ثنائی_ہندسہ_Repr_As_Signed = Int.get_Ext_قدر();
    ثابت uint64_t ثنائی_ہندسہ_Repr_As_Unsigned =
        reinterpret_cast<ثابت uint64_t &>(ثنائی_ہندسہ_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, ثنائی_ہندسہ_Repr_As_Signed, Z3_ترتیب_دیو)
            : Z3_mk_unsigned_int64(Context.Context, ثنائی_ہندسہ_Repr_As_Unsigned, Z3_ترتیب_دیو);
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef ترتیب_دیو_2 =
        get_Float_ترتیب_دیو(llvm::APFloat::semantics_ناپ_وچ_بٹ(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_منزل_APInt(), خطأ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_ثنائی_ہندسہ_چوڑائی());
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(
        Context, Z3_mk_fpa_منزل_fp_bv(Context.Context, منزل_Z3_اظہاریہ(*Z3Int).AST,
                                    منزل_Z3_ترتیب_دیو(*ترتیب_دیو_2).ترتیب_دیو_2)));
  }

  SMTExprRef mk_علامت(ثابت حرف *Name, SMTSortRef ترتیب_دیو_2) override {
    أرجع new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_علامت(Context.Context, Name),
                                    منزل_Z3_ترتیب_دیو(*ترتیب_دیو_2).ترتیب_دیو_2)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, بنانشانی ثنائی_ہندسہ_چوڑائی_2,
                            منطقي isUnsigned) override {
    أرجع llvm::APSInt(
        llvm::APInt(ثنائی_ہندسہ_چوڑائی_2,
                    Z3_get_numeral_string(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقي getBoolean(ثابت SMTExprRef &Exp) override {
    أرجع Z3_get_bool_قدر(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_منوال() override {
    // TODO: Don't assume nearest ties to even rounding mode
    أرجع new_اظہاریہ_Ref(Z3_اظہاریہ(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقي منزل_APFloat(ثابت SMTSortRef &ترتیب_دیو_2, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقي useSemantics) {
    أكد(ترتیب_دیو_2->is_Float_ترتیب_دیو() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(ترتیب_دیو_2->get_Float_ترتیب_دیو_ناپ(), سچ);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(ترتیب_دیو_2->get_Float_ترتیب_دیو_ناپ());
    SMTSortRef BVSort = get_Bitvector_ترتیب_دیو(ترتیب_دیو_2->get_Float_ترتیب_دیو_ناپ());
    إذا (!منزل_APSInt(BVSort, AST, Int, سچ)) {
      أرجع خطأ;
    }

    إذا (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      أكد(خطأ && "Floating-point types don't match!");
      أرجع خطأ;
    }

    Float = llvm::APFloat(Semantics, Int);
    أرجع سچ;
  }

  منطقي منزل_APSInt(ثابت SMTSortRef &ترتیب_دیو_2, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقي useSemantics) {
    إذا (ترتیب_دیو_2->is_Bitvector_ترتیب_دیو()) {
      إذا (useSemantics && Int.get_ثنائی_ہندسہ_چوڑائی() != ترتیب_دیو_2->get_Bitvector_ترتیب_دیو_ناپ()) {
        أكد(خطأ && "Bitvector types don't match!");
        أرجع خطأ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_ثنائی_ہندسہ_چوڑائی(), Int.isUnsigned());
      أرجع سچ;
    }

    إذا (ترتیب_دیو_2->is_Boolean_ترتیب_دیو()) {
      إذا (useSemantics && Int.get_ثنائی_ہندسہ_چوڑائی() < 1) {
        أكد(خطأ && "Boolean type doesn't match!");
        أرجع خطأ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_ثنائی_ہندسہ_چوڑائی(), getBoolean(AST)),
                         Int.isUnsigned());
      أرجع سچ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_منزل_app(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef سونپو_2 = new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتیب_دیو_2 = get_ترتیب_دیو(سونپو_2);
    أرجع منزل_APSInt(ترتیب_دیو_2, سونپو_2, Int, سچ);
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_منزل_app(Context.Context, منزل_Z3_اظہاریہ(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef سونپو_2 = new_اظہاریہ_Ref(
        Z3_اظہاریہ(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتیب_دیو_2 = get_ترتیب_دیو(سونپو_2);
    أرجع منزل_APFloat(ترتیب_دیو_2, سونپو_2, Float, سچ);
  }

  std::optional<منطقي> check() ثابت override {
    Z3_solver_سیٹ_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    إذا (res == Z3_L_TRUE)
      أرجع سچ;

    إذا (res == Z3_L_FALSE)
      أرجع خطأ;

    أرجع std::nullopt;
  }

  فراغ ڈالو() override { أرجع Z3_solver_ڈالو(Context.Context, Solver); }

  فراغ نکالو(بنانشانی عدد_حالتاں = 1) override {
    أكد(Z3_solver_get_عدد_scopes(Context.Context, Solver) >= عدد_حالتاں);
    أرجع Z3_solver_نکالو(Context.Context, Solver, عدد_حالتاں);
  }

  منطقي isFPSupported() override { أرجع سچ; }

  /// Reset the solver and remove all constraints.
  فراغ reset() override { Z3_solver_reset(Context.Context, Solver); }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_منزل_string(Context.Context, Solver);
  }

  فراغ سیٹ_Unsigned_Param(StringRef کلید, بنانشانی قدر) override {
    Z3_علامت Sym = Z3_mk_string_علامت(Context.Context, کلید.str().c_str());
    Z3_params_سیٹ_uint(Context.Context, Params, Sym, قدر);
  }

  فراغ سیٹ_Bool_Param(StringRef کلید, منطقي قدر) override {
    Z3_علامت Sym = Z3_mk_string_علامت(Context.Context, کلید.str().c_str());
    Z3_params_سیٹ_bool(Context.Context, Params, Sym, قدر);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

صنف Z3Statistics final : عام SMTSolverStatistics {
عام:
  مزدوج getDouble(StringRef کلید) ثابت override {
    خودکار It = Double_اقدار.لبھو(کلید.str());
    أكد(It != Double_اقدار.سرا());
    أرجع It->دوجا_2;
  };
  بنانشانی getUnsigned(StringRef کلید) ثابت override {
    خودکار It = Unsigned_اقدار.لبھو(کلید.str());
    إذا (It == Unsigned_اقدار.سرا())
      أرجع 0;
    أرجع It->دوجا_2;
  };

  فراغ print(raw_ostream &OS) ثابت override {
    لكل (ثابت خودکار &E : Unsigned_اقدار)
      OS << E.پہلا_2() << ": " << E.دوجا_2 << '\n';
    لكل (ثابت خودکار &E : Double_اقدار) {
      write_double(OS << E.پہلا_2() << ": ", E.دوجا_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

خاص:
  یار صنف Z3Solver;
  StringMap<بنانشانی> Unsigned_اقدار;
  StringMap<مزدوج> Double_اقدار;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  خودکار ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics نتیجہ;

  بنانشانی عدد_کلیدیں = Z3_stats_ناپ(C, S);
  لكل (بنانشانی Idx = 0; Idx < عدد_کلیدیں; ++Idx) {
    ثابت حرف *کلید = Z3_stats_get_کلید(C, S, Idx);
    إذا (Z3_stats_is_uint(C, S, Idx)) {
      خودکار قدر = Z3_stats_get_uint_قدر(C, S, Idx);
      نتیجہ.Unsigned_اقدار.try_emplace(کلید, قدر);
    } وإلا {
      أكد(Z3_stats_is_double(C, S, Idx));
      خودکار قدر = Z3_stats_get_double_قدر(C, S, Idx);
      نتیجہ.Double_اقدار.try_emplace(کلید, قدر);
    }
  }
  أرجع std::make_unique<Z3Statistics>(std::move(نتیجہ));
}

} // end anonymous namespace

#جے_انت

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#إذا LLVM_WITH_Z3
  أرجع std::make_unique<Z3Solver>();
#وإلا
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           خطأ);
  أرجع خالی_سوچک;
#جے_انت
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ فراغ SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طریقہ فراغ SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طریقہ فراغ SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طریقہ فراغ SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#جے_انت
