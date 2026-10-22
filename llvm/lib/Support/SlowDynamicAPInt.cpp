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

उपयुज्य नामाकाश llvm;
उपयुज्य नामाकाश llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/सत्य) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(ध्रुव APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः=(int64_t Val) {
  प्रत्यागम *एतत् = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::संक्रियकः int64_t() ध्रुव { प्रत्यागम Val.get_SExt_मानम्(); }

hash_संहिता llvm::detail::hash_मानम्(ध्रुव SlowDynamicAPInt &X) {
  प्रत्यागम hash_मानम्(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::संक्रियकः+=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::संक्रियकः-=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::संक्रियकः*=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::संक्रियकः/=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::संक्रियकः%=(SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A %= SlowDynamicAPInt(B);
}

तार्किक llvm::detail::संक्रियकः==(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A == SlowDynamicAPInt(B);
}
तार्किक llvm::detail::संक्रियकः!=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A != SlowDynamicAPInt(B);
}
तार्किक llvm::detail::संक्रियकः>(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A > SlowDynamicAPInt(B);
}
तार्किक llvm::detail::संक्रियकः<(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A < SlowDynamicAPInt(B);
}
तार्किक llvm::detail::संक्रियकः<=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A <= SlowDynamicAPInt(B);
}
तार्किक llvm::detail::संक्रियकः>=(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::संक्रियकः+(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::संक्रियकः-(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::संक्रियकः*(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::संक्रियकः/(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::संक्रियकः%(ध्रुव SlowDynamicAPInt &A, int64_t B) {
  प्रत्यागम A % SlowDynamicAPInt(B);
}

तार्किक llvm::detail::संक्रियकः==(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) == B;
}
तार्किक llvm::detail::संक्रियकः!=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) != B;
}
तार्किक llvm::detail::संक्रियकः>(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) > B;
}
तार्किक llvm::detail::संक्रियकः<(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) < B;
}
तार्किक llvm::detail::संक्रियकः<=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) <= B;
}
तार्किक llvm::detail::संक्रियकः>=(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::संक्रियकः+(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::संक्रियकः-(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::संक्रियकः*(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::संक्रियकः/(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::संक्रियकः%(int64_t A, ध्रुव SlowDynamicAPInt &B) {
  प्रत्यागम SlowDynamicAPInt(A) % B;
}

स्थिर अचिह्नित get_Max_विस्तारः(ध्रुव APInt &A, ध्रुव APInt &B) {
  प्रत्यागम std::max(A.get_Bit_विस्तारः(), B.get_Bit_विस्तारः());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
तार्किक SlowDynamicAPInt::संक्रियकः==(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = get_Max_विस्तारः(Val, O.Val);
  प्रत्यागम Val.sext(विस्तारः) == O.Val.sext(विस्तारः);
}
तार्किक SlowDynamicAPInt::संक्रियकः!=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = get_Max_विस्तारः(Val, O.Val);
  प्रत्यागम Val.sext(विस्तारः) != O.Val.sext(विस्तारः);
}
तार्किक SlowDynamicAPInt::संक्रियकः>(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = get_Max_विस्तारः(Val, O.Val);
  प्रत्यागम Val.sext(विस्तारः).sgt(O.Val.sext(विस्तारः));
}
तार्किक SlowDynamicAPInt::संक्रियकः<(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = get_Max_विस्तारः(Val, O.Val);
  प्रत्यागम Val.sext(विस्तारः).slt(O.Val.sext(विस्तारः));
}
तार्किक SlowDynamicAPInt::संक्रियकः<=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = get_Max_विस्तारः(Val, O.Val);
  प्रत्यागम Val.sext(विस्तारः).sle(O.Val.sext(विस्तारः));
}
तार्किक SlowDynamicAPInt::संक्रियकः>=(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = get_Max_विस्तारः(Val, O.Val);
  प्रत्यागम Val.sext(विस्तारः).sge(O.Val.sext(विस्तारः));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt सततम्_Op_With_Expand_On_अतिप्रवाहः(
    ध्रुव APInt &A, ध्रुव APInt &B,
    function_ref<APInt(ध्रुव APInt &, ध्रुव APInt &, तार्किक &अतिप्रवाहः)> Op) {
  तार्किक अतिप्रवाहः;
  अचिह्नित विस्तारः = get_Max_विस्तारः(A, B);
  APInt Ret = Op(A.sext(विस्तारः), B.sext(विस्तारः), अतिप्रवाहः);
  यदि (!अतिप्रवाहः)
    प्रत्यागम Ret;

  विस्तारः *= 2;
  Ret = Op(A.sext(विस्तारः), B.sext(विस्तारः), अतिप्रवाहः);
  प्रतिज्ञा(!अतिप्रवाहः && "double width should be sufficient to avoid overflow!");
  प्रत्यागम Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::संक्रियकः+(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      सततम्_Op_With_Expand_On_अतिप्रवाहः(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::संक्रियकः-(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      सततम्_Op_With_Expand_On_अतिप्रवाहः(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::संक्रियकः*(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      सततम्_Op_With_Expand_On_अतिप्रवाहः(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::संक्रियकः/(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  प्रत्यागम SlowDynamicAPInt(
      सततम्_Op_With_Expand_On_अतिप्रवाहः(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::निरपेक्षम्(ध्रुव SlowDynamicAPInt &X) {
  प्रत्यागम X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(ध्रुव SlowDynamicAPInt &LHS,
                                       ध्रुव SlowDynamicAPInt &RHS) {
  यदि (RHS == -1)
    प्रत्यागम -LHS;
  अचिह्नित विस्तारः = get_Max_विस्तारः(LHS.Val, RHS.Val);
  प्रत्यागम SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(विस्तारः), RHS.Val.sext(विस्तारः), APInt::Rounding::ऊर्ध्वम्));
}
SlowDynamicAPInt llvm::detail::floorDiv(ध्रुव SlowDynamicAPInt &LHS,
                                        ध्रुव SlowDynamicAPInt &RHS) {
  यदि (RHS == -1)
    प्रत्यागम -LHS;
  अचिह्नित विस्तारः = get_Max_विस्तारः(LHS.Val, RHS.Val);
  प्रत्यागम SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(विस्तारः), RHS.Val.sext(विस्तारः), APInt::Rounding::अधस्तात्));
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
  अचिह्नित विस्तारः = get_Max_विस्तारः(A.Val, B.Val);
  प्रत्यागम SlowDynamicAPInt(
      APIntOps::Greatest_Common_भाजकः(A.Val.sext(विस्तारः), B.Val.sext(विस्तारः)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(ध्रुव SlowDynamicAPInt &A,
                                   ध्रुव SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = निरपेक्षम्(A);
  SlowDynamicAPInt Y = निरपेक्षम्(B);
  प्रत्यागम (X * Y) / gcd(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::संक्रियकः%(ध्रुव SlowDynamicAPInt &O) ध्रुव {
  अचिह्नित विस्तारः = std::max(Val.get_Bit_विस्तारः(), O.Val.get_Bit_विस्तारः());
  प्रत्यागम SlowDynamicAPInt(Val.sext(विस्तारः).srem(O.Val.sext(विस्तारः)));
}

SlowDynamicAPInt SlowDynamicAPInt::संक्रियकः-() ध्रुव {
  यदि (Val.is_Min_Signed_मानम्()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_Bit_विस्तारः());
    प्रत्यागम SlowDynamicAPInt(-Ret);
  }
  प्रत्यागम SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः+=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् + O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः-=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् - O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः*=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् * O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः/=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् / O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः%=(ध्रुव SlowDynamicAPInt &O) {
  *एतत् = *एतत् % O;
  प्रत्यागम *एतत्;
}
SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः++() {
  *एतत् += 1;
  प्रत्यागम *एतत्;
}

SlowDynamicAPInt &SlowDynamicAPInt::संक्रियकः--() {
  *एतत् -= 1;
  प्रत्यागम *एतत्;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
शून्य SlowDynamicAPInt::print(raw_ostream &OS) ध्रुव { OS << Val; }

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य SlowDynamicAPInt::dump() ध्रुव { print(dbgs()); }
#यदि_अन्तः
