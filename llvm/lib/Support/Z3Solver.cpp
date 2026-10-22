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

उपयुज्य नामाकाश llvm;

#यदि LLVM_WITH_Z3

#अन्तर्भाव "llvm/ADT/SmallString.h"
#अन्तर्भाव "llvm/ADT/StringMap.h"
#अन्तर्भाव "llvm/ADT/Twine.h"

#अन्तर्भाव <set>

#अन्तर्भाव <z3.h>

नामाकाश {

/// Configuration class for Z3
वर्ग Z3Config {
  मित्रम् वर्ग Z3Context;

  Z3_config Config = Z3_mk_config();

सार्वजनिक:
  Z3Config() = स्वभाव;
  Z3Config(ध्रुव Z3Config &) = विलोपय;
  Z3Config(Z3Config &&) = स्वभाव;
  Z3Config &संक्रियकः=(Z3Config &) = विलोपय;
  Z3Config &संक्रियकः=(Z3Config &&) = स्वभाव;
  ~Z3Config() { Z3_del_config(Config); }
}; // end class Z3Config

// Function used to report errors
शून्य Z3ErrorHandler(Z3_context Context, Z3_error_संहिता Error) {
  llvm::report_fatal_error("Z3 error: " +
                           llvm::Twine(Z3_get_error_सन्देशः(Context, Error)));
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
    Z3_समुच्चयः_error_handler(Context, Z3ErrorHandler);
  }

  Z3Context(ध्रुव Z3Context &) = विलोपय;
  Z3Context(Z3Context &&) = स्वभाव;
  Z3Context &संक्रियकः=(Z3Context &) = विलोपय;
  Z3Context &संक्रियकः=(Z3Context &&) = स्वभाव;

  ~Z3Context() {
    Z3_del_context(Context);
    Context = शून्यसूचकः;
  }
}; // end class Z3Context

/// Wrapper for Z3 Sort
वर्ग Z3_क्रमय : सार्वजनिक SMTSort {
  मित्रम् वर्ग Z3Solver;

  Z3Context &Context;

  Z3_क्रमय_2 क्रमय_2;

सार्वजनिक:
  /// Default constructor, mainly used by make_shared
  Z3_क्रमय(Z3Context &C, Z3_क्रमय_2 ZS) : Context(C), क्रमय_2(ZS) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(क्रमय_2));
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_क्रमय(ध्रुव Z3_क्रमय &Other) : Context(Other.Context), क्रमय_2(Other.क्रमय_2) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(क्रमय_2));
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_क्रमय &संक्रियकः=(ध्रुव Z3_क्रमय &Other) {
    Z3_inc_ref(Context.Context, reinterpret_cast<Z3_ast>(Other.क्रमय_2));
    Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(क्रमय_2));
    क्रमय_2 = Other.क्रमय_2;
    प्रत्यागम *एतत्;
  }

  Z3_क्रमय(Z3_क्रमय &&Other) = विलोपय;
  Z3_क्रमय &संक्रियकः=(Z3_क्रमय &&Other) = विलोपय;

  ~Z3_क्रमय() {
    यदि (क्रमय_2)
      Z3_dec_ref(Context.Context, reinterpret_cast<Z3_ast>(क्रमय_2));
  }

  शून्य Profile(llvm::Folding_समुच्चयः_ग्रन्थिः_ID &ID) ध्रुव override {
    ID.सङ्कलय_Integer(
        Z3_get_ast_id(Context.Context, reinterpret_cast<Z3_ast>(क्रमय_2)));
  }

  तार्किक is_Bitvector_क्रमय_Impl() ध्रुव override {
    प्रत्यागम (Z3_get_क्रमय_विधा(Context.Context, क्रमय_2) == Z3_BV_क्रमय);
  }

  तार्किक is_Float_क्रमय_Impl() ध्रुव override {
    प्रत्यागम (Z3_get_क्रमय_विधा(Context.Context, क्रमय_2) == Z3_FLOATING_POINT_क्रमय);
  }

  तार्किक is_Boolean_क्रमय_Impl() ध्रुव override {
    प्रत्यागम (Z3_get_क्रमय_विधा(Context.Context, क्रमय_2) == Z3_BOOL_क्रमय);
  }

  अचिह्नित get_Bitvector_क्रमय_आकार_Impl() ध्रुव override {
    प्रत्यागम Z3_get_bv_क्रमय_आकार(Context.Context, क्रमय_2);
  }

  अचिह्नित get_Float_क्रमय_आकार_Impl() ध्रुव override {
    प्रत्यागम Z3_fpa_get_ebits(Context.Context, क्रमय_2) +
           Z3_fpa_get_sbits(Context.Context, क्रमय_2);
  }

  तार्किक equal_गन्तव्यम्(SMTSort ध्रुव &Other) ध्रुव override {
    प्रत्यागम Z3_is_eq_क्रमय(Context.Context, क्रमय_2,
                         static_cast<ध्रुव Z3_क्रमय &>(Other).क्रमय_2);
  }

  शून्य print(raw_ostream &OS) ध्रुव override {
    OS << Z3_क्रमय_गन्तव्यम्_string(Context.Context, क्रमय_2);
  }
}; // end class Z3Sort

