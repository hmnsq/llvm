//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/ScopeExit.h"
#अन्तर्भाव "llvm/Config/config.h"
#अन्तर्भाव "llvm/Support/NativeFormatting.h"
#अन्तर्भाव "llvm/Support/SMTAPI.h"

using नामाकाश llvm;

#जर LLVM_WITH_Z3

#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/Twine.h"

#अन्तर्भाव <set>

#अन्तर्भाव <z3.h>

नामाकाश {

/// Configuration class for Z3
वर्ग Z3Config {
  मित्र वर्ग Z3Context;

  Z3_config Config = Z3_mk_config();

सार्वजनिक:
  Z3Config() = पूर्वनिर्धारित;
  Z3Config(स्थिरांक Z3Config &) = delete;
  Z3Config(Z3Config &&) = पूर्वनिर्धारित;
  Z3Config &चालक=(Z3Config &) = delete;
  Z3Config &चालक=(Z3Config &&) = पूर्वनिर्धारित;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
शून्य Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_संदेश(Context, Error)));
}

/// Wrapper for Z3 context
वर्ग Z3Context {
सार्वजनिक:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(स्थिरांक Z3Context &) = delete;
  Z3Context(Z3Context &&) = पूर्वनिर्धारित;
  Z3Context &चालक=(Z3Context &) = delete;
  Z3Context &चालक=(Z3Context &&) = पूर्वनिर्धारित;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
वर्ग Z3Sort : सार्वजनिक SMTSort {
  मित्र वर्ग Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

सार्वजनिक:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(स्थिरांक Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &चालक=(स्थिरांक Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    परत *हे;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &चालक=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    जर (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  शून्य Profile(llvm::Folding_Set_गाठ_ID &ID) स्थिरांक override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  तार्किक isBitvectorSortImpl() स्थिरांक override {
    परत (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  तार्किक isFloatSortImpl() स्थिरांक override {
    परत (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  तार्किक isBooleanSortImpl() स्थिरांक override {
    परत (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  अचिन्हित getBitvectorSortSizeImpl() स्थिरांक override {
    परत Z3_get_bv_sort_size(Context.Context, Sort);
  }

  अचिन्हित getFloatSortSizeImpl() स्थिरांक override {
    परत Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  तार्किक equal_to(SMTSort स्थिरांक &Other) स्थिरांक override {
    परत Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<स्थिरांक Z3Sort &>(Other).Sort);
  }

  शून्य print(raw_ostream &OS) स्थिरांक override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

स्थिर स्थिरांक Z3Sort &toZ3Sort(स्थिरांक SMTSort &S) {
  परत static_cast<स्थिरांक Z3Sort &>(S);
}

वर्ग Z3_अभिव्यक्ती : सार्वजनिक SMTExpr {
  मित्र वर्ग Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

सार्वजनिक:
  Z3_अभिव्यक्ती(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_अभिव्यक्ती(स्थिरांक Z3_अभिव्यक्ती &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_अभिव्यक्ती &चालक=(स्थिरांक Z3_अभिव्यक्ती &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    परत *हे;
  }

  Z3_अभिव्यक्ती(Z3_अभिव्यक्ती &&Other) = delete;
  Z3_अभिव्यक्ती &चालक=(Z3_अभिव्यक्ती &&Other) = delete;

  ~Z3_अभिव्यक्ती() {
    जर (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  शून्य Profile(llvm::Folding_Set_गाठ_ID &ID) स्थिरांक override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  तार्किक equal_to(SMTExpr स्थिरांक &Other) स्थिरांक override {
    प्रतिज्ञा(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<स्थिरांक Z3_अभिव्यक्ती &>(Other).AST)) &&
           "AST's must have the same sort");
    परत Z3_is_eq_ast(Context.Context, AST,
                        static_cast<स्थिरांक Z3_अभिव्यक्ती &>(Other).AST);
  }

  शून्य print(raw_ostream &OS) स्थिरांक override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

स्थिर स्थिरांक Z3_अभिव्यक्ती &to_Z3_अभिव्यक्ती(स्थिरांक SMTExpr &E) {
  परत static_cast<स्थिरांक Z3_अभिव्यक्ती &>(E);
}

वर्ग Z3Model {
  मित्र वर्ग Z3Solver;

  Z3Context &Context;

  Z3_model Model;

सार्वजनिक:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(स्थिरांक Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &चालक=(Z3Model &Other) = delete;
  Z3Model &चालक=(Z3Model &&Other) = delete;

  ~Z3Model() {
    जर (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  शून्य print(raw_ostream &OS) स्थिरांक {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD शून्य dump() स्थिरांक { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
स्थिर स्थिरांक llvm::fltSemantics &getFloatSemantics(अचिन्हित BitWidth) {
  निवड (BitWidth) {
  पूर्वनिर्धारित:
    llvm_unreachable("Unsupported floating-point semantics!");
    थांबा;
  स्थिती 16:
    परत llvm::APFloat::IEEEhalf();
  स्थिती 32:
    परत llvm::APFloat::IEEEsingle();
  स्थिती 64:
    परत llvm::APFloat::IEEEdouble();
  स्थिती 128:
    परत llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
स्थिर तार्किक areEquivalent(स्थिरांक llvm::fltSemantics &LHS,
                          स्थिरांक llvm::fltSemantics &RHS) {
  परत (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

वर्ग Z3Solver : सार्वजनिक SMTSolver {
  मित्र वर्ग Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [हे] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    परत S;
  }();

  Z3_params Params = [हे] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    परत P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_अभिव्यक्ती> CachedExprs;

सार्वजनिक:
  Z3Solver() = पूर्वनिर्धारित;
  Z3Solver(स्थिरांक Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &चालक=(Z3Solver &Other) = delete;
  Z3Solver &चालक=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  शून्य addConstraint(स्थिरांक SMTExprRef &Exp) स्थिरांक override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_अभिव्यक्ती(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(स्थिरांक SMTSort &Sort) {
    स्वतः It = CachedSorts.घाला(toZ3Sort(Sort));
    परत &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_अभिव्यक्ती_Ref(स्थिरांक SMTExpr &Exp) {
    स्वतः It = CachedExprs.घाला(to_Z3_अभिव्यक्ती(Exp));
    परत &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    परत newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(अचिन्हित BitWidth) override {
    परत newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(स्थिरांक SMTExprRef &Exp) override {
    परत newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    परत newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    परत newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    परत newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    परत newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvneg(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkBVNot(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvnot(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkNot(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_not(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvadd(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSub(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvsub(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVMul(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvmul(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvsrem(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVURem(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvurem(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvudiv(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVShl(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvshl(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvashr(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvlshr(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVXor(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvxor(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVOr(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvor(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                   to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvand(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvult(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvslt(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvugt(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvsgt(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVUle(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvule(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSle(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvsle(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVUge(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvuge(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVSge(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_bvsge(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                    to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkAnd(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_अभिव्यक्ती(*LHS).AST, to_Z3_अभिव्यक्ती(*RHS).AST};
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_अभिव्यक्ती(*LHS).AST, to_Z3_अभिव्यक्ती(*RHS).AST};
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_eq(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                 to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkFPMul(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ती(*LHS).AST, to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ती(*LHS).AST, to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPRem(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                      to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ती(*LHS).AST, to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPSub(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ती(*LHS).AST, to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPLt(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPGt(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPLe(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                      to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPGe(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                      to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkIte(स्थिरांक SMTExprRef &Cond, स्थिरांक SMTExprRef &T,
                   स्थिरांक SMTExprRef &F) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_ite(Context.Context, to_Z3_अभिव्यक्ती(*Cond).AST,
                                  to_Z3_अभिव्यक्ती(*T).AST, to_Z3_अभिव्यक्ती(*F).AST)));
  }

  SMTExprRef mkBVSignExt(अचिन्हित i, स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(अचिन्हित i, स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(अचिन्हित वरचा_2, अचिन्हित खालचा_2,
                         स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(Context, Z3_mk_extract(Context.Context, वरचा_2, खालचा_2,
                                                    to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS,
                               तार्किक isSigned) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                         to_Z3_अभिव्यक्ती(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(स्थिरांक SMTExprRef &LHS,
                                स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                          to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(स्थिरांक SMTExprRef &LHS,
                               स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                         to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS,
                                तार्किक isSigned) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                          to_Z3_अभिव्यक्ती(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(स्थिरांक SMTExprRef &LHS,
                                स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                          to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(स्थिरांक SMTExprRef &Exp) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS,
                               तार्किक isSigned) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                         to_Z3_अभिव्यक्ती(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(स्थिरांक SMTExprRef &LHS,
                                स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                          to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(स्थिरांक SMTExprRef &LHS, स्थिरांक SMTExprRef &RHS) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_concat(Context.Context, to_Z3_अभिव्यक्ती(*LHS).AST,
                                     to_Z3_अभिव्यक्ती(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(स्थिरांक SMTExprRef &From, स्थिरांक SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                              to_Z3_अभिव्यक्ती(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(स्थिरांक SMTExprRef &From, स्थिरांक SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                               to_Z3_अभिव्यक्ती(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(स्थिरांक SMTExprRef &From, स्थिरांक SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                                 to_Z3_अभिव्यक्ती(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(स्थिरांक SMTExprRef &From, अचिन्हित ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                                  to_Z3_अभिव्यक्ती(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(स्थिरांक SMTExprRef &From, अचिन्हित ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_अभिव्यक्ती(*RoundingMode).AST,
                                  to_Z3_अभिव्यक्ती(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(स्थिरांक तार्किक b) override {
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(स्थिरांक llvm::APSInt Int, अचिन्हित BitWidth) override {
    स्थिरांक Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    जर (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    स्थिरांक int64_t BitReprAsSigned = Int.get_Ext_मूल्य();
    स्थिरांक uint64_t BitReprAsUnsigned =
        reinterpret_cast<स्थिरांक uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(Context, Literal));
  }

  SMTExprRef mkFloat(स्थिरांक llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), असत्य);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_अभिव्यक्ती(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_चिन्ह(स्थिरांक अक्षर *Name, SMTSortRef Sort) override {
    परत new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_चिन्ह(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(स्थिरांक SMTExprRef &Exp, अचिन्हित BitWidth,
                            तार्किक isUnsigned) override {
    परत llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST),
                    10),
        isUnsigned);
  }

  तार्किक getBoolean(स्थिरांक SMTExprRef &Exp) override {
    परत Z3_get_bool_मूल्य(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    परत new_अभिव्यक्ती_Ref(Z3_अभिव्यक्ती(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  तार्किक toAPFloat(स्थिरांक SMTSortRef &Sort, स्थिरांक SMTExprRef &AST,
                 llvm::APFloat &Float, तार्किक useSemantics) {
    प्रतिज्ञा(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), सत्य);
    स्थिरांक llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    जर (!toAPSInt(BVSort, AST, Int, सत्य)) {
      परत असत्य;
    }

    जर (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      प्रतिज्ञा(असत्य && "Floating-point types don't match!");
      परत असत्य;
    }

    Float = llvm::APFloat(Semantics, Int);
    परत सत्य;
  }

  तार्किक toAPSInt(स्थिरांक SMTSortRef &Sort, स्थिरांक SMTExprRef &AST,
                llvm::APSInt &Int, तार्किक useSemantics) {
    जर (Sort->isBitvectorSort()) {
      जर (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        प्रतिज्ञा(असत्य && "Bitvector types don't match!");
        परत असत्य;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      परत सत्य;
    }

    जर (Sort->isBooleanSort()) {
      जर (useSemantics && Int.getBitWidth() < 1) {
        प्रतिज्ञा(असत्य && "Boolean type doesn't match!");
        परत असत्य;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      परत सत्य;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  तार्किक getInterpretation(स्थिरांक SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST));
    जर (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      परत असत्य;

    SMTExprRef Assign = new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    परत toAPSInt(Sort, Assign, Int, सत्य);
  }

  तार्किक getInterpretation(स्थिरांक SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_अभिव्यक्ती(*Exp).AST));
    जर (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      परत असत्य;

    SMTExprRef Assign = new_अभिव्यक्ती_Ref(
        Z3_अभिव्यक्ती(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    परत toAPFloat(Sort, Assign, Float, सत्य);
  }

  std::optional<तार्किक> check() स्थिरांक override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    जर (res == Z3_L_TRUE)
      परत सत्य;

    जर (res == Z3_L_FALSE)
      परत असत्य;

    परत std::nullopt;
  }

  शून्य ढकला() override { परत Z3_solver_ढकला(Context.Context, Solver); }

  शून्य काढा(अचिन्हित अंक_States = 1) override {
    प्रतिज्ञा(Z3_solver_get_अंक_scopes(Context.Context, Solver) >= अंक_States);
    परत Z3_solver_काढा(Context.Context, Solver, अंक_States);
  }

  तार्किक isFPSupported() override { परत सत्य; }

  /// Reset the solver and remove all constraints.
  शून्य reset() override { Z3_solver_reset(Context.Context, Solver); }

  शून्य print(raw_ostream &OS) स्थिरांक override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  शून्य setUnsignedParam(StringRef किल्ली, अचिन्हित मूल्य) override {
    Z3_चिन्ह Sym = Z3_mk_string_चिन्ह(Context.Context, किल्ली.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, मूल्य);
  }

  शून्य setBoolParam(StringRef किल्ली, तार्किक मूल्य) override {
    Z3_चिन्ह Sym = Z3_mk_string_चिन्ह(Context.Context, किल्ली.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, मूल्य);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() स्थिरांक override;
}; // end class Z3Solver

वर्ग Z3Statistics final : सार्वजनिक SMTSolverStatistics {
सार्वजनिक:
  द्विगुण getDouble(StringRef किल्ली) स्थिरांक override {
    स्वतः It = Double_मूल्ये.find(किल्ली.str());
    प्रतिज्ञा(It != Double_मूल्ये.end());
    परत It->second;
  };
  अचिन्हित getUnsigned(StringRef किल्ली) स्थिरांक override {
    स्वतः It = Unsigned_मूल्ये.find(किल्ली.str());
    जर (It == Unsigned_मूल्ये.end())
      परत 0;
    परत It->second;
  };

  शून्य print(raw_ostream &OS) स्थिरांक override {
    साठी (स्थिरांक स्वतः &E : Unsigned_मूल्ये)
      OS << E.first() << ": " << E.second << '\n';
    साठी (स्थिरांक स्वतः &E : Double_मूल्ये) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

खाजगी:
  मित्र वर्ग Z3Solver;
  StringMap<अचिन्हित> Unsigned_मूल्ये;
  StringMap<द्विगुण> Double_मूल्ये;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() स्थिरांक {
  स्वतः स्थिरांक &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  अचिन्हित अंक_किल्ल्या = Z3_stats_size(C, S);
  साठी (अचिन्हित Idx = 0; Idx < अंक_किल्ल्या; ++Idx) {
    स्थिरांक अक्षर *किल्ली = Z3_stats_get_किल्ली(C, S, Idx);
    जर (Z3_stats_is_uint(C, S, Idx)) {
      स्वतः मूल्य = Z3_stats_get_uint_मूल्य(C, S, Idx);
      Result.Unsigned_मूल्ये.try_emplace(किल्ली, मूल्य);
    } अन्यथा {
      प्रतिज्ञा(Z3_stats_is_double(C, S, Idx));
      स्वतः मूल्य = Z3_stats_get_double_मूल्य(C, S, Idx);
      Result.Double_मूल्ये.try_emplace(किल्ली, मूल्य);
    }
  }
  परत std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#जर LLVM_WITH_Z3
  परत std::make_unique<Z3Solver>();
#अन्यथा
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           असत्य);
  परत nullptr;
#endif
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SMTSort::dump() स्थिरांक { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTExpr::dump() स्थिरांक { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTSolver::dump() स्थिरांक { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTSolverStatistics::dump() स्थिरांक { print(llvm::errs()); }
#endif
