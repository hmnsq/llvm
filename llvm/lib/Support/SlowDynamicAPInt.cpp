//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/SlowDynamicAPInt.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/Hashing.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/Debug.h"
#ଅନ୍ତର୍ଭାଵ "llvm/Support/raw_ostream.h"

using ନାମାକାଶ llvm;
using ନାମାକାଶ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/ସତ୍ୟ) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ଅଚଳ APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  ଫେରନ୍ତୁ *ଏହି = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() ଅଚଳ { ଫେରନ୍ତୁ Val.get_SExt_ମୂଲ୍ୟ(); }

hash_code llvm::detail::hash_ମୂଲ୍ୟ(ଅଚଳ SlowDynamicAPInt &X) {
  ଫେରନ୍ତୁ hash_ମୂଲ୍ୟ(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A %= SlowDynamicAPInt(B);
}

ତାର୍କିକ llvm::detail::operator==(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A == SlowDynamicAPInt(B);
}
ତାର୍କିକ llvm::detail::operator!=(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A != SlowDynamicAPInt(B);
}
ତାର୍କିକ llvm::detail::operator>(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A > SlowDynamicAPInt(B);
}
ତାର୍କିକ llvm::detail::operator<(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A < SlowDynamicAPInt(B);
}
ତାର୍କିକ llvm::detail::operator<=(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A <= SlowDynamicAPInt(B);
}
ତାର୍କିକ llvm::detail::operator>=(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(ଅଚଳ SlowDynamicAPInt &A, int64_t B) {
  ଫେରନ୍ତୁ A % SlowDynamicAPInt(B);
}

ତାର୍କିକ llvm::detail::operator==(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) == B;
}
ତାର୍କିକ llvm::detail::operator!=(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) != B;
}
ତାର୍କିକ llvm::detail::operator>(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) > B;
}
ତାର୍କିକ llvm::detail::operator<(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) < B;
}
ତାର୍କିକ llvm::detail::operator<=(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) <= B;
}
ତାର୍କିକ llvm::detail::operator>=(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, ଅଚଳ SlowDynamicAPInt &B) {
  ଫେରନ୍ତୁ SlowDynamicAPInt(A) % B;
}

ସ୍ଥିର unsigned getMaxWidth(ଅଚଳ APInt &A, ଅଚଳ APInt &B) {
  ଫେରନ୍ତୁ std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
ତାର୍କିକ SlowDynamicAPInt::operator==(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ଫେରନ୍ତୁ Val.sext(Width) == O.Val.sext(Width);
}
ତାର୍କିକ SlowDynamicAPInt::operator!=(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ଫେରନ୍ତୁ Val.sext(Width) != O.Val.sext(Width);
}
ତାର୍କିକ SlowDynamicAPInt::operator>(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ଫେରନ୍ତୁ Val.sext(Width).sgt(O.Val.sext(Width));
}
ତାର୍କିକ SlowDynamicAPInt::operator<(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ଫେରନ୍ତୁ Val.sext(Width).slt(O.Val.sext(Width));
}
ତାର୍କିକ SlowDynamicAPInt::operator<=(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ଫେରନ୍ତୁ Val.sext(Width).sle(O.Val.sext(Width));
}
ତାର୍କିକ SlowDynamicAPInt::operator>=(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ଫେରନ୍ତୁ Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    ଅଚଳ APInt &A, ଅଚଳ APInt &B,
    function_ref<APInt(ଅଚଳ APInt &, ଅଚଳ APInt &, ତାର୍କିକ &Overflow)> Op) {
  ତାର୍କିକ Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  ଯଦି (!Overflow)
    ଫେରନ୍ତୁ Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  ପ୍ରତିଜ୍ଞା(!Overflow && "double width should be sufficient to avoid overflow!");
  ଫେରନ୍ତୁ Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  ଫେରନ୍ତୁ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  ଫେରନ୍ତୁ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  ଫେରନ୍ତୁ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  ଫେରନ୍ତୁ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(ଅଚଳ SlowDynamicAPInt &X) {
  ଫେରନ୍ତୁ X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ଅଚଳ SlowDynamicAPInt &LHS,
                                       ଅଚଳ SlowDynamicAPInt &RHS) {
  ଯଦି (RHS == -1)
    ଫେରନ୍ତୁ -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  ଫେରନ୍ତୁ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ଅଚଳ SlowDynamicAPInt &LHS,
                                        ଅଚଳ SlowDynamicAPInt &RHS) {
  ଯଦି (RHS == -1)
    ଫେରନ୍ତୁ -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  ଫେରନ୍ତୁ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ଅଚଳ SlowDynamicAPInt &LHS,
                                   ଅଚଳ SlowDynamicAPInt &RHS) {
  ପ୍ରତିଜ୍ଞା(RHS >= 1 && "mod is only supported for positive divisors!");
  ଫେରନ୍ତୁ LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ଅଚଳ SlowDynamicAPInt &A,
                                   ଅଚଳ SlowDynamicAPInt &B) {
  ପ୍ରତିଜ୍ଞା(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  ଫେରନ୍ତୁ SlowDynamicAPInt(
      APIntOps::Greatest_Common_ଭାଜକ(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ଅଚଳ SlowDynamicAPInt &A,
                                   ଅଚଳ SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  ଫେରନ୍ତୁ (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(ଅଚଳ SlowDynamicAPInt &O) ଅଚଳ {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  ଫେରନ୍ତୁ SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() ଅଚଳ {
  ଯଦି (Val.is_Min_Signed_ମୂଲ୍ୟ()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    ଫେରନ୍ତୁ SlowDynamicAPInt(-Ret);
  }
  ଫେରନ୍ତୁ SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(ଅଚଳ SlowDynamicAPInt &O) {
  *ଏହି = *ଏହି + O;
  ଫେରନ୍ତୁ *ଏହି;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(ଅଚଳ SlowDynamicAPInt &O) {
  *ଏହି = *ଏହି - O;
  ଫେରନ୍ତୁ *ଏହି;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(ଅଚଳ SlowDynamicAPInt &O) {
  *ଏହି = *ଏହି * O;
  ଫେରନ୍ତୁ *ଏହି;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(ଅଚଳ SlowDynamicAPInt &O) {
  *ଏହି = *ଏହି / O;
  ଫେରନ୍ତୁ *ଏହି;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(ଅଚଳ SlowDynamicAPInt &O) {
  *ଏହି = *ଏହି % O;
  ଫେରନ୍ତୁ *ଏହି;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *ଏହି += 1;
  ଫେରନ୍ତୁ *ଏହି;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *ଏହି -= 1;
  ଫେରନ୍ତୁ *ଏହି;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
ଶୂନ୍ୟ SlowDynamicAPInt::print(raw_ostream &OS) ଅଚଳ { OS << Val; }

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ SlowDynamicAPInt::dump() ଅଚଳ { print(dbgs()); }
#endif
