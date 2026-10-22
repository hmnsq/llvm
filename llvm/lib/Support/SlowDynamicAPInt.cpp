//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#அந்தர்பாவ "llvm/ADT/SlowDynamicAPInt.h"
#அந்தர்பாவ "llvm/ADT/Hashing.h"
#அந்தர்பாவ "llvm/Support/Debug.h"
#அந்தர்பாவ "llvm/Support/raw_ostream.h"

using நாமாகாஶ llvm;
using நாமாகாஶ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/உண்மை) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(மாறிலி APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  திரும்பு *இது = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() மாறிலி { திரும்பு Val.get_SExt_மதிப்பு(); }

hash_code llvm::detail::hash_மதிப்பு(மாறிலி SlowDynamicAPInt &X) {
  திரும்பு hash_மதிப்பு(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A %= SlowDynamicAPInt(B);
}

தருக்க llvm::detail::operator==(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A == SlowDynamicAPInt(B);
}
தருக்க llvm::detail::operator!=(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A != SlowDynamicAPInt(B);
}
தருக்க llvm::detail::operator>(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A > SlowDynamicAPInt(B);
}
தருக்க llvm::detail::operator<(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A < SlowDynamicAPInt(B);
}
தருக்க llvm::detail::operator<=(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A <= SlowDynamicAPInt(B);
}
தருக்க llvm::detail::operator>=(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(மாறிலி SlowDynamicAPInt &A, int64_t B) {
  திரும்பு A % SlowDynamicAPInt(B);
}

தருக்க llvm::detail::operator==(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) == B;
}
தருக்க llvm::detail::operator!=(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) != B;
}
தருக்க llvm::detail::operator>(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) > B;
}
தருக்க llvm::detail::operator<(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) < B;
}
தருக்க llvm::detail::operator<=(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) <= B;
}
தருக்க llvm::detail::operator>=(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, மாறிலி SlowDynamicAPInt &B) {
  திரும்பு SlowDynamicAPInt(A) % B;
}

நிலையான unsigned getMaxWidth(மாறிலி APInt &A, மாறிலி APInt &B) {
  திரும்பு std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
தருக்க SlowDynamicAPInt::operator==(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = getMaxWidth(Val, O.Val);
  திரும்பு Val.sext(Width) == O.Val.sext(Width);
}
தருக்க SlowDynamicAPInt::operator!=(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = getMaxWidth(Val, O.Val);
  திரும்பு Val.sext(Width) != O.Val.sext(Width);
}
தருக்க SlowDynamicAPInt::operator>(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = getMaxWidth(Val, O.Val);
  திரும்பு Val.sext(Width).sgt(O.Val.sext(Width));
}
தருக்க SlowDynamicAPInt::operator<(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = getMaxWidth(Val, O.Val);
  திரும்பு Val.sext(Width).slt(O.Val.sext(Width));
}
தருக்க SlowDynamicAPInt::operator<=(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = getMaxWidth(Val, O.Val);
  திரும்பு Val.sext(Width).sle(O.Val.sext(Width));
}
தருக்க SlowDynamicAPInt::operator>=(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = getMaxWidth(Val, O.Val);
  திரும்பு Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    மாறிலி APInt &A, மாறிலி APInt &B,
    function_ref<APInt(மாறிலி APInt &, மாறிலி APInt &, தருக்க &Overflow)> Op) {
  தருக்க Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  எனில் (!Overflow)
    திரும்பு Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  ப்ரதிஜ்ஞா(!Overflow && "double width should be sufficient to avoid overflow!");
  திரும்பு Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  திரும்பு SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  திரும்பு SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  திரும்பு SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  திரும்பு SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(மாறிலி SlowDynamicAPInt &X) {
  திரும்பு X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(மாறிலி SlowDynamicAPInt &LHS,
                                       மாறிலி SlowDynamicAPInt &RHS) {
  எனில் (RHS == -1)
    திரும்பு -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  திரும்பு SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(மாறிலி SlowDynamicAPInt &LHS,
                                        மாறிலி SlowDynamicAPInt &RHS) {
  எனில் (RHS == -1)
    திரும்பு -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  திரும்பு SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(மாறிலி SlowDynamicAPInt &LHS,
                                   மாறிலி SlowDynamicAPInt &RHS) {
  ப்ரதிஜ்ஞா(RHS >= 1 && "mod is only supported for positive divisors!");
  திரும்பு LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(மாறிலி SlowDynamicAPInt &A,
                                   மாறிலி SlowDynamicAPInt &B) {
  ப்ரதிஜ்ஞா(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  திரும்பு SlowDynamicAPInt(
      APIntOps::Greatest_Common_வகுஎண்(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(மாறிலி SlowDynamicAPInt &A,
                                   மாறிலி SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  திரும்பு (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(மாறிலி SlowDynamicAPInt &O) மாறிலி {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  திரும்பு SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() மாறிலி {
  எனில் (Val.is_Min_Signed_மதிப்பு()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    திரும்பு SlowDynamicAPInt(-Ret);
  }
  திரும்பு SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(மாறிலி SlowDynamicAPInt &O) {
  *இது = *இது + O;
  திரும்பு *இது;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(மாறிலி SlowDynamicAPInt &O) {
  *இது = *இது - O;
  திரும்பு *இது;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(மாறிலி SlowDynamicAPInt &O) {
  *இது = *இது * O;
  திரும்பு *இது;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(மாறிலி SlowDynamicAPInt &O) {
  *இது = *இது / O;
  திரும்பு *இது;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(மாறிலி SlowDynamicAPInt &O) {
  *இது = *இது % O;
  திரும்பு *இது;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *இது += 1;
  திரும்பு *இது;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *இது -= 1;
  திரும்பு *இது;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
வெறுமை SlowDynamicAPInt::print(raw_ostream &OS) மாறிலி { OS << Val; }

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை SlowDynamicAPInt::dump() மாறிலி { print(dbgs()); }
#endif
