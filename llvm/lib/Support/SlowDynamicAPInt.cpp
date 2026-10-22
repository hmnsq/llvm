//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/SlowDynamicAPInt.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/Hashing.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/Debug.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/Support/raw_ostream.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;
ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/ਸੱਚ) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ਅਚਲ APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ=(int64_t Val) {
  ਵਾਪਸ *ਇਹ = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::ਸੰਚਾਲਕ int64_t() ਅਚਲ { ਵਾਪਸ Val.get_SExt_ਮੁੱਲ(); }

hash_ਕੋਡ llvm::detail::hash_ਮੁੱਲ(ਅਚਲ SlowDynamicAPInt &X) {
  ਵਾਪਸ hash_ਮੁੱਲ(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::ਸੰਚਾਲਕ+=(SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ਸੰਚਾਲਕ-=(SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ਸੰਚਾਲਕ*=(SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ਸੰਚਾਲਕ/=(SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::ਸੰਚਾਲਕ%=(SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A %= SlowDynamicAPInt(B);
}

ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ==(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A == SlowDynamicAPInt(B);
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ!=(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A != SlowDynamicAPInt(B);
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ>(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A > SlowDynamicAPInt(B);
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ<(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A < SlowDynamicAPInt(B);
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ<=(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A <= SlowDynamicAPInt(B);
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ>=(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ+(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ-(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ*(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ/(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ%(ਅਚਲ SlowDynamicAPInt &A, int64_t B) {
  ਵਾਪਸ A % SlowDynamicAPInt(B);
}

ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ==(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) == B;
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ!=(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) != B;
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ>(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) > B;
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ<(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) < B;
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ<=(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) <= B;
}
ਤਾਰਕਿਕ llvm::detail::ਸੰਚਾਲਕ>=(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ+(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ-(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ*(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ/(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::ਸੰਚਾਲਕ%(int64_t A, ਅਚਲ SlowDynamicAPInt &B) {
  ਵਾਪਸ SlowDynamicAPInt(A) % B;
}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ get_Max_ਚੌੜਾਈ(ਅਚਲ APInt &A, ਅਚਲ APInt &B) {
  ਵਾਪਸ std::max(A.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), B.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
ਤਾਰਕਿਕ SlowDynamicAPInt::ਸੰਚਾਲਕ==(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(Val, O.Val);
  ਵਾਪਸ Val.sext(ਚੌੜਾਈ) == O.Val.sext(ਚੌੜਾਈ);
}
ਤਾਰਕਿਕ SlowDynamicAPInt::ਸੰਚਾਲਕ!=(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(Val, O.Val);
  ਵਾਪਸ Val.sext(ਚੌੜਾਈ) != O.Val.sext(ਚੌੜਾਈ);
}
ਤਾਰਕਿਕ SlowDynamicAPInt::ਸੰਚਾਲਕ>(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(Val, O.Val);
  ਵਾਪਸ Val.sext(ਚੌੜਾਈ).sgt(O.Val.sext(ਚੌੜਾਈ));
}
ਤਾਰਕਿਕ SlowDynamicAPInt::ਸੰਚਾਲਕ<(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(Val, O.Val);
  ਵਾਪਸ Val.sext(ਚੌੜਾਈ).slt(O.Val.sext(ਚੌੜਾਈ));
}
ਤਾਰਕਿਕ SlowDynamicAPInt::ਸੰਚਾਲਕ<=(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(Val, O.Val);
  ਵਾਪਸ Val.sext(ਚੌੜਾਈ).sle(O.Val.sext(ਚੌੜਾਈ));
}
ਤਾਰਕਿਕ SlowDynamicAPInt::ਸੰਚਾਲਕ>=(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(Val, O.Val);
  ਵਾਪਸ Val.sext(ਚੌੜਾਈ).sge(O.Val.sext(ਚੌੜਾਈ));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt ਲੜੀਵਾਰ_Op_With_Expand_On_ਉਛਾਲ(
    ਅਚਲ APInt &A, ਅਚਲ APInt &B,
    function_ref<APInt(ਅਚਲ APInt &, ਅਚਲ APInt &, ਤਾਰਕਿਕ &ਉਛਾਲ)> Op) {
  ਤਾਰਕਿਕ ਉਛਾਲ;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(A, B);
  APInt Ret = Op(A.sext(ਚੌੜਾਈ), B.sext(ਚੌੜਾਈ), ਉਛਾਲ);
  ਜੇ (!ਉਛਾਲ)
    ਵਾਪਸ Ret;

  ਚੌੜਾਈ *= 2;
  Ret = Op(A.sext(ਚੌੜਾਈ), B.sext(ਚੌੜਾਈ), ਉਛਾਲ);
  ਪ੍ਰਤਿਜ੍ਞਾ(!ਉਛਾਲ && "double width should be sufficient to avoid overflow!");
  ਵਾਪਸ Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::ਸੰਚਾਲਕ+(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਵਾਪਸ SlowDynamicAPInt(
      ਲੜੀਵਾਰ_Op_With_Expand_On_ਉਛਾਲ(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::ਸੰਚਾਲਕ-(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਵਾਪਸ SlowDynamicAPInt(
      ਲੜੀਵਾਰ_Op_With_Expand_On_ਉਛਾਲ(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::ਸੰਚਾਲਕ*(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਵਾਪਸ SlowDynamicAPInt(
      ਲੜੀਵਾਰ_Op_With_Expand_On_ਉਛਾਲ(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::ਸੰਚਾਲਕ/(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਵਾਪਸ SlowDynamicAPInt(
      ਲੜੀਵਾਰ_Op_With_Expand_On_ਉਛਾਲ(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::ਨਿਰਪੇਖ(ਅਚਲ SlowDynamicAPInt &X) {
  ਵਾਪਸ X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ਅਚਲ SlowDynamicAPInt &LHS,
                                       ਅਚਲ SlowDynamicAPInt &RHS) {
  ਜੇ (RHS == -1)
    ਵਾਪਸ -LHS;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(LHS.Val, RHS.Val);
  ਵਾਪਸ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(ਚੌੜਾਈ), RHS.Val.sext(ਚੌੜਾਈ), APInt::Rounding::ਉੱਪਰ));
}
SlowDynamicAPInt llvm::detail::floorDiv(ਅਚਲ SlowDynamicAPInt &LHS,
                                        ਅਚਲ SlowDynamicAPInt &RHS) {
  ਜੇ (RHS == -1)
    ਵਾਪਸ -LHS;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(LHS.Val, RHS.Val);
  ਵਾਪਸ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(ਚੌੜਾਈ), RHS.Val.sext(ਚੌੜਾਈ), APInt::Rounding::ਥੱਲੇ));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(ਅਚਲ SlowDynamicAPInt &LHS,
                                   ਅਚਲ SlowDynamicAPInt &RHS) {
  ਪ੍ਰਤਿਜ੍ਞਾ(RHS >= 1 && "mod is only supported for positive divisors!");
  ਵਾਪਸ LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::ਮਹੱਤਮ_ਸਾਂਝਾ_ਭਾਜਕ(ਅਚਲ SlowDynamicAPInt &A,
                                   ਅਚਲ SlowDynamicAPInt &B) {
  ਪ੍ਰਤਿਜ੍ਞਾ(A >= 0 && B >= 0 && "operands must be non-negative!");
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = get_Max_ਚੌੜਾਈ(A.Val, B.Val);
  ਵਾਪਸ SlowDynamicAPInt(
      APIntOps::Greatest_Common_ਭਾਜਕ(A.Val.sext(ਚੌੜਾਈ), B.Val.sext(ਚੌੜਾਈ)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ਅਚਲ SlowDynamicAPInt &A,
                                   ਅਚਲ SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = ਨਿਰਪੇਖ(A);
  SlowDynamicAPInt Y = ਨਿਰਪੇਖ(B);
  ਵਾਪਸ (X * Y) / ਮਹੱਤਮ_ਸਾਂਝਾ_ਭਾਜਕ(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::ਸੰਚਾਲਕ%(ਅਚਲ SlowDynamicAPInt &O) ਅਚਲ {
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਚੌੜਾਈ = std::max(Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ(), O.Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
  ਵਾਪਸ SlowDynamicAPInt(Val.sext(ਚੌੜਾਈ).srem(O.Val.sext(ਚੌੜਾਈ)));
}

SlowDynamicAPInt SlowDynamicAPInt::ਸੰਚਾਲਕ-() ਅਚਲ {
  ਜੇ (Val.is_Min_Signed_ਮੁੱਲ()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_ਦੋਆਧਾਰੀ_ਅੰਕ_ਚੌੜਾਈ());
    ਵਾਪਸ SlowDynamicAPInt(-Ret);
  }
  ਵਾਪਸ SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ+=(ਅਚਲ SlowDynamicAPInt &O) {
  *ਇਹ = *ਇਹ + O;
  ਵਾਪਸ *ਇਹ;
}
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ-=(ਅਚਲ SlowDynamicAPInt &O) {
  *ਇਹ = *ਇਹ - O;
  ਵਾਪਸ *ਇਹ;
}
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ*=(ਅਚਲ SlowDynamicAPInt &O) {
  *ਇਹ = *ਇਹ * O;
  ਵਾਪਸ *ਇਹ;
}
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ/=(ਅਚਲ SlowDynamicAPInt &O) {
  *ਇਹ = *ਇਹ / O;
  ਵਾਪਸ *ਇਹ;
}
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ%=(ਅਚਲ SlowDynamicAPInt &O) {
  *ਇਹ = *ਇਹ % O;
  ਵਾਪਸ *ਇਹ;
}
SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ++() {
  *ਇਹ += 1;
  ਵਾਪਸ *ਇਹ;
}

SlowDynamicAPInt &SlowDynamicAPInt::ਸੰਚਾਲਕ--() {
  *ਇਹ -= 1;
  ਵਾਪਸ *ਇਹ;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
ਖਾਲੀ SlowDynamicAPInt::print(raw_ostream &OS) ਅਚਲ { OS << Val; }

#ਜੇ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_ਵਿਧੀ ਖਾਲੀ SlowDynamicAPInt::dump() ਅਚਲ { print(dbgs()); }
#ਜੇ_ਅੰਤ
