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

استفاده نطاق llvm;
استفاده نطاق llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/درست) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ثابت APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::عملگر=(int64_t Val) {
  أرجع *هذا = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::عملگر int64_t() ثابت { أرجع Val.get_SExt_مقدار(); }

hash_کد llvm::detail::hash_مقدار(ثابت SlowDynamicAPInt &X) {
  أرجع hash_مقدار(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::عملگر+=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عملگر-=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عملگر*=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عملگر/=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عملگر%=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A %= SlowDynamicAPInt(B);
}

منطقي llvm::detail::عملگر==(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A == SlowDynamicAPInt(B);
}
منطقي llvm::detail::عملگر!=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A != SlowDynamicAPInt(B);
}
منطقي llvm::detail::عملگر>(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A > SlowDynamicAPInt(B);
}
منطقي llvm::detail::عملگر<(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A < SlowDynamicAPInt(B);
}
منطقي llvm::detail::عملگر<=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A <= SlowDynamicAPInt(B);
}
منطقي llvm::detail::عملگر>=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عملگر+(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عملگر-(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عملگر*(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عملگر/(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عملگر%(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A % SlowDynamicAPInt(B);
}

منطقي llvm::detail::عملگر==(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) == B;
}
منطقي llvm::detail::عملگر!=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) != B;
}
منطقي llvm::detail::عملگر>(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) > B;
}
منطقي llvm::detail::عملگر<(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) < B;
}
منطقي llvm::detail::عملگر<=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) <= B;
}
منطقي llvm::detail::عملگر>=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::عملگر+(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::عملگر-(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::عملگر*(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::عملگر/(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::عملگر%(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) % B;
}

ساكن بیعلامت get_Max_عرض(ثابت APInt &A, ثابت APInt &B) {
  أرجع std::max(A.get_بیت_عرض(), B.get_بیت_عرض());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
منطقي SlowDynamicAPInt::عملگر==(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = get_Max_عرض(Val, O.Val);
  أرجع Val.sext(عرض) == O.Val.sext(عرض);
}
منطقي SlowDynamicAPInt::عملگر!=(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = get_Max_عرض(Val, O.Val);
  أرجع Val.sext(عرض) != O.Val.sext(عرض);
}
منطقي SlowDynamicAPInt::عملگر>(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = get_Max_عرض(Val, O.Val);
  أرجع Val.sext(عرض).sgt(O.Val.sext(عرض));
}
منطقي SlowDynamicAPInt::عملگر<(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = get_Max_عرض(Val, O.Val);
  أرجع Val.sext(عرض).slt(O.Val.sext(عرض));
}
منطقي SlowDynamicAPInt::عملگر<=(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = get_Max_عرض(Val, O.Val);
  أرجع Val.sext(عرض).sle(O.Val.sext(عرض));
}
منطقي SlowDynamicAPInt::عملگر>=(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = get_Max_عرض(Val, O.Val);
  أرجع Val.sext(عرض).sge(O.Val.sext(عرض));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt پیاپی_Op_With_Expand_On_سرریز(
    ثابت APInt &A, ثابت APInt &B,
    function_ref<APInt(ثابت APInt &, ثابت APInt &, منطقي &سرریز)> Op) {
  منطقي سرریز;
  بیعلامت عرض = get_Max_عرض(A, B);
  APInt Ret = Op(A.sext(عرض), B.sext(عرض), سرریز);
  إذا (!سرریز)
    أرجع Ret;

  عرض *= 2;
  Ret = Op(A.sext(عرض), B.sext(عرض), سرریز);
  أكد(!سرریز && "double width should be sufficient to avoid overflow!");
  أرجع Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::عملگر+(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پیاپی_Op_With_Expand_On_سرریز(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عملگر-(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پیاپی_Op_With_Expand_On_سرریز(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عملگر*(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پیاپی_Op_With_Expand_On_سرریز(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عملگر/(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پیاپی_Op_With_Expand_On_سرریز(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::قدر_مطلق(ثابت SlowDynamicAPInt &X) {
  أرجع X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ثابت SlowDynamicAPInt &LHS,
                                       ثابت SlowDynamicAPInt &RHS) {
  إذا (RHS == -1)
    أرجع -LHS;
  بیعلامت عرض = get_Max_عرض(LHS.Val, RHS.Val);
  أرجع SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(عرض), RHS.Val.sext(عرض), APInt::Rounding::به_بالا));
}
SlowDynamicAPInt llvm::detail::floorDiv(ثابت SlowDynamicAPInt &LHS,
                                        ثابت SlowDynamicAPInt &RHS) {
  إذا (RHS == -1)
    أرجع -LHS;
  بیعلامت عرض = get_Max_عرض(LHS.Val, RHS.Val);
  أرجع SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(عرض), RHS.Val.sext(عرض), APInt::Rounding::پایین));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ثابت SlowDynamicAPInt &LHS,
                                   ثابت SlowDynamicAPInt &RHS) {
  أكد(RHS >= 1 && "mod is only supported for positive divisors!");
  أرجع LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::بزرگترین_مقسوم_علیه(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  أكد(A >= 0 && B >= 0 && "operands must be non-negative!");
  بیعلامت عرض = get_Max_عرض(A.Val, B.Val);
  أرجع SlowDynamicAPInt(
      APIntOps::Greatest_Common_مقسوم_علیه(A.Val.sext(عرض), B.Val.sext(عرض)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = قدر_مطلق(A);
  SlowDynamicAPInt Y = قدر_مطلق(B);
  أرجع (X * Y) / بزرگترین_مقسوم_علیه(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::عملگر%(ثابت SlowDynamicAPInt &O) ثابت {
  بیعلامت عرض = std::max(Val.get_بیت_عرض(), O.Val.get_بیت_عرض());
  أرجع SlowDynamicAPInt(Val.sext(عرض).srem(O.Val.sext(عرض)));
}

SlowDynamicAPInt SlowDynamicAPInt::عملگر-() ثابت {
  إذا (Val.is_Min_Signed_مقدار()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_بیت_عرض());
    أرجع SlowDynamicAPInt(-Ret);
  }
  أرجع SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::عملگر+=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا + O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عملگر-=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا - O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عملگر*=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا * O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عملگر/=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا / O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عملگر%=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا % O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::عملگر++() {
  *هذا += 1;
  أرجع *هذا;
}

SlowDynamicAPInt &SlowDynamicAPInt::عملگر--() {
  *هذا -= 1;
  أرجع *هذا;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
فراغ SlowDynamicAPInt::print(raw_ostream &OS) ثابت { OS << Val; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_روش فراغ SlowDynamicAPInt::dump() ثابت { print(dbgs()); }
#پایان_اگر
