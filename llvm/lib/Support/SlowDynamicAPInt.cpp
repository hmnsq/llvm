//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/SlowDynamicAPInt.h"
#شامل "llvm/ADT/Hashing.h"
#شامل "llvm/Support/Debug.h"
#شامل "llvm/Support/raw_ostream.h"

using فضاینام llvm;
using فضاینام llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/درست) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ثابت APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  بازگشت *این = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() ثابت { بازگشت Val.get_SExt_مقدار(); }

hash_code llvm::detail::hash_مقدار(ثابت SlowDynamicAPInt &X) {
  بازگشت hash_مقدار(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  بازگشت A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  بازگشت A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  بازگشت A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  بازگشت A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  بازگشت A %= SlowDynamicAPInt(B);
}

منطقی llvm::detail::operator==(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A == SlowDynamicAPInt(B);
}
منطقی llvm::detail::operator!=(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A != SlowDynamicAPInt(B);
}
منطقی llvm::detail::operator>(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A > SlowDynamicAPInt(B);
}
منطقی llvm::detail::operator<(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A < SlowDynamicAPInt(B);
}
منطقی llvm::detail::operator<=(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A <= SlowDynamicAPInt(B);
}
منطقی llvm::detail::operator>=(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(ثابت SlowDynamicAPInt &A, int64_t B) {
  بازگشت A % SlowDynamicAPInt(B);
}

منطقی llvm::detail::operator==(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) == B;
}
منطقی llvm::detail::operator!=(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) != B;
}
منطقی llvm::detail::operator>(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) > B;
}
منطقی llvm::detail::operator<(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) < B;
}
منطقی llvm::detail::operator<=(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) <= B;
}
منطقی llvm::detail::operator>=(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, ثابت SlowDynamicAPInt &B) {
  بازگشت SlowDynamicAPInt(A) % B;
}

ایستا unsigned getMaxWidth(ثابت APInt &A, ثابت APInt &B) {
  بازگشت std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
منطقی SlowDynamicAPInt::operator==(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = getMaxWidth(Val, O.Val);
  بازگشت Val.sext(Width) == O.Val.sext(Width);
}
منطقی SlowDynamicAPInt::operator!=(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = getMaxWidth(Val, O.Val);
  بازگشت Val.sext(Width) != O.Val.sext(Width);
}
منطقی SlowDynamicAPInt::operator>(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = getMaxWidth(Val, O.Val);
  بازگشت Val.sext(Width).sgt(O.Val.sext(Width));
}
منطقی SlowDynamicAPInt::operator<(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = getMaxWidth(Val, O.Val);
  بازگشت Val.sext(Width).slt(O.Val.sext(Width));
}
منطقی SlowDynamicAPInt::operator<=(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = getMaxWidth(Val, O.Val);
  بازگشت Val.sext(Width).sle(O.Val.sext(Width));
}
منطقی SlowDynamicAPInt::operator>=(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = getMaxWidth(Val, O.Val);
  بازگشت Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    ثابت APInt &A, ثابت APInt &B,
    function_ref<APInt(ثابت APInt &, ثابت APInt &, منطقی &Overflow)> Op) {
  منطقی Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  اگر (!Overflow)
    بازگشت Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  أكد(!Overflow && "double width should be sufficient to avoid overflow!");
  بازگشت Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(ثابت SlowDynamicAPInt &O) ثابت {
  بازگشت SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(ثابت SlowDynamicAPInt &O) ثابت {
  بازگشت SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(ثابت SlowDynamicAPInt &O) ثابت {
  بازگشت SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(ثابت SlowDynamicAPInt &O) ثابت {
  بازگشت SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(ثابت SlowDynamicAPInt &X) {
  بازگشت X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ثابت SlowDynamicAPInt &LHS,
                                       ثابت SlowDynamicAPInt &RHS) {
  اگر (RHS == -1)
    بازگشت -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  بازگشت SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ثابت SlowDynamicAPInt &LHS,
                                        ثابت SlowDynamicAPInt &RHS) {
  اگر (RHS == -1)
    بازگشت -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  بازگشت SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ثابت SlowDynamicAPInt &LHS,
                                   ثابت SlowDynamicAPInt &RHS) {
  أكد(RHS >= 1 && "mod is only supported for positive divisors!");
  بازگشت LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  أكد(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  بازگشت SlowDynamicAPInt(
      APIntOps::Greatest_Common_مقسوم_علیه(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  بازگشت (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(ثابت SlowDynamicAPInt &O) ثابت {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  بازگشت SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() ثابت {
  اگر (Val.is_Min_Signed_مقدار()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    بازگشت SlowDynamicAPInt(-Ret);
  }
  بازگشت SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(ثابت SlowDynamicAPInt &O) {
  *این = *این + O;
  بازگشت *این;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(ثابت SlowDynamicAPInt &O) {
  *این = *این - O;
  بازگشت *این;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(ثابت SlowDynamicAPInt &O) {
  *این = *این * O;
  بازگشت *این;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(ثابت SlowDynamicAPInt &O) {
  *این = *این / O;
  بازگشت *این;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(ثابت SlowDynamicAPInt &O) {
  *این = *این % O;
  بازگشت *این;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *این += 1;
  بازگشت *این;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *این -= 1;
  بازگشت *این;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
تهی SlowDynamicAPInt::print(raw_ostream &OS) ثابت { OS << Val; }

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی SlowDynamicAPInt::dump() ثابت { print(dbgs()); }
#endif
