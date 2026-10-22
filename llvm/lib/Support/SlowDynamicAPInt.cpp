//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SlowDynamicAPInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

using ನಾಮಾಕಾಶ llvm;
using ನಾಮಾಕಾಶ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/ಸತ್ಯ) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ಧ್ರುವ APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್ = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() ಧ್ರುವ { ಪ್ರತ್ಯಾಗಮ Val.get_SExt_ಮೌಲ್ಯ(); }

hash_code llvm::detail::hash_ಮೌಲ್ಯ(ಧ್ರುವ SlowDynamicAPInt &X) {
  ಪ್ರತ್ಯಾಗಮ hash_ಮೌಲ್ಯ(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A %= SlowDynamicAPInt(B);
}

ತಾರ್ಕಿಕ llvm::detail::operator==(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A == SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::operator!=(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A != SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::operator>(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A > SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::operator<(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A < SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::operator<=(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A <= SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::operator>=(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(ಧ್ರುವ SlowDynamicAPInt &A, int64_t B) {
  ಪ್ರತ್ಯಾಗಮ A % SlowDynamicAPInt(B);
}

ತಾರ್ಕಿಕ llvm::detail::operator==(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) == B;
}
ತಾರ್ಕಿಕ llvm::detail::operator!=(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) != B;
}
ತಾರ್ಕಿಕ llvm::detail::operator>(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) > B;
}
ತಾರ್ಕಿಕ llvm::detail::operator<(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) < B;
}
ತಾರ್ಕಿಕ llvm::detail::operator<=(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) <= B;
}
ತಾರ್ಕಿಕ llvm::detail::operator>=(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(A) % B;
}

ಸ್ಥಿರ unsigned getMaxWidth(ಧ್ರುವ APInt &A, ಧ್ರುವ APInt &B) {
  ಪ್ರತ್ಯಾಗಮ std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
ತಾರ್ಕಿಕ SlowDynamicAPInt::operator==(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ಪ್ರತ್ಯಾಗಮ Val.sext(Width) == O.Val.sext(Width);
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::operator!=(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ಪ್ರತ್ಯಾಗಮ Val.sext(Width) != O.Val.sext(Width);
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::operator>(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ಪ್ರತ್ಯಾಗಮ Val.sext(Width).sgt(O.Val.sext(Width));
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::operator<(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ಪ್ರತ್ಯಾಗಮ Val.sext(Width).slt(O.Val.sext(Width));
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::operator<=(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ಪ್ರತ್ಯಾಗಮ Val.sext(Width).sle(O.Val.sext(Width));
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::operator>=(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = getMaxWidth(Val, O.Val);
  ಪ್ರತ್ಯಾಗಮ Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    ಧ್ರುವ APInt &A, ಧ್ರುವ APInt &B,
    function_ref<APInt(ಧ್ರುವ APInt &, ಧ್ರುವ APInt &, ತಾರ್ಕಿಕ &Overflow)> Op) {
  ತಾರ್ಕಿಕ Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  ಯದಿ (!Overflow)
    ಪ್ರತ್ಯಾಗಮ Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  ಪ್ರತಿಜ್ಞಾ(!Overflow && "double width should be sufficient to avoid overflow!");
  ಪ್ರತ್ಯಾಗಮ Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(ಧ್ರುವ SlowDynamicAPInt &X) {
  ಪ್ರತ್ಯಾಗಮ X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ಧ್ರುವ SlowDynamicAPInt &LHS,
                                       ಧ್ರುವ SlowDynamicAPInt &RHS) {
  ಯದಿ (RHS == -1)
    ಪ್ರತ್ಯಾಗಮ -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ಧ್ರುವ SlowDynamicAPInt &LHS,
                                        ಧ್ರುವ SlowDynamicAPInt &RHS) {
  ಯದಿ (RHS == -1)
    ಪ್ರತ್ಯಾಗಮ -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ಧ್ರುವ SlowDynamicAPInt &LHS,
                                   ಧ್ರುವ SlowDynamicAPInt &RHS) {
  ಪ್ರತಿಜ್ಞಾ(RHS >= 1 && "mod is only supported for positive divisors!");
  ಪ್ರತ್ಯಾಗಮ LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ಧ್ರುವ SlowDynamicAPInt &A,
                                   ಧ್ರುವ SlowDynamicAPInt &B) {
  ಪ್ರತಿಜ್ಞಾ(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(
      APIntOps::Greatest_Common_ಭಾಜಕ(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ಧ್ರುವ SlowDynamicAPInt &A,
                                   ಧ್ರುವ SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  ಪ್ರತ್ಯಾಗಮ (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(ಧ್ರುವ SlowDynamicAPInt &O) ಧ್ರುವ {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() ಧ್ರುವ {
  ಯದಿ (Val.is_Min_Signed_ಮೌಲ್ಯ()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(-Ret);
  }
  ಪ್ರತ್ಯಾಗಮ SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(ಧ್ರುವ SlowDynamicAPInt &O) {
  *ಏತತ್ = *ಏತತ್ + O;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(ಧ್ರುವ SlowDynamicAPInt &O) {
  *ಏತತ್ = *ಏತತ್ - O;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(ಧ್ರುವ SlowDynamicAPInt &O) {
  *ಏತತ್ = *ಏತತ್ * O;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(ಧ್ರುವ SlowDynamicAPInt &O) {
  *ಏತತ್ = *ಏತತ್ / O;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(ಧ್ರುವ SlowDynamicAPInt &O) {
  *ಏತತ್ = *ಏತತ್ % O;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *ಏತತ್ += 1;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *ಏತತ್ -= 1;
  ಪ್ರತ್ಯಾಗಮ *ಏತತ್;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
ಶೂನ್ಯ SlowDynamicAPInt::print(raw_ostream &OS) ಧ್ರುವ { OS << Val; }

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ SlowDynamicAPInt::dump() ಧ್ರುವ { print(dbgs()); }
#endif
