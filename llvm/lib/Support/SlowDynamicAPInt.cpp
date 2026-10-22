//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/SlowDynamicAPInt.h"
#अन्तर्भाव "llvm/ADT/Hashing.h"
#अन्तर्भाव "llvm/Support/Debug.h"
#अन्तर्भाव "llvm/Support/raw_ostream.h"

using नामाकाश llvm;
using नामाकाश llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/सच्च) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ध्रुव APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  पटिनिवत्त *एतत् = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() ध्रुव { पटिनिवत्त Val.get_SExt_मानम्(); }

hash_code llvm::detail::hash_मानम्(ध्रुव SlowDynamicAPInt &X) {
  पटिनिवत्त hash_मानम्(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A %= SlowDynamicAPInt(B);
}

तार्किक llvm::detail::operator==(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A == SlowDynamicAPInt(B);
}
तार्किक llvm::detail::operator!=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A != SlowDynamicAPInt(B);
}
तार्किक llvm::detail::operator>(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A > SlowDynamicAPInt(B);
}
तार्किक llvm::detail::operator<(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A < SlowDynamicAPInt(B);
}
तार्किक llvm::detail::operator<=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A <= SlowDynamicAPInt(B);
}
तार्किक llvm::detail::operator>=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  पटिनिवत्त A % SlowDynamicAPInt(B);
}

तार्किक llvm::detail::operator==(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) == B;
}
तार्किक llvm::detail::operator!=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) != B;
}
तार्किक llvm::detail::operator>(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) > B;
}
तार्किक llvm::detail::operator<(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) < B;
}
तार्किक llvm::detail::operator<=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) <= B;
}
तार्किक llvm::detail::operator>=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  पटिनिवत्त SlowDynamicAPInt(A) % B;
}

स्थिर unsigned getMaxWidth(ध्रुव APInt &A, ध्रुव APInt &B) {
  पटिनिवत्त std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
तार्किक SlowDynamicAPInt::operator==(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = getMaxWidth(Val, O.Val);
  पटिनिवत्त Val.sext(Width) == O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::operator!=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = getMaxWidth(Val, O.Val);
  पटिनिवत्त Val.sext(Width) != O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::operator>(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = getMaxWidth(Val, O.Val);
  पटिनिवत्त Val.sext(Width).sgt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::operator<(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = getMaxWidth(Val, O.Val);
  पटिनिवत्त Val.sext(Width).slt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::operator<=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = getMaxWidth(Val, O.Val);
  पटिनिवत्त Val.sext(Width).sle(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::operator>=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = getMaxWidth(Val, O.Val);
  पटिनिवत्त Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    ध्रुव APInt &A, ध्रुव APInt &B,
    function_ref<APInt(ध्रुव APInt &, ध्रुव APInt &, तार्किक &Overflow)> Op) {
  तार्किक Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  सचे (!Overflow)
    पटिनिवत्त Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  प्रतिज्ञा(!Overflow && "double width should be sufficient to avoid overflow!");
  पटिनिवत्त Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  पटिनिवत्त SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  पटिनिवत्त SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  पटिनिवत्त SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  पटिनिवत्त SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(ध्रुव SlowDynamicAPInt &X) {
  पटिनिवत्त X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ध्रुव SlowDynamicAPInt &LHS,
                                       ध्रुव SlowDynamicAPInt &RHS) {
  सचे (RHS == -1)
    पटिनिवत्त -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  पटिनिवत्त SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ध्रुव SlowDynamicAPInt &LHS,
                                        ध्रुव SlowDynamicAPInt &RHS) {
  सचे (RHS == -1)
    पटिनिवत्त -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  पटिनिवत्त SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ध्रुव SlowDynamicAPInt &LHS,
                                   ध्रुव SlowDynamicAPInt &RHS) {
  प्रतिज्ञा(RHS >= 1 && "mod is only supported for positive divisors!");
  पटिनिवत्त LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ध्रुव SlowDynamicAPInt &A,
                                   ध्रुव SlowDynamicAPInt &B) {
  प्रतिज्ञा(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  पटिनिवत्त SlowDynamicAPInt(
      APIntOps::Greatest_Common_भाजकः(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ध्रुव SlowDynamicAPInt &A,
                                   ध्रुव SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  पटिनिवत्त (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  पटिनिवत्त SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() ध्रुव {
  सचे (Val.is_Min_Signed_मानम्()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    पटिनिवत्त SlowDynamicAPInt(-Ret);
  }
  पटिनिवत्त SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् + O;
  पटिनिवत्त *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् - O;
  पटिनिवत्त *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् * O;
  पटिनिवत्त *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् / O;
  पटिनिवत्त *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् % O;
  पटिनिवत्त *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *एतत् += 1;
  पटिनिवत्त *एतत्;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *एतत् -= 1;
  पटिनिवत्त *एतत्;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
शून्य SlowDynamicAPInt::print(raw_ostream &OS) ध्रुव { OS << Val; }

#सचे !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SlowDynamicAPInt::dump() ध्रुव { print(dbgs()); }
#endif
