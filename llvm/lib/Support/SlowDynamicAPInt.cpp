//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/SlowDynamicAPInt.h"
#ضمن "llvm/ADT/Hashing.h"
#ضمن "llvm/Support/Debug.h"
#ضمن "llvm/Support/raw_ostream.h"

استعمال نطاق llvm;
استعمال نطاق llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/سچ) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ثابت APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::عامل=(int64_t Val) {
  أرجع *هذا = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::عامل int64_t() ثابت { أرجع Val.get_SExt_قدر(); }

hash_ڪوڊ llvm::detail::hash_قدر(ثابت SlowDynamicAPInt &X) {
  أرجع hash_قدر(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::عامل+=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل-=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل*=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل/=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل%=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A %= SlowDynamicAPInt(B);
}

منطقي llvm::detail::عامل==(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A == SlowDynamicAPInt(B);
}
منطقي llvm::detail::عامل!=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A != SlowDynamicAPInt(B);
}
منطقي llvm::detail::عامل>(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A > SlowDynamicAPInt(B);
}
منطقي llvm::detail::عامل<(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A < SlowDynamicAPInt(B);
}
منطقي llvm::detail::عامل<=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A <= SlowDynamicAPInt(B);
}
منطقي llvm::detail::عامل>=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل+(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل-(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل*(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل/(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل%(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A % SlowDynamicAPInt(B);
}

منطقي llvm::detail::عامل==(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) == B;
}
منطقي llvm::detail::عامل!=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) != B;
}
منطقي llvm::detail::عامل>(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) > B;
}
منطقي llvm::detail::عامل<(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) < B;
}
منطقي llvm::detail::عامل<=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) <= B;
}
منطقي llvm::detail::عامل>=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::عامل+(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::عامل-(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::عامل*(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::عامل/(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::عامل%(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) % B;
}

ساكن بنانشان get_Max_ويڪر(ثابت APInt &A, ثابت APInt &B) {
  أرجع std::max(A.get_ٻٽو_انگ_ويڪر(), B.get_ٻٽو_انگ_ويڪر());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
منطقي SlowDynamicAPInt::عامل==(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = get_Max_ويڪر(Val, O.Val);
  أرجع Val.sext(ويڪر) == O.Val.sext(ويڪر);
}
منطقي SlowDynamicAPInt::عامل!=(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = get_Max_ويڪر(Val, O.Val);
  أرجع Val.sext(ويڪر) != O.Val.sext(ويڪر);
}
منطقي SlowDynamicAPInt::عامل>(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = get_Max_ويڪر(Val, O.Val);
  أرجع Val.sext(ويڪر).sgt(O.Val.sext(ويڪر));
}
منطقي SlowDynamicAPInt::عامل<(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = get_Max_ويڪر(Val, O.Val);
  أرجع Val.sext(ويڪر).slt(O.Val.sext(ويڪر));
}
منطقي SlowDynamicAPInt::عامل<=(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = get_Max_ويڪر(Val, O.Val);
  أرجع Val.sext(ويڪر).sle(O.Val.sext(ويڪر));
}
منطقي SlowDynamicAPInt::عامل>=(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = get_Max_ويڪر(Val, O.Val);
  أرجع Val.sext(ويڪر).sge(O.Val.sext(ويڪر));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt لاڳيتو_Op_With_Expand_On_حد_کان_مٿي(
    ثابت APInt &A, ثابت APInt &B,
    function_ref<APInt(ثابت APInt &, ثابت APInt &, منطقي &حد_کان_مٿي)> Op) {
  منطقي حد_کان_مٿي;
  بنانشان ويڪر = get_Max_ويڪر(A, B);
  APInt Ret = Op(A.sext(ويڪر), B.sext(ويڪر), حد_کان_مٿي);
  إذا (!حد_کان_مٿي)
    أرجع Ret;

  ويڪر *= 2;
  Ret = Op(A.sext(ويڪر), B.sext(ويڪر), حد_کان_مٿي);
  أكد(!حد_کان_مٿي && "double width should be sufficient to avoid overflow!");
  أرجع Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::عامل+(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      لاڳيتو_Op_With_Expand_On_حد_کان_مٿي(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عامل-(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      لاڳيتو_Op_With_Expand_On_حد_کان_مٿي(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عامل*(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      لاڳيتو_Op_With_Expand_On_حد_کان_مٿي(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عامل/(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      لاڳيتو_Op_With_Expand_On_حد_کان_مٿي(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::مطلق_قدر(ثابت SlowDynamicAPInt &X) {
  أرجع X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ثابت SlowDynamicAPInt &LHS,
                                       ثابت SlowDynamicAPInt &RHS) {
  إذا (RHS == -1)
    أرجع -LHS;
  بنانشان ويڪر = get_Max_ويڪر(LHS.Val, RHS.Val);
  أرجع SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(ويڪر), RHS.Val.sext(ويڪر), APInt::Rounding::مٿي));
}
SlowDynamicAPInt llvm::detail::floorDiv(ثابت SlowDynamicAPInt &LHS,
                                        ثابت SlowDynamicAPInt &RHS) {
  إذا (RHS == -1)
    أرجع -LHS;
  بنانشان ويڪر = get_Max_ويڪر(LHS.Val, RHS.Val);
  أرجع SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(ويڪر), RHS.Val.sext(ويڪر), APInt::Rounding::هيٺ_ڏانهن));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ثابت SlowDynamicAPInt &LHS,
                                   ثابت SlowDynamicAPInt &RHS) {
  أكد(RHS >= 1 && "mod is only supported for positive divisors!");
  أرجع LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::وڏو_گڏيل_ونڊيندڙ(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  أكد(A >= 0 && B >= 0 && "operands must be non-negative!");
  بنانشان ويڪر = get_Max_ويڪر(A.Val, B.Val);
  أرجع SlowDynamicAPInt(
      APIntOps::Greatest_Common_ورهائيندڙ(A.Val.sext(ويڪر), B.Val.sext(ويڪر)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = مطلق_قدر(A);
  SlowDynamicAPInt Y = مطلق_قدر(B);
  أرجع (X * Y) / وڏو_گڏيل_ونڊيندڙ(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::عامل%(ثابت SlowDynamicAPInt &O) ثابت {
  بنانشان ويڪر = std::max(Val.get_ٻٽو_انگ_ويڪر(), O.Val.get_ٻٽو_انگ_ويڪر());
  أرجع SlowDynamicAPInt(Val.sext(ويڪر).srem(O.Val.sext(ويڪر)));
}

SlowDynamicAPInt SlowDynamicAPInt::عامل-() ثابت {
  إذا (Val.is_Min_Signed_قدر()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_ٻٽو_انگ_ويڪر());
    أرجع SlowDynamicAPInt(-Ret);
  }
  أرجع SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::عامل+=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا + O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل-=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا - O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل*=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا * O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل/=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا / O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل%=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا % O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل++() {
  *هذا += 1;
  أرجع *هذا;
}

SlowDynamicAPInt &SlowDynamicAPInt::عامل--() {
  *هذا -= 1;
  أرجع *هذا;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
فراغ SlowDynamicAPInt::print(raw_ostream &OS) ثابت { OS << Val; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طريقو فراغ SlowDynamicAPInt::dump() ثابت { print(dbgs()); }
#جيڪڏهن_پورو
