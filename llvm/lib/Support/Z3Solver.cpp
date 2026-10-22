//== Z3Solver.cpp -----------------------------------------------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/ScopeExit.h"
#అన్తర్భావ "llvm/Config/config.h"
#అన్తర్భావ "llvm/Support/NativeFormatting.h"
#అన్తర్భావ "llvm/Support/SMTAPI.h"

ఉపయోగించి నామాకాశ llvm;

#ఒకవేళ LLVM_WITH_Z3

#అన్తర్భావ "llvm/ADT/SmallString.h"
#అన్తర్భావ "llvm/ADT/StringMap.h"
#అన్తర్భావ "llvm/ADT/Twine.h"

#అన్తర్భావ <set>

#అన్తర్భావ <z3.h>

నామాకాశ {

/// Configuration class for Z3
తరగతి Z3Config {
  మిత్ర తరగతి Z3Context;

  Z3_config Config = Z3_mk_config();

బహిరంగ:
  Z3Config() = అప్రమేయ;
  Z3Config(స్థిరరాశి Z3Config &) = తొలగించు;
  Z3Config(Z3Config &&) = అప్రమేయ;
  Z3Config &క్రియాకారకం=(Z3Config &) = తొలగించు;
  Z3Config &క్రియాకారకం=(Z3Config &&) = అప్రమేయ;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
శూన్య Z3ErrorHandler(Z3_context Context, Z3_error_సంకేతం Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_సందేశం(Context, Error)));
}

