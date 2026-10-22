//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#অন্তর্ভাব "llvm/ADT/SlowDynamicAPInt.h"
#অন্তর্ভাব "llvm/ADT/Hashing.h"
#অন্তর্ভাব "llvm/Support/Debug.h"
#অন্তর্ভাব "llvm/Support/raw_ostream.h"

using নামাকাশ llvm;
using নামাকাশ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/সত্য) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ধ্রুব APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  প্রত্যাগম *এতত্ = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() ধ্রুব { প্রত্যাগম Val.get_SExt_মান(); }

hash_code llvm::detail::hash_মান(ধ্রুব SlowDynamicAPInt &X) {
  প্রত্যাগম hash_মান(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A %= SlowDynamicAPInt(B);
}

তার্কিক llvm::detail::operator==(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A == SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator!=(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A != SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator>(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A > SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator<(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A < SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator<=(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A <= SlowDynamicAPInt(B);
}
তার্কিক llvm::detail::operator>=(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(ধ্রুব SlowDynamicAPInt &A, int64_t B) {
  প্রত্যাগম A % SlowDynamicAPInt(B);
}

তার্কিক llvm::detail::operator==(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) == B;
}
তার্কিক llvm::detail::operator!=(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) != B;
}
তার্কিক llvm::detail::operator>(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) > B;
}
তার্কিক llvm::detail::operator<(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) < B;
}
তার্কিক llvm::detail::operator<=(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) <= B;
}
তার্কিক llvm::detail::operator>=(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, ধ্রুব SlowDynamicAPInt &B) {
  প্রত্যাগম SlowDynamicAPInt(A) % B;
}

স্থির unsigned getMaxWidth(ধ্রুব APInt &A, ধ্রুব APInt &B) {
  প্রত্যাগম std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
তার্কিক SlowDynamicAPInt::operator==(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = getMaxWidth(Val, O.Val);
  প্রত্যাগম Val.sext(Width) == O.Val.sext(Width);
}
তার্কিক SlowDynamicAPInt::operator!=(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = getMaxWidth(Val, O.Val);
  প্রত্যাগম Val.sext(Width) != O.Val.sext(Width);
}
তার্কিক SlowDynamicAPInt::operator>(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = getMaxWidth(Val, O.Val);
  প্রত্যাগম Val.sext(Width).sgt(O.Val.sext(Width));
}
তার্কিক SlowDynamicAPInt::operator<(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = getMaxWidth(Val, O.Val);
  প্রত্যাগম Val.sext(Width).slt(O.Val.sext(Width));
}
তার্কিক SlowDynamicAPInt::operator<=(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = getMaxWidth(Val, O.Val);
  প্রত্যাগম Val.sext(Width).sle(O.Val.sext(Width));
}
তার্কিক SlowDynamicAPInt::operator>=(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = getMaxWidth(Val, O.Val);
  প্রত্যাগম Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    ধ্রুব APInt &A, ধ্রুব APInt &B,
    function_ref<APInt(ধ্রুব APInt &, ধ্রুব APInt &, তার্কিক &Overflow)> Op) {
  তার্কিক Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  যদি (!Overflow)
    প্রত্যাগম Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  প্রতিজ্ঞা(!Overflow && "double width should be sufficient to avoid overflow!");
  প্রত্যাগম Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  প্রত্যাগম SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  প্রত্যাগম SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  প্রত্যাগম SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  প্রত্যাগম SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(ধ্রুব SlowDynamicAPInt &X) {
  প্রত্যাগম X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ধ্রুব SlowDynamicAPInt &LHS,
                                       ধ্রুব SlowDynamicAPInt &RHS) {
  যদি (RHS == -1)
    প্রত্যাগম -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  প্রত্যাগম SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ধ্রুব SlowDynamicAPInt &LHS,
                                        ধ্রুব SlowDynamicAPInt &RHS) {
  যদি (RHS == -1)
    প্রত্যাগম -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  প্রত্যাগম SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ধ্রুব SlowDynamicAPInt &LHS,
                                   ধ্রুব SlowDynamicAPInt &RHS) {
  প্রতিজ্ঞা(RHS >= 1 && "mod is only supported for positive divisors!");
  প্রত্যাগম LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ধ্রুব SlowDynamicAPInt &A,
                                   ধ্রুব SlowDynamicAPInt &B) {
  প্রতিজ্ঞা(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  প্রত্যাগম SlowDynamicAPInt(
      APIntOps::Greatest_Common_ভাজক(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ধ্রুব SlowDynamicAPInt &A,
                                   ধ্রুব SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  প্রত্যাগম (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(ধ্রুব SlowDynamicAPInt &O) ধ্রুব {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  প্রত্যাগম SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() ধ্রুব {
  যদি (Val.is_Min_Signed_মান()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    প্রত্যাগম SlowDynamicAPInt(-Ret);
  }
  প্রত্যাগম SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(ধ্রুব SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ + O;
  প্রত্যাগম *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(ধ্রুব SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ - O;
  প্রত্যাগম *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(ধ্রুব SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ * O;
  প্রত্যাগম *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(ধ্রুব SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ / O;
  প্রত্যাগম *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(ধ্রুব SlowDynamicAPInt &O) {
  *এতত্ = *এতত্ % O;
  প্রত্যাগম *এতত্;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *এতত্ += 1;
  প্রত্যাগম *এতত্;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *এতত্ -= 1;
  প্রত্যাগম *এতত্;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
শূন্য SlowDynamicAPInt::print(raw_ostream &OS) ধ্রুব { OS << Val; }

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য SlowDynamicAPInt::dump() ধ্রুব { print(dbgs()); }
#endif
