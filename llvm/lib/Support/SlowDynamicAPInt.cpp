//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#അന്തര്ഭാവ "llvm/ADT/SlowDynamicAPInt.h"
#അന്തര്ഭാവ "llvm/ADT/Hashing.h"
#അന്തര്ഭാവ "llvm/Support/Debug.h"
#അന്തര്ഭാവ "llvm/Support/raw_ostream.h"

using നാമാകാശ llvm;
using നാമാകാശ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/സത്യം) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(സ്ഥിരം APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::operator=(int64_t Val) {
  തിരികെ *ഇത് = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::operator int64_t() സ്ഥിരം { തിരികെ Val.get_SExt_മൂല്യം(); }

hash_code llvm::detail::hash_മൂല്യം(സ്ഥിരം SlowDynamicAPInt &X) {
  തിരികെ hash_മൂല്യം(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::operator+=(SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator-=(SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator*=(SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator/=(SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::operator%=(SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A %= SlowDynamicAPInt(B);
}

തർക്കം llvm::detail::operator==(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A == SlowDynamicAPInt(B);
}
തർക്കം llvm::detail::operator!=(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A != SlowDynamicAPInt(B);
}
തർക്കം llvm::detail::operator>(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A > SlowDynamicAPInt(B);
}
തർക്കം llvm::detail::operator<(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A < SlowDynamicAPInt(B);
}
തർക്കം llvm::detail::operator<=(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A <= SlowDynamicAPInt(B);
}
തർക്കം llvm::detail::operator>=(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator+(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator-(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator*(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator/(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::operator%(സ്ഥിരം SlowDynamicAPInt &A, int64_t B) {
  തിരികെ A % SlowDynamicAPInt(B);
}

തർക്കം llvm::detail::operator==(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) == B;
}
തർക്കം llvm::detail::operator!=(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) != B;
}
തർക്കം llvm::detail::operator>(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) > B;
}
തർക്കം llvm::detail::operator<(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) < B;
}
തർക്കം llvm::detail::operator<=(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) <= B;
}
തർക്കം llvm::detail::operator>=(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::operator+(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::operator-(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::operator*(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::operator/(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::operator%(int64_t A, സ്ഥിരം SlowDynamicAPInt &B) {
  തിരികെ SlowDynamicAPInt(A) % B;
}

സ്ഥിതം unsigned getMaxWidth(സ്ഥിരം APInt &A, സ്ഥിരം APInt &B) {
  തിരികെ std::max(A.getBitWidth(), B.getBitWidth());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
തർക്കം SlowDynamicAPInt::operator==(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = getMaxWidth(Val, O.Val);
  തിരികെ Val.sext(Width) == O.Val.sext(Width);
}
തർക്കം SlowDynamicAPInt::operator!=(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = getMaxWidth(Val, O.Val);
  തിരികെ Val.sext(Width) != O.Val.sext(Width);
}
തർക്കം SlowDynamicAPInt::operator>(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = getMaxWidth(Val, O.Val);
  തിരികെ Val.sext(Width).sgt(O.Val.sext(Width));
}
തർക്കം SlowDynamicAPInt::operator<(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = getMaxWidth(Val, O.Val);
  തിരികെ Val.sext(Width).slt(O.Val.sext(Width));
}
തർക്കം SlowDynamicAPInt::operator<=(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = getMaxWidth(Val, O.Val);
  തിരികെ Val.sext(Width).sle(O.Val.sext(Width));
}
തർക്കം SlowDynamicAPInt::operator>=(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = getMaxWidth(Val, O.Val);
  തിരികെ Val.sext(Width).sge(O.Val.sext(Width));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt runOpWithExpandOnOverflow(
    സ്ഥിരം APInt &A, സ്ഥിരം APInt &B,
    function_ref<APInt(സ്ഥിരം APInt &, സ്ഥിരം APInt &, തർക്കം &Overflow)> Op) {
  തർക്കം Overflow;
  unsigned Width = getMaxWidth(A, B);
  APInt Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  എങ്കിൽ (!Overflow)
    തിരികെ Ret;

  Width *= 2;
  Ret = Op(A.sext(Width), B.sext(Width), Overflow);
  പ്രതിജ്ഞാ(!Overflow && "double width should be sufficient to avoid overflow!");
  തിരികെ Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::operator+(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  തിരികെ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator-(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  തിരികെ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator*(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  തിരികെ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::operator/(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  തിരികെ SlowDynamicAPInt(
      runOpWithExpandOnOverflow(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::abs(സ്ഥിരം SlowDynamicAPInt &X) {
  തിരികെ X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(സ്ഥിരം SlowDynamicAPInt &LHS,
                                       സ്ഥിരം SlowDynamicAPInt &RHS) {
  എങ്കിൽ (RHS == -1)
    തിരികെ -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  തിരികെ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::UP));
}
SlowDynamicAPInt llvm::detail::floorDiv(സ്ഥിരം SlowDynamicAPInt &LHS,
                                        സ്ഥിരം SlowDynamicAPInt &RHS) {
  എങ്കിൽ (RHS == -1)
    തിരികെ -LHS;
  unsigned Width = getMaxWidth(LHS.Val, RHS.Val);
  തിരികെ SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(Width), RHS.Val.sext(Width), APInt::Rounding::DOWN));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(സ്ഥിരം SlowDynamicAPInt &LHS,
                                   സ്ഥിരം SlowDynamicAPInt &RHS) {
  പ്രതിജ്ഞാ(RHS >= 1 && "mod is only supported for positive divisors!");
  തിരികെ LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::gcd(സ്ഥിരം SlowDynamicAPInt &A,
                                   സ്ഥിരം SlowDynamicAPInt &B) {
  പ്രതിജ്ഞാ(A >= 0 && B >= 0 && "operands must be non-negative!");
  unsigned Width = getMaxWidth(A.Val, B.Val);
  തിരികെ SlowDynamicAPInt(
      APIntOps::Greatest_Common_ഹാരകം(A.Val.sext(Width), B.Val.sext(Width)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(സ്ഥിരം SlowDynamicAPInt &A,
                                   സ്ഥിരം SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = abs(A);
  SlowDynamicAPInt Y = abs(B);
  തിരികെ (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::operator%(സ്ഥിരം SlowDynamicAPInt &O) സ്ഥിരം {
  unsigned Width = std::max(Val.getBitWidth(), O.Val.getBitWidth());
  തിരികെ SlowDynamicAPInt(Val.sext(Width).srem(O.Val.sext(Width)));
}

SlowDynamicAPInt SlowDynamicAPInt::operator-() സ്ഥിരം {
  എങ്കിൽ (Val.is_Min_Signed_മൂല്യം()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.getBitWidth());
    തിരികെ SlowDynamicAPInt(-Ret);
  }
  തിരികെ SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::operator+=(സ്ഥിരം SlowDynamicAPInt &O) {
  *ഇത് = *ഇത് + O;
  തിരികെ *ഇത്;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator-=(സ്ഥിരം SlowDynamicAPInt &O) {
  *ഇത് = *ഇത് - O;
  തിരികെ *ഇത്;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator*=(സ്ഥിരം SlowDynamicAPInt &O) {
  *ഇത് = *ഇത് * O;
  തിരികെ *ഇത്;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator/=(സ്ഥിരം SlowDynamicAPInt &O) {
  *ഇത് = *ഇത് / O;
  തിരികെ *ഇത്;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator%=(സ്ഥിരം SlowDynamicAPInt &O) {
  *ഇത് = *ഇത് % O;
  തിരികെ *ഇത്;
}
SlowDynamicAPInt &SlowDynamicAPInt::operator++() {
  *ഇത് += 1;
  തിരികെ *ഇത്;
}

SlowDynamicAPInt &SlowDynamicAPInt::operator--() {
  *ഇത് -= 1;
  തിരികെ *ഇത്;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
ശൂന്യം SlowDynamicAPInt::print(raw_ostream &OS) സ്ഥിരം { OS << Val; }

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം SlowDynamicAPInt::dump() സ്ഥിരം { print(dbgs()); }
#endif