/// Wrapper for Z3 context
తరగతి Z3Context {
బహిరంగ:
  Z3Config Config;
  Z3_context Context;

  Z3Context() {
    Context = Z3_mk_context_rc(Config.Config);
    // The error function is set here because the context is the first object
    // created by the backend
    Z3_సమితి_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(స్థిరరాశి Z3Context &) = తొలగించు;
  Z3Context(Z3Context &&) = అప్రమేయ;
  Z3Context &క్రియాకారకం=(Z3Context &) = తొలగించు;
  Z3Context &క్రియాకారకం=(Z3Context &&) = అప్రమేయ;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = శూన్యసూచిక;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
తరగతి Z3_క్రమీకరించు : బహిరంగ SMTSort {
  మిత్ర తరగతి Z3Solver;

  Z3Context &Context;

  Z3_క్రమీకరించు_2 క్రమీకరించు_2;

బహిరంగ:
  /// Default constructor, mainly used by make_shared
  Z3_క్రమీకరించు(Z3Context &C, Z3_క్రమీకరించు_2 ZS) : Context(C), క్రమీకరించు_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(క్రమీకరించు_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_క్రమీకరించు(స్థిరరాశి Z3_క్రమీకరించు &Other) : Context(Other.Context), క్రమీకరించు_2(Other.క్రమీకరించు_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(క్రమీకరించు_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_క్రమీకరించు &క్రియాకారకం=(స్థిరరాశి Z3_క్రమీకరించు &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.క్రమీకరించు_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(క్రమీకరించు_2));
    క్రమీకరించు_2 = Other.క్రమీకరించు_2;
    తిరిగి *ఇది;
  }

  Z3_క్రమీకరించు(Z3_క్రమీకరించు &&Other) = తొలగించు;
  Z3_క్రమీకరించు &క్రియాకారకం=(Z3_క్రమీకరించు &&Other) = తొలగించు;

  ~Z3_క్రమీకరించు() {
    ఒకవేళ (క్రమీకరించు_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(క్రమీకరించు_2));
  }

  శూన్య Profile(llvm::Folding_సమితి_కణుపు_ID &ID) స్థిరరాశి override {
    ID.కూడు_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(క్రమీకరించు_2)));
  }

  తార్కిక is_Bitvector_క్రమీకరించు_Impl() స్థిరరాశి override {
    తిరిగి (Z3_get_క్రమీకరించు_రకం(Context.Context, క్రమీకరించు_2) == Z3_BV_క్రమీకరించు);
  }

  తార్కిక is_Float_క్రమీకరించు_Impl() స్థిరరాశి override {
    తిరిగి (Z3_get_క్రమీకరించు_రకం(Context.Context, క్రమీకరించు_2) == Z3_FLOATING_బిందువు_క్రమీకరించు);
  }

  తార్కిక is_Boolean_క్రమీకరించు_Impl() స్థిరరాశి override {
    తిరిగి (Z3_get_క్రమీకరించు_రకం(Context.Context, క్రమీకరించు_2) == Z3_BOOL_క్రమీకరించు);
  }

  గుర్తులేని get_Bitvector_క్రమీకరించు_కొలత_Impl() స్థిరరాశి override {
    తిరిగి Z3_get_bv_క్రమీకరించు_కొలత(Context.Context, క్రమీకరించు_2);
  }

  గుర్తులేని get_Float_క్రమీకరించు_కొలత_Impl() స్థిరరాశి override {
    తిరిగి Z3_fpa_get_ebits(Context.Context, క్రమీకరించు_2) +
           Z3_fpa_get_sbits(Context.Context, క్రమీకరించు_2);
  }

  తార్కిక equal_గమ్యం(SMTSort స్థిరరాశి &Other) స్థిరరాశి override {
    తిరిగి Z3_is_eq_క్రమీకరించు(Context.Context, క్రమీకరించు_2,
                         static_cast<స్థిరరాశి Z3_క్రమీకరించు &>(Other).క్రమీకరించు_2);
  }

  శూన్య print(raw_ostream &OS) స్థిరరాశి override {
    OS << Z3_క్రమీకరించు_గమ్యం_string(Context.Context, క్రమీకరించు_2);
  }
}; // end class Z3Sort

స్థిర స్థిరరాశి Z3_క్రమీకరించు &గమ్యం_Z3_క్రమీకరించు(స్థిరరాశి SMTSort &S) {
  తిరిగి static_cast<స్థిరరాశి Z3_క్రమీకరించు &>(S);
}

తరగతి Z3_సమాసం : బహిరంగ SMTExpr {
  మిత్ర తరగతి Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

బహిరంగ:
  Z3_సమాసం(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_సమాసం(స్థిరరాశి Z3_సమాసం &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_సమాసం &క్రియాకారకం=(స్థిరరాశి Z3_సమాసం &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    తిరిగి *ఇది;
  }

  Z3_సమాసం(Z3_సమాసం &&Other) = తొలగించు;
  Z3_సమాసం &క్రియాకారకం=(Z3_సమాసం &&Other) = తొలగించు;

  ~Z3_సమాసం() {
    ఒకవేళ (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  శూన్య Profile(llvm::Folding_సమితి_కణుపు_ID &ID) స్థిరరాశి override {
    ID.కూడు_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  తార్కిక equal_గమ్యం(SMTExpr స్థిరరాశి &Other) స్థిరరాశి override {
    ప్రతిజ్ఞా(Z3_is_eq_క్రమీకరించు(Context.Context, Z3_get_క్రమీకరించు(Context.Context, AST),
                         Z3_get_క్రమీకరించు(Context.Context,
                                     static_cast<స్థిరరాశి Z3_సమాసం &>(Other).AST)) &&
           "AST's must have the same sort");
    తిరిగి Z3_is_eq_ast(Context.Context, AST,
                        static_cast<స్థిరరాశి Z3_సమాసం &>(Other).AST);
  }

  శూన్య print(raw_ostream &OS) స్థిరరాశి override {
    OS << Z3_ast_గమ్యం_string(Context.Context, AST);
  }
}; // end class Z3Expr

స్థిర స్థిరరాశి Z3_సమాసం &గమ్యం_Z3_సమాసం(స్థిరరాశి SMTExpr &E) {
  తిరిగి static_cast<స్థిరరాశి Z3_సమాసం &>(E);
}

తరగతి Z3Model {
  మిత్ర తరగతి Z3Solver;

  Z3Context &Context;

  Z3_model Model;

బహిరంగ:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(స్థిరరాశి Z3Model &Other) = తొలగించు;
  Z3Model(Z3Model &&Other) = తొలగించు;
  Z3Model &క్రియాకారకం=(Z3Model &Other) = తొలగించు;
  Z3Model &క్రియాకారకం=(Z3Model &&Other) = తొలగించు;

  ~Z3Model() {
    ఒకవేళ (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  శూన్య print(raw_ostream &OS) స్థిరరాశి {
    OS << Z3_model_గమ్యం_string(Context.Context, Model);
  }

  LLVM_DUMP_పద్ధతి శూన్య dump() స్థిరరాశి { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
స్థిర స్థిరరాశి llvm::fltSemantics &getFloatSemantics(గుర్తులేని బిట్టు_వెడల్పు_2) {
  ఎంపిక (బిట్టు_వెడల్పు_2) {
  అప్రమేయ:
    llvm_unreachable("Unsupported floating-point semantics!");
    ఆపు;
  స్థితి 16:
    తిరిగి llvm::APFloat::IEEEhalf();
  స్థితి 32:
    తిరిగి llvm::APFloat::IEEEsingle();
  స్థితి 64:
    తిరిగి llvm::APFloat::IEEEdouble();
  స్థితి 128:
    తిరిగి llvm::APFloat::IEEEquad();
  }
}

// Determine whether two float semantics are equivalent
స్థిర తార్కిక areEquivalent(స్థిరరాశి llvm::fltSemantics &LHS,
                          స్థిరరాశి llvm::fltSemantics &RHS) {
  తిరిగి (llvm::APFloat::semanticsPrecision(LHS) ==
          llvm::APFloat::semanticsPrecision(RHS)) &&
         (llvm::APFloat::semanticsMinExponent(LHS) ==
          llvm::APFloat::semanticsMinExponent(RHS)) &&
         (llvm::APFloat::semanticsMaxExponent(LHS) ==
          llvm::APFloat::semanticsMaxExponent(RHS)) &&
         (llvm::APFloat::semantics_కొలత_లోపల_బిట్లు(LHS) ==
          llvm::APFloat::semantics_కొలత_లోపల_బిట్లు(RHS));
}

తరగతి Z3Solver : బహిరంగ SMTSolver {
  మిత్ర తరగతి Z3ConstraintManager;

  Z3Context Context;

  Z3_solver Solver = [ఇది] {
    Z3_solver S = Z3_mk_simple_solver(Context.Context);
    Z3_solver_inc_ref(Context.Context, S);
    తిరిగి S;
  }();

  Z3_params Params = [ఇది] {
    Z3_params P = Z3_mk_params(Context.Context);
    Z3_params_inc_ref(Context.Context, P);
    తిరిగి P;
  }();

  // Cache Sorts
  std::సమితి<Z3_క్రమీకరించు> CachedSorts;

  // Cache Exprs
  std::సమితి<Z3_సమాసం> CachedExprs;

బహిరంగ:
  Z3Solver() = అప్రమేయ;
  Z3Solver(స్థిరరాశి Z3Solver &Other) = తొలగించు;
  Z3Solver(Z3Solver &&Other) = తొలగించు;
  Z3Solver &క్రియాకారకం=(Z3Solver &Other) = తొలగించు;
  Z3Solver &క్రియాకారకం=(Z3Solver &&Other) = తొలగించు;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  శూన్య కూడు_Constraint(స్థిరరాశి SMTExprRef &Exp) స్థిరరాశి override {
    Z3_solver_assert(Context.Context, Solver, గమ్యం_Z3_సమాసం(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_క్రమీకరించు_Ref(స్థిరరాశి SMTSort &క్రమీకరించు_2) {
    స్వయంచాలక It = CachedSorts.చొప్పించు(గమ్యం_Z3_క్రమీకరించు(క్రమీకరించు_2));
    తిరిగి &(*It.మొదటి_2);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_సమాసం_Ref(స్థిరరాశి SMTExpr &Exp) {
    స్వయంచాలక It = CachedExprs.చొప్పించు(గమ్యం_Z3_సమాసం(Exp));
    తిరిగి &(*It.మొదటి_2);
  }

  SMTSortRef get_Bool_క్రమీకరించు() override {
    తిరిగి new_క్రమీకరించు_Ref(Z3_క్రమీకరించు(Context, Z3_mk_bool_క్రమీకరించు(Context.Context)));
  }

  SMTSortRef get_Bitvector_క్రమీకరించు(గుర్తులేని బిట్టు_వెడల్పు_2) override {
    తిరిగి new_క్రమీకరించు_Ref(
        Z3_క్రమీకరించు(Context, Z3_mk_bv_క్రమీకరించు(Context.Context, బిట్టు_వెడల్పు_2)));
  }

  SMTSortRef get_క్రమీకరించు(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_క్రమీకరించు_Ref(
        Z3_క్రమీకరించు(Context, Z3_get_క్రమీకరించు(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTSortRef get_Float16_క్రమీకరించు() override {
    తిరిగి new_క్రమీకరించు_Ref(Z3_క్రమీకరించు(Context, Z3_mk_fpa_క్రమీకరించు_16(Context.Context)));
  }

  SMTSortRef get_Float32_క్రమీకరించు() override {
    తిరిగి new_క్రమీకరించు_Ref(Z3_క్రమీకరించు(Context, Z3_mk_fpa_క్రమీకరించు_32(Context.Context)));
  }

  SMTSortRef get_Float64_క్రమీకరించు() override {
    తిరిగి new_క్రమీకరించు_Ref(Z3_క్రమీకరించు(Context, Z3_mk_fpa_క్రమీకరించు_64(Context.Context)));
  }

  SMTSortRef get_Float128_క్రమీకరించు() override {
    తిరిగి new_క్రమీకరించు_Ref(Z3_క్రమీకరించు(Context, Z3_mk_fpa_క్రమీకరించు_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvneg(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkBVNot(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvnot(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkNot(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_not(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvadd(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSub(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvsub(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVMul(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvmul(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvsrem(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVURem(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvurem(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvsdiv(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvudiv(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVShl(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvshl(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvashr(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvlshr(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVXor(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvxor(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVOr(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvor(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                   గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvand(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvult(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvslt(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvugt(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvsgt(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVUle(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvule(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSle(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvsle(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVUge(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvuge(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVSge(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_bvsge(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                    గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkAnd(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    Z3_ast వాదనలు[2] = {గమ్యం_Z3_సమాసం(*LHS).AST, గమ్యం_Z3_సమాసం(*RHS).AST};
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(Context, Z3_mk_and(Context.Context, 2, వాదనలు)));
  }

  SMTExprRef mkOr(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    Z3_ast వాదనలు[2] = {గమ్యం_Z3_సమాసం(*LHS).AST, గమ్యం_Z3_సమాసం(*RHS).AST};
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(Context, Z3_mk_or(Context.Context, 2, వాదనలు)));
  }

  SMTExprRef mkEqual(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_eq(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                 గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_neg(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_fpa_is_infinite(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_is_nan(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_సాధారణ(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_fpa_is_సాధారణ(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_fpa_is_zero(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkFPMul(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context,
               Z3_mk_fpa_mul(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                             గమ్యం_Z3_సమాసం(*LHS).AST, గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context,
               Z3_mk_fpa_div(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                             గమ్యం_Z3_సమాసం(*LHS).AST, గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPRem(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_rem(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                      గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context,
               Z3_mk_fpa_కూడు(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                             గమ్యం_Z3_సమాసం(*LHS).AST, గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPSub(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context,
               Z3_mk_fpa_sub(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                             గమ్యం_Z3_సమాసం(*LHS).AST, గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPLt(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_lt(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPGt(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_gt(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPLe(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_leq(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                      గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPGe(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_geq(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                      గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_fpa_eq(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkIte(స్థిరరాశి SMTExprRef &Cond, స్థిరరాశి SMTExprRef &T,
                   స్థిరరాశి SMTExprRef &F) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_ite(Context.Context, గమ్యం_Z3_సమాసం(*Cond).AST,
                                  గమ్యం_Z3_సమాసం(*T).AST, గమ్యం_Z3_సమాసం(*F).AST)));
  }

  SMTExprRef mkBVSignExt(గుర్తులేని i, స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_sign_ext(Context.Context, i, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(గుర్తులేని i, స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_zero_ext(Context.Context, i, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(గుర్తులేని ఎగువ_2, గుర్తులేని దిగువ_2,
                         స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(Context, Z3_mk_వెలికితీయి(Context.Context, ఎగువ_2, దిగువ_2,
                                                    గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_అధికప్రవాహం(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS,
                               తార్కిక isSigned) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvadd_no_అధికప్రవాహం(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                         గమ్యం_Z3_సమాసం(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(స్థిరరాశి SMTExprRef &LHS,
                                స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                          గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_అధికప్రవాహం(స్థిరరాశి SMTExprRef &LHS,
                               స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvsub_no_అధికప్రవాహం(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                         గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS,
                                తార్కిక isSigned) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                          గమ్యం_Z3_సమాసం(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_అధికప్రవాహం(స్థిరరాశి SMTExprRef &LHS,
                                స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvsdiv_no_అధికప్రవాహం(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                          గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_అధికప్రవాహం(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvneg_no_అధికప్రవాహం(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_అధికప్రవాహం(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS,
                               తార్కిక isSigned) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvmul_no_అధికప్రవాహం(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                         గమ్యం_Z3_సమాసం(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(స్థిరరాశి SMTExprRef &LHS,
                                స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                          గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(స్థిరరాశి SMTExprRef &LHS, స్థిరరాశి SMTExprRef &RHS) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_concat(Context.Context, గమ్యం_Z3_సమాసం(*LHS).AST,
                                     గమ్యం_Z3_సమాసం(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(స్థిరరాశి SMTExprRef &From, స్థిరరాశి SMTSortRef &గమ్యం) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context,
        Z3_mk_fpa_గమ్యం_fp_float(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                              గమ్యం_Z3_సమాసం(*From).AST, గమ్యం_Z3_క్రమీకరించు(*గమ్యం).క్రమీకరించు_2)));
  }

  SMTExprRef mkSBVtoFP(స్థిరరాశి SMTExprRef &From, స్థిరరాశి SMTSortRef &గమ్యం) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context,
        Z3_mk_fpa_గమ్యం_fp_signed(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                               గమ్యం_Z3_సమాసం(*From).AST, గమ్యం_Z3_క్రమీకరించు(*గమ్యం).క్రమీకరించు_2)));
  }

  SMTExprRef mkUBVtoFP(స్థిరరాశి SMTExprRef &From, స్థిరరాశి SMTSortRef &గమ్యం) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context,
        Z3_mk_fpa_గమ్యం_fp_unsigned(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                                 గమ్యం_Z3_సమాసం(*From).AST, గమ్యం_Z3_క్రమీకరించు(*గమ్యం).క్రమీకరించు_2)));
  }

  SMTExprRef mkFPtoSBV(స్థిరరాశి SMTExprRef &From, గుర్తులేని గమ్యం_వెడల్పు) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_fpa_గమ్యం_sbv(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                                  గమ్యం_Z3_సమాసం(*From).AST, గమ్యం_వెడల్పు)));
  }

  SMTExprRef mkFPtoUBV(స్థిరరాశి SMTExprRef &From, గుర్తులేని గమ్యం_వెడల్పు) override {
    SMTExprRef Rounding_బహుళకం = get_Float_Rounding_బహుళకం();
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_fpa_గమ్యం_ubv(Context.Context, గమ్యం_Z3_సమాసం(*Rounding_బహుళకం).AST,
                                  గమ్యం_Z3_సమాసం(*From).AST, గమ్యం_వెడల్పు)));
  }

  SMTExprRef mkBoolean(స్థిరరాశి తార్కిక b) override {
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(స్థిరరాశి llvm::APSInt Int, గుర్తులేని బిట్టు_వెడల్పు_2) override {
    స్థిరరాశి Z3_క్రమీకరించు_2 Z3_క్రమీకరించు = గమ్యం_Z3_క్రమీకరించు(*get_Bitvector_క్రమీకరించు(బిట్టు_వెడల్పు_2)).క్రమీకరించు_2;

    // Slow path, when 64 bits are not enough.
    ఒకవేళ (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> బఫరు;
      Int.గమ్యం_String(బఫరు, 10);
      తిరిగి new_సమాసం_Ref(Z3_సమాసం(
          Context, Z3_mk_numeral(Context.Context, బఫరు.c_str(), Z3_క్రమీకరించు)));
    }

    స్థిరరాశి int64_t బిట్టు_Repr_As_Signed = Int.get_Ext_విలువ();
    స్థిరరాశి uint64_t బిట్టు_Repr_As_Unsigned =
        reinterpret_cast<స్థిరరాశి uint64_t &>(బిట్టు_Repr_As_Signed);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, బిట్టు_Repr_As_Signed, Z3_క్రమీకరించు)
            : Z3_mk_unsigned_int64(Context.Context, బిట్టు_Repr_As_Unsigned, Z3_క్రమీకరించు);
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(Context, Literal));
  }

  SMTExprRef mkFloat(స్థిరరాశి llvm::APFloat Float) override {
    SMTSortRef క్రమీకరించు_2 =
        get_Float_క్రమీకరించు(llvm::APFloat::semantics_కొలత_లోపల_బిట్లు(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_గమ్యం_APInt(), అబద్ధం);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_బిట్టు_వెడల్పు());
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(
        Context, Z3_mk_fpa_గమ్యం_fp_bv(Context.Context, గమ్యం_Z3_సమాసం(*Z3Int).AST,
                                    గమ్యం_Z3_క్రమీకరించు(*క్రమీకరించు_2).క్రమీకరించు_2)));
  }

  SMTExprRef mk_చిహ్నం(స్థిరరాశి అక్షరం *Name, SMTSortRef క్రమీకరించు_2) override {
    తిరిగి new_సమాసం_Ref(
        Z3_సమాసం(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_చిహ్నం(Context.Context, Name),
                                    గమ్యం_Z3_క్రమీకరించు(*క్రమీకరించు_2).క్రమీకరించు_2)));
  }

  llvm::APSInt getBitvector(స్థిరరాశి SMTExprRef &Exp, గుర్తులేని బిట్టు_వెడల్పు_2,
                            తార్కిక isUnsigned) override {
    తిరిగి llvm::APSInt(
        llvm::APInt(బిట్టు_వెడల్పు_2,
                    Z3_get_numeral_string(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST),
                    10),
        isUnsigned);
  }

  తార్కిక getBoolean(స్థిరరాశి SMTExprRef &Exp) override {
    తిరిగి Z3_get_bool_విలువ(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_బహుళకం() override {
    // TODO: Don't assume nearest ties to even rounding mode
    తిరిగి new_సమాసం_Ref(Z3_సమాసం(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  తార్కిక గమ్యం_APFloat(స్థిరరాశి SMTSortRef &క్రమీకరించు_2, స్థిరరాశి SMTExprRef &AST,
                 llvm::APFloat &Float, తార్కిక useSemantics) {
    ప్రతిజ్ఞా(క్రమీకరించు_2->is_Float_క్రమీకరించు() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(క్రమీకరించు_2->get_Float_క్రమీకరించు_కొలత(), నిజం);
    స్థిరరాశి llvm::fltSemantics &Semantics =
        getFloatSemantics(క్రమీకరించు_2->get_Float_క్రమీకరించు_కొలత());
    SMTSortRef BVSort = get_Bitvector_క్రమీకరించు(క్రమీకరించు_2->get_Float_క్రమీకరించు_కొలత());
    ఒకవేళ (!గమ్యం_APSInt(BVSort, AST, Int, నిజం)) {
      తిరిగి అబద్ధం;
    }

    ఒకవేళ (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      ప్రతిజ్ఞా(అబద్ధం && "Floating-point types don't match!");
      తిరిగి అబద్ధం;
    }

    Float = llvm::APFloat(Semantics, Int);
    తిరిగి నిజం;
  }

  తార్కిక గమ్యం_APSInt(స్థిరరాశి SMTSortRef &క్రమీకరించు_2, స్థిరరాశి SMTExprRef &AST,
                llvm::APSInt &Int, తార్కిక useSemantics) {
    ఒకవేళ (క్రమీకరించు_2->is_Bitvector_క్రమీకరించు()) {
      ఒకవేళ (useSemantics && Int.get_బిట్టు_వెడల్పు() != క్రమీకరించు_2->get_Bitvector_క్రమీకరించు_కొలత()) {
        ప్రతిజ్ఞా(అబద్ధం && "Bitvector types don't match!");
        తిరిగి అబద్ధం;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_బిట్టు_వెడల్పు(), Int.isUnsigned());
      తిరిగి నిజం;
    }

    ఒకవేళ (క్రమీకరించు_2->is_Boolean_క్రమీకరించు()) {
      ఒకవేళ (useSemantics && Int.get_బిట్టు_వెడల్పు() < 1) {
        ప్రతిజ్ఞా(అబద్ధం && "Boolean type doesn't match!");
        తిరిగి అబద్ధం;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_బిట్టు_వెడల్పు(), getBoolean(AST)),
                         Int.isUnsigned());
      తిరిగి నిజం;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  తార్కిక getInterpretation(స్థిరరాశి SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_గమ్యం_app(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST));
    ఒకవేళ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      తిరిగి అబద్ధం;

    SMTExprRef కేటాయించు_2 = new_సమాసం_Ref(
        Z3_సమాసం(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef క్రమీకరించు_2 = get_క్రమీకరించు(కేటాయించు_2);
    తిరిగి గమ్యం_APSInt(క్రమీకరించు_2, కేటాయించు_2, Int, నిజం);
  }

  తార్కిక getInterpretation(స్థిరరాశి SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_గమ్యం_app(Context.Context, గమ్యం_Z3_సమాసం(*Exp).AST));
    ఒకవేళ (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      తిరిగి అబద్ధం;

    SMTExprRef కేటాయించు_2 = new_సమాసం_Ref(
        Z3_సమాసం(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef క్రమీకరించు_2 = get_క్రమీకరించు(కేటాయించు_2);
    తిరిగి గమ్యం_APFloat(క్రమీకరించు_2, కేటాయించు_2, Float, నిజం);
  }

  std::optional<తార్కిక> check() స్థిరరాశి override {
    Z3_solver_సమితి_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    ఒకవేళ (res == Z3_L_TRUE)
      తిరిగి నిజం;

    ఒకవేళ (res == Z3_L_FALSE)
      తిరిగి అబద్ధం;

    తిరిగి std::nullopt;
  }

  శూన్య నెట్టు() override { తిరిగి Z3_solver_నెట్టు(Context.Context, Solver); }

  శూన్య తీయి(గుర్తులేని అంకె_స్థితులు = 1) override {
    ప్రతిజ్ఞా(Z3_solver_get_అంకె_scopes(Context.Context, Solver) >= అంకె_స్థితులు);
    తిరిగి Z3_solver_తీయి(Context.Context, Solver, అంకె_స్థితులు);
  }

  తార్కిక isFPSupported() override { తిరిగి నిజం; }

  /// Reset the solver and remove all constraints.
  శూన్య reset() override { Z3_solver_reset(Context.Context, Solver); }

  శూన్య print(raw_ostream &OS) స్థిరరాశి override {
    OS << Z3_solver_గమ్యం_string(Context.Context, Solver);
  }

  శూన్య సమితి_Unsigned_Param(StringRef కీలకం, గుర్తులేని విలువ) override {
    Z3_చిహ్నం Sym = Z3_mk_string_చిహ్నం(Context.Context, కీలకం.str().c_str());
    Z3_params_సమితి_uint(Context.Context, Params, Sym, విలువ);
  }

  శూన్య సమితి_Bool_Param(StringRef కీలకం, తార్కిక విలువ) override {
    Z3_చిహ్నం Sym = Z3_mk_string_చిహ్నం(Context.Context, కీలకం.str().c_str());
    Z3_params_సమితి_bool(Context.Context, Params, Sym, విలువ);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() స్థిరరాశి override;
}; // end class Z3Solver

తరగతి Z3Statistics final : బహిరంగ SMTSolverStatistics {
బహిరంగ:
  ద్విగుణ getDouble(StringRef కీలకం) స్థిరరాశి override {
    స్వయంచాలక It = Double_విలువలు.కనుగొను(కీలకం.str());
    ప్రతిజ్ఞా(It != Double_విలువలు.చివర());
    తిరిగి It->రెండవ_2;
  };
  గుర్తులేని getUnsigned(StringRef కీలకం) స్థిరరాశి override {
    స్వయంచాలక It = Unsigned_విలువలు.కనుగొను(కీలకం.str());
    ఒకవేళ (It == Unsigned_విలువలు.చివర())
      తిరిగి 0;
    తిరిగి It->రెండవ_2;
  };

  శూన్య print(raw_ostream &OS) స్థిరరాశి override {
    కొరకు (స్థిరరాశి స్వయంచాలక &E : Unsigned_విలువలు)
      OS << E.మొదటి_2() << ": " << E.రెండవ_2 << '\n';
    కొరకు (స్థిరరాశి స్వయంచాలక &E : Double_విలువలు) {
      write_double(OS << E.మొదటి_2() << ": ", E.రెండవ_2, FloatStyle::Fixed);
      OS << '\n';
    }
  }

అంతరంగ:
  మిత్ర తరగతి Z3Solver;
  StringMap<గుర్తులేని> Unsigned_విలువలు;
  StringMap<ద్విగుణ> Double_విలువలు;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() స్థిరరాశి {
  స్వయంచాలక స్థిరరాశి &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics ఫలితం;

  గుర్తులేని అంకె_కీలకాలు = Z3_stats_కొలత(C, S);
  కొరకు (గుర్తులేని Idx = 0; Idx < అంకె_కీలకాలు; ++Idx) {
    స్థిరరాశి అక్షరం *కీలకం = Z3_stats_get_కీలకం(C, S, Idx);
    ఒకవేళ (Z3_stats_is_uint(C, S, Idx)) {
      స్వయంచాలక విలువ = Z3_stats_get_uint_విలువ(C, S, Idx);
      ఫలితం.Unsigned_విలువలు.try_emplace(కీలకం, విలువ);
    } లేకపోతే {
      ప్రతిజ్ఞా(Z3_stats_is_double(C, S, Idx));
      స్వయంచాలక విలువ = Z3_stats_get_double_విలువ(C, S, Idx);
      ఫలితం.Double_విలువలు.try_emplace(కీలకం, విలువ);
    }
  }
  తిరిగి std::make_unique<Z3Statistics>(std::move(ఫలితం));
}

} // end anonymous namespace

#అయితే_ముగింపు

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#ఒకవేళ LLVM_WITH_Z3
  తిరిగి std::make_unique<Z3Solver>();
#లేకపోతే
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           అబద్ధం);
  తిరిగి శూన్యసూచిక;
#అయితే_ముగింపు
}

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య SMTSort::dump() స్థిరరాశి { print(llvm::errs()); }
LLVM_DUMP_పద్ధతి శూన్య SMTExpr::dump() స్థిరరాశి { print(llvm::errs()); }
LLVM_DUMP_పద్ధతి శూన్య SMTSolver::dump() స్థిరరాశి { print(llvm::errs()); }
LLVM_DUMP_పద్ధతి శూన్య SMTSolverStatistics::dump() స్థిరరాశి { print(llvm::errs()); }
#అయితే_ముగింపు
