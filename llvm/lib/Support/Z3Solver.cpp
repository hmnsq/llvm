//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/ScopeExit.h"
#समावेश "llvm/Config/config.h"
#समावेश "llvm/Support/NativeFormatting.h"
#समावेश "llvm/Support/SMTAPI.h"

using namespace llvm;

#अगर LLVM_WITH_Z3

#समावेश "llvm/ADT/SmallString.h"
#समावेश "llvm/ADT/StringMap.h"
#समावेश "llvm/ADT/Twine.h"

#समावेश <set>

#समावेश <z3.h>

namespace {

/// Configuration class for Z3
श्रेणी Z3Config {
  मित्र श्रेणी Z3Context;

  Z3_config Config = Z3_mk_config();

खुला:
  Z3Config() = शेष;
  Z3Config(स्थिर Z3Config &) = मिटाओ;
  Z3Config(Z3Config &&) = शेष;
  Z3Config &चालक=(Z3Config &) = मिटाओ;
  Z3Config &चालक=(Z3Config &&) = शेष;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
व्योम Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_संदेश(Context, Error)));
}

/// Wrapper for Z3 context
श्रेणी Z3Context {
खुला:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(स्थिर Z3Context &) = मिटाओ;
  Z3Context(Z3Context &&) = शेष;
  Z3Context &चालक=(Z3Context &) = मिटाओ;
  Z3Context &चालक=(Z3Context &&) = शेष;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
श्रेणी Z3Sort : खुला SMTSort {
  मित्र श्रेणी Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

खुला:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(स्थिर Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &चालक=(स्थिर Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    वापस *यह;
  }

  Z3Sort(Z3Sort &&Other) = मिटाओ;
  Z3Sort &चालक=(Z3Sort &&Other) = मिटाओ;

  ~Z3Sort() {
    अगर (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  व्योम Profile(llvm::Folding_Set_गाँठ_ID &ID) स्थिर override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  bool isBitvectorSortImpl() स्थिर override {
    वापस (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  bool isFloatSortImpl() स्थिर override {
    वापस (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  bool isBooleanSortImpl() स्थिर override {
    वापस (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  अचिन्हित get_Bitvector_Sort_आकार_Impl() स्थिर override {
    वापस Z3_get_bv_sort_आकार(Context.Context, Sort);
  }

  अचिन्हित get_Float_Sort_आकार_Impl() स्थिर override {
    वापस Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  bool equal_to(SMTSort स्थिर &Other) स्थिर override {
    वापस Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<स्थिर Z3Sort &>(Other).Sort);
  }

  व्योम print(raw_ostream &OS) स्थिर override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

अटल स्थिर Z3Sort &toZ3Sort(स्थिर SMTSort &S) {
  वापस static_cast<स्थिर Z3Sort &>(S);
}

श्रेणी Z3_व्यंजक : खुला SMTExpr {
  मित्र श्रेणी Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

खुला:
  Z3_व्यंजक(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_व्यंजक(स्थिर Z3_व्यंजक &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_व्यंजक &चालक=(स्थिर Z3_व्यंजक &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    वापस *यह;
  }

  Z3_व्यंजक(Z3_व्यंजक &&Other) = मिटाओ;
  Z3_व्यंजक &चालक=(Z3_व्यंजक &&Other) = मिटाओ;

  ~Z3_व्यंजक() {
    अगर (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  व्योम Profile(llvm::Folding_Set_गाँठ_ID &ID) स्थिर override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  bool equal_to(SMTExpr स्थिर &Other) स्थिर override {
    निश्चित(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<स्थिर Z3_व्यंजक &>(Other).AST)) &&
           "AST's must have the same sort");
    वापस Z3_is_eq_ast(Context.Context, AST,
                        static_cast<स्थिर Z3_व्यंजक &>(Other).AST);
  }

  व्योम print(raw_ostream &OS) स्थिर override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

अटल स्थिर Z3_व्यंजक &to_Z3_व्यंजक(स्थिर SMTExpr &E) {
  वापस static_cast<स्थिर Z3_व्यंजक &>(E);
}

श्रेणी Z3Model {
  मित्र श्रेणी Z3Solver;

  Z3Context &Context;

  Z3_model Model;

खुला:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(स्थिर Z3Model &Other) = मिटाओ;
  Z3Model(Z3Model &&Other) = मिटाओ;
  Z3Model &चालक=(Z3Model &Other) = मिटाओ;
  Z3Model &चालक=(Z3Model &&Other) = मिटाओ;

  ~Z3Model() {
    अगर (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  व्योम print(raw_ostream &OS) स्थिर {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD व्योम dump() स्थिर { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
अटल स्थिर llvm::fltSemantics &getFloatSemantics(अचिन्हित BitWidth) {
  चयन (BitWidth) {
  शेष:
    llvm_unreachable("Unsupported floating-point semantics!");
    अवरोध;
  हाल 16:
    वापस llvm::APFloat::IEEEhalf();
  हाल 32:
    वापस llvm::APFloat::IEEEsingle();
  हाल 64:
    वापस llvm::APFloat::IEEEdouble();
  हाल 128:
    वापस llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
अटल bool areEquivalent(स्थिर llvm::fltSemantics &LHS,
                          स्थिर llvm::fltSemantics &RHS) {
  वापस (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_आकार_In_Bits(LHS) ==
          llvm::APFloat::semantics_आकार_In_Bits(RHS));
}

श्रेणी Z3Solver : खुला SMTSolver {
  मित्र श्रेणी Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [यह] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    वापस S;
  }();

  Z3_params Params = [यह] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    वापस P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_व्यंजक> CachedExprs;

खुला:
  Z3Solver() = शेष;
  Z3Solver(स्थिर Z3Solver &Other) = मिटाओ;
  Z3Solver(Z3Solver &&Other) = मिटाओ;
  Z3Solver &चालक=(Z3Solver &Other) = मिटाओ;
  Z3Solver &चालक=(Z3Solver &&Other) = मिटाओ;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  व्योम addConstraint(स्थिर SMTExprRef &Exp) स्थिर override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_व्यंजक(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(स्थिर SMTSort &Sort) {
    स्वतः It = CachedSorts.जोड़ो(toZ3Sort(Sort));
    वापस &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_व्यंजक_Ref(स्थिर SMTExpr &Exp) {
    स्वतः It = CachedExprs.जोड़ो(to_Z3_व्यंजक(Exp));
    वापस &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    वापस newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(अचिन्हित BitWidth) override {
    वापस newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(स्थिर SMTExprRef &Exp) override {
    वापस newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    वापस newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    वापस newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    वापस newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    वापस newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvneg(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVNot(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvnot(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkNot(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_not(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvadd(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSub(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsub(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVMul(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvmul(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsrem(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVURem(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvurem(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvudiv(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVShl(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvshl(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvashr(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvlshr(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVXor(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvxor(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVOr(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvor(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                   to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvand(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvult(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvslt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvugt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsgt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUle(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvule(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSle(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsle(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUge(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvuge(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSge(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsge(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkAnd(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST};
    वापस new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST};
    वापस new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_eq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                 to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPMul(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPRem(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                      to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPSub(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPLt(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPGt(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPLe(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                      to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPGe(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                      to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkIte(स्थिर SMTExprRef &Cond, स्थिर SMTExprRef &T,
                   स्थिर SMTExprRef &F) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_ite(Context.Context, to_Z3_व्यंजक(*Cond).AST,
                                  to_Z3_व्यंजक(*T).AST, to_Z3_व्यंजक(*F).AST)));
  }

  SMTExprRef mkBVSignExt(अचिन्हित i, स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(अचिन्हित i, स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(अचिन्हित ऊपरी_2, अचिन्हित निचला_2,
                         स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_extract(Context.Context, ऊपरी_2, निचला_2,
                                                    to_Z3_व्यंजक(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS,
                               bool isSigned) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                         to_Z3_व्यंजक(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(स्थिर SMTExprRef &LHS,
                                स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(स्थिर SMTExprRef &LHS,
                               स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                         to_Z3_व्यंजक(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS,
                                bool isSigned) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(स्थिर SMTExprRef &LHS,
                                स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(स्थिर SMTExprRef &Exp) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS,
                               bool isSigned) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                         to_Z3_व्यंजक(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(स्थिर SMTExprRef &LHS,
                                स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(स्थिर SMTExprRef &LHS, स्थिर SMTExprRef &RHS) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_concat(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(स्थिर SMTExprRef &From, स्थिर SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                              to_Z3_व्यंजक(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(स्थिर SMTExprRef &From, स्थिर SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                               to_Z3_व्यंजक(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(स्थिर SMTExprRef &From, स्थिर SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                                 to_Z3_व्यंजक(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(स्थिर SMTExprRef &From, अचिन्हित ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                                  to_Z3_व्यंजक(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(स्थिर SMTExprRef &From, अचिन्हित ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                                  to_Z3_व्यंजक(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(स्थिर bool b) override {
    वापस new_व्यंजक_Ref(Z3_व्यंजक(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(स्थिर llvm::APSInt Int, अचिन्हित BitWidth) override {
    स्थिर Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    अगर (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> बफर;
      Int.toString(बफर, 10);
      वापस new_व्यंजक_Ref(Z3_व्यंजक(
          Context, Z3_mk_numeral(Context.Context, बफर.c_str(), Z3Sort)));
    }

    स्थिर int64_t BitReprAsSigned = Int.get_Ext_मूल्य();
    स्थिर uint64_t BitReprAsUnsigned =
        reinterpret_cast<स्थिर uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    वापस new_व्यंजक_Ref(Z3_व्यंजक(Context, Literal));
  }

  SMTExprRef mkFloat(स्थिर llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semantics_आकार_In_Bits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), false);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    वापस new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_व्यंजक(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_प्रतीक(स्थिर अक्षर *Name, SMTSortRef Sort) override {
    वापस new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_प्रतीक(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(स्थिर SMTExprRef &Exp, अचिन्हित BitWidth,
                            bool isUnsigned) override {
    वापस llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_व्यंजक(*Exp).AST),
                    10),
        isUnsigned);
  }

  bool getBoolean(स्थिर SMTExprRef &Exp) override {
    वापस Z3_get_bool_मूल्य(Context.Context, to_Z3_व्यंजक(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    वापस new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  bool toAPFloat(स्थिर SMTSortRef &Sort, स्थिर SMTExprRef &AST,
                 llvm::APFloat &Float, bool useSemantics) {
    निश्चित(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->get_Float_Sort_आकार(), true);
    स्थिर llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->get_Float_Sort_आकार());
    SMTSortRef BVSort = getBitvectorSort(Sort->get_Float_Sort_आकार());
    अगर (!toAPSInt(BVSort, AST, Int, true)) {
      वापस false;
    }

    अगर (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      निश्चित(false && "Floating-point types don't match!");
      वापस false;
    }

    Float = llvm::APFloat(Semantics, Int);
    वापस true;
  }

  bool toAPSInt(स्थिर SMTSortRef &Sort, स्थिर SMTExprRef &AST,
                llvm::APSInt &Int, bool useSemantics) {
    अगर (Sort->isBitvectorSort()) {
      अगर (useSemantics && Int.getBitWidth() != Sort->get_Bitvector_Sort_आकार()) {
        निश्चित(false && "Bitvector types don't match!");
        वापस false;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      वापस true;
    }

    अगर (Sort->isBooleanSort()) {
      अगर (useSemantics && Int.getBitWidth() < 1) {
        निश्चित(false && "Boolean type doesn't match!");
        वापस false;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      वापस true;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  bool getInterpretation(स्थिर SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_व्यंजक(*Exp).AST));
    अगर (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      वापस false;

    SMTExprRef Assign = new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    वापस toAPSInt(Sort, Assign, Int, true);
  }

  bool getInterpretation(स्थिर SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_व्यंजक(*Exp).AST));
    अगर (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      वापस false;

    SMTExprRef Assign = new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    वापस toAPFloat(Sort, Assign, Float, true);
  }

  std::optional<bool> check() स्थिर override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    अगर (res == Z3_L_TRUE)
      वापस true;

    अगर (res == Z3_L_FALSE)
      वापस false;

    वापस std::nullopt;
  }

  व्योम धकेलो() override { वापस Z3_solver_धकेलो(Context.Context, Solver); }

  व्योम निकालो(अचिन्हित अंक_States = 1) override {
    निश्चित(Z3_solver_get_अंक_scopes(Context.Context, Solver) >= अंक_States);
    वापस Z3_solver_निकालो(Context.Context, Solver, अंक_States);
  }

  bool isFPSupported() override { वापस true; }

  /// Reset the solver and remove all constraints.
  व्योम reset() override { Z3_solver_reset(Context.Context, Solver); }

  व्योम print(raw_ostream &OS) स्थिर override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  व्योम setUnsignedParam(StringRef कुंजी, अचिन्हित मूल्य) override {
    Z3_प्रतीक Sym = Z3_mk_string_प्रतीक(Context.Context, कुंजी.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, मूल्य);
  }

  व्योम setBoolParam(StringRef कुंजी, bool मूल्य) override {
    Z3_प्रतीक Sym = Z3_mk_string_प्रतीक(Context.Context, कुंजी.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, मूल्य);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() स्थिर override;
}; // end class Z3Solver

श्रेणी Z3Statistics final : खुला SMTSolverStatistics {
खुला:
  द्विगुन getDouble(StringRef कुंजी) स्थिर override {
    स्वतः It = Double_मूल्यावली.find(कुंजी.str());
    निश्चित(It != Double_मूल्यावली.end());
    वापस It->second;
  };
  अचिन्हित getUnsigned(StringRef कुंजी) स्थिर override {
    स्वतः It = Unsigned_मूल्यावली.find(कुंजी.str());
    अगर (It == Unsigned_मूल्यावली.end())
      वापस 0;
    वापस It->second;
  };

  व्योम print(raw_ostream &OS) स्थिर override {
    क्रम (स्थिर स्वतः &E : Unsigned_मूल्यावली)
      OS << E.first() << ": " << E.second << '\n';
    क्रम (स्थिर स्वतः &E : Double_मूल्यावली) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

निजी:
  मित्र श्रेणी Z3Solver;
  StringMap<अचिन्हित> Unsigned_मूल्यावली;
  StringMap<द्विगुन> Double_मूल्यावली;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() स्थिर {
  स्वतः स्थिर &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics परिणाम;

  अचिन्हित अंक_कुंजियाँ = Z3_stats_आकार(C, S);
  क्रम (अचिन्हित Idx = 0; Idx < अंक_कुंजियाँ; ++Idx) {
    स्थिर अक्षर *कुंजी = Z3_stats_get_कुंजी(C, S, Idx);
    अगर (Z3_stats_is_uint(C, S, Idx)) {
      स्वतः मूल्य = Z3_stats_get_uint_मूल्य(C, S, Idx);
      परिणाम.Unsigned_मूल्यावली.try_emplace(कुंजी, मूल्य);
    } अन्यथा {
      निश्चित(Z3_stats_is_double(C, S, Idx));
      स्वतः मूल्य = Z3_stats_get_double_मूल्य(C, S, Idx);
      परिणाम.Double_मूल्यावली.try_emplace(कुंजी, मूल्य);
    }
  }
  वापस std::make_unique<Z3Statistics>(std::move(परिणाम));
}

} // end anonymous namespace

#पूर्ण_अगर

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#अगर LLVM_WITH_Z3
  वापस std::make_unique<Z3Solver>();
#अन्यथा
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           false);
  वापस nullptr;
#पूर्ण_अगर
}

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम SMTSort::dump() स्थिर { print(llvm::errs()); }
LLVM_DUMP_METHOD व्योम SMTExpr::dump() स्थिर { print(llvm::errs()); }
LLVM_DUMP_METHOD व्योम SMTSolver::dump() स्थिर { print(llvm::errs()); }
LLVM_DUMP_METHOD व्योम SMTSolverStatistics::dump() स्थिर { print(llvm::errs()); }
#पूर्ण_अगर
