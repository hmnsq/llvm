//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/SlowDynamicAPInt.h"
#समावेश "llvm/ADT/Hashing.h"
#समावेश "llvm/Support/Debug.h"
#समावेश "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/true) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(स्थिर APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::चालक=(int64_t Val) {
  वापस *यह = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::चालक int64_t() स्थिर { वापस Val.get_SExt_मूल्य(); }

hash_code llvm::detail::hash_मूल्य(स्थिर SlowDynamicAPInt &X) {
  वापस hash_मूल्य(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::चालक+=(SlowDynamicAPInt &A, int64_t B) {
  वापस A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक-=(SlowDynamicAPInt &A, int64_t B) {
  वापस A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक*=(SlowDynamicAPInt &A, int64_t B) {
  वापस A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक/=(SlowDynamicAPInt &A, int64_t B) {
  वापस A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक%=(SlowDynamicAPInt &A, int64_t B) {
  वापस A %= SlowDynamicAPInt(B);
}

bool llvm::detail::चालक==(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A == SlowDynamicAPInt(B);
}
bool llvm::detail::चालक!=(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A != SlowDynamicAPInt(B);
}
bool llvm::detail::चालक>(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A > SlowDynamicAPInt(B);
}
bool llvm::detail::चालक<(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A < SlowDynamicAPInt(B);
}
bool llvm::detail::चालक<=(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A <= SlowDynamicAPInt(B);
}
bool llvm::detail::चालक>=(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक+(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक-(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक*(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक/(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक%(स्थिर SlowDynamicAPInt &A, int64_t B) {
  वापस A % SlowDynamicAPInt(B);
}

bool llvm::detail::चालक==(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) == B;
}
bool llvm::detail::चालक!=(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) != B;
}
bool llvm::detail::चालक>(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) > B;
}
bool llvm::detail::चालक<(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) < B;
}
bool llvm::detail::चालक<=(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) <= B;
}
bool llvm::detail::चालक>=(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::चालक+(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::चालक-(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::चालक*(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::चालक/(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::चालक%(int64_t A, स्थिर SlowDynamicAPInt &B) {
  वापस SlowDynamicAPInt(A) % B;
}

अटल अचिन्हित getMaxWidth(स्थिर APInt &A, स्थिर APInt &B) {
  वापस std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
bool SlowDynamicAPInt::चालक==(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  वापस Val.sext(Width) == O.Val.sext(Width);
}
bool SlowDynamicAPInt::चालक!=(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  वापस Val.sext(Width) != O.Val.sext(Width);
}
bool SlowDynamicAPInt::चालक>(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  वापस Val.sext(Width).sgt(O.Val.sext(Width));
}
bool SlowDynamicAPInt::चालक<(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  वापस Val.sext(Width).slt(O.Val.sext(Width));
}
bool SlowDynamicAPInt::चालक<=(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  वापस Val.sext(Width).sle(O.Val.sext(Width));
}
bool SlowDynamicAPInt::चालक>=(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  वापस Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    स्थिर APInt &A, स्थिर APInt &B,
    function_ref<APInt(स्थिर APInt &, स्थिर APInt &, bool &Overflow)> Op) {
  bool Overflow;
  अचिन्हित Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  अगर (!Overflow)
    वापस Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  निश्चित(!Overflow && "double width should be sufficient to avoid overflow!");
  वापस Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::चालक+(स्थिर SlowDynamicAPInt &O) स्थिर {
  वापस SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक-(स्थिर SlowDynamicAPInt &O) स्थिर {
  वापस SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक*(स्थिर SlowDynamicAPInt &O) स्थिर {
  वापस SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक/(स्थिर SlowDynamicAPInt &O) स्थिर {
  वापस SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::असल(स्थिर SlowDynamicAPInt &X) {
  वापस X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(स्थिर SlowDynamicAPInt &LHS,
                                       स्थिर SlowDynamicAPInt &RHS) {
  अगर (RHS == -1)
    वापस -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  वापस SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(स्थिर SlowDynamicAPInt &LHS,
                                        स्थिर SlowDynamicAPInt &RHS) {
  अगर (RHS == -1)
    वापस -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  वापस SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(स्थिर SlowDynamicAPInt &LHS,
                                   स्थिर SlowDynamicAPInt &RHS) {
  निश्चित(RHS >= 1 && "mod is only supported for positive divisors!");
  वापस LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(स्थिर SlowDynamicAPInt &A,
                                   स्थिर SlowDynamicAPInt &B) {
  निश्चित(A >= 0 && B >= 0 && "operands must be non-negative!");
  अचिन्हित Width = getMaxWidth(A.Val, B.Val);
  वापस SlowDynamicAPInt(
      APIntOps::Greatest_Common_भाजक(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(स्थिर SlowDynamicAPInt &A,
                                   स्थिर SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = असल(A);
  SlowDynamicAPInt Y = असल(B);
  वापस (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::चालक%(स्थिर SlowDynamicAPInt &O) स्थिर {
  अचिन्हित Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  वापस SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::चालक-() स्थिर {
  अगर (Val.is_Min_Signed_मूल्य()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    वापस SlowDynamicAPInt(-Ret);
  }
  वापस SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::चालक+=(स्थिर SlowDynamicAPInt &O) {
  *यह = *यह + O;
  वापस *यह;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक-=(स्थिर SlowDynamicAPInt &O) {
  *यह = *यह - O;
  वापस *यह;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक*=(स्थिर SlowDynamicAPInt &O) {
  *यह = *यह * O;
  वापस *यह;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक/=(स्थिर SlowDynamicAPInt &O) {
  *यह = *यह / O;
  वापस *यह;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक%=(स्थिर SlowDynamicAPInt &O) {
  *यह = *यह % O;
  वापस *यह;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक++() {
  *यह += 1;
  वापस *यह;
}

SlowDynamicAPInt &SlowDynamicAPInt::चालक--() {
  *यह -= 1;
  वापस *यह;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
व्योम SlowDynamicAPInt::print(raw_ostream &OS) स्थिर { OS << Val; }

#अगर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD व्योम SlowDynamicAPInt::dump() स्थिर { print(dbgs()); }
#पूर्ण_अगर
