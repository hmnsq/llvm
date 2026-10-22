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

استفاده نطاق llvm;

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
  Z3Config(ثابت Z3Config &) = حذف;
  Z3Config(Z3Config &&) = افتراضي;
  Z3Config &عملگر=(Z3Config &) = حذف;
  Z3Config &عملگر=(Z3Config &&) = افتراضي;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
فراغ Z3ErrorHandler(Z3_context Context, Z3_error_کد Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_پیام(Context, Error)));
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
    Z3_مجموعه_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ثابت Z3Context &) = حذف;
  Z3Context(Z3Context &&) = افتراضي;
  Z3Context &عملگر=(Z3Context &) = حذف;
  Z3Context &عملگر=(Z3Context &&) = افتراضي;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = اشاره_تهی;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
صنف Z3_مرتب_سازی : عام SMTSort {
  دوست صنف Z3Solver;

  Z3Context &Context;

  Z3_مرتب_سازی_2 مرتب_سازی_2;

عام:
  /// Default constructor, mainly used by make_shared
  Z3_مرتب_سازی(Z3Context &C, Z3_مرتب_سازی_2 ZS) : Context(C), مرتب_سازی_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(مرتب_سازی_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_مرتب_سازی(ثابت Z3_مرتب_سازی &Other) : Context(Other.Context), مرتب_سازی_2(Other.مرتب_سازی_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(مرتب_سازی_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_مرتب_سازی &عملگر=(ثابت Z3_مرتب_سازی &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.مرتب_سازی_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(مرتب_سازی_2));
    مرتب_سازی_2 = Other.مرتب_سازی_2;
    أرجع *هذا;
  }

  Z3_مرتب_سازی(Z3_مرتب_سازی &&Other) = حذف;
  Z3_مرتب_سازی &عملگر=(Z3_مرتب_سازی &&Other) = حذف;

  ~Z3_مرتب_سازی() {
    إذا (مرتب_سازی_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(مرتب_سازی_2));
  }

  فراغ Profile(llvm::Folding_مجموعه_گره_ID &ID) ثابت override {
    ID.جمع_کن_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(مرتب_سازی_2)));
  }

  منطقي is_Bitvector_مرتب_سازی_Impl() ثابت override {
    أرجع (Z3_get_مرتب_سازی_نوع(Context.Context, مرتب_سازی_2) == Z3_BV_مرتب_سازی);
  }

  منطقي is_Float_مرتب_سازی_Impl() ثابت override {
    أرجع (Z3_get_مرتب_سازی_نوع(Context.Context, مرتب_سازی_2) == Z3_FLOATING_نقطه_مرتب_سازی);
  }

  منطقي is_Boolean_مرتب_سازی_Impl() ثابت override {
    أرجع (Z3_get_مرتب_سازی_نوع(Context.Context, مرتب_سازی_2) == Z3_BOOL_مرتب_سازی);
  }

  بیعلامت get_Bitvector_مرتب_سازی_اندازه_Impl() ثابت override {
    أرجع Z3_get_bv_مرتب_سازی_اندازه(Context.Context, مرتب_سازی_2);
  }

  بیعلامت get_Float_مرتب_سازی_اندازه_Impl() ثابت override {
    أرجع Z3_fpa_get_ebits(Context.Context, مرتب_سازی_2) +
           Z3_fpa_get_sbits(Context.Context, مرتب_سازی_2);
  }

  منطقي equal_مقصد(SMTSort ثابت &Other) ثابت override {
    أرجع Z3_is_eq_مرتب_سازی(Context.Context, مرتب_سازی_2,
                         static_cast<ثابت Z3_مرتب_سازی &>(Other).مرتب_سازی_2);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_مرتب_سازی_مقصد_string(Context.Context, مرتب_سازی_2);
  }
}; // end class Z3Sort

ساكن ثابت Z3_مرتب_سازی &مقصد_Z3_مرتب_سازی(ثابت SMTSort &S) {
  أرجع static_cast<ثابت Z3_مرتب_سازی &>(S);
}

