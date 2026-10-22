//===- SlowDynamicAPInt.cpp - SlowDynamicAPInt Implementation -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#అన్తర్భావ "llvm/ADT/SlowDynamicAPInt.h"
#అన్తర్భావ "llvm/ADT/Hashing.h"
#అన్తర్భావ "llvm/Support/Debug.h"
#అన్తర్భావ "llvm/Support/raw_ostream.h"

ఉపయోగించి నామాకాశ llvm;
ఉపయోగించి నామాకాశ llvm::detail;

SlowDynamicAPInt::SlowDynamicAPInt(int64_t Val)
    : Val(64, Val, /*isSigned=*/నిజం) {}
SlowDynamicAPInt::SlowDynamicAPInt() : SlowDynamicAPInt(0) {}
SlowDynamicAPInt::SlowDynamicAPInt(స్థిరరాశి APInt &Val) : Val(Val) {}
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం=(int64_t Val) {
  తిరిగి *ఇది = SlowDynamicAPInt(Val);
}
SlowDynamicAPInt::క్రియాకారకం int64_t() స్థిరరాశి { తిరిగి Val.get_SExt_విలువ(); }

hash_సంకేతం llvm::detail::hash_విలువ(స్థిరరాశి SlowDynamicAPInt &X) {
  తిరిగి hash_విలువ(X.Val);
}

/// ---------------------------------------------------------------------------
/// Convenience operator overloads for int64_t.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &llvm::detail::క్రియాకారకం+=(SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A += SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::క్రియాకారకం-=(SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A -= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::క్రియాకారకం*=(SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A *= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::క్రియాకారకం/=(SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A /= SlowDynamicAPInt(B);
}
SlowDynamicAPInt &llvm::detail::క్రియాకారకం%=(SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A %= SlowDynamicAPInt(B);
}

తార్కిక llvm::detail::క్రియాకారకం==(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A == SlowDynamicAPInt(B);
}
తార్కిక llvm::detail::క్రియాకారకం!=(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A != SlowDynamicAPInt(B);
}
తార్కిక llvm::detail::క్రియాకారకం>(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A > SlowDynamicAPInt(B);
}
తార్కిక llvm::detail::క్రియాకారకం<(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A < SlowDynamicAPInt(B);
}
తార్కిక llvm::detail::క్రియాకారకం<=(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A <= SlowDynamicAPInt(B);
}
తార్కిక llvm::detail::క్రియాకారకం>=(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A >= SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం+(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A + SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం-(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A - SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం*(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A * SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం/(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A / SlowDynamicAPInt(B);
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం%(స్థిరరాశి SlowDynamicAPInt &A, int64_t B) {
  తిరిగి A % SlowDynamicAPInt(B);
}

తార్కిక llvm::detail::క్రియాకారకం==(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) == B;
}
తార్కిక llvm::detail::క్రియాకారకం!=(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) != B;
}
తార్కిక llvm::detail::క్రియాకారకం>(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) > B;
}
తార్కిక llvm::detail::క్రియాకారకం<(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) < B;
}
తార్కిక llvm::detail::క్రియాకారకం<=(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) <= B;
}
తార్కిక llvm::detail::క్రియాకారకం>=(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) >= B;
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం+(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) + B;
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం-(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) - B;
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం*(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) * B;
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం/(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) / B;
}
SlowDynamicAPInt llvm::detail::క్రియాకారకం%(int64_t A, స్థిరరాశి SlowDynamicAPInt &B) {
  తిరిగి SlowDynamicAPInt(A) % B;
}

స్థిర గుర్తులేని get_Max_వెడల్పు(స్థిరరాశి APInt &A, స్థిరరాశి APInt &B) {
  తిరిగి std::max(A.get_బిట్టు_వెడల్పు(), B.get_బిట్టు_వెడల్పు());
}

/// ---------------------------------------------------------------------------
/// Comparison operators.
/// ---------------------------------------------------------------------------

