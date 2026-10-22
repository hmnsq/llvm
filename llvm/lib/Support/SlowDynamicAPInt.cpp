//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/SlowDynamicAPInt.h"
#અન્તર્ભાવ "llvm/ADT/Hashing.h"
#અન્તર્ભાવ "llvm/Support/Debug.h"
#અન્તર્ભાવ "llvm/Support/raw_ostream.h"

using નામાકાશ llvm;
using નામાકાશ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/સાચું) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(અચલ APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  પાછા *આ = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() અચલ { પાછા Val.get_SExt_કિંમત(); }

hash_code llvm::detail::hash_કિંમત(અચલ SlowDynamicAPInt &X) {
  પાછા hash_કિંમત(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  પાછા A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  પાછા A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  પાછા A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  પાછા A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  પાછા A %= SlowDynamicAPInt(B);
}

તાર્કિક llvm::detail::operator==(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A == SlowDynamicAPInt(B);
}
તાર્કિક llvm::detail::operator!=(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A != SlowDynamicAPInt(B);
}
તાર્કિક llvm::detail::operator>(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A > SlowDynamicAPInt(B);
}
તાર્કિક llvm::detail::operator<(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A < SlowDynamicAPInt(B);
}
તાર્કિક llvm::detail::operator<=(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A <= SlowDynamicAPInt(B);
}
તાર્કિક llvm::detail::operator>=(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(અચલ SlowDynamicAPInt &A, int64_t B) {
  પાછા A % SlowDynamicAPInt(B);
}

તાર્કિક llvm::detail::operator==(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) == B;
}
તાર્કિક llvm::detail::operator!=(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) != B;
}
તાર્કિક llvm::detail::operator>(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) > B;
}
તાર્કિક llvm::detail::operator<(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) < B;
}
તાર્કિક llvm::detail::operator<=(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) <= B;
}
તાર્કિક llvm::detail::operator>=(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, અચલ SlowDynamicAPInt &B) {
  પાછા SlowDynamicAPInt(A) % B;
}

સ્થિર unsigned getMaxWidth(અચલ APInt &A, અચલ APInt &B) {
  પાછા std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
તાર્કિક SlowDynamicAPInt::operator==(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = getMaxWidth(Val, O.Val);
  પાછા Val.sext(Width) == O.Val.sext(Width);
}
તાર્કિક SlowDynamicAPInt::operator!=(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = getMaxWidth(Val, O.Val);
  પાછા Val.sext(Width) != O.Val.sext(Width);
}
તાર્કિક SlowDynamicAPInt::operator>(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = getMaxWidth(Val, O.Val);
  પાછા Val.sext(Width).sgt(O.Val.sext(Width));
}
તાર્કિક SlowDynamicAPInt::operator<(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = getMaxWidth(Val, O.Val);
  પાછા Val.sext(Width).slt(O.Val.sext(Width));
}
તાર્કિક SlowDynamicAPInt::operator<=(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = getMaxWidth(Val, O.Val);
  પાછા Val.sext(Width).sle(O.Val.sext(Width));
}
તાર્કિક SlowDynamicAPInt::operator>=(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = getMaxWidth(Val, O.Val);
  પાછા Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    અચલ APInt &A, અચલ APInt &B,
    function_ref<APInt(અચલ APInt &, અચલ APInt &, તાર્કિક &Overflow)> Op) {
  તાર્કિક Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  જો (!Overflow)
    પાછા Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  પ્રતિજ્ઞા(!Overflow && "double width should be sufficient to avoid overflow!");
  પાછા Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(અચલ SlowDynamicAPInt &O) અચલ {
  પાછા SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(અચલ SlowDynamicAPInt &O) અચલ {
  પાછા SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(અચલ SlowDynamicAPInt &O) અચલ {
  પાછા SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(અચલ SlowDynamicAPInt &O) અચલ {
  પાછા SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(અચલ SlowDynamicAPInt &X) {
  પાછા X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(અચલ SlowDynamicAPInt &LHS,
                                       અચલ SlowDynamicAPInt &RHS) {
  જો (RHS == -1)
    પાછા -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  પાછા SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(અચલ SlowDynamicAPInt &LHS,
                                        અચલ SlowDynamicAPInt &RHS) {
  જો (RHS == -1)
    પાછા -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  પાછા SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(અચલ SlowDynamicAPInt &LHS,
                                   અચલ SlowDynamicAPInt &RHS) {
  પ્રતિજ્ઞા(RHS >= 1 && "mod is only supported for positive divisors!");
  પાછા LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(અચલ SlowDynamicAPInt &A,
                                   અચલ SlowDynamicAPInt &B) {
  પ્રતિજ્ઞા(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  પાછા SlowDynamicAPInt(
      APIntOps::Greatest_Common_ભાજક(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(અચલ SlowDynamicAPInt &A,
                                   અચલ SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  પાછા (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(અચલ SlowDynamicAPInt &O) અચલ {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  પાછા SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() અચલ {
  જો (Val.is_Min_Signed_કિંમત()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    પાછા SlowDynamicAPInt(-Ret);
  }
  પાછા SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(અચલ SlowDynamicAPInt &O) {
  *આ = *આ + O;
  પાછા *આ;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(અચલ SlowDynamicAPInt &O) {
  *આ = *આ - O;
  પાછા *આ;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(અચલ SlowDynamicAPInt &O) {
  *આ = *આ * O;
  પાછા *આ;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(અચલ SlowDynamicAPInt &O) {
  *આ = *આ / O;
  પાછા *આ;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(અચલ SlowDynamicAPInt &O) {
  *આ = *આ % O;
  પાછા *આ;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *આ += 1;
  પાછા *આ;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *આ -= 1;
  પાછા *આ;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
શૂન્ય SlowDynamicAPInt::print(raw_ostream &OS) અચલ { OS << Val; }

#જો !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD શૂન્ય SlowDynamicAPInt::dump() અચલ { print(dbgs()); }
#endif
