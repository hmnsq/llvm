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

#यदि LLVM_WITH_Z3

#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/Twine.h"

#अन्तर्भाव <set>

#अन्तर्भाव <z3.h>

नामाकाश {

/// Configuration class for Z3
वर्ग Z3Config {
  friend वर्ग Z3Context;

  Z3_config Config = Z3_mk_config();

सार्वजनिक:
  Z3Config() = स्वभाव;
  Z3Config(ध्रुव Z3Config &) = delete;
  Z3Config(Z3Config &&) = स्वभाव;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = स्वभाव;
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

  Z3Context(ध्रुव Z3Context &) = delete;
  Z3Context(Z3Context &&) = स्वभाव;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = स्वभाव;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
वर्ग Z3Sort : सार्वजनिक SMTSort {
  friend वर्ग Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

सार्वजनिक:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(ध्रुव Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(ध्रुव Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    प्रत्यागम *एतत्;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    यदि (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  शून्य Profile(llvm::Folding_Set_गाँठ_ID &ID) ध्रुव override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  तार्किक isBitvectorSortImpl() ध्रुव override {
    प्रत्यागम (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  तार्किक isFloatSortImpl() ध्रुव override {
    प्रत्यागम (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  तार्किक isBooleanSortImpl() ध्रुव override {
    प्रत्यागम (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() ध्रुव override {
    प्रत्यागम Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() ध्रुव override {
    प्रत्यागम Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  तार्किक equal_to(SMTSort ध्रुव &Other) ध्रुव override {
    प्रत्यागम Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<ध्रुव Z3Sort &>(Other).Sort);
  }

  शून्य print(raw_ostream &OS) ध्रुव override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

स्थिर ध्रुव Z3Sort &toZ3Sort(ध्रुव SMTSort &S) {
  प्रत्यागम static_cast<ध्रुव Z3Sort &>(S);
}

वर्ग Z3_व्यंजक : सार्वजनिक SMTExpr {
  friend वर्ग Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

सार्वजनिक:
  Z3_व्यंजक(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_व्यंजक(ध्रुव Z3_व्यंजक &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_व्यंजक &operator=(ध्रुव Z3_व्यंजक &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    प्रत्यागम *एतत्;
  }

  Z3_व्यंजक(Z3_व्यंजक &&Other) = delete;
  Z3_व्यंजक &operator=(Z3_व्यंजक &&Other) = delete;

  ~Z3_व्यंजक() {
    यदि (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  शून्य Profile(llvm::Folding_Set_गाँठ_ID &ID) ध्रुव override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  तार्किक equal_to(SMTExpr ध्रुव &Other) ध्रुव override {
    प्रतिज्ञा(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<ध्रुव Z3_व्यंजक &>(Other).AST)) &&
           "AST's must have the same sort");
    प्रत्यागम Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ध्रुव Z3_व्यंजक &>(Other).AST);
  }

  शून्य print(raw_ostream &OS) ध्रुव override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

स्थिर ध्रुव Z3_व्यंजक &to_Z3_व्यंजक(ध्रुव SMTExpr &E) {
  प्रत्यागम static_cast<ध्रुव Z3_व्यंजक &>(E);
}

वर्ग Z3Model {
  friend वर्ग Z3Solver;

  Z3Context &Context;

  Z3_model Model;

सार्वजनिक:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ध्रुव Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    यदि (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  शून्य print(raw_ostream &OS) ध्रुव {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD शून्य dump() ध्रुव { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
स्थिर ध्रुव llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  विकल्प (BitWidth) {
  स्वभाव:
    llvm_unreachable("Unsupported floating-point semantics!");
    भङ्ग;
  पक्ष 16:
    प्रत्यागम llvm::APFloat::IEEEhalf();
  पक्ष 32:
    प्रत्यागम llvm::APFloat::IEEEsingle();
  पक्ष 64:
    प्रत्यागम llvm::APFloat::IEEEdouble();
  पक्ष 128:
    प्रत्यागम llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
स्थिर तार्किक areEquivalent(ध्रुव llvm::fltSemantics &LHS,
                          ध्रुव llvm::fltSemantics &RHS) {
  प्रत्यागम (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

वर्ग Z3Solver : सार्वजनिक SMTSolver {
  friend वर्ग Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [एतत्] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    प्रत्यागम S;
  }();

  Z3_params Params = [एतत्] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    प्रत्यागम P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_व्यंजक> CachedExprs;

सार्वजनिक:
  Z3Solver() = स्वभाव;
  Z3Solver(ध्रुव Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  शून्य addConstraint(ध्रुव SMTExprRef &Exp) ध्रुव override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_व्यंजक(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(ध्रुव SMTSort &Sort) {
    auto It = CachedSorts.जोड़ो(toZ3Sort(Sort));
    प्रत्यागम &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_व्यंजक_Ref(ध्रुव SMTExpr &Exp) {
    auto It = CachedExprs.जोड़ो(to_Z3_व्यंजक(Exp));
    प्रत्यागम &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    प्रत्यागम newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    प्रत्यागम newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    प्रत्यागम newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    प्रत्यागम newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    प्रत्यागम newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    प्रत्यागम newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvneg(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvnot(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkNot(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_not(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvadd(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsub(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvmul(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsrem(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvurem(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvudiv(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvshl(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvashr(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvlshr(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvxor(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvor(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                   to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvand(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvult(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvslt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvugt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsgt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvule(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsle(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvuge(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_bvsge(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                    to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkAnd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST};
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST};
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_eq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                 to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                      to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                             to_Z3_व्यंजक(*LHS).AST, to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                      to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                      to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkIte(ध्रुव SMTExprRef &Cond, ध्रुव SMTExprRef &T,
                   ध्रुव SMTExprRef &F) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_ite(Context.Context, to_Z3_व्यंजक(*Cond).AST,
                                  to_Z3_व्यंजक(*T).AST, to_Z3_व्यंजक(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_व्यंजक(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned ऊपरी_2, unsigned निचला_2,
                         ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_extract(Context.Context, ऊपरी_2, निचला_2,
                                                    to_Z3_व्यंजक(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS,
                               तार्किक isSigned) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                         to_Z3_व्यंजक(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ध्रुव SMTExprRef &LHS,
                                ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(ध्रुव SMTExprRef &LHS,
                               ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                         to_Z3_व्यंजक(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS,
                                तार्किक isSigned) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(ध्रुव SMTExprRef &LHS,
                                ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_व्यंजक(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS,
                               तार्किक isSigned) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                         to_Z3_व्यंजक(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ध्रुव SMTExprRef &LHS,
                                ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                          to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_concat(Context.Context, to_Z3_व्यंजक(*LHS).AST,
                                     to_Z3_व्यंजक(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ध्रुव SMTExprRef &From, ध्रुव SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                              to_Z3_व्यंजक(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(ध्रुव SMTExprRef &From, ध्रुव SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                               to_Z3_व्यंजक(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(ध्रुव SMTExprRef &From, ध्रुव SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                                 to_Z3_व्यंजक(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(ध्रुव SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                                  to_Z3_व्यंजक(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(ध्रुव SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_व्यंजक(*RoundingMode).AST,
                                  to_Z3_व्यंजक(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(ध्रुव तार्किक b) override {
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ध्रुव llvm::APSInt Int, unsigned BitWidth) override {
    ध्रुव Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    यदि (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    ध्रुव int64_t BitReprAsSigned = Int.get_Ext_मूल्य();
    ध्रुव uint64_t BitReprAsUnsigned =
        reinterpret_cast<ध्रुव uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(Context, Literal));
  }

  SMTExprRef mkFloat(ध्रुव llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), असत्य);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_व्यंजक(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_प्रतीक(ध्रुव वर्ण *Name, SMTSortRef Sort) override {
    प्रत्यागम new_व्यंजक_Ref(
        Z3_व्यंजक(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_प्रतीक(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(ध्रुव SMTExprRef &Exp, unsigned BitWidth,
                            तार्किक isUnsigned) override {
    प्रत्यागम llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_व्यंजक(*Exp).AST),
                    10),
        isUnsigned);
  }

  तार्किक getBoolean(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम Z3_get_bool_मूल्य(Context.Context, to_Z3_व्यंजक(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    प्रत्यागम new_व्यंजक_Ref(Z3_व्यंजक(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  तार्किक toAPFloat(ध्रुव SMTSortRef &Sort, ध्रुव SMTExprRef &AST,
                 llvm::APFloat &Float, तार्किक useSemantics) {
    प्रतिज्ञा(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), सत्य);
    ध्रुव llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    यदि (!toAPSInt(BVSort, AST, Int, सत्य)) {
      प्रत्यागम असत्य;
    }

    यदि (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      प्रतिज्ञा(असत्य && "Floating-point types don't match!");
      प्रत्यागम असत्य;
    }

    Float = llvm::APFloat(Semantics, Int);
    प्रत्यागम सत्य;
  }

  तार्किक toAPSInt(ध्रुव SMTSortRef &Sort, ध्रुव SMTExprRef &AST,
                llvm::APSInt &Int, तार्किक useSemantics) {
    यदि (Sort->isBitvectorSort()) {
      यदि (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        प्रतिज्ञा(असत्य && "Bitvector types don't match!");
        प्रत्यागम असत्य;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      प्रत्यागम सत्य;
    }

    यदि (Sort->isBooleanSort()) {
      यदि (useSemantics && Int.getBitWidth() < 1) {
        प्रतिज्ञा(असत्य && "Boolean type doesn't match!");
        प्रत्यागम असत्य;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      प्रत्यागम सत्य;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  तार्किक getInterpretation(ध्रुव SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_व्यंजक(*Exp).AST));
    यदि (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      प्रत्यागम असत्य;

    SMTExprRef Assign = new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    प्रत्यागम toAPSInt(Sort, Assign, Int, सत्य);
  }

  तार्किक getInterpretation(ध्रुव SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_व्यंजक(*Exp).AST));
    यदि (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      प्रत्यागम असत्य;

    SMTExprRef Assign = new_व्यंजक_Ref(
        Z3_व्यंजक(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    प्रत्यागम toAPFloat(Sort, Assign, Float, सत्य);
  }

  std::optional<तार्किक> check() ध्रुव override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    यदि (res == Z3_L_TRUE)
      प्रत्यागम सत्य;

    यदि (res == Z3_L_FALSE)
      प्रत्यागम असत्य;

    प्रत्यागम std::nullopt;
  }

  शून्य धकेलो() override { प्रत्यागम Z3_solver_धकेलो(Context.Context, Solver); }

  शून्य निकालो(unsigned अंक_States = 1) override {
    प्रतिज्ञा(Z3_solver_get_अंक_scopes(Context.Context, Solver) >= अंक_States);
    प्रत्यागम Z3_solver_निकालो(Context.Context, Solver, अंक_States);
  }

  तार्किक isFPSupported() override { प्रत्यागम सत्य; }

  /// Reset the solver and remove all constraints.
  शून्य reset() override { Z3_solver_reset(Context.Context, Solver); }

  शून्य print(raw_ostream &OS) ध्रुव override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  शून्य setUnsignedParam(StringRef कुंजी, unsigned मूल्य) override {
    Z3_प्रतीक Sym = Z3_mk_string_प्रतीक(Context.Context, कुंजी.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, मूल्य);
  }

  शून्य setBoolParam(StringRef कुंजी, तार्किक मूल्य) override {
    Z3_प्रतीक Sym = Z3_mk_string_प्रतीक(Context.Context, कुंजी.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, मूल्य);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ध्रुव override;
}; // end class Z3Solver

वर्ग Z3Statistics final : सार्वजनिक SMTSolverStatistics {
सार्वजनिक:
  द्विगुण getDouble(StringRef कुंजी) ध्रुव override {
    auto It = Double_मूल्यावली.find(कुंजी.str());
    प्रतिज्ञा(It != Double_मूल्यावली.end());
    प्रत्यागम It->second;
  };
  unsigned getUnsigned(StringRef कुंजी) ध्रुव override {
    auto It = Unsigned_मूल्यावली.find(कुंजी.str());
    यदि (It == Unsigned_मूल्यावली.end())
      प्रत्यागम 0;
    प्रत्यागम It->second;
  };

  शून्य print(raw_ostream &OS) ध्रुव override {
    प्रति (ध्रुव auto &E : Unsigned_मूल्यावली)
      OS << E.first() << ": " << E.second << '\n';
    प्रति (ध्रुव auto &E : Double_मूल्यावली) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

गुप्त:
  friend वर्ग Z3Solver;
  StringMap<unsigned> Unsigned_मूल्यावली;
  StringMap<द्विगुण> Double_मूल्यावली;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ध्रुव {
  auto ध्रुव &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned अंक_कुंजियाँ = Z3_stats_size(C, S);
  प्रति (unsigned Idx = 0; Idx < अंक_कुंजियाँ; ++Idx) {
    ध्रुव वर्ण *कुंजी = Z3_stats_get_कुंजी(C, S, Idx);
    यदि (Z3_stats_is_uint(C, S, Idx)) {
      auto मूल्य = Z3_stats_get_uint_मूल्य(C, S, Idx);
      Result.Unsigned_मूल्यावली.try_emplace(कुंजी, मूल्य);
    } अन्यथा {
      प्रतिज्ञा(Z3_stats_is_double(C, S, Idx));
      auto मूल्य = Z3_stats_get_double_मूल्य(C, S, Idx);
      Result.Double_मूल्यावली.try_emplace(कुंजी, मूल्य);
    }
  }
  प्रत्यागम std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#यदि LLVM_WITH_Z3
  प्रत्यागम std::make_unique<Z3Solver>();
#अन्यथा
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           असत्य);
  प्रत्यागम nullptr;
#endif
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SMTSort::dump() ध्रुव { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTExpr::dump() ध्रुव { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTSolver::dump() ध्रुव { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTSolverStatistics::dump() ध्रुव { print(llvm::errs()); }
#endif
