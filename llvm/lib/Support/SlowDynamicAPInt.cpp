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
SlowDynamicAPInt::SlowDynamicAPInt(अचर APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::चालक=(int64_t Val) {
  फर्काऊ *एतत् = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::चालक int64_t() अचर { फर्काऊ Val.get_SExt_मान(); }

hash_code llvm::detail::hash_मान(अचर SlowDynamicAPInt &X) {
  फर्काऊ hash_मान(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::चालक+=(SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक-=(SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक*=(SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक/=(SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक%=(SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A %= SlowDynamicAPInt(B);
}

तार्किक llvm::detail::चालक==(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A == SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक!=(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A != SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक>(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A > SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक<(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A < SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक<=(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A <= SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक>=(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक+(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक-(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक*(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक/(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक%(अचर SlowDynamicAPInt &A, int64_t B) {
  फर्काऊ A % SlowDynamicAPInt(B);
}

तार्किक llvm::detail::चालक==(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) == B;
}
तार्किक llvm::detail::चालक!=(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) != B;
}
तार्किक llvm::detail::चालक>(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) > B;
}
तार्किक llvm::detail::चालक<(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) < B;
}
तार्किक llvm::detail::चालक<=(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) <= B;
}
तार्किक llvm::detail::चालक>=(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::चालक+(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::चालक-(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::चालक*(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::चालक/(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::चालक%(int64_t A, अचर SlowDynamicAPInt &B) {
  फर्काऊ SlowDynamicAPInt(A) % B;
}

स्थिर अचिन्हित getMaxWidth(अचर APInt &A, अचर APInt &B) {
  फर्काऊ std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
तार्किक SlowDynamicAPInt::चालक==(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  फर्काऊ Val.sext(Width) == O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::चालक!=(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  फर्काऊ Val.sext(Width) != O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::चालक>(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  फर्काऊ Val.sext(Width).sgt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक<(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  फर्काऊ Val.sext(Width).slt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक<=(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  फर्काऊ Val.sext(Width).sle(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक>=(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  फर्काऊ Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    अचर APInt &A, अचर APInt &B,
    function_ref<APInt(अचर APInt &, अचर APInt &, तार्किक &Overflow)> Op) {
  तार्किक Overflow;
  अचिन्हित Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  यदि (!Overflow)
    फर्काऊ Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  प्रतिज्ञा(!Overflow && "double width should be sufficient to avoid overflow!");
  फर्काऊ Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::चालक+(अचर SlowDynamicAPInt &O) अचर {
  फर्काऊ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक-(अचर SlowDynamicAPInt &O) अचर {
  फर्काऊ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक*(अचर SlowDynamicAPInt &O) अचर {
  फर्काऊ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक/(अचर SlowDynamicAPInt &O) अचर {
  फर्काऊ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::असल(अचर SlowDynamicAPInt &X) {
  फर्काऊ X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(अचर SlowDynamicAPInt &LHS,
                                       अचर SlowDynamicAPInt &RHS) {
  यदि (RHS == -1)
    फर्काऊ -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  फर्काऊ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(अचर SlowDynamicAPInt &LHS,
                                        अचर SlowDynamicAPInt &RHS) {
  यदि (RHS == -1)
    फर्काऊ -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  फर्काऊ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(अचर SlowDynamicAPInt &LHS,
                                   अचर SlowDynamicAPInt &RHS) {
  प्रतिज्ञा(RHS >= 1 && "mod is only supported for positive divisors!");
  फर्काऊ LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(अचर SlowDynamicAPInt &A,
                                   अचर SlowDynamicAPInt &B) {
  प्रतिज्ञा(A >= 0 && B >= 0 && "operands must be non-negative!");
  अचिन्हित Width = getMaxWidth(A.Val, B.Val);
  फर्काऊ SlowDynamicAPInt(
      APIntOps::Greatest_Common_भाजक(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(अचर SlowDynamicAPInt &A,
                                   अचर SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = असल(A);
  SlowDynamicAPInt Y = असल(B);
  फर्काऊ (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::चालक%(अचर SlowDynamicAPInt &O) अचर {
  अचिन्हित Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  फर्काऊ SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::चालक-() अचर {
  यदि (Val.is_Min_Signed_मान()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    फर्काऊ SlowDynamicAPInt(-Ret);
  }
  फर्काऊ SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::चालक+=(अचर SlowDynamicAPInt &O) {
  *एतत् = *एतत् + O;
  फर्काऊ *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक-=(अचर SlowDynamicAPInt &O) {
  *एतत् = *एतत् - O;
  फर्काऊ *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक*=(अचर SlowDynamicAPInt &O) {
  *एतत् = *एतत् * O;
  फर्काऊ *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक/=(अचर SlowDynamicAPInt &O) {
  *एतत् = *एतत् / O;
  फर्काऊ *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक%=(अचर SlowDynamicAPInt &O) {
  *एतत् = *एतत् % O;
  फर्काऊ *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक++() {
  *एतत् += 1;
  फर्काऊ *एतत्;
}

SlowDynamicAPInt &SlowDynamicAPInt::चालक--() {
  *एतत् -= 1;
  फर्काऊ *एतत्;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
शून्य SlowDynamicAPInt::print(raw_ostream &OS) अचर { OS << Val; }

#यदि !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SlowDynamicAPInt::dump() अचर { print(dbgs()); }
#endif