स्थिर ध्रुव Z3_क्रमय &गन्तव्यम्_Z3_क्रमय(ध्रुव SMTSort &S) {
  प्रत्यागम static_cast<ध्रुव Z3_क्रमय &>(S);
}

वर्ग Z3_व्यञ्जकम् : सार्वजनिक SMTExpr {
  मित्रम् वर्ग Z3Solver;

  Z3Context &Context;

  Z3_ast AST;

सार्वजनिक:
  Z3_व्यञ्जकम्(Z3Context &C, Z3_ast ZA) : SMTExpr(), Context(C), AST(ZA) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy constructor for correct reference counting.
  Z3_व्यञ्जकम्(ध्रुव Z3_व्यञ्जकम् &Copy) : SMTExpr(), Context(Copy.Context), AST(Copy.AST) {
    Z3_inc_ref(Context.Context, AST);
  }

  /// Override implicit copy assignment constructor for correct reference
  /// counting.
  Z3_व्यञ्जकम् &संक्रियकः=(ध्रुव Z3_व्यञ्जकम् &Other) {
    Z3_inc_ref(Context.Context, Other.AST);
    Z3_dec_ref(Context.Context, AST);
    AST = Other.AST;
    प्रत्यागम *एतत्;
  }

  Z3_व्यञ्जकम्(Z3_व्यञ्जकम् &&Other) = विलोपय;
  Z3_व्यञ्जकम् &संक्रियकः=(Z3_व्यञ्जकम् &&Other) = विलोपय;

  ~Z3_व्यञ्जकम्() {
    यदि (AST)
      Z3_dec_ref(Context.Context, AST);
  }

  शून्य Profile(llvm::Folding_समुच्चयः_ग्रन्थिः_ID &ID) ध्रुव override {
    ID.सङ्कलय_Integer(Z3_get_ast_id(Context.Context, AST));
  }

  /// Comparison of AST equality, not model equivalence.
  तार्किक equal_गन्तव्यम्(SMTExpr ध्रुव &Other) ध्रुव override {
    प्रतिज्ञा(Z3_is_eq_क्रमय(Context.Context, Z3_get_क्रमय(Context.Context, AST),
                         Z3_get_क्रमय(Context.Context,
                                     static_cast<ध्रुव Z3_व्यञ्जकम् &>(Other).AST)) &&
           "AST's must have the same sort");
    प्रत्यागम Z3_is_eq_ast(Context.Context, AST,
                        static_cast<ध्रुव Z3_व्यञ्जकम् &>(Other).AST);
  }

  शून्य print(raw_ostream &OS) ध्रुव override {
    OS << Z3_ast_गन्तव्यम्_string(Context.Context, AST);
  }
}; // end class Z3Expr

स्थिर ध्रुव Z3_व्यञ्जकम् &गन्तव्यम्_Z3_व्यञ्जकम्(ध्रुव SMTExpr &E) {
  प्रत्यागम static_cast<ध्रुव Z3_व्यञ्जकम् &>(E);
}

