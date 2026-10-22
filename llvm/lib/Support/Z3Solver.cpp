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

استعمال نطاق llvm;

#إذا LLVM_WITH_Z3

#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/ADT/Twine.h"

#ضمن <set>

#ضمن <z3.h>

نطاق {

/// Configuration class for Z3
صنف Z3Config {
  دوست صنف Z3Context;

  Z3_config Config = Z3_mk_config();

عام:
  Z3Config() = افتراضي;
  Z3Config(ثابت Z3Config &) = ميٽيو;
  Z3Config(Z3Config &&) = افتراضي;
  Z3Config &عامل=(Z3Config &) = ميٽيو;
  Z3Config &عامل=(Z3Config &&) = افتراضي;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
فراغ Z3ErrorHandler(Z3_context Context, Z3_error_ڪوڊ Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_نياپو(Context, Error)));
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
    Z3_سيٽ_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = ميٽيو;
  Z3Context(Z3Context &&) = افتراضي;
  Z3Context &عامل=(Z3Context &) = ميٽيو;
  Z3Context &عامل=(Z3Context &&) = افتراضي;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = خالي_اشارو;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
صنف Z3_ترتيب_ڏيو : عام SMTSort {
  دوست صنف Z3Solver;

  Z3Context &Context;

  Z3_ترتيب_ڏيو_2 ترتيب_ڏيو_2;

عام:
  /// Default constructor, mainly used by make_shared
  Z3_ترتيب_ڏيو(Z3Context &C, Z3_ترتيب_ڏيو_2 ZS) : Context(C), ترتيب_ڏيو_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتيب_ڏيو_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ترتيب_ڏيو(ثابت Z3_ترتيب_ڏيو &Other) : Context(Other.Context), ترتيب_ڏيو_2(Other.ترتيب_ڏيو_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتيب_ڏيو_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ترتيب_ڏيو &عامل=(ثابت Z3_ترتيب_ڏيو &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.ترتيب_ڏيو_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتيب_ڏيو_2));
    ترتيب_ڏيو_2 = Other.ترتيب_ڏيو_2;
    أرجع *هذا;
  }

  Z3_ترتيب_ڏيو(Z3_ترتيب_ڏيو &&Other) = ميٽيو;
  Z3_ترتيب_ڏيو &عامل=(Z3_ترتيب_ڏيو &&Other) = ميٽيو;

  ~Z3_ترتيب_ڏيو() {
    إذا (ترتيب_ڏيو_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتيب_ڏيو_2));
  }

  فراغ Profile(llvm::Folding_سيٽ_ڳنڍ_ID &ID) ثابت override {
    ID.جوڙيو_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(ترتيب_ڏيو_2)));
  }

  منطقي is_Bitvector_ترتيب_ڏيو_Impl() ثابت override {
    أرجع (Z3_get_ترتيب_ڏيو_قسم(Context.Context, ترتيب_ڏيو_2) == Z3_BV_ترتيب_ڏيو);
  }

  منطقي is_Float_ترتيب_ڏيو_Impl() ثابت override {
    أرجع (Z3_get_ترتيب_ڏيو_قسم(Context.Context, ترتيب_ڏيو_2) == Z3_FLOATING_نقطو_ترتيب_ڏيو);
  }

  منطقي is_Boolean_ترتيب_ڏيو_Impl() ثابت override {
    أرجع (Z3_get_ترتيب_ڏيو_قسم(Context.Context, ترتيب_ڏيو_2) == Z3_BOOL_ترتيب_ڏيو);
  }

  بنانشان get_Bitvector_ترتيب_ڏيو_سائيز_Impl() ثابت override {
    أرجع Z3_get_bv_ترتيب_ڏيو_سائيز(Context.Context, ترتيب_ڏيو_2);
  }

  بنانشان get_Float_ترتيب_ڏيو_سائيز_Impl() ثابت override {
    أرجع Z3_fpa_get_ebits(Context.Context, ترتيب_ڏيو_2) +
           Z3_fpa_get_sbits(Context.Context, ترتيب_ڏيو_2);
  }

  منطقي equal_منزل(SMTSort ثابت &Other) ثابت override {
    أرجع Z3_is_eq_ترتيب_ڏيو(Context.Context, ترتيب_ڏيو_2,
                         static_cast<ثابت Z3_ترتيب_ڏيو &>(Other).ترتيب_ڏيو_2);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ترتيب_ڏيو_منزل_string(Context.Context, ترتيب_ڏيو_2);
  }
}; // end class Z3Sort

