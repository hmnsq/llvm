//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/ScopeExit.h"
#അന്തര്ഭാവ "llvm/Config/config.h"
#അന്തര്ഭാവ "llvm/Support/NativeFormatting.h"
#അന്തര്ഭാവ "llvm/Support/SMTAPI.h"

using നാമാകാശ llvm;

#എങ്കിൽ LLVM_WITH_Z3

#അന്തര്ഭാവ "llvm/ADT/SmallString.h"
#അന്തര്ഭാവ "llvm/ADT/StringMap.h"
#അന്തര്ഭാവ "llvm/ADT/Twine.h"

#അന്തര്ഭാവ <set>

#അന്തര്ഭാവ <z3.h>

നാമാകാശ {

/// Configuration class for Z3
വർഗ്ഗം Z3Config {
  friend വർഗ്ഗം Z3Context;

  Z3_config Config = Z3_mk_config();

പൊതു:
  Z3Config() = സ്ഥിരസ്ഥിതി;
  Z3Config(സ്ഥിരം Z3Config &) = delete;
  Z3Config(Z3Config &&) = സ്ഥിരസ്ഥിതി;
  Z3Config &operator=(Z3Config &) = delete;
  Z3Config &operator=(Z3Config &&) = സ്ഥിരസ്ഥിതി;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
ശൂന്യം Z3ErrorHandler(Z3_context Context, Z3_error_code Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_സന്ദേശം(Context, Error)));
}

/// Wrapper for Z3 context
വർഗ്ഗം Z3Context {
പൊതു:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_set_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(സ്ഥിരം Z3Context &) = delete;
  Z3Context(Z3Context &&) = സ്ഥിരസ്ഥിതി;
  Z3Context &operator=(Z3Context &) = delete;
  Z3Context &operator=(Z3Context &&) = സ്ഥിരസ്ഥിതി;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = nullptr;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
വർഗ്ഗം Z3Sort : പൊതു SMTSort {
  friend വർഗ്ഗം Z3Solver;

  Z3Context &Context;

  Z3_sort Sort;

പൊതു:
  /// Default constructor, mainly used by make_shared
  Z3Sort(Z3Context &C, Z3_sort ZS) : Context(C), Sort(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3Sort(സ്ഥിരം Z3Sort &Other) : Context(Other.Context), Sort(Other.Sort) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3Sort &operator=(സ്ഥിരം Z3Sort &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.Sort));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
    Sort = Other.Sort;
    തിരികെ *ഇത്;
  }

  Z3Sort(Z3Sort &&Other) = delete;
  Z3Sort &operator=(Z3Sort &&Other) = delete;

  ~Z3Sort() {
    എങ്കിൽ (Sort)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(Sort));
  }

  ശൂന്യം Profile(llvm::Folding_Set_കണ്ണി_ID &ID) സ്ഥിരം override {
    ID.AddInteger(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(Sort)));
  }

  തർക്കം isBitvectorSortImpl() സ്ഥിരം override {
    തിരികെ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BV_SORT);
  }

  തർക്കം isFloatSortImpl() സ്ഥിരം override {
    തിരികെ (Z3_get_sort_kind(Context.Context, Sort) == Z3_FLOATING_POINT_SORT);
  }

  തർക്കം isBooleanSortImpl() സ്ഥിരം override {
    തിരികെ (Z3_get_sort_kind(Context.Context, Sort) == Z3_BOOL_SORT);
  }

  unsigned getBitvectorSortSizeImpl() സ്ഥിരം override {
    തിരികെ Z3_get_bv_sort_size(Context.Context, Sort);
  }

  unsigned getFloatSortSizeImpl() സ്ഥിരം override {
    തിരികെ Z3_fpa_get_ebits(Context.Context, Sort) +
           Z3_fpa_get_sbits(Context.Context, Sort);
  }

  തർക്കം equal_to(SMTSort സ്ഥിരം &Other) സ്ഥിരം override {
    തിരികെ Z3_is_eq_sort(Context.Context, Sort,
                         static_cast<സ്ഥിരം Z3Sort &>(Other).Sort);
  }

  ശൂന്യം print(raw_ostream &OS) സ്ഥിരം override {
    OS << Z3_sort_to_string(Context.Context, Sort);
  }
}; // end class Z3Sort

സ്ഥിതം സ്ഥിരം Z3Sort &toZ3Sort(സ്ഥിരം SMTSort &S) {
  തിരികെ static_cast<സ്ഥിരം Z3Sort &>(S);
}

