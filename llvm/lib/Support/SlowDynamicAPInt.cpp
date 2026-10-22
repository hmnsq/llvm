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

استعمال نامخلا llvm;
استعمال نامخلا llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/سچ) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ثابت APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::عامل=(int64_t Val) {
  واپس *یہ = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::عامل int64_t() ثابت { واپس Val.get_SExt_قدر(); }

hash_رمز llvm::detail::hash_قدر(ثابت SlowDynamicAPInt &X) {
  واپس hash_قدر(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::عامل+=(SlowDynamicAPInt &A, int64_t B) {
  واپس A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل-=(SlowDynamicAPInt &A, int64_t B) {
  واپس A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل*=(SlowDynamicAPInt &A, int64_t B) {
  واپس A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل/=(SlowDynamicAPInt &A, int64_t B) {
  واپس A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::عامل%=(SlowDynamicAPInt &A, int64_t B) {
  واپس A %= SlowDynamicAPInt(B);
}

منطقی llvm::detail::عامل==(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A == SlowDynamicAPInt(B);
}
منطقی llvm::detail::عامل!=(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A != SlowDynamicAPInt(B);
}
منطقی llvm::detail::عامل>(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A > SlowDynamicAPInt(B);
}
منطقی llvm::detail::عامل<(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A < SlowDynamicAPInt(B);
}
منطقی llvm::detail::عامل<=(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A <= SlowDynamicAPInt(B);
}
منطقی llvm::detail::عامل>=(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل+(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل-(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل*(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل/(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::عامل%(ثابت SlowDynamicAPInt &A, int64_t B) {
  واپس A % SlowDynamicAPInt(B);
}

منطقی llvm::detail::عامل==(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) == B;
}
منطقی llvm::detail::عامل!=(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) != B;
}
منطقی llvm::detail::عامل>(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) > B;
}
منطقی llvm::detail::عامل<(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) < B;
}
منطقی llvm::detail::عامل<=(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) <= B;
}
منطقی llvm::detail::عامل>=(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::عامل+(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::عامل-(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::عامل*(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::عامل/(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::عامل%(int64_t A, ثابت SlowDynamicAPInt &B) {
  واپس SlowDynamicAPInt(A) % B;
}

ساکن غیرعلامتی get_Max_چوڑائی(ثابت APInt &A, ثابت APInt &B) {
  واپس std::max(A.get_ثنائی_ہندسہ_چوڑائی(), B.get_ثنائی_ہندسہ_چوڑائی());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
منطقی SlowDynamicAPInt::عامل==(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = get_Max_چوڑائی(Val, O.Val);
  واپس Val.sext(چوڑائی) == O.Val.sext(چوڑائی);
}
منطقی SlowDynamicAPInt::عامل!=(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = get_Max_چوڑائی(Val, O.Val);
  واپس Val.sext(چوڑائی) != O.Val.sext(چوڑائی);
}
منطقی SlowDynamicAPInt::عامل>(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = get_Max_چوڑائی(Val, O.Val);
  واپس Val.sext(چوڑائی).sgt(O.Val.sext(چوڑائی));
}
منطقی SlowDynamicAPInt::عامل<(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = get_Max_چوڑائی(Val, O.Val);
  واپس Val.sext(چوڑائی).slt(O.Val.sext(چوڑائی));
}
منطقی SlowDynamicAPInt::عامل<=(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = get_Max_چوڑائی(Val, O.Val);
  واپس Val.sext(چوڑائی).sle(O.Val.sext(چوڑائی));
}
منطقی SlowDynamicAPInt::عامل>=(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = get_Max_چوڑائی(Val, O.Val);
  واپس Val.sext(چوڑائی).sge(O.Val.sext(چوڑائی));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt تسلسل_Op_With_Expand_On_حد_سے_تجاوز(
    ثابت APInt &A, ثابت APInt &B,
    function_ref<APInt(ثابت APInt &, ثابت APInt &, منطقی &حد_سے_تجاوز)> Op) {
  منطقی حد_سے_تجاوز;
  غیرعلامتی چوڑائی = get_Max_چوڑائی(A, B);
  APInt Ret = Op(A.sext(چوڑائی), B.sext(چوڑائی), حد_سے_تجاوز);
  اگر (!حد_سے_تجاوز)
    واپس Ret;

  چوڑائی *= 2;
  Ret = Op(A.sext(چوڑائی), B.sext(چوڑائی), حد_سے_تجاوز);
  أكد(!حد_سے_تجاوز && "double width should be sufficient to avoid overflow!");
  واپس Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::عامل+(ثابت SlowDynamicAPInt &O) ثابت {
  واپس SlowDynamicAPInt(
      تسلسل_Op_With_Expand_On_حد_سے_تجاوز(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عامل-(ثابت SlowDynamicAPInt &O) ثابت {
  واپس SlowDynamicAPInt(
      تسلسل_Op_With_Expand_On_حد_سے_تجاوز(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عامل*(ثابت SlowDynamicAPInt &O) ثابت {
  واپس SlowDynamicAPInt(
      تسلسل_Op_With_Expand_On_حد_سے_تجاوز(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::عامل/(ثابت SlowDynamicAPInt &O) ثابت {
  واپس SlowDynamicAPInt(
      تسلسل_Op_With_Expand_On_حد_سے_تجاوز(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::مطلق_قدر(ثابت SlowDynamicAPInt &X) {
  واپس X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ثابت SlowDynamicAPInt &LHS,
                                       ثابت SlowDynamicAPInt &RHS) {
  اگر (RHS == -1)
    واپس -LHS;
  غیرعلامتی چوڑائی = get_Max_چوڑائی(LHS.Val, RHS.Val);
  واپس SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(چوڑائی), RHS.Val.sext(چوڑائی), APInt::Rounding::اوپر));
}
SlowDynamicAPInt llvm::detail::floorDiv(ثابت SlowDynamicAPInt &LHS,
                                        ثابت SlowDynamicAPInt &RHS) {
  اگر (RHS == -1)
    واپس -LHS;
  غیرعلامتی چوڑائی = get_Max_چوڑائی(LHS.Val, RHS.Val);
  واپس SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(چوڑائی), RHS.Val.sext(چوڑائی), APInt::Rounding::نیچے_کو));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ثابت SlowDynamicAPInt &LHS,
                                   ثابت SlowDynamicAPInt &RHS) {
  أكد(RHS >= 1 && "mod is only supported for positive divisors!");
  واپس LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::عاد_اعظم(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  أكد(A >= 0 && B >= 0 && "operands must be non-negative!");
  غیرعلامتی چوڑائی = get_Max_چوڑائی(A.Val, B.Val);
  واپس SlowDynamicAPInt(
      APIntOps::Greatest_Common_مقسوم_علیہ(A.Val.sext(چوڑائی), B.Val.sext(چوڑائی)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ثابت SlowDynamicAPInt &A,
                                   ثابت SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = مطلق_قدر(A);
  SlowDynamicAPInt Y = مطلق_قدر(B);
  واپس (X * Y) / عاد_اعظم(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::عامل%(ثابت SlowDynamicAPInt &O) ثابت {
  غیرعلامتی چوڑائی = std::max(Val.get_ثنائی_ہندسہ_چوڑائی(), O.Val.get_ثنائی_ہندسہ_چوڑائی());
  واپس SlowDynamicAPInt(Val.sext(چوڑائی).srem(O.Val.sext(چوڑائی)));
}

SlowDynamicAPInt SlowDynamicAPInt::عامل-() ثابت {
  اگر (Val.is_Min_Signed_قدر()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_ثنائی_ہندسہ_چوڑائی());
    واپس SlowDynamicAPInt(-Ret);
  }
  واپس SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::عامل+=(ثابت SlowDynamicAPInt &O) {
  *یہ = *یہ + O;
  واپس *یہ;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل-=(ثابت SlowDynamicAPInt &O) {
  *یہ = *یہ - O;
  واپس *یہ;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل*=(ثابت SlowDynamicAPInt &O) {
  *یہ = *یہ * O;
  واپس *یہ;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل/=(ثابت SlowDynamicAPInt &O) {
  *یہ = *یہ / O;
  واپس *یہ;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل%=(ثابت SlowDynamicAPInt &O) {
  *یہ = *یہ % O;
  واپس *یہ;
}
SlowDynamicAPInt &SlowDynamicAPInt::عامل++() {
  *یہ += 1;
  واپس *یہ;
}

SlowDynamicAPInt &SlowDynamicAPInt::عامل--() {
  *یہ -= 1;
  واپس *یہ;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
خالی SlowDynamicAPInt::print(raw_ostream &OS) ثابت { OS << Val; }

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی SlowDynamicAPInt::dump() ثابت { print(dbgs()); }
#اگر_ختم
