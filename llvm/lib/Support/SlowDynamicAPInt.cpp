//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/SlowDynamicAPInt.h"
#include "llvm/ADT/Hashing.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
using namespace llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/true) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(const APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  返回 *this = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() const { 返回 Val.getSExtValue(); }

hash_code llvm::detail::hash_value(const SlowDynamicAPInt &X) {
  返回 hash_value(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  返回 A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  返回 A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  返回 A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  返回 A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  返回 A %= SlowDynamicAPInt(B);
}

bool llvm::detail::operator==(const SlowDynamicAPInt &A, int64_t B) {
  返回 A == SlowDynamicAPInt(B);
}
bool llvm::detail::operator!=(const SlowDynamicAPInt &A, int64_t B) {
  返回 A != SlowDynamicAPInt(B);
}
bool llvm::detail::operator>(const SlowDynamicAPInt &A, int64_t B) {
  返回 A > SlowDynamicAPInt(B);
}
bool llvm::detail::operator<(const SlowDynamicAPInt &A, int64_t B) {
  返回 A < SlowDynamicAPInt(B);
}
bool llvm::detail::operator<=(const SlowDynamicAPInt &A, int64_t B) {
  返回 A <= SlowDynamicAPInt(B);
}
bool llvm::detail::operator>=(const SlowDynamicAPInt &A, int64_t B) {
  返回 A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(const SlowDynamicAPInt &A, int64_t B) {
  返回 A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(const SlowDynamicAPInt &A, int64_t B) {
  返回 A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(const SlowDynamicAPInt &A, int64_t B) {
  返回 A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(const SlowDynamicAPInt &A, int64_t B) {
  返回 A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(const SlowDynamicAPInt &A, int64_t B) {
  返回 A % SlowDynamicAPInt(B);
}

bool llvm::detail::operator==(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) == B;
}
bool llvm::detail::operator!=(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) != B;
}
bool llvm::detail::operator>(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) > B;
}
bool llvm::detail::operator<(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) < B;
}
bool llvm::detail::operator<=(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) <= B;
}
bool llvm::detail::operator>=(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, const SlowDynamicAPInt &B) {
  返回 SlowDynamicAPInt(A) % B;
}

static unsigned getMaxWidth(const APInt &A, const APInt &B) {
  返回 std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
bool SlowDynamicAPInt::operator==(const SlowDynamicAPInt &O) const {
  unsigned Width = getMaxWidth(Val, O.Val);
  返回 Val.sext(Width) == O.Val.sext(Width);
}
bool SlowDynamicAPInt::operator!=(const SlowDynamicAPInt &O) const {
  unsigned Width = getMaxWidth(Val, O.Val);
  返回 Val.sext(Width) != O.Val.sext(Width);
}
bool SlowDynamicAPInt::operator>(const SlowDynamicAPInt &O) const {
  unsigned Width = getMaxWidth(Val, O.Val);
  返回 Val.sext(Width).sgt(O.Val.sext(Width));
}
bool SlowDynamicAPInt::operator<(const SlowDynamicAPInt &O) const {
  unsigned Width = getMaxWidth(Val, O.Val);
  返回 Val.sext(Width).slt(O.Val.sext(Width));
}
bool SlowDynamicAPInt::operator<=(const SlowDynamicAPInt &O) const {
  unsigned Width = getMaxWidth(Val, O.Val);
  返回 Val.sext(Width).sle(O.Val.sext(Width));
}
bool SlowDynamicAPInt::operator>=(const SlowDynamicAPInt &O) const {
  unsigned Width = getMaxWidth(Val, O.Val);
  返回 Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    const APInt &A, const APInt &B,
    function_ref<APInt(const APInt &, const APInt &, bool &Overflow)> Op) {
  bool Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  如果 (!Overflow)
    返回 Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  assert(!Overflow && "double width should be sufficient to avoid overflow!");
  返回 Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(const SlowDynamicAPInt &O) const {
  返回 SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(const SlowDynamicAPInt &O) const {
  返回 SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(const SlowDynamicAPInt &O) const {
  返回 SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(const SlowDynamicAPInt &O) const {
  返回 SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(const SlowDynamicAPInt &X) {
  返回 X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(const SlowDynamicAPInt &LHS,
                                       const SlowDynamicAPInt &RHS) {
  如果 (RHS == -1)
    返回 -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  返回 SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(const SlowDynamicAPInt &LHS,
                                        const SlowDynamicAPInt &RHS) {
  如果 (RHS == -1)
    返回 -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  返回 SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(const SlowDynamicAPInt &LHS,
                                   const SlowDynamicAPInt &RHS) {
  assert(RHS >= 1 && "mod is only supported for positive divisors!");
  返回 LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(const SlowDynamicAPInt &A,
                                   const SlowDynamicAPInt &B) {
  assert(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  返回 SlowDynamicAPInt(
      APIntOps::GreatestCommonDivisor(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(const SlowDynamicAPInt &A,
                                   const SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  返回 (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(const SlowDynamicAPInt &O) const {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  返回 SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() const {
  如果 (Val.isMinSignedValue()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    返回 SlowDynamicAPInt(-Ret);
  }
  返回 SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(const SlowDynamicAPInt &O) {
  *this = *this + O;
  返回 *this;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(const SlowDynamicAPInt &O) {
  *this = *this - O;
  返回 *this;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(const SlowDynamicAPInt &O) {
  *this = *this * O;
  返回 *this;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(const SlowDynamicAPInt &O) {
  *this = *this / O;
  返回 *this;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(const SlowDynamicAPInt &O) {
  *this = *this % O;
  返回 *this;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *this += 1;
  返回 *this;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *this -= 1;
  返回 *this;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
void SlowDynamicAPInt::print(raw_ostream &OS) const { OS << Val; }

#如果 !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void SlowDynamicAPInt::dump() const { print(dbgs()); }
#endif
