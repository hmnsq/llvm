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

کارول نطاق llvm;

#إذا LLVM_WITH_Z3

#ضمن "llvm/ADT/SmallString.h"
#ضمن "llvm/ADT/StringMap.h"
#ضمن "llvm/ADT/Twine.h"

#ضمن <set>

#ضمن <z3.h>

نطاق {

/// Configuration class for Z3
صنف Z3Config {
  ملګری صنف Z3Context;

  Z3_config Config = Z3_mk_config();

عام:
  Z3Config() = افتراضي;
  Z3Config(ثابت Z3Config &) = ړنګول;
  Z3Config(Z3Config &&) = افتراضي;
  Z3Config &چلوونکی=(Z3Config &) = ړنګول;
  Z3Config &چلوونکی=(Z3Config &&) = افتراضي;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
فراغ Z3ErrorHandler(Z3_context Context, Z3_error_کوډ Error) {
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
    Z3_سېټ_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = ړنګول;
  Z3Context(Z3Context &&) = افتراضي;
  Z3Context &چلوونکی=(Z3Context &) = ړنګول;
  Z3Context &چلوونکی=(Z3Context &&) = افتراضي;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = تش_اشاره;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
صنف Z3_ترتیبول : عام SMTSort {
  ملګری صنف Z3Solver;

  Z3Context &Context;

  Z3_ترتیبول_2 ترتیبول_2;

عام:
  /// Default constructor, mainly used by make_shared
  Z3_ترتیبول(Z3Context &C, Z3_ترتیبول_2 ZS) : Context(C), ترتیبول_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیبول_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_ترتیبول(ثابت Z3_ترتیبول &Other) : Context(Other.Context), ترتیبول_2(Other.ترتیبول_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیبول_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_ترتیبول &چلوونکی=(ثابت Z3_ترتیبول &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.ترتیبول_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیبول_2));
    ترتیبول_2 = Other.ترتیبول_2;
    أرجع *هذا;
  }

  Z3_ترتیبول(Z3_ترتیبول &&Other) = ړنګول;
  Z3_ترتیبول &چلوونکی=(Z3_ترتیبول &&Other) = ړنګول;

  ~Z3_ترتیبول() {
    إذا (ترتیبول_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(ترتیبول_2));
  }

  فراغ Profile(llvm::Folding_سېټ_غوټه_ID &ID) ثابت override {
    ID.جمع_کړه_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(ترتیبول_2)));
  }

  منطقي is_Bitvector_ترتیبول_Impl() ثابت override {
    أرجع (Z3_get_ترتیبول_ډول(Context.Context, ترتیبول_2) == Z3_BV_ترتیبول);
  }

  منطقي is_Float_ترتیبول_Impl() ثابت override {
    أرجع (Z3_get_ترتیبول_ډول(Context.Context, ترتیبول_2) == Z3_FLOATING_ټکی_ترتیبول);
  }

  منطقي is_Boolean_ترتیبول_Impl() ثابت override {
    أرجع (Z3_get_ترتیبول_ډول(Context.Context, ترتیبول_2) == Z3_BOOL_ترتیبول);
  }

  بینښه get_Bitvector_ترتیبول_اندازه_Impl() ثابت override {
    أرجع Z3_get_bv_ترتیبول_اندازه(Context.Context, ترتیبول_2);
  }

  بینښه get_Float_ترتیبول_اندازه_Impl() ثابت override {
    أرجع Z3_fpa_get_ebits(Context.Context, ترتیبول_2) +
           Z3_fpa_get_sbits(Context.Context, ترتیبول_2);
  }

  منطقي equal_منزل(SMTSort ثابت &Other) ثابت override {
    أرجع Z3_is_eq_ترتیبول(Context.Context, ترتیبول_2,
                         static_cast<ثابت Z3_ترتیبول &>(Other).ترتیبول_2);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ترتیبول_منزل_string(Context.Context, ترتیبول_2);
  }
}; // end class Z3Sort

ساكن ثابت Z3_ترتیبول &منزل_Z3_ترتیبول(ثابت SMTSort &S) {
  أرجع static_cast<ثابت Z3_ترتیبول &>(S);
}