ساكن ثابت Z3_ترتيب_ڏيو &منزل_Z3_ترتيب_ڏيو(ثابت SMTSort &S) {
  أرجع static_cast<ثابت Z3_ترتيب_ڏيو &>(S);
}

صنف Z3_اظهار : عام SMTExpr {
  دوست صنف Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عام:
  Z3_اظهار(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_اظهار(ثابت Z3_اظهار &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_اظهار &عامل=(ثابت Z3_اظهار &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    أرجع *هذا;
  }

  Z3_اظهار(Z3_اظهار &&Other) = ميٽيو;
  Z3_اظهار &عامل=(Z3_اظهار &&Other) = ميٽيو;

  ~Z3_اظهار() {
    إذا (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  فراغ Profile(llvm::Folding_سيٽ_ڳنڍ_ID &ID) ثابت override {
    ID.جوڙيو_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقي equal_منزل(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_ترتيب_ڏيو(Context.Context, Z3_get_ترتيب_ڏيو(Context.Context, AST),
                         Z3_get_ترتيب_ڏيو(Context.Context,
                                     static_cast<ثابت Z3_اظهار &>(Other).AST)) &&
           "AST's must have the same sort");
    أرجع Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_اظهار &>(Other).AST);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_منزل_string(Context.Context, AST);
  }
}; // end class Z3Expr

ساكن ثابت Z3_اظهار &منزل_Z3_اظهار(ثابت SMTExpr &E) {
  أرجع static_cast<ثابت Z3_اظهار &>(E);
}

صنف Z3Model {
  دوست صنف Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عام:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = ميٽيو;
  Z3Model(Z3Model &&Other) = ميٽيو;
  Z3Model &عامل=(Z3Model &Other) = ميٽيو;
  Z3Model &عامل=(Z3Model &&Other) = ميٽيو;

  ~Z3Model() {
    إذا (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  فراغ print(raw_ostream &OS) ثابت {
    OS << Z3_model_منزل_string(Context.Context, Model);
  }

  LLVM_DUMP_طريقو فراغ dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ساكن ثابت llvm::fltSemantics &getFloatSemantics(بنانشان ٻٽو_انگ_ويڪر_2) {
  بدل (ٻٽو_انگ_ويڪر_2) {
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
         (llvm::APFloat::semantics_سائيز_اندريون_بٽ(LHS) ==
          llvm::APFloat::semantics_سائيز_اندريون_بٽ(RHS));
}

صنف Z3Solver : عام SMTSolver {
  دوست صنف Z3ConstraintManager;

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
  std::سيٽ<Z3_ترتيب_ڏيو> CachedSorts;

  // Cache Exprs
  std::سيٽ<Z3_اظهار> CachedExprs;

عام:
  Z3Solver() = افتراضي;
  Z3Solver(ثابت Z3Solver &Other) = ميٽيو;
  Z3Solver(Z3Solver &&Other) = ميٽيو;
  Z3Solver &عامل=(Z3Solver &Other) = ميٽيو;
  Z3Solver &عامل=(Z3Solver &&Other) = ميٽيو;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  فراغ جوڙيو_Constraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, منزل_Z3_اظهار(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_ترتيب_ڏيو_Ref(ثابت SMTSort &ترتيب_ڏيو_2) {
    خودڪار It = CachedSorts.وجهو(منزل_Z3_ترتيب_ڏيو(ترتيب_ڏيو_2));
    أرجع &(*It.پهريون_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_اظهار_Ref(ثابت SMTExpr &Exp) {
    خودڪار It = CachedExprs.وجهو(منزل_Z3_اظهار(Exp));
    أرجع &(*It.پهريون_2);
  }

  SMTSortRef get_Bool_ترتيب_ڏيو() override {
    أرجع new_ترتيب_ڏيو_Ref(Z3_ترتيب_ڏيو(Context, Z3_mk_bool_ترتيب_ڏيو(Context.Context)));
  }

  SMTSortRef get_Bitvector_ترتيب_ڏيو(بنانشان ٻٽو_انگ_ويڪر_2) override {
    أرجع new_ترتيب_ڏيو_Ref(
        Z3_ترتيب_ڏيو(Context, Z3_mk_bv_ترتيب_ڏيو(Context.Context, ٻٽو_انگ_ويڪر_2)));
  }

  SMTSortRef get_ترتيب_ڏيو(ثابت SMTExprRef &Exp) override {
    أرجع new_ترتيب_ڏيو_Ref(
        Z3_ترتيب_ڏيو(Context, Z3_get_ترتيب_ڏيو(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTSortRef get_Float16_ترتيب_ڏيو() override {
    أرجع new_ترتيب_ڏيو_Ref(Z3_ترتيب_ڏيو(Context, Z3_mk_fpa_ترتيب_ڏيو_16(Context.Context)));
  }

  SMTSortRef get_Float32_ترتيب_ڏيو() override {
    أرجع new_ترتيب_ڏيو_Ref(Z3_ترتيب_ڏيو(Context, Z3_mk_fpa_ترتيب_ڏيو_32(Context.Context)));
  }

  SMTSortRef get_Float64_ترتيب_ڏيو() override {
    أرجع new_ترتيب_ڏيو_Ref(Z3_ترتيب_ڏيو(Context, Z3_mk_fpa_ترتيب_ڏيو_64(Context.Context)));
  }

  SMTSortRef get_Float128_ترتيب_ڏيو() override {
    أرجع new_ترتيب_ڏيو_Ref(Z3_ترتيب_ڏيو(Context, Z3_mk_fpa_ترتيب_ڏيو_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvneg(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvnot(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_not(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvadd(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvsub(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvmul(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvsrem(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvurem(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvsdiv(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvudiv(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvshl(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvashr(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvlshr(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvxor(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvor(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                   منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvand(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvult(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvslt(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvugt(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvsgt(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvule(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvsle(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvuge(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_bvsge(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                    منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دليلون[2] = {منزل_Z3_اظهار(*LHS).AST, منزل_Z3_اظهار(*RHS).AST};
    أرجع new_اظهار_Ref(Z3_اظهار(Context, Z3_mk_and(Context.Context, 2, دليلون)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دليلون[2] = {منزل_Z3_اظهار(*LHS).AST, منزل_Z3_اظهار(*RHS).AST};
    أرجع new_اظهار_Ref(Z3_اظهار(Context, Z3_mk_or(Context.Context, 2, دليلون)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_eq(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                 منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_neg(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_fpa_is_infinite(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_is_nan(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_عام(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_fpa_is_عام(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_fpa_is_zero(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context,
               Z3_mk_fpa_mul(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                             منزل_Z3_اظهار(*LHS).AST, منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context,
               Z3_mk_fpa_div(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                             منزل_Z3_اظهار(*LHS).AST, منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_rem(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                      منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context,
               Z3_mk_fpa_جوڙيو(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                             منزل_Z3_اظهار(*LHS).AST, منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context,
               Z3_mk_fpa_sub(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                             منزل_Z3_اظهار(*LHS).AST, منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_lt(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_gt(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_leq(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                      منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_geq(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                      منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_fpa_eq(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_ite(Context.Context, منزل_Z3_اظهار(*Cond).AST,
                                  منزل_Z3_اظهار(*T).AST, منزل_Z3_اظهار(*F).AST)));
  }

  SMTExprRef mkBVSignExt(بنانشان i, ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_sign_ext(Context.Context, i, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(بنانشان i, ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_zero_ext(Context.Context, i, منزل_Z3_اظهار(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(بنانشان مٿاهون_2, بنانشان هيٺيون_2,
                         ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(Context, Z3_mk_ڪڍو(Context.Context, مٿاهون_2, هيٺيون_2,
                                                    منزل_Z3_اظهار(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_حد_کان_مٿي(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvadd_no_حد_کان_مٿي(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                         منزل_Z3_اظهار(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                          منزل_Z3_اظهار(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_حد_کان_مٿي(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvsub_no_حد_کان_مٿي(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                         منزل_Z3_اظهار(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقي isSigned) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                          منزل_Z3_اظهار(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_حد_کان_مٿي(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvsdiv_no_حد_کان_مٿي(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                          منزل_Z3_اظهار(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_حد_کان_مٿي(ثابت SMTExprRef &Exp) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvneg_no_حد_کان_مٿي(Context.Context, منزل_Z3_اظهار(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_حد_کان_مٿي(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvmul_no_حد_کان_مٿي(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                         منزل_Z3_اظهار(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                          منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_concat(Context.Context, منزل_Z3_اظهار(*LHS).AST,
                                     منزل_Z3_اظهار(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context,
        Z3_mk_fpa_منزل_fp_float(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                              منزل_Z3_اظهار(*From).AST, منزل_Z3_ترتيب_ڏيو(*منزل).ترتيب_ڏيو_2)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context,
        Z3_mk_fpa_منزل_fp_signed(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                               منزل_Z3_اظهار(*From).AST, منزل_Z3_ترتيب_ڏيو(*منزل).ترتيب_ڏيو_2)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context,
        Z3_mk_fpa_منزل_fp_unsigned(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                                 منزل_Z3_اظهار(*From).AST, منزل_Z3_ترتيب_ڏيو(*منزل).ترتيب_ڏيو_2)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, بنانشان منزل_ويڪر) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_fpa_منزل_sbv(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                                  منزل_Z3_اظهار(*From).AST, منزل_ويڪر)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, بنانشان منزل_ويڪر) override {
    SMTExprRef Rounding_گهڻائي = get_Float_Rounding_گهڻائي();
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_fpa_منزل_ubv(Context.Context, منزل_Z3_اظهار(*Rounding_گهڻائي).AST,
                                  منزل_Z3_اظهار(*From).AST, منزل_ويڪر)));
  }

  SMTExprRef mkBoolean(ثابت منطقي b) override {
    أرجع new_اظهار_Ref(Z3_اظهار(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, بنانشان ٻٽو_انگ_ويڪر_2) override {
    ثابت Z3_ترتيب_ڏيو_2 Z3_ترتيب_ڏيو = منزل_Z3_ترتيب_ڏيو(*get_Bitvector_ترتيب_ڏيو(ٻٽو_انگ_ويڪر_2)).ترتيب_ڏيو_2;

    // Slow path, when 64 bits are not enough.
    إذا (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> بفر;
      Int.منزل_String(بفر, 10);
      أرجع new_اظهار_Ref(Z3_اظهار(
          Context, Z3_mk_numeral(Context.Context, بفر.c_str(), Z3_ترتيب_ڏيو)));
    }

    ثابت int64_t ٻٽو_انگ_Repr_As_Signed = Int.get_Ext_قدر();
    ثابت uint64_t ٻٽو_انگ_Repr_As_Unsigned =
        reinterpret_cast<ثابت uint64_t &>(ٻٽو_انگ_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, ٻٽو_انگ_Repr_As_Signed, Z3_ترتيب_ڏيو)
            : Z3_mk_unsigned_int64(Context.Context, ٻٽو_انگ_Repr_As_Unsigned, Z3_ترتيب_ڏيو);
    أرجع new_اظهار_Ref(Z3_اظهار(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef ترتيب_ڏيو_2 =
        get_Float_ترتيب_ڏيو(llvm::APFloat::semantics_سائيز_اندريون_بٽ(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_منزل_APInt(), خطأ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_ٻٽو_انگ_ويڪر());
    أرجع new_اظهار_Ref(Z3_اظهار(
        Context, Z3_mk_fpa_منزل_fp_bv(Context.Context, منزل_Z3_اظهار(*Z3Int).AST,
                                    منزل_Z3_ترتيب_ڏيو(*ترتيب_ڏيو_2).ترتيب_ڏيو_2)));
  }

  SMTExprRef mk_نشان(ثابت حرف *Name, SMTSortRef ترتيب_ڏيو_2) override {
    أرجع new_اظهار_Ref(
        Z3_اظهار(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_نشان(Context.Context, Name),
                                    منزل_Z3_ترتيب_ڏيو(*ترتيب_ڏيو_2).ترتيب_ڏيو_2)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, بنانشان ٻٽو_انگ_ويڪر_2,
                            منطقي isUnsigned) override {
    أرجع llvm::APSInt(
        llvm::APInt(ٻٽو_انگ_ويڪر_2,
                    Z3_get_numeral_string(Context.Context, منزل_Z3_اظهار(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقي getBoolean(ثابت SMTExprRef &Exp) override {
    أرجع Z3_get_bool_قدر(Context.Context, منزل_Z3_اظهار(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_گهڻائي() override {
    // TODO: Don't assume nearest ties to even rounding mode
    أرجع new_اظهار_Ref(Z3_اظهار(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقي منزل_APFloat(ثابت SMTSortRef &ترتيب_ڏيو_2, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقي useSemantics) {
    أكد(ترتيب_ڏيو_2->is_Float_ترتيب_ڏيو() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(ترتيب_ڏيو_2->get_Float_ترتيب_ڏيو_سائيز(), سچ);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(ترتيب_ڏيو_2->get_Float_ترتيب_ڏيو_سائيز());
    SMTSortRef BVSort = get_Bitvector_ترتيب_ڏيو(ترتيب_ڏيو_2->get_Float_ترتيب_ڏيو_سائيز());
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

  منطقي منزل_APSInt(ثابت SMTSortRef &ترتيب_ڏيو_2, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقي useSemantics) {
    إذا (ترتيب_ڏيو_2->is_Bitvector_ترتيب_ڏيو()) {
      إذا (useSemantics && Int.get_ٻٽو_انگ_ويڪر() != ترتيب_ڏيو_2->get_Bitvector_ترتيب_ڏيو_سائيز()) {
        أكد(خطأ && "Bitvector types don't match!");
        أرجع خطأ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_ٻٽو_انگ_ويڪر(), Int.isUnsigned());
      أرجع سچ;
    }

    إذا (ترتيب_ڏيو_2->is_Boolean_ترتيب_ڏيو()) {
      إذا (useSemantics && Int.get_ٻٽو_انگ_ويڪر() < 1) {
        أكد(خطأ && "Boolean type doesn't match!");
        أرجع خطأ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_ٻٽو_انگ_ويڪر(), getBoolean(AST)),
                         Int.isUnsigned());
      أرجع سچ;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_منزل_app(Context.Context, منزل_Z3_اظهار(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef سونپيو_2 = new_اظهار_Ref(
        Z3_اظهار(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتيب_ڏيو_2 = get_ترتيب_ڏيو(سونپيو_2);
    أرجع منزل_APSInt(ترتيب_ڏيو_2, سونپيو_2, Int, سچ);
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_منزل_app(Context.Context, منزل_Z3_اظهار(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef سونپيو_2 = new_اظهار_Ref(
        Z3_اظهار(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتيب_ڏيو_2 = get_ترتيب_ڏيو(سونپيو_2);
    أرجع منزل_APFloat(ترتيب_ڏيو_2, سونپيو_2, Float, سچ);
  }

  std::optional<منطقي> check() ثابت override {
    Z3_solver_سيٽ_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    إذا (res == Z3_L_TRUE)
      أرجع سچ;

    إذا (res == Z3_L_FALSE)
      أرجع خطأ;

    أرجع std::nullopt;
  }

  فراغ ڌڪو() override { أرجع Z3_solver_ڌڪو(Context.Context, Solver); }

  فراغ ڪڍو(بنانشان انگ_حالتون = 1) override {
    أكد(Z3_solver_get_انگ_scopes(Context.Context, Solver) >= انگ_حالتون);
    أرجع Z3_solver_ڪڍو(Context.Context, Solver, انگ_حالتون);
  }

  منطقي isFPSupported() override { أرجع سچ; }

  /// Reset the solver and remove all constraints.
  فراغ reset() override { Z3_solver_reset(Context.Context, Solver); }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_منزل_string(Context.Context, Solver);
  }

  فراغ سيٽ_Unsigned_Param(StringRef ڪنجي, بنانشان قدر) override {
    Z3_نشان Sym = Z3_mk_string_نشان(Context.Context, ڪنجي.str().c_str());
    Z3_params_سيٽ_uint(Context.Context, Params, Sym, قدر);
  }

  فراغ سيٽ_Bool_Param(StringRef ڪنجي, منطقي قدر) override {
    Z3_نشان Sym = Z3_mk_string_نشان(Context.Context, ڪنجي.str().c_str());
    Z3_params_سيٽ_bool(Context.Context, Params, Sym, قدر);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

صنف Z3Statistics final : عام SMTSolverStatistics {
عام:
  مزدوج getDouble(StringRef ڪنجي) ثابت override {
    خودڪار It = Double_قدرون.ڳوليو(ڪنجي.str());
    أكد(It != Double_قدرون.پڇاڙي());
    أرجع It->ٻيو_2;
  };
  بنانشان getUnsigned(StringRef ڪنجي) ثابت override {
    خودڪار It = Unsigned_قدرون.ڳوليو(ڪنجي.str());
    إذا (It == Unsigned_قدرون.پڇاڙي())
      أرجع 0;
    أرجع It->ٻيو_2;
  };

  فراغ print(raw_ostream &OS) ثابت override {
    لكل (ثابت خودڪار &E : Unsigned_قدرون)
      OS << E.پهريون_2() << ": " << E.ٻيو_2 << '\n';
    لكل (ثابت خودڪار &E : Double_قدرون) {
      write_double(OS << E.پهريون_2() << ": ", E.ٻيو_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

خاص:
  دوست صنف Z3Solver;
  StringMap<بنانشان> Unsigned_قدرون;
  StringMap<مزدوج> Double_قدرون;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  خودڪار ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics نتيجو;

  بنانشان انگ_ڪنجيون = Z3_stats_سائيز(C, S);
  لكل (بنانشان Idx = 0; Idx < انگ_ڪنجيون; ++Idx) {
    ثابت حرف *ڪنجي = Z3_stats_get_ڪنجي(C, S, Idx);
    إذا (Z3_stats_is_uint(C, S, Idx)) {
      خودڪار قدر = Z3_stats_get_uint_قدر(C, S, Idx);
      نتيجو.Unsigned_قدرون.try_emplace(ڪنجي, قدر);
    } وإلا {
      أكد(Z3_stats_is_double(C, S, Idx));
      خودڪار قدر = Z3_stats_get_double_قدر(C, S, Idx);
      نتيجو.Double_قدرون.try_emplace(ڪنجي, قدر);
    }
  }
  أرجع std::make_unique<Z3Statistics>(std::move(نتيجو));
}

} // end anonymous namespace

#جيڪڏهن_پورو

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#إذا LLVM_WITH_Z3
  أرجع std::make_unique<Z3Solver>();
#وإلا
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           خطأ);
  أرجع خالي_اشارو;
#جيڪڏهن_پورو
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طريقو فراغ SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طريقو فراغ SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طريقو فراغ SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_طريقو فراغ SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#جيڪڏهن_پورو
