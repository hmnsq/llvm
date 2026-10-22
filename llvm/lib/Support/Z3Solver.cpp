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
  मित्र वर्ग Z3Context;

  Z3_config Config = Z3_mk_config();

सार्वजनिक:
  Z3Config() = स्वभाव;
  Z3Config(अचर Z3Config &) = delete;
  Z3Config(Z3Config &&) = स्वभाव;
  Z3Config &चालक=(Z3Config &) = delete;
  Z3Config &चालक=(Z3Config &&) = स्वभाव;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
शून्य Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_सन्देश(Context, Error)));
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

  Z3Context(अचर Z3Context &) = delete;
  Z3Context(Z3Context &&) = स्वभाव;
  Z3Context &चालक=(Z3Context &) = delete;
  Z3Context &चालक=(Z3Context &&) = स्वभाव;

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
  Z3Sort(अचर Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &चालक=(अचर Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    फर्काऊ *एतत्;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &चालक=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    यदि (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  शून्य Profile(llvm::Folding_Set_गाँठो_ID &ID) अचर override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  तार्किक isBitvectorSortImpl() अचर override {
    फर्काऊ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  तार्किक isFloatSortImpl() अचर override {
    फर्काऊ (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  तार्किक isBooleanSortImpl() अचर override {
    फर्काऊ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  अचिन्हित getBitvectorSortSizeImpl() अचर override {
    फर्काऊ Z3_get_bv_sort_size(Context.Context, Sort);
  }

  अचिन्हित getFloatSortSizeImpl() अचर override {
    फर्काऊ Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  तार्किक equal_to(SMTSort अचर &Other) अचर override {
    फर्काऊ Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<अचर Z3Sort &>(Other).Sort);
  }

  शून्य print(raw_ostream &OS) अचर override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

स्थिर अचर Z3Sort &toZ3Sort(अचर SMTSort &S) {
  फर्काऊ static_cast<अचर Z3Sort &>(S);
}

वर्ग Z3_अभिव्यक्ति : सार्वजनिक SMTExpr {
  मित्र वर्ग Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

सार्वजनिक:
  Z3_अभिव्यक्ति(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_अभिव्यक्ति(अचर Z3_अभिव्यक्ति &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_अभिव्यक्ति &चालक=(अचर Z3_अभिव्यक्ति &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    फर्काऊ *एतत्;
  }

  Z3_अभिव्यक्ति(Z3_अभिव्यक्ति &&Other) = delete;
  Z3_अभिव्यक्ति &चालक=(Z3_अभिव्यक्ति &&Other) = delete;

  ~Z3_अभिव्यक्ति() {
    यदि (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  शून्य Profile(llvm::Folding_Set_गाँठो_ID &ID) अचर override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  तार्किक equal_to(SMTExpr अचर &Other) अचर override {
    प्रतिज्ञा(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<अचर Z3_अभिव्यक्ति &>(Other).AST)) &&
           "AST's must have the same sort");
    फर्काऊ Z3_is_eq_ast(Context.Context, AST,
                        static_cast<अचर Z3_अभिव्यक्ति &>(Other).AST);
  }

  शून्य print(raw_ostream &OS) अचर override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

स्थिर अचर Z3_अभिव्यक्ति &to_Z3_अभिव्यक्ति(अचर SMTExpr &E) {
  फर्काऊ static_cast<अचर Z3_अभिव्यक्ति &>(E);
}

वर्ग Z3Model {
  मित्र वर्ग Z3Solver;

  Z3Context &Context;

  Z3_model Model;

सार्वजनिक:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(अचर Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &चालक=(Z3Model &Other) = delete;
  Z3Model &चालक=(Z3Model &&Other) = delete;

  ~Z3Model() {
    यदि (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  शून्य print(raw_ostream &OS) अचर {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD शून्य dump() अचर { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
स्थिर अचर llvm::fltSemantics &getFloatSemantics(अचिन्हित BitWidth) {
  विकल्प (BitWidth) {
  स्वभाव:
    llvm_unreachable("Unsupported floating-point semantics!");
    रोक;
  पक्ष 16:
    फर्काऊ llvm::APFloat::IEEEhalf();
  पक्ष 32:
    फर्काऊ llvm::APFloat::IEEEsingle();
  पक्ष 64:
    फर्काऊ llvm::APFloat::IEEEdouble();
  पक्ष 128:
    फर्काऊ llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
स्थिर तार्किक areEquivalent(अचर llvm::fltSemantics &LHS,
                          अचर llvm::fltSemantics &RHS) {
  फर्काऊ (llvm::APFloat::semanticsPrecision(LHS) ==
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

  Z3_solver Solver = [एतत्] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    फर्काऊ S;
  }();

  Z3_params Params = [एतत्] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    फर्काऊ P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_अभिव्यक्ति> CachedExprs;

सार्वजनिक:
  Z3Solver() = स्वभाव;
  Z3Solver(अचर Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &चालक=(Z3Solver &Other) = delete;
  Z3Solver &चालक=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  शून्य addConstraint(अचर SMTExprRef &Exp) अचर override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_अभिव्यक्ति(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(अचर SMTSort &Sort) {
    स्वतः It = CachedSorts.घुसाउ(toZ3Sort(Sort));
    फर्काऊ &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_अभिव्यक्ति_Ref(अचर SMTExpr &Exp) {
    स्वतः It = CachedExprs.घुसाउ(to_Z3_अभिव्यक्ति(Exp));
    फर्काऊ &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    फर्काऊ newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(अचिन्हित BitWidth) override {
    फर्काऊ newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(अचर SMTExprRef &Exp) override {
    फर्काऊ newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    फर्काऊ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    फर्काऊ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    फर्काऊ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    फर्काऊ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvneg(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkBVNot(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvnot(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkNot(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_not(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvadd(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSub(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvsub(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVMul(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvmul(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvsrem(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVURem(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvurem(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvudiv(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVShl(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvshl(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvashr(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvlshr(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVXor(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvxor(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVOr(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvor(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                   to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvand(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvult(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvslt(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvugt(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvsgt(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVUle(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvule(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSle(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvsle(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVUge(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvuge(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVSge(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_bvsge(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                    to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkAnd(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_अभिव्यक्ति(*LHS).AST, to_Z3_अभिव्यक्ति(*RHS).AST};
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_अभिव्यक्ति(*LHS).AST, to_Z3_अभिव्यक्ति(*RHS).AST};
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_eq(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                 to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkFPMul(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ति(*LHS).AST, to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ति(*LHS).AST, to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPRem(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                      to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ति(*LHS).AST, to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPSub(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                             to_Z3_अभिव्यक्ति(*LHS).AST, to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPLt(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPGt(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPLe(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                      to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPGe(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                      to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkIte(अचर SMTExprRef &Cond, अचर SMTExprRef &T,
                   अचर SMTExprRef &F) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_ite(Context.Context, to_Z3_अभिव्यक्ति(*Cond).AST,
                                  to_Z3_अभिव्यक्ति(*T).AST, to_Z3_अभिव्यक्ति(*F).AST)));
  }

  SMTExprRef mkBVSignExt(अचिन्हित i, अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(अचिन्हित i, अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(अचिन्हित माथिल्लो_2, अचिन्हित तल्लो_2,
                         अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(Context, Z3_mk_extract(Context.Context, माथिल्लो_2, तल्लो_2,
                                                    to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS,
                               तार्किक isSigned) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                         to_Z3_अभिव्यक्ति(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(अचर SMTExprRef &LHS,
                                अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                          to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(अचर SMTExprRef &LHS,
                               अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                         to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS,
                                तार्किक isSigned) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                          to_Z3_अभिव्यक्ति(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(अचर SMTExprRef &LHS,
                                अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                          to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(अचर SMTExprRef &Exp) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS,
                               तार्किक isSigned) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                         to_Z3_अभिव्यक्ति(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(अचर SMTExprRef &LHS,
                                अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                          to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(अचर SMTExprRef &LHS, अचर SMTExprRef &RHS) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_concat(Context.Context, to_Z3_अभिव्यक्ति(*LHS).AST,
                                     to_Z3_अभिव्यक्ति(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(अचर SMTExprRef &From, अचर SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                              to_Z3_अभिव्यक्ति(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(अचर SMTExprRef &From, अचर SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                               to_Z3_अभिव्यक्ति(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(अचर SMTExprRef &From, अचर SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                                 to_Z3_अभिव्यक्ति(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(अचर SMTExprRef &From, अचिन्हित ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                                  to_Z3_अभिव्यक्ति(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(अचर SMTExprRef &From, अचिन्हित ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_अभिव्यक्ति(*RoundingMode).AST,
                                  to_Z3_अभिव्यक्ति(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(अचर तार्किक b) override {
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(अचर llvm::APSInt Int, अचिन्हित BitWidth) override {
    अचर Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    यदि (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    अचर int64_t BitReprAsSigned = Int.get_Ext_मान();
    अचर uint64_t BitReprAsUnsigned =
        reinterpret_cast<अचर uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(Context, Literal));
  }

  SMTExprRef mkFloat(अचर llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), असत्य);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_अभिव्यक्ति(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_चिह्न(अचर वर्ण *Name, SMTSortRef Sort) override {
    फर्काऊ new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_चिह्न(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(अचर SMTExprRef &Exp, अचिन्हित BitWidth,
                            तार्किक isUnsigned) override {
    फर्काऊ llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST),
                    10),
        isUnsigned);
  }

  तार्किक getBoolean(अचर SMTExprRef &Exp) override {
    फर्काऊ Z3_get_bool_मान(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    फर्काऊ new_अभिव्यक्ति_Ref(Z3_अभिव्यक्ति(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  तार्किक toAPFloat(अचर SMTSortRef &Sort, अचर SMTExprRef &AST,
                 llvm::APFloat &Float, तार्किक useSemantics) {
    प्रतिज्ञा(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), सत्य);
    अचर llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    यदि (!toAPSInt(BVSort, AST, Int, सत्य)) {
      फर्काऊ असत्य;
    }

    यदि (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      प्रतिज्ञा(असत्य && "Floating-point types don't match!");
      फर्काऊ असत्य;
    }

    Float = llvm::APFloat(Semantics, Int);
    फर्काऊ सत्य;
  }

  तार्किक toAPSInt(अचर SMTSortRef &Sort, अचर SMTExprRef &AST,
                llvm::APSInt &Int, तार्किक useSemantics) {
    यदि (Sort->isBitvectorSort()) {
      यदि (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        प्रतिज्ञा(असत्य && "Bitvector types don't match!");
        फर्काऊ असत्य;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      फर्काऊ सत्य;
    }

    यदि (Sort->isBooleanSort()) {
      यदि (useSemantics && Int.getBitWidth() < 1) {
        प्रतिज्ञा(असत्य && "Boolean type doesn't match!");
        फर्काऊ असत्य;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      फर्काऊ सत्य;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  तार्किक getInterpretation(अचर SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST));
    यदि (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      फर्काऊ असत्य;

    SMTExprRef Assign = new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    फर्काऊ toAPSInt(Sort, Assign, Int, सत्य);
  }

  तार्किक getInterpretation(अचर SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_अभिव्यक्ति(*Exp).AST));
    यदि (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      फर्काऊ असत्य;

    SMTExprRef Assign = new_अभिव्यक्ति_Ref(
        Z3_अभिव्यक्ति(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    फर्काऊ toAPFloat(Sort, Assign, Float, सत्य);
  }

  std::optional<तार्किक> check() अचर override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    यदि (res == Z3_L_TRUE)
      फर्काऊ सत्य;

    यदि (res == Z3_L_FALSE)
      फर्काऊ असत्य;

    फर्काऊ std::nullopt;
  }

  शून्य थप() override { फर्काऊ Z3_solver_थप(Context.Context, Solver); }

  शून्य झिक(अचिन्हित अङ्क_States = 1) override {
    प्रतिज्ञा(Z3_solver_get_अङ्क_scopes(Context.Context, Solver) >= अङ्क_States);
    फर्काऊ Z3_solver_झिक(Context.Context, Solver, अङ्क_States);
  }

  तार्किक isFPSupported() override { फर्काऊ सत्य; }

  /// Reset the solver and remove all constraints.
  शून्य reset() override { Z3_solver_reset(Context.Context, Solver); }

  शून्य print(raw_ostream &OS) अचर override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  शून्य setUnsignedParam(StringRef साँचो, अचिन्हित मान) override {
    Z3_चिह्न Sym = Z3_mk_string_चिह्न(Context.Context, साँचो.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, मान);
  }

  शून्य setBoolParam(StringRef साँचो, तार्किक मान) override {
    Z3_चिह्न Sym = Z3_mk_string_चिह्न(Context.Context, साँचो.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, मान);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() अचर override;
}; // end class Z3Solver

वर्ग Z3Statistics final : सार्वजनिक SMTSolverStatistics {
सार्वजनिक:
  द्विगुण getDouble(StringRef साँचो) अचर override {
    स्वतः It = Double_मानहरू.find(साँचो.str());
    प्रतिज्ञा(It != Double_मानहरू.end());
    फर्काऊ It->second;
  };
  अचिन्हित getUnsigned(StringRef साँचो) अचर override {
    स्वतः It = Unsigned_मानहरू.find(साँचो.str());
    यदि (It == Unsigned_मानहरू.end())
      फर्काऊ 0;
    फर्काऊ It->second;
  };

  शून्य print(raw_ostream &OS) अचर override {
    लागि (अचर स्वतः &E : Unsigned_मानहरू)
      OS << E.first() << ": " << E.second << '\n';
    लागि (अचर स्वतः &E : Double_मानहरू) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

गुप्त:
  मित्र वर्ग Z3Solver;
  StringMap<अचिन्हित> Unsigned_मानहरू;
  StringMap<द्विगुण> Double_मानहरू;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() अचर {
  स्वतः अचर &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  अचिन्हित अङ्क_साँचोहरू = Z3_stats_size(C, S);
  लागि (अचिन्हित Idx = 0; Idx < अङ्क_साँचोहरू; ++Idx) {
    अचर वर्ण *साँचो = Z3_stats_get_साँचो(C, S, Idx);
    यदि (Z3_stats_is_uint(C, S, Idx)) {
      स्वतः मान = Z3_stats_get_uint_मान(C, S, Idx);
      Result.Unsigned_मानहरू.try_emplace(साँचो, मान);
    } अन्यथा {
      प्रतिज्ञा(Z3_stats_is_double(C, S, Idx));
      स्वतः मान = Z3_stats_get_double_मान(C, S, Idx);
      Result.Double_मानहरू.try_emplace(साँचो, मान);
    }
  }
  फर्काऊ std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#यदि LLVM_WITH_Z3
  फर्काऊ std::make_unique<Z3Solver>();
#अन्यथा
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           असत्य);
  फर्काऊ nullptr;
#endif
}

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SMTSort::dump() अचर { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTExpr::dump() अचर { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTSolver::dump() अचर { print(llvm::errs()); }
LLVM_DUMP_METHOD शून्य SMTSolverStatistics::dump() अचर { print(llvm::errs()); }
#endif