صنف Z3_بیان : عام SMTExpr {
  ملګری صنف Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عام:
  Z3_بیان(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_بیان(ثابت Z3_بیان &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_بیان &چلوونکی=(ثابت Z3_بیان &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    أرجع *هذا;
  }

  Z3_بیان(Z3_بیان &&Other) = ړنګول;
  Z3_بیان &چلوونکی=(Z3_بیان &&Other) = ړنګول;

  ~Z3_بیان() {
    إذا (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  فراغ Profile(llvm::Folding_سېټ_غوټه_ID &ID) ثابت override {
    ID.جمع_کړه_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقي equal_منزل(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_ترتیبول(Context.Context, Z3_get_ترتیبول(Context.Context, AST),
                         Z3_get_ترتیبول(Context.Context,
                                     static_cast<ثابت Z3_بیان &>(Other).AST)) &&
           "AST's must have the same sort");
    أرجع Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_بیان &>(Other).AST);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_منزل_string(Context.Context, AST);
  }
}; // end class Z3Expr

ساكن ثابت Z3_بیان &منزل_Z3_بیان(ثابت SMTExpr &E) {
  أرجع static_cast<ثابت Z3_بیان &>(E);
}

صنف Z3Model {
  ملګری صنف Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عام:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = ړنګول;
  Z3Model(Z3Model &&Other) = ړنګول;
  Z3Model &چلوونکی=(Z3Model &Other) = ړنګول;
  Z3Model &چلوونکی=(Z3Model &&Other) = ړنګول;

  ~Z3Model() {
    إذا (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  فراغ print(raw_ostream &OS) ثابت {
    OS << Z3_model_منزل_string(Context.Context, Model);
  }

  LLVM_DUMP_لاره فراغ dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ساكن ثابت llvm::fltSemantics &getFloatSemantics(بینښه بټ_پلنوالی_2) {
  بدل (بټ_پلنوالی_2) {
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
         (llvm::APFloat::semantics_اندازه_دننه_بټونه(LHS) ==
          llvm::APFloat::semantics_اندازه_دننه_بټونه(RHS));
}

صنف Z3Solver : عام SMTSolver {
  ملګری صنف Z3ConstraintManager;

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
  std::سېټ<Z3_ترتیبول> CachedSorts;

  // Cache Exprs
  std::سېټ<Z3_بیان> CachedExprs;

عام:
  Z3Solver() = افتراضي;
  Z3Solver(ثابت Z3Solver &Other) = ړنګول;
  Z3Solver(Z3Solver &&Other) = ړنګول;
  Z3Solver &چلوونکی=(Z3Solver &Other) = ړنګول;
  Z3Solver &چلوونکی=(Z3Solver &&Other) = ړنګول;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  فراغ جمع_کړه_Constraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, منزل_Z3_بیان(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_ترتیبول_Ref(ثابت SMTSort &ترتیبول_2) {
    خپلکاری It = CachedSorts.ورننباسه(منزل_Z3_ترتیبول(ترتیبول_2));
    أرجع &(*It.لومړی_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_بیان_Ref(ثابت SMTExpr &Exp) {
    خپلکاری It = CachedExprs.ورننباسه(منزل_Z3_بیان(Exp));
    أرجع &(*It.لومړی_2);
  }

  SMTSortRef get_Bool_ترتیبول() override {
    أرجع new_ترتیبول_Ref(Z3_ترتیبول(Context, Z3_mk_bool_ترتیبول(Context.Context)));
  }

  SMTSortRef get_Bitvector_ترتیبول(بینښه بټ_پلنوالی_2) override {
    أرجع new_ترتیبول_Ref(
        Z3_ترتیبول(Context, Z3_mk_bv_ترتیبول(Context.Context, بټ_پلنوالی_2)));
  }

  SMTSortRef get_ترتیبول(ثابت SMTExprRef &Exp) override {
    أرجع new_ترتیبول_Ref(
        Z3_ترتیبول(Context, Z3_get_ترتیبول(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTSortRef get_Float16_ترتیبول() override {
    أرجع new_ترتیبول_Ref(Z3_ترتیبول(Context, Z3_mk_fpa_ترتیبول_16(Context.Context)));
  }

  SMTSortRef get_Float32_ترتیبول() override {
    أرجع new_ترتیبول_Ref(Z3_ترتیبول(Context, Z3_mk_fpa_ترتیبول_32(Context.Context)));
  }

  SMTSortRef get_Float64_ترتیبول() override {
    أرجع new_ترتیبول_Ref(Z3_ترتیبول(Context, Z3_mk_fpa_ترتیبول_64(Context.Context)));
  }

  SMTSortRef get_Float128_ترتیبول() override {
    أرجع new_ترتیبول_Ref(Z3_ترتیبول(Context, Z3_mk_fpa_ترتیبول_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvneg(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvnot(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_not(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvadd(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvsub(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvmul(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvsrem(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvurem(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvsdiv(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvudiv(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvshl(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvashr(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvlshr(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvxor(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvor(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                   منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvand(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvult(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvslt(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvugt(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvsgt(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvule(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvsle(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvuge(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_bvsge(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                    منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دلیلونه[2] = {منزل_Z3_بیان(*LHS).AST, منزل_Z3_بیان(*RHS).AST};
    أرجع new_بیان_Ref(Z3_بیان(Context, Z3_mk_and(Context.Context, 2, دلیلونه)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast دلیلونه[2] = {منزل_Z3_بیان(*LHS).AST, منزل_Z3_بیان(*RHS).AST};
    أرجع new_بیان_Ref(Z3_بیان(Context, Z3_mk_or(Context.Context, 2, دلیلونه)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_eq(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                 منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_neg(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_fpa_is_infinite(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_is_nan(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_نورمال(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_fpa_is_نورمال(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_fpa_is_zero(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(
        Z3_بیان(Context,
               Z3_mk_fpa_mul(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                             منزل_Z3_بیان(*LHS).AST, منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(
        Z3_بیان(Context,
               Z3_mk_fpa_div(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                             منزل_Z3_بیان(*LHS).AST, منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_rem(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                      منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(
        Z3_بیان(Context,
               Z3_mk_fpa_جمع_کړه(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                             منزل_Z3_بیان(*LHS).AST, منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(
        Z3_بیان(Context,
               Z3_mk_fpa_sub(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                             منزل_Z3_بیان(*LHS).AST, منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_lt(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_gt(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_leq(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                      منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_geq(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                      منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_fpa_eq(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_ite(Context.Context, منزل_Z3_بیان(*Cond).AST,
                                  منزل_Z3_بیان(*T).AST, منزل_Z3_بیان(*F).AST)));
  }

  SMTExprRef mkBVSignExt(بینښه i, ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_sign_ext(Context.Context, i, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(بینښه i, ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_zero_ext(Context.Context, i, منزل_Z3_بیان(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(بینښه لوړ_2, بینښه ټیټ_2,
                         ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(Context, Z3_mk_ایستل(Context.Context, لوړ_2, ټیټ_2,
                                                    منزل_Z3_بیان(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_ډکېدنه(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvadd_no_ډکېدنه(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                         منزل_Z3_بیان(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                          منزل_Z3_بیان(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_ډکېدنه(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvsub_no_ډکېدنه(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                         منزل_Z3_بیان(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقي isSigned) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                          منزل_Z3_بیان(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_ډکېدنه(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvsdiv_no_ډکېدنه(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                          منزل_Z3_بیان(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_ډکېدنه(ثابت SMTExprRef &Exp) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvneg_no_ډکېدنه(Context.Context, منزل_Z3_بیان(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_ډکېدنه(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvmul_no_ډکېدنه(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                         منزل_Z3_بیان(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                          منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_concat(Context.Context, منزل_Z3_بیان(*LHS).AST,
                                     منزل_Z3_بیان(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(Z3_بیان(
        Context,
        Z3_mk_fpa_منزل_fp_float(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                              منزل_Z3_بیان(*From).AST, منزل_Z3_ترتیبول(*منزل).ترتیبول_2)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(Z3_بیان(
        Context,
        Z3_mk_fpa_منزل_fp_signed(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                               منزل_Z3_بیان(*From).AST, منزل_Z3_ترتیبول(*منزل).ترتیبول_2)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &منزل) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(Z3_بیان(
        Context,
        Z3_mk_fpa_منزل_fp_unsigned(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                                 منزل_Z3_بیان(*From).AST, منزل_Z3_ترتیبول(*منزل).ترتیبول_2)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, بینښه منزل_پلنوالی) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_fpa_منزل_sbv(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                                  منزل_Z3_بیان(*From).AST, منزل_پلنوالی)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, بینښه منزل_پلنوالی) override {
    SMTExprRef Rounding_ډېرپېښ = get_Float_Rounding_ډېرپېښ();
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_fpa_منزل_ubv(Context.Context, منزل_Z3_بیان(*Rounding_ډېرپېښ).AST,
                                  منزل_Z3_بیان(*From).AST, منزل_پلنوالی)));
  }

  SMTExprRef mkBoolean(ثابت منطقي b) override {
    أرجع new_بیان_Ref(Z3_بیان(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, بینښه بټ_پلنوالی_2) override {
    ثابت Z3_ترتیبول_2 Z3_ترتیبول = منزل_Z3_ترتیبول(*get_Bitvector_ترتیبول(بټ_پلنوالی_2)).ترتیبول_2;

    // Slow path, when 64 bits are not enough.
    إذا (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> بفر;
      Int.منزل_String(بفر, 10);
      أرجع new_بیان_Ref(Z3_بیان(
          Context, Z3_mk_numeral(Context.Context, بفر.c_str(), Z3_ترتیبول)));
    }

    ثابت int64_t بټ_Repr_As_Signed = Int.get_Ext_ارزښت();
    ثابت uint64_t بټ_Repr_As_Unsigned =
        reinterpret_cast<ثابت uint64_t &>(بټ_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, بټ_Repr_As_Signed, Z3_ترتیبول)
            : Z3_mk_unsigned_int64(Context.Context, بټ_Repr_As_Unsigned, Z3_ترتیبول);
    أرجع new_بیان_Ref(Z3_بیان(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef ترتیبول_2 =
        get_Float_ترتیبول(llvm::APFloat::semantics_اندازه_دننه_بټونه(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_منزل_APInt(), خطأ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_بټ_پلنوالی());
    أرجع new_بیان_Ref(Z3_بیان(
        Context, Z3_mk_fpa_منزل_fp_bv(Context.Context, منزل_Z3_بیان(*Z3Int).AST,
                                    منزل_Z3_ترتیبول(*ترتیبول_2).ترتیبول_2)));
  }

  SMTExprRef mk_نښه(ثابت حرف *Name, SMTSortRef ترتیبول_2) override {
    أرجع new_بیان_Ref(
        Z3_بیان(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_نښه(Context.Context, Name),
                                    منزل_Z3_ترتیبول(*ترتیبول_2).ترتیبول_2)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, بینښه بټ_پلنوالی_2,
                            منطقي isUnsigned) override {
    أرجع llvm::APSInt(
        llvm::APInt(بټ_پلنوالی_2,
                    Z3_get_numeral_string(Context.Context, منزل_Z3_بیان(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقي getBoolean(ثابت SMTExprRef &Exp) override {
    أرجع Z3_get_bool_ارزښت(Context.Context, منزل_Z3_بیان(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_ډېرپېښ() override {
    // TODO: Don't assume nearest ties to even rounding mode
    أرجع new_بیان_Ref(Z3_بیان(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقي منزل_APFloat(ثابت SMTSortRef &ترتیبول_2, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقي useSemantics) {
    أكد(ترتیبول_2->is_Float_ترتیبول() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(ترتیبول_2->get_Float_ترتیبول_اندازه(), ریښتیا);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(ترتیبول_2->get_Float_ترتیبول_اندازه());
    SMTSortRef BVSort = get_Bitvector_ترتیبول(ترتیبول_2->get_Float_ترتیبول_اندازه());
    إذا (!منزل_APSInt(BVSort, AST, Int, ریښتیا)) {
      أرجع خطأ;
    }

    إذا (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      أكد(خطأ && "Floating-point types don't match!");
      أرجع خطأ;
    }

    Float = llvm::APFloat(Semantics, Int);
    أرجع ریښتیا;
  }

  منطقي منزل_APSInt(ثابت SMTSortRef &ترتیبول_2, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقي useSemantics) {
    إذا (ترتیبول_2->is_Bitvector_ترتیبول()) {
      إذا (useSemantics && Int.get_بټ_پلنوالی() != ترتیبول_2->get_Bitvector_ترتیبول_اندازه()) {
        أكد(خطأ && "Bitvector types don't match!");
        أرجع خطأ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_بټ_پلنوالی(), Int.isUnsigned());
      أرجع ریښتیا;
    }

    إذا (ترتیبول_2->is_Boolean_ترتیبول()) {
      إذا (useSemantics && Int.get_بټ_پلنوالی() < 1) {
        أكد(خطأ && "Boolean type doesn't match!");
        أرجع خطأ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_بټ_پلنوالی(), getBoolean(AST)),
                         Int.isUnsigned());
      أرجع ریښتیا;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_منزل_app(Context.Context, منزل_Z3_بیان(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef ټاکل_2 = new_بیان_Ref(
        Z3_بیان(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتیبول_2 = get_ترتیبول(ټاکل_2);
    أرجع منزل_APSInt(ترتیبول_2, ټاکل_2, Int, ریښتیا);
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_منزل_app(Context.Context, منزل_Z3_بیان(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef ټاکل_2 = new_بیان_Ref(
        Z3_بیان(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef ترتیبول_2 = get_ترتیبول(ټاکل_2);
    أرجع منزل_APFloat(ترتیبول_2, ټاکل_2, Float, ریښتیا);
  }

  std::optional<منطقي> check() ثابت override {
    Z3_solver_سېټ_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    إذا (res == Z3_L_TRUE)
      أرجع ریښتیا;

    إذا (res == Z3_L_FALSE)
      أرجع خطأ;

    أرجع std::nullopt;
  }

  فراغ ټېل() override { أرجع Z3_solver_ټېل(Context.Context, Solver); }

  فراغ راوباسه(بینښه عدد_حالتونه = 1) override {
    أكد(Z3_solver_get_عدد_scopes(Context.Context, Solver) >= عدد_حالتونه);
    أرجع Z3_solver_راوباسه(Context.Context, Solver, عدد_حالتونه);
  }

  منطقي isFPSupported() override { أرجع ریښتیا; }

  /// Reset the solver and remove all constraints.
  فراغ reset() override { Z3_solver_reset(Context.Context, Solver); }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_منزل_string(Context.Context, Solver);
  }

  فراغ سېټ_Unsigned_Param(StringRef کلی, بینښه ارزښت) override {
    Z3_نښه Sym = Z3_mk_string_نښه(Context.Context, کلی.str().c_str());
    Z3_params_سېټ_uint(Context.Context, Params, Sym, ارزښت);
  }

  فراغ سېټ_Bool_Param(StringRef کلی, منطقي ارزښت) override {
    Z3_نښه Sym = Z3_mk_string_نښه(Context.Context, کلی.str().c_str());
    Z3_params_سېټ_bool(Context.Context, Params, Sym, ارزښت);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

صنف Z3Statistics final : عام SMTSolverStatistics {
عام:
  مزدوج getDouble(StringRef کلی) ثابت override {
    خپلکاری It = Double_ارزښتونه.پیدا_کړه(کلی.str());
    أكد(It != Double_ارزښتونه.پای_ټکی());
    أرجع It->دوهم_2;
  };
  بینښه getUnsigned(StringRef کلی) ثابت override {
    خپلکاری It = Unsigned_ارزښتونه.پیدا_کړه(کلی.str());
    إذا (It == Unsigned_ارزښتونه.پای_ټکی())
      أرجع 0;
    أرجع It->دوهم_2;
  };

  فراغ print(raw_ostream &OS) ثابت override {
    لكل (ثابت خپلکاری &E : Unsigned_ارزښتونه)
      OS << E.لومړی_2() << ": " << E.دوهم_2 << '\n';
    لكل (ثابت خپلکاری &E : Double_ارزښتونه) {
      write_double(OS << E.لومړی_2() << ": ", E.دوهم_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

خاص:
  ملګری صنف Z3Solver;
  StringMap<بینښه> Unsigned_ارزښتونه;
  StringMap<مزدوج> Double_ارزښتونه;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  خپلکاری ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics پایله;

  بینښه عدد_کلیګانې = Z3_stats_اندازه(C, S);
  لكل (بینښه Idx = 0; Idx < عدد_کلیګانې; ++Idx) {
    ثابت حرف *کلی = Z3_stats_get_کلی(C, S, Idx);
    إذا (Z3_stats_is_uint(C, S, Idx)) {
      خپلکاری ارزښت = Z3_stats_get_uint_ارزښت(C, S, Idx);
      پایله.Unsigned_ارزښتونه.try_emplace(کلی, ارزښت);
    } وإلا {
      أكد(Z3_stats_is_double(C, S, Idx));
      خپلکاری ارزښت = Z3_stats_get_double_ارزښت(C, S, Idx);
      پایله.Double_ارزښتونه.try_emplace(کلی, ارزښت);
    }
  }
  أرجع std::make_unique<Z3Statistics>(std::move(پایله));
}

} // end anonymous namespace

#د_که_پای

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#إذا LLVM_WITH_Z3
  أرجع std::make_unique<Z3Solver>();
#وإلا
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           خطأ);
  أرجع تش_اشاره;
#د_که_پای
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_لاره فراغ SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_لاره فراغ SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_لاره فراغ SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#د_که_پای
