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
SlowDynamicAPInt::SlowDynamicAPInt(स्थिरांक APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::चालक=(int64_t Val) {
  परत *हे = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::चालक int64_t() स्थिरांक { परत Val.get_SExt_मूल्य(); }

hash_code llvm::detail::hash_मूल्य(स्थिरांक SlowDynamicAPInt &X) {
  परत hash_मूल्य(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::चालक+=(SlowDynamicAPInt &A, int64_t B) {
  परत A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक-=(SlowDynamicAPInt &A, int64_t B) {
  परत A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक*=(SlowDynamicAPInt &A, int64_t B) {
  परत A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक/=(SlowDynamicAPInt &A, int64_t B) {
  परत A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::चालक%=(SlowDynamicAPInt &A, int64_t B) {
  परत A %= SlowDynamicAPInt(B);
}

तार्किक llvm::detail::चालक==(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A == SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक!=(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A != SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक>(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A > SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक<(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A < SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक<=(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A <= SlowDynamicAPInt(B);
}
तार्किक llvm::detail::चालक>=(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक+(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक-(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक*(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक/(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::चालक%(स्थिरांक SlowDynamicAPInt &A, int64_t B) {
  परत A % SlowDynamicAPInt(B);
}

तार्किक llvm::detail::चालक==(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) == B;
}
तार्किक llvm::detail::चालक!=(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) != B;
}
तार्किक llvm::detail::चालक>(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) > B;
}
तार्किक llvm::detail::चालक<(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) < B;
}
तार्किक llvm::detail::चालक<=(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) <= B;
}
तार्किक llvm::detail::चालक>=(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::चालक+(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::चालक-(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::चालक*(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::चालक/(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::चालक%(int64_t A, स्थिरांक SlowDynamicAPInt &B) {
  परत SlowDynamicAPInt(A) % B;
}

स्थिर अचिन्हित getMaxWidth(स्थिरांक APInt &A, स्थिरांक APInt &B) {
  परत std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
तार्किक SlowDynamicAPInt::चालक==(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  परत Val.sext(Width) == O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::चालक!=(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  परत Val.sext(Width) != O.Val.sext(Width);
}
तार्किक SlowDynamicAPInt::चालक>(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  परत Val.sext(Width).sgt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक<(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  परत Val.sext(Width).slt(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक<=(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  परत Val.sext(Width).sle(O.Val.sext(Width));
}
तार्किक SlowDynamicAPInt::चालक>=(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = getMaxWidth(Val, O.Val);
  परत Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    स्थिरांक APInt &A, स्थिरांक APInt &B,
    function_ref<APInt(स्थिरांक APInt &, स्थिरांक APInt &, तार्किक &Overflow)> Op) {
  तार्किक Overflow;
  अचिन्हित Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  जर (!Overflow)
    परत Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  प्रतिज्ञा(!Overflow && "double width should be sufficient to avoid overflow!");
  परत Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::चालक+(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  परत SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक-(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  परत SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक*(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  परत SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::चालक/(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  परत SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::असल(स्थिरांक SlowDynamicAPInt &X) {
  परत X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(स्थिरांक SlowDynamicAPInt &LHS,
                                       स्थिरांक SlowDynamicAPInt &RHS) {
  जर (RHS == -1)
    परत -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  परत SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(स्थिरांक SlowDynamicAPInt &LHS,
                                        स्थिरांक SlowDynamicAPInt &RHS) {
  जर (RHS == -1)
    परत -LHS;
  अचिन्हित Width = getMaxWidth(LHS.Val, RHS.Val);
  परत SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(स्थिरांक SlowDynamicAPInt &LHS,
                                   स्थिरांक SlowDynamicAPInt &RHS) {
  प्रतिज्ञा(RHS >= 1 && "mod is only supported for positive divisors!");
  परत LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(स्थिरांक SlowDynamicAPInt &A,
                                   स्थिरांक SlowDynamicAPInt &B) {
  प्रतिज्ञा(A >= 0 && B >= 0 && "operands must be non-negative!");
  अचिन्हित Width = getMaxWidth(A.Val, B.Val);
  परत SlowDynamicAPInt(
      APIntOps::Greatest_Common_भाजक(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(स्थिरांक SlowDynamicAPInt &A,
                                   स्थिरांक SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = असल(A);
  SlowDynamicAPInt Y = असल(B);
  परत (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::चालक%(स्थिरांक SlowDynamicAPInt &O) स्थिरांक {
  अचिन्हित Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  परत SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::चालक-() स्थिरांक {
  जर (Val.is_Min_Signed_मूल्य()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    परत SlowDynamicAPInt(-Ret);
  }
  परत SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::चालक+=(स्थिरांक SlowDynamicAPInt &O) {
  *हे = *हे + O;
  परत *हे;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक-=(स्थिरांक SlowDynamicAPInt &O) {
  *हे = *हे - O;
  परत *हे;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक*=(स्थिरांक SlowDynamicAPInt &O) {
  *हे = *हे * O;
  परत *हे;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक/=(स्थिरांक SlowDynamicAPInt &O) {
  *हे = *हे / O;
  परत *हे;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक%=(स्थिरांक SlowDynamicAPInt &O) {
  *हे = *हे % O;
  परत *हे;
}
SlowDynamicAPInt &SlowDynamicAPInt::चालक++() {
  *हे += 1;
  परत *हे;
}

SlowDynamicAPInt &SlowDynamicAPInt::चालक--() {
  *हे -= 1;
  परत *हे;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
शून्य SlowDynamicAPInt::print(raw_ostream &OS) स्थिरांक { OS << Val; }

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य SlowDynamicAPInt::dump() स्थिरांक { print(dbgs()); }
#endif
