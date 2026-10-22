//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ಅನ್ತರ್ಭಾವ "llvm/ADT/SlowDynamicAPInt.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/Hashing.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/Debug.h"
#ಅನ್ತರ್ಭಾವ "llvm/Support/raw_ostream.h"

ಬಳಸಿ ನಾಮಾಕಾಶ llvm;
ಬಳಸಿ ನಾಮಾಕಾಶ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/ನಿಜ) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ಸ್ಥಿರಾಂಕ APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ=(int64_t Val) {
  ಹಿಂತಿರುಗಿ *ಇದು = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ int64_t() ಸ್ಥಿರಾಂಕ { ಹಿಂತಿರುಗಿ Val.get_SExt_ಮೌಲ್ಯ(); }

hash_ಸಂಕೇತ llvm::detail::hash_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &X) {
  ಹಿಂತಿರುಗಿ hash_ಮೌಲ್ಯ(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::ಕ್ರಿಯಾಕಾರಕ+=(SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ಕ್ರಿಯಾಕಾರಕ-=(SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ಕ್ರಿಯಾಕಾರಕ*=(SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ಕ್ರಿಯಾಕಾರಕ/=(SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ಕ್ರಿಯಾಕಾರಕ%=(SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A %= SlowDynamicAPInt(B);
}

ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A == SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ!=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A != SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ>(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A > SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ<(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A < SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ<=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A <= SlowDynamicAPInt(B);
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ>=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ+(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ-(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ*(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ/(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ%(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A, int64_t B) {
  ಹಿಂತಿರುಗಿ A % SlowDynamicAPInt(B);
}

ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ==(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) == B;
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ!=(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) != B;
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ>(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) > B;
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ<(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) < B;
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ<=(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) <= B;
}
ತಾರ್ಕಿಕ llvm::detail::ಕ್ರಿಯಾಕಾರಕ>=(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ+(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ-(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ*(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ/(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::ಕ್ರಿಯಾಕಾರಕ%(int64_t A, ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(A) % B;
}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ get_Max_ಅಗಲ(ಸ್ಥಿರಾಂಕ APInt &A, ಸ್ಥಿರಾಂಕ APInt &B) {
  ಹಿಂತಿರುಗಿ std::max(A.get_ಬಿಟ್_ಅಗಲ(), B.get_ಬಿಟ್_ಅಗಲ());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
ತಾರ್ಕಿಕ SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ==(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(Val, O.Val);
  ಹಿಂತಿರುಗಿ Val.sext(ಅಗಲ) == O.Val.sext(ಅಗಲ);
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ!=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(Val, O.Val);
  ಹಿಂತಿರುಗಿ Val.sext(ಅಗಲ) != O.Val.sext(ಅಗಲ);
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ>(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(Val, O.Val);
  ಹಿಂತಿರುಗಿ Val.sext(ಅಗಲ).sgt(O.Val.sext(ಅಗಲ));
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ<(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(Val, O.Val);
  ಹಿಂತಿರುಗಿ Val.sext(ಅಗಲ).slt(O.Val.sext(ಅಗಲ));
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ<=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(Val, O.Val);
  ಹಿಂತಿರುಗಿ Val.sext(ಅಗಲ).sle(O.Val.sext(ಅಗಲ));
}
ತಾರ್ಕಿಕ SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ>=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(Val, O.Val);
  ಹಿಂತಿರುಗಿ Val.sext(ಅಗಲ).sge(O.Val.sext(ಅಗಲ));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt ಸತತ_Op_With_Expand_On_ಅತಿಪ್ರವಾಹ(
    ಸ್ಥಿರಾಂಕ APInt &A, ಸ್ಥಿರಾಂಕ APInt &B,
    function_ref<APInt(ಸ್ಥಿರಾಂಕ APInt &, ಸ್ಥಿರಾಂಕ APInt &, ತಾರ್ಕಿಕ &ಅತಿಪ್ರವಾಹ)> Op) {
  ತಾರ್ಕಿಕ ಅತಿಪ್ರವಾಹ;
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(A, B);
  APInt Ret = Op(A.sext(ಅಗಲ), B.sext(ಅಗಲ), ಅತಿಪ್ರವಾಹ);
  ಆದರೆ (!ಅತಿಪ್ರವಾಹ)
    ಹಿಂತಿರುಗಿ Ret;

  ಅಗಲ *= 2;
  Ret = Op(A.sext(ಅಗಲ), B.sext(ಅಗಲ), ಅತಿಪ್ರವಾಹ);
  ಪ್ರತಿಜ್ಞಾ(!ಅತಿಪ್ರವಾಹ && "double width should be sufficient to avoid overflow!");
  ಹಿಂತಿರುಗಿ Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ+(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(
      ಸತತ_Op_With_Expand_On_ಅತಿಪ್ರವಾಹ(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ-(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(
      ಸತತ_Op_With_Expand_On_ಅತಿಪ್ರವಾಹ(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ*(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(
      ಸತತ_Op_With_Expand_On_ಅತಿಪ್ರವಾಹ(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ/(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(
      ಸತತ_Op_With_Expand_On_ಅತಿಪ್ರವಾಹ(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::ನಿರಪೇಕ್ಷ(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &X) {
  ಹಿಂತಿರುಗಿ X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &LHS,
                                       ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &RHS) {
  ಆದರೆ (RHS == -1)
    ಹಿಂತಿರುಗಿ -LHS;
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(LHS.Val, RHS.Val);
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(ಅಗಲ), RHS.Val.sext(ಅಗಲ), APInt::Rounding::ಮೇಲೆ));
}
SlowDynamicAPInt llvm::detail::floorDiv(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &LHS,
                                        ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &RHS) {
  ಆದರೆ (RHS == -1)
    ಹಿಂತಿರುಗಿ -LHS;
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(LHS.Val, RHS.Val);
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(ಅಗಲ), RHS.Val.sext(ಅಗಲ), APInt::Rounding::ಕೆಳಕ್ಕೆ));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &LHS,
                                   ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &RHS) {
  ಪ್ರತಿಜ್ಞಾ(RHS >= 1 && "mod is only supported for positive divisors!");
  ಹಿಂತಿರುಗಿ LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::ಮಹತ್ತಮ_ಸಾಮಾನ್ಯ_ಅಪವರ್ತನ(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A,
                                   ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  ಪ್ರತಿಜ್ಞಾ(A >= 0 && B >= 0 && "operands must be non-negative!");
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = get_Max_ಅಗಲ(A.Val, B.Val);
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(
      APIntOps::Greatest_Common_ಭಾಜಕ(A.Val.sext(ಅಗಲ), B.Val.sext(ಅಗಲ)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &A,
                                   ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = ನಿರಪೇಕ್ಷ(A);
  SlowDynamicAPInt Y = ನಿರಪೇಕ್ಷ(B);
  ಹಿಂತಿರುಗಿ (X * Y) / ಮಹತ್ತಮ_ಸಾಮಾನ್ಯ_ಅಪವರ್ತನ(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ%(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) ಸ್ಥಿರಾಂಕ {
  ಚಿಹ್ನೆರಹಿತ ಅಗಲ = std::max(Val.get_ಬಿಟ್_ಅಗಲ(), O.Val.get_ಬಿಟ್_ಅಗಲ());
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(Val.sext(ಅಗಲ).srem(O.Val.sext(ಅಗಲ)));
}

SlowDynamicAPInt SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ-() ಸ್ಥಿರಾಂಕ {
  ಆದರೆ (Val.is_Min_Signed_ಮೌಲ್ಯ()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_ಬಿಟ್_ಅಗಲ());
    ಹಿಂತಿರುಗಿ SlowDynamicAPInt(-Ret);
  }
  ಹಿಂತಿರುಗಿ SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ+=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) {
  *ಇದು = *ಇದು + O;
  ಹಿಂತಿರುಗಿ *ಇದು;
}
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ-=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) {
  *ಇದು = *ಇದು - O;
  ಹಿಂತಿರುಗಿ *ಇದು;
}
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ*=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) {
  *ಇದು = *ಇದು * O;
  ಹಿಂತಿರುಗಿ *ಇದು;
}
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ/=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) {
  *ಇದು = *ಇದು / O;
  ಹಿಂತಿರುಗಿ *ಇದು;
}
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ%=(ಸ್ಥಿರಾಂಕ SlowDynamicAPInt &O) {
  *ಇದು = *ಇದು % O;
  ಹಿಂತಿರುಗಿ *ಇದು;
}
SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ++() {
  *ಇದು += 1;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

SlowDynamicAPInt &SlowDynamicAPInt::ಕ್ರಿಯಾಕಾರಕ--() {
  *ಇದು -= 1;
  ಹಿಂತಿರುಗಿ *ಇದು;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
ಶೂನ್ಯ SlowDynamicAPInt::print(raw_ostream &OS) ಸ್ಥಿರಾಂಕ { OS << Val; }

#ಆದರೆ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ವಿಧಾನ ಶೂನ್ಯ SlowDynamicAPInt::dump() ಸ್ಥಿರಾಂಕ { print(dbgs()); }
#ಆದರೆ_ಅಂತ್ಯ
