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
    : Val(64, Val, /*isSigned=*/सत्य) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ध्रुव APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::चालक=(int64_t Val) {
  प्रत्यागम *एतत् = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::चालक int64_t() ध्रुव { प्रत्यागम Val.get_SExt_मूल्य(); }

hash_code llvm::detail::hash_मूल्य(ध्रुव SlowDynamicAPInt &X) {
  प्रत्यागम hash_मूल्य(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::चालक+=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक-=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक*=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक/=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक%=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A %= SlowDynamicAPInt(B);
}

तार्किक llvm::detail::चालक==(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A == SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक!=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A != SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक>(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A > SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक<(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A < SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक<=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A <= SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक>=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक+(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक-(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक*(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक/(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक%(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A % SlowDynamicAPInt(B);
}

तार्किक llvm::detail::चालक==(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) == B;
}
तार्किक llvm::detail::चालक!=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) != B;
}
तार्किक llvm::detail::चालक>(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) > B;
}
तार्किक llvm::detail::चालक<(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) < B;
}
तार्किक llvm::detail::चालक<=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) <= B;
}
तार्किक llvm::detail::चालक>=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::चालक+(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::चालक-(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::चालक*(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::चालक/(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::चालक%(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) % B;
}

स्थिर अचिन्हित getMaxWidth(ध्रुव APInt &A, ध्रुव APInt &B) {
  प्रत्यागम std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
तार्किक SlowDynamicAPInt::चालक==(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  प्रत्यागम Val.sext(Width) == O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::चालक!=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  प्रत्यागम Val.sext(Width) != O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::चालक>(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  प्रत्यागम Val.sext(Width).sgt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक<(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  प्रत्यागम Val.sext(Width).slt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक<=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  प्रत्यागम Val.sext(Width).sle(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक>=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  प्रत्यागम Val.sext(Width).sge(O.Val.sext(Width));
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
  अचिन्हित Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  यदि (!Overflow)
    प्रत्यागम Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  प्रतिज्ञा(!Overflow && "double width should be sufficient to avoid overflow!");
  प्रत्यागम Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::चालक+(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक-(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक*(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक/(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::असल(ध्रुव SlowDynamicAPInt &X) {
  प्रत्यागम X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ध्रुव SlowDynamicAPInt &LHS,
                                       ध्रुव SlowDynamicAPInt &RHS) {
  यदि (RHS == -1)
    प्रत्यागम -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  प्रत्यागम SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(ध्रुव SlowDynamicAPInt &LHS,
                                        ध्रुव SlowDynamicAPInt &RHS) {
  यदि (RHS == -1)
    प्रत्यागम -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  प्रत्यागम SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ध्रुव SlowDynamicAPInt &LHS,
                                   ध्रुव SlowDynamicAPInt &RHS) {
  प्रतिज्ञा(RHS >= 1 && "mod is only supported for positive divisors!");
  प्रत्यागम LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(ध्रुव SlowDynamicAPInt &A,
                                   ध्रुव SlowDynamicAPInt &B) {
  प्रतिज्ञा(A >= 0 && B >= 0 && "operands must be non-negative!");
  अचिन्हित Width = getMaxWidth(A.Val, B.Val);
  प्रत्यागम SlowDynamicAPInt(
      APIntOps::Greatest_Common_भाजक(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ध्रुव SlowDynamicAPInt &A,
                                   ध्रुव SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = असल(A);
  SlowDynamicAPInt Y = असल(B);
  प्रत्यागम (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::चालक%(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिन्हित Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  प्रत्यागम SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::चालक-() ध्रुव {
  यदि (Val.is_Min_Signed_मूल्य()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    प्रत्यागम SlowDynamicAPInt(-Ret);
  }
  प्रत्यागम SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::चालक+=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् + O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक-=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् - O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक*=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् * O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक/=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् / O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक%=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् % O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक++() {
  *एतत् += 1;
  प्रत्यागम *एतत्;
}

SlowDynamicAPInt &SlowDynamicAPInt::चालक--() {
  *एतत् -= 1;
  प्रत्यागम *एतत्;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
शून्य SlowDynamicAPInt::print(raw_ostream &OS) ध्रुव { OS << Val; }

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SlowDynamicAPInt::dump() ध्रुव { print(dbgs()); }
#endif
