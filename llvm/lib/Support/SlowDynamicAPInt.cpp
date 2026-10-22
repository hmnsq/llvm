//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাৱ "llvm/ADT/SlowDynamicAPInt.h"
#অন্তর্ভাৱ "llvm/ADT/Hashing.h"
#অন্তর্ভাৱ "llvm/Support/Debug.h"
#অন্তর্ভাৱ "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/সঁচা) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ধ্রুৱ APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  ঘূৰাই *এতত্ = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() ধ্রুৱ { ঘূৰাই Val.get_SExt_মান(); }

hash_code llvm::detail::hash_মান(ধ্রুৱ SlowDynamicAPInt &X) {
  ঘূৰাই hash_মান(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A %= SlowDynamicAPInt(B);
}

তার্কিক llvm::detail::operator==(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A == SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator!=(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A != SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator>(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A > SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator<(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A < SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator<=(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A <= SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator>=(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(ধ্রুৱ SlowDynamicAPInt &A, int64_t B) {
  ঘূৰাই A % SlowDynamicAPInt(B);
}

তার্কিক llvm::detail::operator==(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) == B;
}
তার্কিক llvm::detail::operator!=(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) != B;
}
তার্কিক llvm::detail::operator>(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) > B;
}
তার্কিক llvm::detail::operator<(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) < B;
}
তার্কিক llvm::detail::operator<=(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) <= B;
}
তার্কিক llvm::detail::operator>=(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, ধ্রুৱ SlowDynamicAPInt &B) {
  ঘূৰাই SlowDynamicAPInt(A) % B;
}

স্থির unsigned getMaxWidth(ধ্রুৱ APInt &A, ধ্রুৱ APInt &B) {
  ঘূৰাই std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
তার্কিক SlowDynamicAPInt::operator==(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ঘূৰাই Val.sext(Width) == O.Val.sext(Width);
}
তার্কিক SlowDynamicAPInt::operator!=(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ঘূৰাই Val.sext(Width) != O.Val.sext(Width);
}
তার্কিক SlowDynamicAPInt::operator>(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ঘূৰাই Val.sext(Width).sgt(O.Val.sext(Width));
}
তার্কিক SlowDynamicAPInt::operator<(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ঘূৰাই Val.sext(Width).slt(O.Val.sext(Width));
}
তার্কিক SlowDynamicAPInt::operator<=(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ঘূৰাই Val.sext(Width).sle(O.Val.sext(Width));
}
তার্কিক SlowDynamicAPInt::operator>=(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ঘূৰাই Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    ধ্রুৱ APInt &A, ধ্রুৱ APInt &B,
    function_ref<APInt(ধ্রুৱ APInt &, ধ্রুৱ APInt &, তার্কিক &Overflow)> Op) {
  তার্কিক Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  যদি (!Overflow)
    ঘূৰাই Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  প্রতিজ্ঞা(!Overflow && "double width should be sufficient to avoid overflow!");
  ঘূৰাই Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  ঘূৰাই SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  ঘূৰাই SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  ঘূৰাই SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  ঘূৰাই SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(ধ্রুৱ SlowDynamicAPInt &X) {
  ঘূৰাই X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ধ্রুৱ SlowDynamicAPInt &LHS,
                                       ধ্রুৱ SlowDynamicAPInt &RHS) {
  যদি (RHS == -1)
    ঘূৰাই -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  ঘূৰাই SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ধ্রুৱ SlowDynamicAPInt &LHS,
                                        ধ্রুৱ SlowDynamicAPInt &RHS) {
  যদি (RHS == -1)
    ঘূৰাই -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  ঘূৰাই SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ধ্রুৱ SlowDynamicAPInt &LHS,
                                   ধ্রুৱ SlowDynamicAPInt &RHS) {
  প্রতিজ্ঞা(RHS >= 1 && "mod is only supported for positive divisors!");
  ঘূৰাই LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ধ্রুৱ SlowDynamicAPInt &A,
                                   ধ্রুৱ SlowDynamicAPInt &B) {
  প্রতিজ্ঞা(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  ঘূৰাই SlowDynamicAPInt(
      APIntOps::Greatest_Common_ভাজক(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ধ্রুৱ SlowDynamicAPInt &A,
                                   ধ্রুৱ SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  ঘূৰাই (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(ধ্রুৱ SlowDynamicAPInt &O) ধ্রুৱ {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  ঘূৰাই SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() ধ্রুৱ {
  যদি (Val.is_Min_Signed_মান()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    ঘূৰাই SlowDynamicAPInt(-Ret);
  }
  ঘূৰাই SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(ধ্রুৱ SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ + O;
  ঘূৰাই *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(ধ্রুৱ SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ - O;
  ঘূৰাই *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(ধ্রুৱ SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ * O;
  ঘূৰাই *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(ধ্রুৱ SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ / O;
  ঘূৰাই *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(ধ্রুৱ SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ % O;
  ঘূৰাই *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *এতত্ += 1;
  ঘূৰাই *এতত্;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *এতত্ -= 1;
  ঘূৰাই *এতত্;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
শূন্য SlowDynamicAPInt::print(raw_ostream &OS) ধ্রুৱ { OS << Val; }

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য SlowDynamicAPInt::dump() ধ্রুৱ { print(dbgs()); }
#endif