// TODO: consider instead making APInt::compare available and using that.
తార్కిక SlowDynamicAPInt::క్రియాకారకం==(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(Val, O.Val);
  తిరిగి Val.sext(వెడల్పు) == O.Val.sext(వెడల్పు);
}
తార్కిక SlowDynamicAPInt::క్రియాకారకం!=(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(Val, O.Val);
  తిరిగి Val.sext(వెడల్పు) != O.Val.sext(వెడల్పు);
}
తార్కిక SlowDynamicAPInt::క్రియాకారకం>(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(Val, O.Val);
  తిరిగి Val.sext(వెడల్పు).sgt(O.Val.sext(వెడల్పు));
}
తార్కిక SlowDynamicAPInt::క్రియాకారకం<(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(Val, O.Val);
  తిరిగి Val.sext(వెడల్పు).slt(O.Val.sext(వెడల్పు));
}
తార్కిక SlowDynamicAPInt::క్రియాకారకం<=(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(Val, O.Val);
  తిరిగి Val.sext(వెడల్పు).sle(O.Val.sext(వెడల్పు));
}
తార్కిక SlowDynamicAPInt::క్రియాకారకం>=(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(Val, O.Val);
  తిరిగి Val.sext(వెడల్పు).sge(O.Val.sext(వెడల్పు));
}

/// ---------------------------------------------------------------------------
/// Arithmetic operators.
/// ---------------------------------------------------------------------------

/// Bring a and b to have the same width and then call op(a, b, overflow).
/// If the overflow bit becomes set, resize a and b to double the width and
/// call op(a, b, overflow), returning its result. The operation with double
/// widths should not also overflow.
APInt నిరంతరం_Op_With_Expand_On_అధికప్రవాహం(
    స్థిరరాశి APInt &A, స్థిరరాశి APInt &B,
    function_ref<APInt(స్థిరరాశి APInt &, స్థిరరాశి APInt &, తార్కిక &అధికప్రవాహం)> Op) {
  తార్కిక అధికప్రవాహం;
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(A, B);
  APInt Ret = Op(A.sext(వెడల్పు), B.sext(వెడల్పు), అధికప్రవాహం);
  ఒకవేళ (!అధికప్రవాహం)
    తిరిగి Ret;

  వెడల్పు *= 2;
  Ret = Op(A.sext(వెడల్పు), B.sext(వెడల్పు), అధికప్రవాహం);
  ప్రతిజ్ఞా(!అధికప్రవాహం && "double width should be sufficient to avoid overflow!");
  తిరిగి Ret;
}