വർഗ്ഗം Z3_വ്യഞ്ജകം : പൊതു SMTExpr {
  friend വർഗ്ഗം Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

പൊതു:
  Z3_വ്യഞ്ജകം(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_വ്യഞ്ജകം(സ്ഥിരം Z3_വ്യഞ്ജകം &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_വ്യഞ്ജകം &operator=(സ്ഥിരം Z3_വ്യഞ്ജകം &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    തിരികെ *ഇത്;
  }

  Z3_വ്യഞ്ജകം(Z3_വ്യഞ്ജകം &&Other) = delete;
  Z3_വ്യഞ്ജകം &operator=(Z3_വ്യഞ്ജകം &&Other) = delete;

  ~Z3_വ്യഞ്ജകം() {
    എങ്കിൽ (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  ശൂന്യം Profile(llvm::Folding_Set_കണ്ണി_ID &ID) സ്ഥിരം override {
    ID.AddInteger(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  തർക്കം equal_to(SMTExpr സ്ഥിരം &Other) സ്ഥിരം override {
    പ്രതിജ്ഞാ(Z3_is_eq_sort(Context.Context, Z3_get_sort(Context.Context, AST),
                         Z3_get_sort(Context.Context,
                                     static_cast<സ്ഥിരം Z3_വ്യഞ്ജകം &>(Other).AST)) &&
           "AST's must have the same sort");
    തിരികെ Z3_is_eq_ast(Context.Context, AST,
                        static_cast<സ്ഥിരം Z3_വ്യഞ്ജകം &>(Other).AST);
  }

  ശൂന്യം print(raw_ostream &OS) സ്ഥിരം override {
    OS << Z3_ast_to_string(Context.Context, AST);
  }
}; // end class Z3Expr

സ്ഥിതം സ്ഥിരം Z3_വ്യഞ്ജകം &to_Z3_വ്യഞ്ജകം(സ്ഥിരം SMTExpr &E) {
  തിരികെ static_cast<സ്ഥിരം Z3_വ്യഞ്ജകം &>(E);
}

വർഗ്ഗം Z3Model {
  friend വർഗ്ഗം Z3Solver;

  Z3Context &Context;

  Z3_model Model;

പൊതു:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(സ്ഥിരം Z3Model &Other) = delete;
  Z3Model(Z3Model &&Other) = delete;
  Z3Model &operator=(Z3Model &Other) = delete;
  Z3Model &operator=(Z3Model &&Other) = delete;

  ~Z3Model() {
    എങ്കിൽ (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  ശൂന്യം print(raw_ostream &OS) സ്ഥിരം {
    OS << Z3_model_to_string(Context.Context, Model);
  }

  LLVM_DUMP_METHOD ശൂന്യം dump() സ്ഥിരം { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
സ്ഥിതം സ്ഥിരം llvm::fltSemantics &getFloatSemantics(unsigned BitWidth) {
  തിരഞ്ഞെടുപ്പ് (BitWidth) {
  സ്ഥിരസ്ഥിതി:
    llvm_unreachable("Unsupported floating-point semantics!");
    നിർത്തുക;
  അവസ്ഥ 16:
    തിരികെ llvm::APFloat::IEEEhalf();
  അവസ്ഥ 32:
    തിരികെ llvm::APFloat::IEEEsingle();
  അവസ്ഥ 64:
    തിരികെ llvm::APFloat::IEEEdouble();
  അവസ്ഥ 128:
    തിരികെ llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
സ്ഥിതം തർക്കം areEquivalent(സ്ഥിരം llvm::fltSemantics &LHS,
                          സ്ഥിരം llvm::fltSemantics &RHS) {
  തിരികെ (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semanticsSizeInBits(LHS) ==
          llvm::APFloat::semanticsSizeInBits(RHS));
}

വർഗ്ഗം Z3Solver : പൊതു SMTSolver {
  friend വർഗ്ഗം Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [ഇത്] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    തിരികെ S;
  }();

  Z3_params Params = [ഇത്] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    തിരികെ P;
  }();

  // Cache Sorts
  std::set<Z3Sort> CachedSorts;

  // Cache Exprs
  std::set<Z3_വ്യഞ്ജകം> CachedExprs;

പൊതു:
  Z3Solver() = സ്ഥിരസ്ഥിതി;
  Z3Solver(സ്ഥിരം Z3Solver &Other) = delete;
  Z3Solver(Z3Solver &&Other) = delete;
  Z3Solver &operator=(Z3Solver &Other) = delete;
  Z3Solver &operator=(Z3Solver &&Other) = delete;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  ശൂന്യം addConstraint(സ്ഥിരം SMTExprRef &Exp) സ്ഥിരം override {
    Z3_solver_assert(Context.Context, Solver, to_Z3_വ്യഞ്ജകം(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef newSortRef(സ്ഥിരം SMTSort &Sort) {
    auto It = CachedSorts.തിരുകുക(toZ3Sort(Sort));
    തിരികെ &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_വ്യഞ്ജകം_Ref(സ്ഥിരം SMTExpr &Exp) {
    auto It = CachedExprs.തിരുകുക(to_Z3_വ്യഞ്ജകം(Exp));
    തിരികെ &(*It.first);
  }

  SMTSortRef getBoolSort() override {
    തിരികെ newSortRef(Z3Sort(Context, Z3_mk_bool_sort(Context.Context)));
  }

  SMTSortRef getBitvectorSort(unsigned BitWidth) override {
    തിരികെ newSortRef(
        Z3Sort(Context, Z3_mk_bv_sort(Context.Context, BitWidth)));
  }

  SMTSortRef getSort(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ newSortRef(
        Z3Sort(Context, Z3_get_sort(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTSortRef getFloat16Sort() override {
    തിരികെ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_16(Context.Context)));
  }

  SMTSortRef getFloat32Sort() override {
    തിരികെ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_32(Context.Context)));
  }

  SMTSortRef getFloat64Sort() override {
    തിരികെ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_64(Context.Context)));
  }

  SMTSortRef getFloat128Sort() override {
    തിരികെ newSortRef(Z3Sort(Context, Z3_mk_fpa_sort_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvneg(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkBVNot(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvnot(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkNot(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_not(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvadd(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSub(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvsub(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVMul(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvmul(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvsrem(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVURem(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvurem(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvsdiv(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvudiv(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVShl(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvshl(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvashr(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvlshr(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVXor(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvxor(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVOr(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvor(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                   to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvand(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvult(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvslt(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvugt(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvsgt(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVUle(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvule(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSle(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvsle(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVUge(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvuge(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVSge(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_bvsge(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                    to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkAnd(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_വ്യഞ്ജകം(*LHS).AST, to_Z3_വ്യഞ്ജകം(*RHS).AST};
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(Context, Z3_mk_and(Context.Context, 2, Args)));
  }

  SMTExprRef mkOr(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    Z3_ast Args[2] = {to_Z3_വ്യഞ്ജകം(*LHS).AST, to_Z3_വ്യഞ്ജകം(*RHS).AST};
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(Context, Z3_mk_or(Context.Context, 2, Args)));
  }

  SMTExprRef mkEqual(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_eq(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                 to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_neg(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_fpa_is_infinite(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_is_nan(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkFPIsNormal(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_fpa_is_normal(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_fpa_is_zero(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkFPMul(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context,
               Z3_mk_fpa_mul(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                             to_Z3_വ്യഞ്ജകം(*LHS).AST, to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context,
               Z3_mk_fpa_div(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                             to_Z3_വ്യഞ്ജകം(*LHS).AST, to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPRem(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_rem(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                      to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context,
               Z3_mk_fpa_add(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                             to_Z3_വ്യഞ്ജകം(*LHS).AST, to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPSub(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context,
               Z3_mk_fpa_sub(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                             to_Z3_വ്യഞ്ജകം(*LHS).AST, to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPLt(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_lt(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPGt(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_gt(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPLe(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_leq(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                      to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPGe(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_geq(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                      to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_eq(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkIte(സ്ഥിരം SMTExprRef &Cond, സ്ഥിരം SMTExprRef &T,
                   സ്ഥിരം SMTExprRef &F) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_ite(Context.Context, to_Z3_വ്യഞ്ജകം(*Cond).AST,
                                  to_Z3_വ്യഞ്ജകം(*T).AST, to_Z3_വ്യഞ്ജകം(*F).AST)));
  }

  SMTExprRef mkBVSignExt(unsigned i, സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_sign_ext(Context.Context, i, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(unsigned i, സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_zero_ext(Context.Context, i, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(unsigned മുകളിലെ_2, unsigned താഴെയുള്ള_2,
                         സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(Context, Z3_mk_extract(Context.Context, മുകളിലെ_2, താഴെയുള്ള_2,
                                                    to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mkBVAddNoOverflow(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS,
                               തർക്കം isSigned) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvadd_no_overflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                         to_Z3_വ്യഞ്ജകം(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(സ്ഥിരം SMTExprRef &LHS,
                                സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                          to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mkBVSubNoOverflow(സ്ഥിരം SMTExprRef &LHS,
                               സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvsub_no_overflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                         to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS,
                                തർക്കം isSigned) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                          to_Z3_വ്യഞ്ജകം(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mkBVSDivNoOverflow(സ്ഥിരം SMTExprRef &LHS,
                                സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvsdiv_no_overflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                          to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mkBVNegNoOverflow(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvneg_no_overflow(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mkBVMulNoOverflow(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS,
                               തർക്കം isSigned) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvmul_no_overflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                         to_Z3_വ്യഞ്ജകം(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(സ്ഥിരം SMTExprRef &LHS,
                                സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                          to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(സ്ഥിരം SMTExprRef &LHS, സ്ഥിരം SMTExprRef &RHS) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_concat(Context.Context, to_Z3_വ്യഞ്ജകം(*LHS).AST,
                                     to_Z3_വ്യഞ്ജകം(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(സ്ഥിരം SMTExprRef &From, സ്ഥിരം SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context,
        Z3_mk_fpa_to_fp_float(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                              to_Z3_വ്യഞ്ജകം(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkSBVtoFP(സ്ഥിരം SMTExprRef &From, സ്ഥിരം SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context,
        Z3_mk_fpa_to_fp_signed(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                               to_Z3_വ്യഞ്ജകം(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkUBVtoFP(സ്ഥിരം SMTExprRef &From, സ്ഥിരം SMTSortRef &To) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context,
        Z3_mk_fpa_to_fp_unsigned(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                                 to_Z3_വ്യഞ്ജകം(*From).AST, toZ3Sort(*To).Sort)));
  }

  SMTExprRef mkFPtoSBV(സ്ഥിരം SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_fpa_to_sbv(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                                  to_Z3_വ്യഞ്ജകം(*From).AST, ToWidth)));
  }

  SMTExprRef mkFPtoUBV(സ്ഥിരം SMTExprRef &From, unsigned ToWidth) override {
    SMTExprRef RoundingMode = getFloatRoundingMode();
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_fpa_to_ubv(Context.Context, to_Z3_വ്യഞ്ജകം(*RoundingMode).AST,
                                  to_Z3_വ്യഞ്ജകം(*From).AST, ToWidth)));
  }

  SMTExprRef mkBoolean(സ്ഥിരം തർക്കം b) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(സ്ഥിരം llvm::APSInt Int, unsigned BitWidth) override {
    സ്ഥിരം Z3_sort Z3Sort = toZ3Sort(*getBitvectorSort(BitWidth)).Sort;

    // Slow path, when 64 bits are not enough.
    എങ്കിൽ (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> Buffer;
      Int.toString(Buffer, 10);
      തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
          Context, Z3_mk_numeral(Context.Context, Buffer.c_str(), Z3Sort)));
    }

    സ്ഥിരം int64_t BitReprAsSigned = Int.get_Ext_മൂല്യം();
    സ്ഥിരം uint64_t BitReprAsUnsigned =
        reinterpret_cast<സ്ഥിരം uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3Sort)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3Sort);
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(Context, Literal));
  }

  SMTExprRef mkFloat(സ്ഥിരം llvm::APFloat Float) override {
    SMTSortRef Sort =
        getFloatSort(llvm::APFloat::semanticsSizeInBits(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcastToAPInt(), അസത്യം);
    SMTExprRef Z3Int = mkBitvector(Int, Int.getBitWidth());
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(
        Context, Z3_mk_fpa_to_fp_bv(Context.Context, to_Z3_വ്യഞ്ജകം(*Z3Int).AST,
                                    toZ3Sort(*Sort).Sort)));
  }

  SMTExprRef mk_ചിഹ്നം(സ്ഥിരം അക്ഷരം *Name, SMTSortRef Sort) override {
    തിരികെ new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_ചിഹ്നം(Context.Context, Name),
                                    toZ3Sort(*Sort).Sort)));
  }

  llvm::APSInt getBitvector(സ്ഥിരം SMTExprRef &Exp, unsigned BitWidth,
                            തർക്കം isUnsigned) override {
    തിരികെ llvm::APSInt(
        llvm::APInt(BitWidth,
                    Z3_get_numeral_string(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST),
                    10),
        isUnsigned);
  }

  തർക്കം getBoolean(സ്ഥിരം SMTExprRef &Exp) override {
    തിരികെ Z3_get_bool_മൂല്യം(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef getFloatRoundingMode() override {
    // TODO: Don't assume nearest ties to even rounding mode
    തിരികെ new_വ്യഞ്ജകം_Ref(Z3_വ്യഞ്ജകം(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  തർക്കം toAPFloat(സ്ഥിരം SMTSortRef &Sort, സ്ഥിരം SMTExprRef &AST,
                 llvm::APFloat &Float, തർക്കം useSemantics) {
    പ്രതിജ്ഞാ(Sort->isFloatSort() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(Sort->getFloatSortSize(), സത്യം);
    സ്ഥിരം llvm::fltSemantics &Semantics =
        getFloatSemantics(Sort->getFloatSortSize());
    SMTSortRef BVSort = getBitvectorSort(Sort->getFloatSortSize());
    എങ്കിൽ (!toAPSInt(BVSort, AST, Int, സത്യം)) {
      തിരികെ അസത്യം;
    }

    എങ്കിൽ (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      പ്രതിജ്ഞാ(അസത്യം && "Floating-point types don't match!");
      തിരികെ അസത്യം;
    }

    Float = llvm::APFloat(Semantics, Int);
    തിരികെ സത്യം;
  }

  തർക്കം toAPSInt(സ്ഥിരം SMTSortRef &Sort, സ്ഥിരം SMTExprRef &AST,
                llvm::APSInt &Int, തർക്കം useSemantics) {
    എങ്കിൽ (Sort->isBitvectorSort()) {
      എങ്കിൽ (useSemantics && Int.getBitWidth() != Sort->getBitvectorSortSize()) {
        പ്രതിജ്ഞാ(അസത്യം && "Bitvector types don't match!");
        തിരികെ അസത്യം;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.getBitWidth(), Int.isUnsigned());
      തിരികെ സത്യം;
    }

    എങ്കിൽ (Sort->isBooleanSort()) {
      എങ്കിൽ (useSemantics && Int.getBitWidth() < 1) {
        പ്രതിജ്ഞാ(അസത്യം && "Boolean type doesn't match!");
        തിരികെ അസത്യം;
      }

      Int = llvm::APSInt(llvm::APInt(Int.getBitWidth(), getBoolean(AST)),
                         Int.isUnsigned());
      തിരികെ സത്യം;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  തർക്കം getInterpretation(സ്ഥിരം SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST));
    എങ്കിൽ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      തിരികെ അസത്യം;

    SMTExprRef Assign = new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    തിരികെ toAPSInt(Sort, Assign, Int, സത്യം);
  }

  തർക്കം getInterpretation(സ്ഥിരം SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_to_app(Context.Context, to_Z3_വ്യഞ്ജകം(*Exp).AST));
    എങ്കിൽ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      തിരികെ അസത്യം;

    SMTExprRef Assign = new_വ്യഞ്ജകം_Ref(
        Z3_വ്യഞ്ജകം(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef Sort = getSort(Assign);
    തിരികെ toAPFloat(Sort, Assign, Float, സത്യം);
  }

  std::optional<തർക്കം> check() സ്ഥിരം override {
    Z3_solver_set_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    എങ്കിൽ (res == Z3_L_TRUE)
      തിരികെ സത്യം;

    എങ്കിൽ (res == Z3_L_FALSE)
      തിരികെ അസത്യം;

    തിരികെ std::nullopt;
  }

  ശൂന്യം തള്ളുക() override { തിരികെ Z3_solver_തള്ളുക(Context.Context, Solver); }

  ശൂന്യം എടുക്കുക(unsigned അക്കം_States = 1) override {
    പ്രതിജ്ഞാ(Z3_solver_get_അക്കം_scopes(Context.Context, Solver) >= അക്കം_States);
    തിരികെ Z3_solver_എടുക്കുക(Context.Context, Solver, അക്കം_States);
  }

  തർക്കം isFPSupported() override { തിരികെ സത്യം; }

  /// Reset the solver and remove all constraints.
  ശൂന്യം reset() override { Z3_solver_reset(Context.Context, Solver); }

  ശൂന്യം print(raw_ostream &OS) സ്ഥിരം override {
    OS << Z3_solver_to_string(Context.Context, Solver);
  }

  ശൂന്യം setUnsignedParam(StringRef താക്കോൽ, unsigned മൂല്യം) override {
    Z3_ചിഹ്നം Sym = Z3_mk_string_ചിഹ്നം(Context.Context, താക്കോൽ.str().c_str());
    Z3_params_set_uint(Context.Context, Params, Sym, മൂല്യം);
  }

  ശൂന്യം setBoolParam(StringRef താക്കോൽ, തർക്കം മൂല്യം) override {
    Z3_ചിഹ്നം Sym = Z3_mk_string_ചിഹ്നം(Context.Context, താക്കോൽ.str().c_str());
    Z3_params_set_bool(Context.Context, Params, Sym, മൂല്യം);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() സ്ഥിരം override;
}; // end class Z3Solver

വർഗ്ഗം Z3Statistics final : പൊതു SMTSolverStatistics {
പൊതു:
  ഇരട്ട getDouble(StringRef താക്കോൽ) സ്ഥിരം override {
    auto It = Double_മൂല്യങ്ങൾ.find(താക്കോൽ.str());
    പ്രതിജ്ഞാ(It != Double_മൂല്യങ്ങൾ.end());
    തിരികെ It->second;
  };
  unsigned getUnsigned(StringRef താക്കോൽ) സ്ഥിരം override {
    auto It = Unsigned_മൂല്യങ്ങൾ.find(താക്കോൽ.str());
    എങ്കിൽ (It == Unsigned_മൂല്യങ്ങൾ.end())
      തിരികെ 0;
    തിരികെ It->second;
  };

  ശൂന്യം print(raw_ostream &OS) സ്ഥിരം override {
    വേണ്ടി (സ്ഥിരം auto &E : Unsigned_മൂല്യങ്ങൾ)
      OS << E.first() << ": " << E.second << '\n';
    വേണ്ടി (സ്ഥിരം auto &E : Double_മൂല്യങ്ങൾ) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

സ്വകാര്യ:
  friend വർഗ്ഗം Z3Solver;
  StringMap<unsigned> Unsigned_മൂല്യങ്ങൾ;
  StringMap<ഇരട്ട> Double_മൂല്യങ്ങൾ;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() സ്ഥിരം {
  auto സ്ഥിരം &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics Result;

  unsigned അക്കം_താക്കോലുകൾ = Z3_stats_size(C, S);
  വേണ്ടി (unsigned Idx = 0; Idx < അക്കം_താക്കോലുകൾ; ++Idx) {
    സ്ഥിരം അക്ഷരം *താക്കോൽ = Z3_stats_get_താക്കോൽ(C, S, Idx);
    എങ്കിൽ (Z3_stats_is_uint(C, S, Idx)) {
      auto മൂല്യം = Z3_stats_get_uint_മൂല്യം(C, S, Idx);
      Result.Unsigned_മൂല്യങ്ങൾ.try_emplace(താക്കോൽ, മൂല്യം);
    } അല്ലെങ്കിൽ {
      പ്രതിജ്ഞാ(Z3_stats_is_double(C, S, Idx));
      auto മൂല്യം = Z3_stats_get_double_മൂല്യം(C, S, Idx);
      Result.Double_മൂല്യങ്ങൾ.try_emplace(താക്കോൽ, മൂല്യം);
    }
  }
  തിരികെ std::make_unique<Z3Statistics>(std::move(Result));
}

} // end anonymous namespace

#endif

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#എങ്കിൽ LLVM_WITH_Z3
  തിരികെ std::make_unique<Z3Solver>();
#അല്ലെങ്കിൽ
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           അസത്യം);
  തിരികെ nullptr;
#endif
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം SMTSort::dump() സ്ഥിരം { print(llvm::errs()); }
LLVM_DUMP_METHOD ശൂന്യം SMTExpr::dump() സ്ഥിരം { print(llvm::errs()); }
LLVM_DUMP_METHOD ശൂന്യം SMTSolver::dump() സ്ഥിരം { print(llvm::errs()); }
LLVM_DUMP_METHOD ശൂന്യം SMTSolverStatistics::dump() സ്ഥിരം { print(llvm::errs()); }
#endif
