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

کارول نطاق llvm;
کارول نطاق llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/ریښتیا) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ثابت APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی=(int64_t Val) {
  أرجع *هذا = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::چلوونکی int64_t() ثابت { أرجع Val.get_SExt_ارزښت(); }

hash_کوډ llvm::detail::hash_ارزښت(ثابت SlowDynamicAPInt &X) {
  أرجع hash_ارزښت(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::چلوونکی+=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::چلوونکی-=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::چلوونکی*=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::چلوونکی/=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::چلوونکی%=(SlowDynamicAPInt &A, int64_t B) {
  أرجع A %= SlowDynamicAPInt(B);
}

منطقي llvm::detail::چلوونکی==(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A == SlowDynamicAPInt(B);
}
منطقي llvm::detail::چلوونکی!=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A != SlowDynamicAPInt(B);
}
منطقي llvm::detail::چلوونکی>(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A > SlowDynamicAPInt(B);
}
منطقي llvm::detail::چلوونکی<(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A < SlowDynamicAPInt(B);
}
منطقي llvm::detail::چلوونکی<=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A <= SlowDynamicAPInt(B);
}
منطقي llvm::detail::چلوونکی>=(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::چلوونکی+(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::چلوونکی-(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::چلوونکی*(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::چلوونکی/(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::چلوونکی%(ثابت SlowDynamicAPInt &A, int64_t B) {
  أرجع A % SlowDynamicAPInt(B);
}

منطقي llvm::detail::چلوونکی==(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) == B;
}
منطقي llvm::detail::چلوونکی!=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) != B;
}
منطقي llvm::detail::چلوونکی>(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) > B;
}
منطقي llvm::detail::چلوونکی<(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) < B;
}
منطقي llvm::detail::چلوونکی<=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) <= B;
}
منطقي llvm::detail::چلوونکی>=(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::چلوونکی+(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::چلوونکی-(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::چلوونکی*(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::چلوونکی/(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::چلوونکی%(int64_t A, ثابت SlowDynamicAPInt &B) {
  أرجع SlowDynamicAPInt(A) % B;
}

ساكن بینښه get_Max_پلنوالی(ثابت APInt &A, ثابت APInt &B) {
  أرجع std::max(A.get_بټ_پلنوالی(), B.get_بټ_پلنوالی());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
منطقي SlowDynamicAPInt::چلوونکی==(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = get_Max_پلنوالی(Val, O.Val);
  أرجع Val.sext(پلنوالی) == O.Val.sext(پلنوالی);
}
منطقي SlowDynamicAPInt::چلوونکی!=(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = get_Max_پلنوالی(Val, O.Val);
  أرجع Val.sext(پلنوالی) != O.Val.sext(پلنوالی);
}
منطقي SlowDynamicAPInt::چلوونکی>(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = get_Max_پلنوالی(Val, O.Val);
  أرجع Val.sext(پلنوالی).sgt(O.Val.sext(پلنوالی));
}
منطقي SlowDynamicAPInt::چلوونکی<(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = get_Max_پلنوالی(Val, O.Val);
  أرجع Val.sext(پلنوالی).slt(O.Val.sext(پلنوالی));
}
منطقي SlowDynamicAPInt::چلوونکی<=(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = get_Max_پلنوالی(Val, O.Val);
  أرجع Val.sext(پلنوالی).sle(O.Val.sext(پلنوالی));
}
منطقي SlowDynamicAPInt::چلوونکی>=(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = get_Max_پلنوالی(Val, O.Val);
  أرجع Val.sext(پلنوالی).sge(O.Val.sext(پلنوالی));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt پرلپسې_Op_With_Expand_On_ډکېدنه(
    ثابت APInt &A, ثابت APInt &B,
    function_ref<APInt(ثابت APInt &, ثابت APInt &, منطقي &ډکېدنه)> Op) {
  منطقي ډکېدنه;
  بینښه پلنوالی = get_Max_پلنوالی(A, B);
  APInt Ret = Op(A.sext(پلنوالی), B.sext(پلنوالی), ډکېدنه);
  إذا (!ډکېدنه)
    أرجع Ret;

  پلنوالی *= 2;
  Ret = Op(A.sext(پلنوالی), B.sext(پلنوالی), ډکېدنه);
  أكد(!ډکېدنه && "double width should be sufficient to avoid overflow!");
  أرجع Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::چلوونکی+(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پرلپسې_Op_With_Expand_On_ډکېدنه(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::چلوونکی-(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پرلپسې_Op_With_Expand_On_ډکېدنه(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::چلوونکی*(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پرلپسې_Op_With_Expand_On_ډکېدنه(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::چلوونکی/(ثابت SlowDynamicAPInt &O) ثابت {
  أرجع SlowDynamicAPInt(
      پرلپسې_Op_With_Expand_On_ډکېدنه(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::مطلق_قیمت(ثابت SlowDynamicAPInt &X) {
  أرجع X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ثابت SlowDynamicAPInt &LHS,
                                       ثابت SlowDynamicAPInt &RHS) {
  إذا (RHS == -1)
    أرجع -LHS;
  بینښه پلنوالی = get_Max_پلنوالی(LHS.Val, RHS.Val);
  أرجع SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(پلنوالی), RHS.Val.sext(پلنوالی), APInt::Rounding::پورته));
}
SlowDynamicAPInt llvm::detail::floorDiv(ثابت SlowDynamicAPInt &LHS,
                                        ثابت SlowDynamicAPInt &RHS) {
  إذا (RHS == -1)
    أرجع -LHS;
  بینښه پلنوالی = get_Max_پلنوالی(LHS.Val, RHS.Val);
  أرجع SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(پلنوالی), RHS.Val.sext(پلنوالی), APInt::Rounding::ښکته));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ثابت SlowDynamicAPInt &LHS,
                                   ثابت SlowDynamicAPInt &RHS) {
  أكد(RHS >= 1 && "mod is only supported for positive divisors!");
  أرجع LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::لوی_ګډ_وېشونکی(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  أكد(A >= 0 && B >= 0 && "operands must be non-negative!");
  بینښه پلنوالی = get_Max_پلنوالی(A.Val, B.Val);
  أرجع SlowDynamicAPInt(
      APIntOps::Greatest_Common_وېشونکی(A.Val.sext(پلنوالی), B.Val.sext(پلنوالی)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = مطلق_قیمت(A);
  SlowDynamicAPInt Y = مطلق_قیمت(B);
  أرجع (X * Y) / لوی_ګډ_وېشونکی(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::چلوونکی%(ثابت SlowDynamicAPInt &O) ثابت {
  بینښه پلنوالی = std::max(Val.get_بټ_پلنوالی(), O.Val.get_بټ_پلنوالی());
  أرجع SlowDynamicAPInt(Val.sext(پلنوالی).srem(O.Val.sext(پلنوالی)));
}

SlowDynamicAPInt SlowDynamicAPInt::چلوونکی-() ثابت {
  إذا (Val.is_Min_Signed_ارزښت()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_بټ_پلنوالی());
    أرجع SlowDynamicAPInt(-Ret);
  }
  أرجع SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی+=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا + O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی-=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا - O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی*=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا * O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی/=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا / O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی%=(ثابت SlowDynamicAPInt &O) {
  *هذا = *هذا % O;
  أرجع *هذا;
}
SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی++() {
  *هذا += 1;
  أرجع *هذا;
}

SlowDynamicAPInt &SlowDynamicAPInt::چلوونکی--() {
  *هذا -= 1;
  أرجع *هذا;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
فراغ SlowDynamicAPInt::print(raw_ostream &OS) ثابت { OS << Val; }

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ SlowDynamicAPInt::dump() ثابت { print(dbgs()); }
#د_که_پای