SlowDynamicAPInt SlowDynamicAPInt::క్రియాకారకం+(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  తిరిగి SlowDynamicAPInt(
      నిరంతరం_Op_With_Expand_On_అధికప్రవాహం(Val, O.Val, std::mem_fn(&APInt::sadd_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::క్రియాకారకం-(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  తిరిగి SlowDynamicAPInt(
      నిరంతరం_Op_With_Expand_On_అధికప్రవాహం(Val, O.Val, std::mem_fn(&APInt::ssub_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::క్రియాకారకం*(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  తిరిగి SlowDynamicAPInt(
      నిరంతరం_Op_With_Expand_On_అధికప్రవాహం(Val, O.Val, std::mem_fn(&APInt::smul_ov)));
}
SlowDynamicAPInt SlowDynamicAPInt::క్రియాకారకం/(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  తిరిగి SlowDynamicAPInt(
      నిరంతరం_Op_With_Expand_On_అధికప్రవాహం(Val, O.Val, std::mem_fn(&APInt::sdiv_ov)));
}
SlowDynamicAPInt llvm::detail::నిరపేక్ష(స్థిరరాశి SlowDynamicAPInt &X) {
  తిరిగి X >= 0 ? X : -X;
}
SlowDynamicAPInt llvm::detail::ceilDiv(స్థిరరాశి SlowDynamicAPInt &LHS,
                                       స్థిరరాశి SlowDynamicAPInt &RHS) {
  ఒకవేళ (RHS == -1)
    తిరిగి -LHS;
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(LHS.Val, RHS.Val);
  తిరిగి SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(వెడల్పు), RHS.Val.sext(వెడల్పు), APInt::Rounding::పైకి));
}
SlowDynamicAPInt llvm::detail::floorDiv(స్థిరరాశి SlowDynamicAPInt &LHS,
                                        స్థిరరాశి SlowDynamicAPInt &RHS) {
  ఒకవేళ (RHS == -1)
    తిరిగి -LHS;
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(LHS.Val, RHS.Val);
  తిరిగి SlowDynamicAPInt(APIntOps::RoundingSDiv(
      LHS.Val.sext(వెడల్పు), RHS.Val.sext(వెడల్పు), APInt::Rounding::కిందికి));
}
// The RHS is always expected to be positive, and the result
/// is always non-negative.
SlowDynamicAPInt llvm::detail::mod(స్థిరరాశి SlowDynamicAPInt &LHS,
                                   స్థిరరాశి SlowDynamicAPInt &RHS) {
  ప్రతిజ్ఞా(RHS >= 1 && "mod is only supported for positive divisors!");
  తిరిగి LHS % RHS < 0 ? LHS % RHS + RHS : LHS % RHS;
}

SlowDynamicAPInt llvm::detail::గరిష్ఠ_సామాన్య_భాజకం(స్థిరరాశి SlowDynamicAPInt &A,
                                   స్థిరరాశి SlowDynamicAPInt &B) {
  ప్రతిజ్ఞా(A >= 0 && B >= 0 && "operands must be non-negative!");
  గుర్తులేని వెడల్పు = get_Max_వెడల్పు(A.Val, B.Val);
  తిరిగి SlowDynamicAPInt(
      APIntOps::Greatest_Common_భాజకం(A.Val.sext(వెడల్పు), B.Val.sext(వెడల్పు)));
}

/// Returns the least common multiple of A and B.
SlowDynamicAPInt llvm::detail::lcm(స్థిరరాశి SlowDynamicAPInt &A,
                                   స్థిరరాశి SlowDynamicAPInt &B) {
  SlowDynamicAPInt X = నిరపేక్ష(A);
  SlowDynamicAPInt Y = నిరపేక్ష(B);
  తిరిగి (X * Y) / గరిష్ఠ_సామాన్య_భాజకం(X, Y);
}

/// This operation cannot overflow.
SlowDynamicAPInt SlowDynamicAPInt::క్రియాకారకం%(స్థిరరాశి SlowDynamicAPInt &O) స్థిరరాశి {
  గుర్తులేని వెడల్పు = std::max(Val.get_బిట్టు_వెడల్పు(), O.Val.get_బిట్టు_వెడల్పు());
  తిరిగి SlowDynamicAPInt(Val.sext(వెడల్పు).srem(O.Val.sext(వెడల్పు)));
}

SlowDynamicAPInt SlowDynamicAPInt::క్రియాకారకం-() స్థిరరాశి {
  ఒకవేళ (Val.is_Min_Signed_విలువ()) {
    /// Overflow only occurs when the value is the minimum possible value.
    APInt Ret = Val.sext(2 * Val.get_బిట్టు_వెడల్పు());
    తిరిగి SlowDynamicAPInt(-Ret);
  }
  తిరిగి SlowDynamicAPInt(-Val);
}

/// ---------------------------------------------------------------------------
/// Assignment operators, preincrement, predecrement.
/// ---------------------------------------------------------------------------
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం+=(స్థిరరాశి SlowDynamicAPInt &O) {
  *ఇది = *ఇది + O;
  తిరిగి *ఇది;
}
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం-=(స్థిరరాశి SlowDynamicAPInt &O) {
  *ఇది = *ఇది - O;
  తిరిగి *ఇది;
}
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం*=(స్థిరరాశి SlowDynamicAPInt &O) {
  *ఇది = *ఇది * O;
  తిరిగి *ఇది;
}
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం/=(స్థిరరాశి SlowDynamicAPInt &O) {
  *ఇది = *ఇది / O;
  తిరిగి *ఇది;
}
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం%=(స్థిరరాశి SlowDynamicAPInt &O) {
  *ఇది = *ఇది % O;
  తిరిగి *ఇది;
}
SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం++() {
  *ఇది += 1;
  తిరిగి *ఇది;
}

SlowDynamicAPInt &SlowDynamicAPInt::క్రియాకారకం--() {
  *ఇది -= 1;
  తిరిగి *ఇది;
}

/// ---------------------------------------------------------------------------
/// Printing.
/// ---------------------------------------------------------------------------
శూన్య SlowDynamicAPInt::print(raw_ostream &OS) స్థిరరాశి { OS << Val; }

#ఒకవేళ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_పద్ధతి శూన్య SlowDynamicAPInt::dump() స్థిరరాశి { print(dbgs()); }
#అయితే_ముగింపు