वर्ग Z3Model {
  मित्रम् वर्ग Z3Solver;

  Z3Context &Context;

  Z3_model Model;

सार्वजनिक:
  Z3Model(Z3Context &C, Z3_model ZM) : Context(C), Model(ZM) {
    Z3_model_inc_ref(Context.Context, Model);
  }

  Z3Model(ध्रुव Z3Model &Other) = विलोपय;
  Z3Model(Z3Model &&Other) = विलोपय;
  Z3Model &संक्रियकः=(Z3Model &Other) = विलोपय;
  Z3Model &संक्रियकः=(Z3Model &&Other) = विलोपय;

  ~Z3Model() {
    यदि (Model)
      Z3_model_dec_ref(Context.Context, Model);
  }

  शून्य print(raw_ostream &OS) ध्रुव {
    OS << Z3_model_गन्तव्यम्_string(Context.Context, Model);
  }

  LLVM_DUMP_विधानम् शून्य dump() ध्रुव { print(llvm::errs()); }
}; // end class Z3Model

/// Get the corresponding IEEE floating-point type for a given bitwidth.
स्थिर ध्रुव llvm::fltSemantics &getFloatSemantics(अचिह्नित Bit_विस्तारः) {
  विकल्प (Bit_विस्तारः) {
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
         (llvm::APFloat::semantics_आकार_अन्तर्_द्वयङ्काः(LHS) ==
          llvm::APFloat::semantics_आकार_अन्तर्_द्वयङ्काः(RHS));
}

वर्ग Z3Solver : सार्वजनिक SMTSolver {
  मित्रम् वर्ग Z3ConstraintManager;

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
  std::समुच्चयः<Z3_क्रमय> CachedSorts;

  // Cache Exprs
  std::समुच्चयः<Z3_व्यञ्जकम्> CachedExprs;

सार्वजनिक:
  Z3Solver() = स्वभाव;
  Z3Solver(ध्रुव Z3Solver &Other) = विलोपय;
  Z3Solver(Z3Solver &&Other) = विलोपय;
  Z3Solver &संक्रियकः=(Z3Solver &Other) = विलोपय;
  Z3Solver &संक्रियकः=(Z3Solver &&Other) = विलोपय;

  ~Z3Solver() override {
    Z3_params_dec_ref(Context.Context, Params);
    Z3_solver_dec_ref(Context.Context, Solver);
  }

  शून्य सङ्कलय_Constraint(ध्रुव SMTExprRef &Exp) ध्रुव override {
    Z3_solver_assert(Context.Context, Solver, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST);
  }

  // Given an SMTSort, adds/retrives it from the cache and returns
  // an SMTSortRef to the SMTSort in the cache
  SMTSortRef new_क्रमय_Ref(ध्रुव SMTSort &क्रमय_2) {
    स्वचालित It = CachedSorts.निवेशय(गन्तव्यम्_Z3_क्रमय(क्रमय_2));
    प्रत्यागम &(*It.first);
  }

  // Given an SMTExpr, adds/retrives it from the cache and returns
  // an SMTExprRef to the SMTExpr in the cache
  SMTExprRef new_व्यञ्जकम्_Ref(ध्रुव SMTExpr &Exp) {
    स्वचालित It = CachedExprs.निवेशय(गन्तव्यम्_Z3_व्यञ्जकम्(Exp));
    प्रत्यागम &(*It.first);
  }

  SMTSortRef get_Bool_क्रमय() override {
    प्रत्यागम new_क्रमय_Ref(Z3_क्रमय(Context, Z3_mk_bool_क्रमय(Context.Context)));
  }

  SMTSortRef get_Bitvector_क्रमय(अचिह्नित Bit_विस्तारः) override {
    प्रत्यागम new_क्रमय_Ref(
        Z3_क्रमय(Context, Z3_mk_bv_क्रमय(Context.Context, Bit_विस्तारः)));
  }

  SMTSortRef get_क्रमय(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_क्रमय_Ref(
        Z3_क्रमय(Context, Z3_get_क्रमय(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTSortRef get_Float16_क्रमय() override {
    प्रत्यागम new_क्रमय_Ref(Z3_क्रमय(Context, Z3_mk_fpa_क्रमय_16(Context.Context)));
  }

  SMTSortRef get_Float32_क्रमय() override {
    प्रत्यागम new_क्रमय_Ref(Z3_क्रमय(Context, Z3_mk_fpa_क्रमय_32(Context.Context)));
  }

  SMTSortRef get_Float64_क्रमय() override {
    प्रत्यागम new_क्रमय_Ref(Z3_क्रमय(Context, Z3_mk_fpa_क्रमय_64(Context.Context)));
  }

  SMTSortRef get_Float128_क्रमय() override {
    प्रत्यागम new_क्रमय_Ref(Z3_क्रमय(Context, Z3_mk_fpa_क्रमय_128(Context.Context)));
  }

  SMTExprRef mkBVNeg(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvneg(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkBVNot(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvnot(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkNot(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_not(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkBVAdd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvadd(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSub(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvsub(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVMul(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvmul(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSRem(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvsrem(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVURem(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvurem(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSDiv(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvsdiv(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVUDiv(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvudiv(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVShl(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvshl(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVAshr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvashr(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVLshr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvlshr(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVXor(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvxor(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVOr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvor(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                   गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVAnd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvand(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVUlt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvult(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSlt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvslt(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVUgt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvugt(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSgt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvsgt(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVUle(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvule(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSle(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvsle(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVUge(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvuge(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVSge(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_bvsge(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                    गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkAnd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    Z3_ast तर्काः[2] = {गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST};
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(Context, Z3_mk_and(Context.Context, 2, तर्काः)));
  }

  SMTExprRef mkOr(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    Z3_ast तर्काः[2] = {गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST};
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(Context, Z3_mk_or(Context.Context, 2, तर्काः)));
  }

  SMTExprRef mkEqual(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_eq(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                 गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPNeg(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_neg(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkFPIsInfinite(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_fpa_is_infinite(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkFPIsNaN(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_is_nan(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mk_FPIs_प्रसामान्यम्(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_fpa_is_प्रसामान्यम्(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkFPIsZero(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_fpa_is_zero(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkFPMul(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context,
               Z3_mk_fpa_mul(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                             गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPDiv(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context,
               Z3_mk_fpa_div(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                             गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPRem(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_rem(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                      गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPAdd(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context,
               Z3_mk_fpa_सङ्कलय(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                             गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPSub(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context,
               Z3_mk_fpa_sub(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                             गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPLt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_lt(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPGt(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_gt(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPLe(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_leq(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                      गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPGe(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_geq(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                      गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPEqual(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_fpa_eq(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkIte(ध्रुव SMTExprRef &Cond, ध्रुव SMTExprRef &T,
                   ध्रुव SMTExprRef &F) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_ite(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Cond).AST,
                                  गन्तव्यम्_Z3_व्यञ्जकम्(*T).AST, गन्तव्यम्_Z3_व्यञ्जकम्(*F).AST)));
  }

  SMTExprRef mkBVSignExt(अचिह्नित i, ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_sign_ext(Context.Context, i, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkBVZeroExt(अचिह्नित i, ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_zero_ext(Context.Context, i, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  SMTExprRef mkBVExtract(अचिह्नित उच्चः_2, अचिह्नित नीचः_2,
                         ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(Context, Z3_mk_extract(Context.Context, उच्चः_2, नीचः_2,
                                                    गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector addition
  /// operation
  SMTExprRef mk_BVAdd_No_अतिप्रवाहः(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS,
                               तार्किक isSigned) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvadd_no_अतिप्रवाहः(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                         गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// addition operation
  SMTExprRef mkBVAddNoUnderflow(ध्रुव SMTExprRef &LHS,
                                ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvadd_no_underflow(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                          गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// subtraction operation
  SMTExprRef mk_BVSub_No_अतिप्रवाहः(ध्रुव SMTExprRef &LHS,
                               ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvsub_no_अतिप्रवाहः(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                         गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  /// Creates a predicate that checks for underflow in a bitvector subtraction
  /// operation
  SMTExprRef mkBVSubNoUnderflow(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS,
                                तार्किक isSigned) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvsub_no_underflow(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                          गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for overflow in a signed bitvector
  /// division/modulus operation
  SMTExprRef mk_BVSDiv_No_अतिप्रवाहः(ध्रुव SMTExprRef &LHS,
                                ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvsdiv_no_अतिप्रवाहः(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                          गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector negation
  /// operation
  SMTExprRef mk_BVNeg_No_अतिप्रवाहः(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvneg_no_अतिप्रवाहः(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST)));
  }

  /// Creates a predicate that checks for overflow in a bitvector multiplication
  /// operation
  SMTExprRef mk_BVMul_No_अतिप्रवाहः(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS,
                               तार्किक isSigned) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvmul_no_अतिप्रवाहः(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                         गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST, isSigned)));
  }

  /// Creates a predicate that checks for underflow in a signed bitvector
  /// multiplication operation
  SMTExprRef mkBVMulNoUnderflow(ध्रुव SMTExprRef &LHS,
                                ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_bvmul_no_underflow(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                          गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkBVConcat(ध्रुव SMTExprRef &LHS, ध्रुव SMTExprRef &RHS) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_concat(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*LHS).AST,
                                     गन्तव्यम्_Z3_व्यञ्जकम्(*RHS).AST)));
  }

  SMTExprRef mkFPtoFP(ध्रुव SMTExprRef &From, ध्रुव SMTSortRef &गन्तव्यम्) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context,
        Z3_mk_fpa_गन्तव्यम्_fp_float(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                              गन्तव्यम्_Z3_व्यञ्जकम्(*From).AST, गन्तव्यम्_Z3_क्रमय(*गन्तव्यम्).क्रमय_2)));
  }

  SMTExprRef mkSBVtoFP(ध्रुव SMTExprRef &From, ध्रुव SMTSortRef &गन्तव्यम्) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context,
        Z3_mk_fpa_गन्तव्यम्_fp_signed(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                               गन्तव्यम्_Z3_व्यञ्जकम्(*From).AST, गन्तव्यम्_Z3_क्रमय(*गन्तव्यम्).क्रमय_2)));
  }

  SMTExprRef mkUBVtoFP(ध्रुव SMTExprRef &From, ध्रुव SMTSortRef &गन्तव्यम्) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context,
        Z3_mk_fpa_गन्तव्यम्_fp_unsigned(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                                 गन्तव्यम्_Z3_व्यञ्जकम्(*From).AST, गन्तव्यम्_Z3_क्रमय(*गन्तव्यम्).क्रमय_2)));
  }

  SMTExprRef mkFPtoSBV(ध्रुव SMTExprRef &From, अचिह्नित गन्तव्यम्_विस्तारः) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_fpa_गन्तव्यम्_sbv(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                                  गन्तव्यम्_Z3_व्यञ्जकम्(*From).AST, गन्तव्यम्_विस्तारः)));
  }

  SMTExprRef mkFPtoUBV(ध्रुव SMTExprRef &From, अचिह्नित गन्तव्यम्_विस्तारः) override {
    SMTExprRef Rounding_बहुलकम् = get_Float_Rounding_बहुलकम्();
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_fpa_गन्तव्यम्_ubv(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Rounding_बहुलकम्).AST,
                                  गन्तव्यम्_Z3_व्यञ्जकम्(*From).AST, गन्तव्यम्_विस्तारः)));
  }

  SMTExprRef mkBoolean(ध्रुव तार्किक b) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(Context, b ? Z3_mk_true(Context.Context)
                                        : Z3_mk_false(Context.Context)));
  }

  SMTExprRef mkBitvector(ध्रुव llvm::APSInt Int, अचिह्नित Bit_विस्तारः) override {
    ध्रुव Z3_क्रमय_2 Z3_क्रमय = गन्तव्यम्_Z3_क्रमय(*get_Bitvector_क्रमय(Bit_विस्तारः)).क्रमय_2;

    // Slow path, when 64 bits are not enough.
    यदि (LLVM_UNLIKELY(!Int.isRepresentableByInt64())) {
      SmallString<40> बफर;
      Int.गन्तव्यम्_String(बफर, 10);
      प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
          Context, Z3_mk_numeral(Context.Context, बफर.c_str(), Z3_क्रमय)));
    }

    ध्रुव int64_t BitReprAsSigned = Int.get_Ext_मानम्();
    ध्रुव uint64_t BitReprAsUnsigned =
        reinterpret_cast<ध्रुव uint64_t &>(BitReprAsSigned);

    Z3_ast Literal =
        Int.isSigned()
            ? Z3_mk_int64(Context.Context, BitReprAsSigned, Z3_क्रमय)
            : Z3_mk_unsigned_int64(Context.Context, BitReprAsUnsigned, Z3_क्रमय);
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(Context, Literal));
  }

  SMTExprRef mkFloat(ध्रुव llvm::APFloat Float) override {
    SMTSortRef क्रमय_2 =
        get_Float_क्रमय(llvm::APFloat::semantics_आकार_अन्तर्_द्वयङ्काः(Float.getSemantics()));

    llvm::APSInt Int = llvm::APSInt(Float.bitcast_गन्तव्यम्_APInt(), असत्य);
    SMTExprRef Z3Int = mkBitvector(Int, Int.get_Bit_विस्तारः());
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(
        Context, Z3_mk_fpa_गन्तव्यम्_fp_bv(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Z3Int).AST,
                                    गन्तव्यम्_Z3_क्रमय(*क्रमय_2).क्रमय_2)));
  }

  SMTExprRef mk_चिह्नम्(ध्रुव वर्ण *Name, SMTSortRef क्रमय_2) override {
    प्रत्यागम new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context, Z3_mk_const(Context.Context,
                                    Z3_mk_string_चिह्नम्(Context.Context, Name),
                                    गन्तव्यम्_Z3_क्रमय(*क्रमय_2).क्रमय_2)));
  }

  llvm::APSInt getBitvector(ध्रुव SMTExprRef &Exp, अचिह्नित Bit_विस्तारः,
                            तार्किक isUnsigned) override {
    प्रत्यागम llvm::APSInt(
        llvm::APInt(Bit_विस्तारः,
                    Z3_get_numeral_string(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST),
                    10),
        isUnsigned);
  }

  तार्किक getBoolean(ध्रुव SMTExprRef &Exp) override {
    प्रत्यागम Z3_get_bool_मानम्(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST) == Z3_L_TRUE;
  }

  SMTExprRef get_Float_Rounding_बहुलकम्() override {
    // TODO: Don't assume nearest ties to even rounding mode
    प्रत्यागम new_व्यञ्जकम्_Ref(Z3_व्यञ्जकम्(Context, Z3_mk_fpa_rne(Context.Context)));
  }

  तार्किक गन्तव्यम्_APFloat(ध्रुव SMTSortRef &क्रमय_2, ध्रुव SMTExprRef &AST,
                 llvm::APFloat &Float, तार्किक useSemantics) {
    प्रतिज्ञा(क्रमय_2->is_Float_क्रमय() && "Unsupported sort to floating-point!");

    llvm::APSInt Int(क्रमय_2->get_Float_क्रमय_आकार(), सत्य);
    ध्रुव llvm::fltSemantics &Semantics =
        getFloatSemantics(क्रमय_2->get_Float_क्रमय_आकार());
    SMTSortRef BVSort = get_Bitvector_क्रमय(क्रमय_2->get_Float_क्रमय_आकार());
    यदि (!गन्तव्यम्_APSInt(BVSort, AST, Int, सत्य)) {
      प्रत्यागम असत्य;
    }

    यदि (useSemantics && !areEquivalent(Float.getSemantics(), Semantics)) {
      प्रतिज्ञा(असत्य && "Floating-point types don't match!");
      प्रत्यागम असत्य;
    }

    Float = llvm::APFloat(Semantics, Int);
    प्रत्यागम सत्य;
  }

  तार्किक गन्तव्यम्_APSInt(ध्रुव SMTSortRef &क्रमय_2, ध्रुव SMTExprRef &AST,
                llvm::APSInt &Int, तार्किक useSemantics) {
    यदि (क्रमय_2->is_Bitvector_क्रमय()) {
      यदि (useSemantics && Int.get_Bit_विस्तारः() != क्रमय_2->get_Bitvector_क्रमय_आकार()) {
        प्रतिज्ञा(असत्य && "Bitvector types don't match!");
        प्रत्यागम असत्य;
      }

      // FIXME: This function is also used to retrieve floating-point values,
      // which can be 16, 32, 64 or 128 bits long. In the future, we need proper
      // calls in the backend to retrieve floating-points and its special values
      // (NaN, +/-infinity, +/-zero).
      Int = getBitvector(AST, Int.get_Bit_विस्तारः(), Int.isUnsigned());
      प्रत्यागम सत्य;
    }

    यदि (क्रमय_2->is_Boolean_क्रमय()) {
      यदि (useSemantics && Int.get_Bit_विस्तारः() < 1) {
        प्रतिज्ञा(असत्य && "Boolean type doesn't match!");
        प्रत्यागम असत्य;
      }

      Int = llvm::APSInt(llvm::APInt(Int.get_Bit_विस्तारः(), getBoolean(AST)),
                         Int.isUnsigned());
      प्रत्यागम सत्य;
    }

    llvm_unreachable("Unsupported sort to integer!");
  }

  तार्किक getInterpretation(ध्रुव SMTExprRef &Exp, llvm::APSInt &Int) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_गन्तव्यम्_app(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST));
    यदि (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      प्रत्यागम असत्य;

    SMTExprRef नियोजय_2 = new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef क्रमय_2 = get_क्रमय(नियोजय_2);
    प्रत्यागम गन्तव्यम्_APSInt(क्रमय_2, नियोजय_2, Int, सत्य);
  }

  तार्किक getInterpretation(ध्रुव SMTExprRef &Exp, llvm::APFloat &Float) override {
    Z3Model Model(Context, Z3_solver_get_model(Context.Context, Solver));
    Z3_func_decl Func = Z3_get_app_decl(
        Context.Context, Z3_गन्तव्यम्_app(Context.Context, गन्तव्यम्_Z3_व्यञ्जकम्(*Exp).AST));
    यदि (Z3_model_has_interp(Context.Context, Model.Model, Func) != Z3_L_TRUE)
      प्रत्यागम असत्य;

    SMTExprRef नियोजय_2 = new_व्यञ्जकम्_Ref(
        Z3_व्यञ्जकम्(Context,
               Z3_model_get_const_interp(Context.Context, Model.Model, Func)));
    SMTSortRef क्रमय_2 = get_क्रमय(नियोजय_2);
    प्रत्यागम गन्तव्यम्_APFloat(क्रमय_2, नियोजय_2, Float, सत्य);
  }

  std::optional<तार्किक> check() ध्रुव override {
    Z3_solver_समुच्चयः_params(Context.Context, Solver, Params);
    Z3_lbool res = Z3_solver_check(Context.Context, Solver);
    यदि (res == Z3_L_TRUE)
      प्रत्यागम सत्य;

    यदि (res == Z3_L_FALSE)
      प्रत्यागम असत्य;

    प्रत्यागम std::nullopt;
  }

  शून्य प्रक्षिप() override { प्रत्यागम Z3_solver_प्रक्षिप(Context.Context, Solver); }

  शून्य निष्कासय(अचिह्नित अङ्कः_अवस्थाः = 1) override {
    प्रतिज्ञा(Z3_solver_get_अङ्कः_scopes(Context.Context, Solver) >= अङ्कः_अवस्थाः);
    प्रत्यागम Z3_solver_निष्कासय(Context.Context, Solver, अङ्कः_अवस्थाः);
  }

  तार्किक isFPSupported() override { प्रत्यागम सत्य; }

  /// Reset the solver and remove all constraints.
  शून्य reset() override { Z3_solver_reset(Context.Context, Solver); }

  शून्य print(raw_ostream &OS) ध्रुव override {
    OS << Z3_solver_गन्तव्यम्_string(Context.Context, Solver);
  }

  शून्य समुच्चयः_Unsigned_Param(StringRef कुञ्चिका, अचिह्नित मानम्) override {
    Z3_चिह्नम् Sym = Z3_mk_string_चिह्नम्(Context.Context, कुञ्चिका.str().c_str());
    Z3_params_समुच्चयः_uint(Context.Context, Params, Sym, मानम्);
  }

  शून्य समुच्चयः_Bool_Param(StringRef कुञ्चिका, तार्किक मानम्) override {
    Z3_चिह्नम् Sym = Z3_mk_string_चिह्नम्(Context.Context, कुञ्चिका.str().c_str());
    Z3_params_समुच्चयः_bool(Context.Context, Params, Sym, मानम्);
  }

  std::unique_ptr<SMTSolverStatistics> getStatistics() ध्रुव override;
}; // end class Z3Solver

वर्ग Z3Statistics final : सार्वजनिक SMTSolverStatistics {
सार्वजनिक:
  द्विगुण getDouble(StringRef कुञ्चिका) ध्रुव override {
    स्वचालित It = Double_मानानि.find(कुञ्चिका.str());
    प्रतिज्ञा(It != Double_मानानि.अन्तः());
    प्रत्यागम It->second;
  };
  अचिह्नित getUnsigned(StringRef कुञ्चिका) ध्रुव override {
    स्वचालित It = Unsigned_मानानि.find(कुञ्चिका.str());
    यदि (It == Unsigned_मानानि.अन्तः())
      प्रत्यागम 0;
    प्रत्यागम It->second;
  };

  शून्य print(raw_ostream &OS) ध्रुव override {
    प्रति (ध्रुव स्वचालित &E : Unsigned_मानानि)
      OS << E.first() << ": " << E.second << '\n';
    प्रति (ध्रुव स्वचालित &E : Double_मानानि) {
      write_double(OS << E.first() << ": ", E.second, FloatStyle::Fixed);
      OS << '\n';
    }
  }

गुप्त:
  मित्रम् वर्ग Z3Solver;
  StringMap<अचिह्नित> Unsigned_मानानि;
  StringMap<द्विगुण> Double_मानानि;
};

std::unique_ptr<SMTSolverStatistics> Z3Solver::getStatistics() ध्रुव {
  स्वचालित ध्रुव &C = Context.Context;
  Z3_stats S = Z3_solver_get_statistics(C, Solver);
  Z3_stats_inc_ref(C, S);
  llvm::scope_exit StatsGuard([&C, &S] { Z3_stats_dec_ref(C, S); });
  Z3Statistics परिणाम;

  अचिह्नित अङ्कः_कुञ्चिकाः = Z3_stats_आकार(C, S);
  प्रति (अचिह्नित Idx = 0; Idx < अङ्कः_कुञ्चिकाः; ++Idx) {
    ध्रुव वर्ण *कुञ्चिका = Z3_stats_get_कुञ्चिका(C, S, Idx);
    यदि (Z3_stats_is_uint(C, S, Idx)) {
      स्वचालित मानम् = Z3_stats_get_uint_मानम्(C, S, Idx);
      परिणाम.Unsigned_मानानि.try_emplace(कुञ्चिका, मानम्);
    } अन्यथा {
      प्रतिज्ञा(Z3_stats_is_double(C, S, Idx));
      स्वचालित मानम् = Z3_stats_get_double_मानम्(C, S, Idx);
      परिणाम.Double_मानानि.try_emplace(कुञ्चिका, मानम्);
    }
  }
  प्रत्यागम std::make_unique<Z3Statistics>(std::move(परिणाम));
}

} // end anonymous namespace

#यदि_अन्तः

llvm::SMTSolverRef llvm::CreateZ3Solver() {
#यदि LLVM_WITH_Z3
  प्रत्यागम std::make_unique<Z3Solver>();
#अन्यथा
  llvm::report_fatal_error("LLVM was not compiled with Z3 support, rebuild "
                           "with -DLLVM_ENABLE_Z3_SOLVER=ON",
                           असत्य);
  प्रत्यागम शून्यसूचकः;
#यदि_अन्तः
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य SMTSort::dump() ध्रुव { print(llvm::errs()); }
LLVM_DUMP_विधानम् शून्य SMTExpr::dump() ध्रुव { print(llvm::errs()); }
LLVM_DUMP_विधानम् शून्य SMTSolver::dump() ध्रुव { print(llvm::errs()); }
LLVM_DUMP_विधानम् शून्य SMTSolverStatistics::dump() ध्रुव { print(llvm::errs()); }
#यदि_अन्तः