صنف Z3_عبارت : عام SMTExpr {
  دوست صنف Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

عام:
  Z3_عبارت(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_عبارت(ثابت Z3_عبارت &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_عبارت &عملگر=(ثابت Z3_عبارت &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    أرجع *هذا;
  }

  Z3_عبارت(Z3_عبارت &&Other) = حذف;
  Z3_عبارت &عملگر=(Z3_عبارت &&Other) = حذف;

  ~Z3_عبارت() {
    إذا (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  فراغ Profile(llvm::Folding_مجموعه_گره_ID &ID) ثابت override {
    ID.جمع_کن_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  منطقي equal_مقصد(SMTExpr ثابت &Other) ثابت override {
    أكد(Z3_is_eq_مرتب_سازی(Context.Context, Z3_get_مرتب_سازی(Context.Context, AST),
                         Z3_get_مرتب_سازی(Context.Context,
                                     static_cast<ثابت Z3_عبارت &>(Other).AST)) &&
           "AST's must have the same sort");
    أرجع Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ثابت Z3_عبارت &>(Other).AST);
  }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_ast_مقصد_string(Context.Context, AST);
  }
}; // end class Z3Expr

ساكن ثابت Z3_عبارت &مقصد_Z3_عبارت(ثابت SMTExpr &E) {
  أرجع static_cast<ثابت Z3_عبارت &>(E);
}

صنف Z3Model {
  دوست صنف Z3Solver;

  Z3Context &Context;

  Z3_model Model;

عام:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ثابت Z3Model &Other) = حذف;
  Z3Model(Z3Model &&Other) = حذف;
  Z3Model &عملگر=(Z3Model &Other) = حذف;
  Z3Model &عملگر=(Z3Model &&Other) = حذف;

  ~Z3Model() {
    إذا (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  فراغ print(raw_ostream &OS) ثابت {
    OS << Z3_model_مقصد_string(Context.Context, Model);
  }

  LLVM_DUMP_روش فراغ dump() ثابت { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
ساكن ثابت llvm::fltSemantics &getFloatSemantics(بیعلامت بیت_عرض_2) {
  بدل (بیت_عرض_2) {
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
         (llvm::APFloat::semantics_اندازه_درون_بیتها(LHS) ==
          llvm::APFloat::semantics_اندازه_درون_بیتها(RHS));
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
  std::مجموعه<Z3_مرتب_سازی> CachedSorts;

  // Cache Exprs
  std::مجموعه<Z3_عبارت> CachedExprs;

عام:
  Z3Solver() = افتراضي;
  Z3Solver(ثابت Z3Solver &Other) = حذف;
  Z3Solver(Z3Solver &&Other) = حذف;
  Z3Solver &عملگر=(Z3Solver &Other) = حذف;
  Z3Solver &عملگر=(Z3Solver &&Other) = حذف;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  فراغ جمع_کن_Constraint(ثابت SMTExprRef &Exp) ثابت override {
    Z3_solver_assert(Context.Context, Solver, مقصد_Z3_عبارت(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_مرتب_سازی_Ref(ثابت SMTSort &مرتب_سازی_2) {
    خودکار It = CachedSorts.درج(مقصد_Z3_مرتب_سازی(مرتب_سازی_2));
    أرجع &(*It.اولی_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_عبارت_Ref(ثابت SMTExpr &Exp) {
    خودکار It = CachedExprs.درج(مقصد_Z3_عبارت(Exp));
    أرجع &(*It.اولی_2);
  }

  SMTSortRef get_Bool_مرتب_سازی() override {
    أرجع new_مرتب_سازی_Ref(Z3_مرتب_سازی(Context, Z3_mk_bool_مرتب_سازی(Context.Context)));
  }

  SMTSortRef get_Bitvector_مرتب_سازی(بیعلامت بیت_عرض_2) override {
    أرجع new_مرتب_سازی_Ref(
        Z3_مرتب_سازی(Context, Z3_mk_bv_مرتب_سازی(Context.Context, بیت_عرض_2)));
  }

  SMTSortRef get_مرتب_سازی(ثابت SMTExprRef &Exp) override {
    أرجع new_مرتب_سازی_Ref(
        Z3_مرتب_سازی(Context, Z3_get_مرتب_سازی(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTSortRef get_Float16_مرتب_سازی() override {
    أرجع new_مرتب_سازی_Ref(Z3_مرتب_سازی(Context, Z3_mk_fpa_مرتب_سازی_16(Context.Context)));
  }

  SMTSortRef get_Float32_مرتب_سازی() override {
    أرجع new_مرتب_سازی_Ref(Z3_مرتب_سازی(Context, Z3_mk_fpa_مرتب_سازی_32(Context.Context)));
  }

  SMTSortRef get_Float64_مرتب_سازی() override {
    أرجع new_مرتب_سازی_Ref(Z3_مرتب_سازی(Context, Z3_mk_fpa_مرتب_سازی_64(Context.Context)));
  }

  SMTSortRef get_Float128_مرتب_سازی() override {
    أرجع new_مرتب_سازی_Ref(Z3_مرتب_سازی(Context, Z3_mk_fpa_مرتب_سازی_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvneg(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvnot(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkNot(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_not(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvadd(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsub(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvmul(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsrem(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvurem(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsdiv(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvudiv(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvshl(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvashr(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvlshr(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvxor(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvor(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                   مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvand(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvult(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvslt(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvugt(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsgt(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvule(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsle(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvuge(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_bvsge(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                    مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkAnd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast آرگومانها[2] = {مقصد_Z3_عبارت(*LHS).AST, مقصد_Z3_عبارت(*RHS).AST};
    أرجع new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_and(Context.Context, 2, آرگومانها)));
  }

  SMTExprRef mkOr(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    Z3_ast آرگومانها[2] = {مقصد_Z3_عبارت(*LHS).AST, مقصد_Z3_عبارت(*RHS).AST};
    أرجع new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_or(Context.Context, 2, آرگومانها)));
  }

  SMTExprRef mkEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_eq(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                 مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_neg(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_is_infinite(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_is_nan(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_نرمال(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_is_نرمال(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_is_zero(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_mul(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                             مقصد_Z3_عبارت(*LHS).AST, مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_div(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                             مقصد_Z3_عبارت(*LHS).AST, مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_rem(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                      مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_جمع_کن(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                             مقصد_Z3_عبارت(*LHS).AST, مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_mk_fpa_sub(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                             مقصد_Z3_عبارت(*LHS).AST, مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_lt(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_gt(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_leq(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                      مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_geq(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                      مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_fpa_eq(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkIte(ثابت SMTExprRef &Cond, ثابت SMTExprRef &T,
                   ثابت SMTExprRef &F) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_ite(Context.Context, مقصد_Z3_عبارت(*Cond).AST,
                                  مقصد_Z3_عبارت(*T).AST, مقصد_Z3_عبارت(*F).AST)));
  }

  SMTExprRef mkBVSignExt(بیعلامت i, ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_sign_ext(Context.Context, i, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(بیعلامت i, ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_zero_ext(Context.Context, i, مقصد_Z3_عبارت(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(بیعلامت بالا_2, بیعلامت کم_2,
                         ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_استخراج(Context.Context, بالا_2, کم_2,
                                                    مقصد_Z3_عبارت(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_سرریز(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvadd_no_سرریز(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                         مقصد_Z3_عبارت(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                          مقصد_Z3_عبارت(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_سرریز(ثابت SMTExprRef &LHS,
                               ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvsub_no_سرریز(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                         مقصد_Z3_عبارت(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                                منطقي isSigned) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                          مقصد_Z3_عبارت(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_سرریز(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvsdiv_no_سرریز(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                          مقصد_Z3_عبارت(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_سرریز(ثابت SMTExprRef &Exp) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvneg_no_سرریز(Context.Context, مقصد_Z3_عبارت(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_سرریز(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS,
                               منطقي isSigned) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvmul_no_سرریز(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                         مقصد_Z3_عبارت(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ثابت SMTExprRef &LHS,
                                ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                          مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ثابت SMTExprRef &LHS, ثابت SMTExprRef &RHS) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_concat(Context.Context, مقصد_Z3_عبارت(*LHS).AST,
                                     مقصد_Z3_عبارت(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &مقصد) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context,
        Z3_mk_fpa_مقصد_fp_float(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                              مقصد_Z3_عبارت(*From).AST, مقصد_Z3_مرتب_سازی(*مقصد).مرتب_سازی_2)));
  }

  SMTExprRef mkSBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &مقصد) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context,
        Z3_mk_fpa_مقصد_fp_signed(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                               مقصد_Z3_عبارت(*From).AST, مقصد_Z3_مرتب_سازی(*مقصد).مرتب_سازی_2)));
  }

  SMTExprRef mkUBVtoFP(ثابت SMTExprRef &From, ثابت SMTSortRef &مقصد) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context,
        Z3_mk_fpa_مقصد_fp_unsigned(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                                 مقصد_Z3_عبارت(*From).AST, مقصد_Z3_مرتب_سازی(*مقصد).مرتب_سازی_2)));
  }

  SMTExprRef mkFPtoSBV(ثابت SMTExprRef &From, بیعلامت مقصد_عرض) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_مقصد_sbv(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                                  مقصد_Z3_عبارت(*From).AST, مقصد_عرض)));
  }

  SMTExprRef mkFPtoUBV(ثابت SMTExprRef &From, بیعلامت مقصد_عرض) override {
    SMTExprRef Rounding_نما = get_Float_Rounding_نما();
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_مقصد_ubv(Context.Context, مقصد_Z3_عبارت(*Rounding_نما).AST,
                                  مقصد_Z3_عبارت(*From).AST, مقصد_عرض)));
  }

  SMTExprRef mkBoolean(ثابت منطقي b) override {
    أرجع new_عبارت_Ref(Z3_عبارت(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ثابت llvm::APSInt Int, بیعلامت بیت_عرض_2) override {
    ثابت Z3_مرتب_سازی_2 Z3_مرتب_سازی = مقصد_Z3_مرتب_سازی(*get_Bitvector_مرتب_سازی(بیت_عرض_2)).مرتب_سازی_2;

    // Slow path, when 64 bits are not enough.
    إذا (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> میانگیر;
      Int.مقصد_String(میانگیر, 10);
      أرجع new_عبارت_Ref(Z3_عبارت(
          Context, Z3_mk_numeral(Context.Context, میانگیر.c_str(), Z3_مرتب_سازی)));
    }

    ثابت int64_t بیت_Repr_As_Signed = Int.get_Ext_مقدار();
    ثابت uint64_t بیت_Repr_As_Unsigned =
        reinterpret_cast<ثابت uint64_t &>(بیت_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, بیت_Repr_As_Signed, Z3_مرتب_سازی)
            : Z3_mk_unsigned_int64(Context.Context, بیت_Repr_As_Unsigned, Z3_مرتب_سازی);
    أرجع new_عبارت_Ref(Z3_عبارت(Context, Literal));
  }

  SMTExprRef mkFloat(ثابت llvm::APFloat Float) override {
    SMTSortRef مرتب_سازی_2 =
        get_Float_مرتب_سازی(llvm::APFloat::semantics_اندازه_درون_بیتها(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_مقصد_APInt(), خطأ);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_بیت_عرض());
    أرجع new_عبارت_Ref(Z3_عبارت(
        Context, Z3_mk_fpa_مقصد_fp_bv(Context.Context, مقصد_Z3_عبارت(*Z3Int).AST,
                                    مقصد_Z3_مرتب_سازی(*مرتب_سازی_2).مرتب_سازی_2)));
  }

  SMTExprRef mk_نماد(ثابت حرف *Name, SMTSortRef مرتب_سازی_2) override {
    أرجع new_عبارت_Ref(
        Z3_عبارت(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_نماد(Context.Context, Name),
                                    مقصد_Z3_مرتب_سازی(*مرتب_سازی_2).مرتب_سازی_2)));
  }

  llvm::APSInt getBitvector(ثابت SMTExprRef &Exp, بیعلامت بیت_عرض_2,
                            منطقي isUnsigned) override {
    أرجع llvm::APSInt(
        llvm::APInt(بیت_عرض_2,
                    Z3_get_numeral_string(Context.Context, مقصد_Z3_عبارت(*Exp).AST),
                    10),
        isUnsigned);
  }

  منطقي getBoolean(ثابت SMTExprRef &Exp) override {
    أرجع Z3_get_bool_مقدار(Context.Context, مقصد_Z3_عبارت(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_نما() override {
    // TODO: Don't assume nearest ties to even rounding mode
    أرجع new_عبارت_Ref(Z3_عبارت(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  منطقي مقصد_APFloat(ثابت SMTSortRef &مرتب_سازی_2, ثابت SMTExprRef &AST,
                 llvm::APFloat &Float, منطقي useSemantics) {
    أكد(مرتب_سازی_2->is_Float_مرتب_سازی() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(مرتب_سازی_2->get_Float_مرتب_سازی_اندازه(), درست);
    ثابت llvm::fltSemantics &Semantics =
        getFloatSemantics(مرتب_سازی_2->get_Float_مرتب_سازی_اندازه());
    SMTSortRef BVSort = get_Bitvector_مرتب_سازی(مرتب_سازی_2->get_Float_مرتب_سازی_اندازه());
    إذا (!مقصد_APSInt(BVSort, AST, Int, درست)) {
      أرجع خطأ;
    }

    إذا (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      أكد(خطأ && "Floating-point types don't match!");
      أرجع خطأ;
    }

    Float = llvm::APFloat(Semantics, Int);
    أرجع درست;
  }

  منطقي مقصد_APSInt(ثابت SMTSortRef &مرتب_سازی_2, ثابت SMTExprRef &AST,
                llvm::APSInt &Int, منطقي useSemantics) {
    إذا (مرتب_سازی_2->is_Bitvector_مرتب_سازی()) {
      إذا (useSemantics && Int.get_بیت_عرض() != مرتب_سازی_2->get_Bitvector_مرتب_سازی_اندازه()) {
        أكد(خطأ && "Bitvector types don't match!");
        أرجع خطأ;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_بیت_عرض(), Int.isUnsigned());
      أرجع درست;
    }

    إذا (مرتب_سازی_2->is_Boolean_مرتب_سازی()) {
      إذا (useSemantics && Int.get_بیت_عرض() < 1) {
        أكد(خطأ && "Boolean type doesn't match!");
        أرجع خطأ;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_بیت_عرض(), getBoolean(AST)),
                         Int.isUnsigned());
      أرجع درست;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_مقصد_app(Context.Context, مقصد_Z3_عبارت(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef تخصیص_بده_2 = new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef مرتب_سازی_2 = get_مرتب_سازی(تخصیص_بده_2);
    أرجع مقصد_APSInt(مرتب_سازی_2, تخصیص_بده_2, Int, درست);
  }

  منطقي getInterpretation(ثابت SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_مقصد_app(Context.Context, مقصد_Z3_عبارت(*Exp).AST));
    إذا (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      أرجع خطأ;

    SMTExprRef تخصیص_بده_2 = new_عبارت_Ref(
        Z3_عبارت(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef مرتب_سازی_2 = get_مرتب_سازی(تخصیص_بده_2);
    أرجع مقصد_APFloat(مرتب_سازی_2, تخصیص_بده_2, Float, درست);
  }

  std::optional<منطقي> check() ثابت override {
    Z3_solver_مجموعه_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    إذا (res == Z3_L_TRUE)
      أرجع درست;

    إذا (res == Z3_L_FALSE)
      أرجع خطأ;

    أرجع std::nullopt;
  }

  فراغ افزودن() override { أرجع Z3_solver_افزودن(Context.Context, Solver); }

  فراغ برداشت(بیعلامت رقم_حالتها = 1) override {
    أكد(Z3_solver_get_رقم_scopes(Context.Context, Solver) >= رقم_حالتها);
    أرجع Z3_solver_برداشت(Context.Context, Solver, رقم_حالتها);
  }

  منطقي isFPSupported() override { أرجع درست; }

  /// Reset the solver and remove all constraints.
  فراغ reset() override { Z3_solver_reset(Context.Context, Solver); }

  فراغ print(raw_ostream &OS) ثابت override {
    OS << Z3_solver_مقصد_string(Context.Context, Solver);
  }

  فراغ مجموعه_Unsigned_Param(StringRef کلید, بیعلامت مقدار) override {
    Z3_نماد Sym = Z3_mk_string_نماد(Context.Context, کلید.str().c_str());
    Z3_params_مجموعه_uint(Context.Context, Params, Sym, مقدار);
  }

  فراغ مجموعه_Bool_Param(StringRef کلید, منطقي مقدار) override {
    Z3_نماد Sym = Z3_mk_string_نماد(Context.Context, کلید.str().c_str());
    Z3_params_مجموعه_bool(Context.Context, Params, Sym, مقدار);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ثابت override;
}; // end class Z3Solver

صنف Z3Statistics final : عام SMTSolverStatistics {
عام:
  مزدوج getDouble(StringRef کلید) ثابت override {
    خودکار It = Double_مقادیر.پیدا_کن(کلید.str());
    أكد(It != Double_مقادیر.انتها());
    أرجع It->دومی_2;
  };
  بیعلامت getUnsigned(StringRef کلید) ثابت override {
    خودکار It = Unsigned_مقادیر.پیدا_کن(کلید.str());
    إذا (It == Unsigned_مقادیر.انتها())
      أرجع 0;
    أرجع It->دومی_2;
  };

  فراغ print(raw_ostream &OS) ثابت override {
    لكل (ثابت خودکار &E : Unsigned_مقادیر)
      OS << E.اولی_2() << ": " << E.دومی_2 << '\n';
    لكل (ثابت خودکار &E : Double_مقادیر) {
      write_double(OS << E.اولی_2() << ": ", E.دومی_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

خاص:
  دوست صنف Z3Solver;
  StringMap<بیعلامت> Unsigned_مقادیر;
  StringMap<مزدوج> Double_مقادیر;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ثابت {
  خودکار ثابت &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics نتیجه;

  بیعلامت رقم_کلیدها = Z3_stats_اندازه(C, S);
  لكل (بیعلامت Idx = 0; Idx < رقم_کلیدها; ++Idx) {
    ثابت حرف *کلید = Z3_stats_get_کلید(C, S, Idx);
    إذا (Z3_stats_is_uint(C, S, Idx)) {
      خودکار مقدار = Z3_stats_get_uint_مقدار(C, S, Idx);
      نتیجه.Unsigned_مقادیر.try_emplace(کلید, مقدار);
    } وإلا {
      أكد(Z3_stats_is_double(C, S, Idx));
      خودکار مقدار = Z3_stats_get_double_مقدار(C, S, Idx);
      نتیجه.Double_مقادیر.try_emplace(کلید, مقدار);
    }
  }
  أرجع std::make_unique<Z3Statistics>(std::move(نتیجه));
}

} // end anonymous namespace

#پایان_اگر

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#إذا LLVM_WITH_Z3
  أرجع std::make_unique<Z3Solver>();
#وإلا
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           خطأ);
  أرجع اشاره_تهی;
#پایان_اگر
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ SMTSort::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_روش فراغ SMTExpr::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_روش فراغ SMTSolver::dump() ثابت { print(llvm::errs()); }
LLVM_DUMP_روش فراغ SMTSolverStatistics::dump() ثابت { print(llvm::errs()); }
#پایان_اگر
