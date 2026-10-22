//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/FloatingPointMode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "llvm/ADT/StringExtras.h"

ਵਰਤੋਂ ਨਾਮਾਕਾਸ਼ llvm;

FPClassTest llvm::fneg(FPClassTest ਨਕਾਬ) {
  FPClassTest New_ਨਕਾਬ = ਨਕਾਬ & fcNan;
  ਜੇ (ਨਕਾਬ & fcNegInf)
    New_ਨਕਾਬ |= fcPosInf;
  ਜੇ (ਨਕਾਬ & fc_Neg_ਸਾਧਾਰਨ)
    New_ਨਕਾਬ |= fc_Pos_ਸਾਧਾਰਨ;
  ਜੇ (ਨਕਾਬ & fcNegSubnormal)
    New_ਨਕਾਬ |= fcPosSubnormal;
  ਜੇ (ਨਕਾਬ & fcNegZero)
    New_ਨਕਾਬ |= fcPosZero;
  ਜੇ (ਨਕਾਬ & fcPosZero)
    New_ਨਕਾਬ |= fcNegZero;
  ਜੇ (ਨਕਾਬ & fcPosSubnormal)
    New_ਨਕਾਬ |= fcNegSubnormal;
  ਜੇ (ਨਕਾਬ & fc_Pos_ਸਾਧਾਰਨ)
    New_ਨਕਾਬ |= fc_Neg_ਸਾਧਾਰਨ;
  ਜੇ (ਨਕਾਬ & fcPosInf)
    New_ਨਕਾਬ |= fcNegInf;
  ਵਾਪਸ New_ਨਕਾਬ;
}

FPClassTest llvm::inverse_fabs(FPClassTest ਨਕਾਬ) {
  FPClassTest New_ਨਕਾਬ = ਨਕਾਬ & fcNan;
  ਜੇ (ਨਕਾਬ & fcPosZero)
    New_ਨਕਾਬ |= fcZero;
  ਜੇ (ਨਕਾਬ & fcPosSubnormal)
    New_ਨਕਾਬ |= fcSubnormal;
  ਜੇ (ਨਕਾਬ & fc_Pos_ਸਾਧਾਰਨ)
    New_ਨਕਾਬ |= fc_ਸਾਧਾਰਨ;
  ਜੇ (ਨਕਾਬ & fcPosInf)
    New_ਨਕਾਬ |= fcInf;
  ਵਾਪਸ New_ਨਕਾਬ;
}

FPClassTest llvm::unknown_sign(FPClassTest ਨਕਾਬ) {
  FPClassTest New_ਨਕਾਬ = ਨਕਾਬ & fcNan;
  ਜੇ (ਨਕਾਬ & fcZero)
    New_ਨਕਾਬ |= fcZero;
  ਜੇ (ਨਕਾਬ & fcSubnormal)
    New_ਨਕਾਬ |= fcSubnormal;
  ਜੇ (ਨਕਾਬ & fc_ਸਾਧਾਰਨ)
    New_ਨਕਾਬ |= fc_ਸਾਧਾਰਨ;
  ਜੇ (ਨਕਾਬ & fcInf)
    New_ਨਕਾਬ |= fcInf;
  ਵਾਪਸ New_ਨਕਾਬ;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
ਸਥਿਰ constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
  {fcAllFlags, "all"},
  {fcNan, "nan"},
  {fcSNan, "snan"},
  {fcQNan, "qnan"},
  {fcInf, "inf"},
  {fcNegInf, "ninf"},
  {fcPosInf, "pinf"},
  {fcZero, "zero"},
  {fcNegZero, "nzero"},
  {fcPosZero, "pzero"},
  {fcSubnormal, "sub"},
  {fcNegSubnormal, "nsub"},
  {fcPosSubnormal, "psub"},
  {fc_ਸਾਧਾਰਨ, "norm"},
  {fc_Neg_ਸਾਧਾਰਨ, "nnorm"},
  {fc_Pos_ਸਾਧਾਰਨ, "pnorm"}
};

raw_ostream &llvm::ਸੰਚਾਲਕ<<(raw_ostream &OS, FPClassTest ਨਕਾਬ) {
  OS << '(';

  ਜੇ (ਨਕਾਬ == fcNone) {
    OS << "none)";
    ਵਾਪਸ OS;
  }

  ListSeparator LS(" ");
  ਲਈ (ਸਵੈਚਾਲਿਤ [ਦੋਆਧਾਰੀ_ਅੰਕ_Test, Name] : NoFPClassName) {
    ਜੇ ((ਨਕਾਬ & ਦੋਆਧਾਰੀ_ਅੰਕ_Test) == ਦੋਆਧਾਰੀ_ਅੰਕ_Test) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      ਨਕਾਬ &= ~ਦੋਆਧਾਰੀ_ਅੰਕ_Test;
    }
  }

  ਪ੍ਰਤਿਜ੍ਞਾ(ਨਕਾਬ == 0 && "didn't print some mask bits");

  OS << ')';
  ਵਾਪਸ OS;
}

ਖਾਲੀ DenormalFPEnv::print(raw_ostream &OS, ਤਾਰਕਿਕ OmitIfSame) ਅਚਲ {
  ਜੇ (F32_ਬਹੁਲਕ == Default_ਬਹੁਲਕ) {
    Default_ਬਹੁਲਕ.print(OS, /*Legacy=*/ਝੂਠ, OmitIfSame);
    ਵਾਪਸ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ਜੇ (Default_ਬਹੁਲਕ != Denormal_ਬਹੁਲਕ::getDefault()) {
    Default_ਬਹੁਲਕ.print(OS, /*Legacy=*/ਝੂਠ, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32_ਬਹੁਲਕ.print(OS, /*Legacy=*/ਝੂਠ, OmitIfSame);
}

ਸਥਿਰ ਤਾਰਕਿਕ cannot_ਕ੍ਰਮ_Strictly_Greater_Impl(FPClassTest LHS, FPClassTest RHS,
                                           ਤਾਰਕਿਕ OrEqual, ਤਾਰਕਿਕ OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  ਜੇ (LHS == fcNone || RHS == fcNone)
    ਵਾਪਸ ਸੱਚ;

  FPClassTest ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_RHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest Highest_ਦੋਆਧਾਰੀ_ਅੰਕ_LHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ਜੇ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ਜੇ (ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_RHS == fcNegZero)
      ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_RHS = fcPosZero;
    ਜੇ (Highest_ਦੋਆਧਾਰੀ_ਅੰਕ_LHS == fcNegZero)
      Highest_ਦੋਆਧਾਰੀ_ਅੰਕ_LHS = fcPosZero;
  }

  ਜੇ (ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_RHS > Highest_ਦੋਆਧਾਰੀ_ਅੰਕ_LHS) {
    ਪ੍ਰਤਿਜ੍ਞਾ((LHS & RHS) == fcNone && "no bits should intersect");
    ਵਾਪਸ ਸੱਚ;
  }

  ਜੇ (ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_RHS < Highest_ਦੋਆਧਾਰੀ_ਅੰਕ_LHS)
    ਵਾਪਸ ਝੂਠ;

  constexpr FPClassTest Exact_ਮੁੱਲਾਂ_ਨਕਾਬ = fcZero | fcInf;
  ਵਾਪਸ !OrEqual && (ਸਭ_ਤੋਂ_ਨੀਵਾਂ_ਦੋਆਧਾਰੀ_ਅੰਕ_RHS & Exact_ਮੁੱਲਾਂ_ਨਕਾਬ) != fcNone;
}

ਤਾਰਕਿਕ llvm::cannot_ਕ੍ਰਮ_Strictly_Greater(FPClassTest LHS, FPClassTest RHS,
                                      ਤਾਰਕਿਕ OrderedZeroSign) {
  ਵਾਪਸ cannot_ਕ੍ਰਮ_Strictly_Greater_Impl(LHS, RHS, ਝੂਠ, OrderedZeroSign);
}

ਤਾਰਕਿਕ llvm::cannot_ਕ੍ਰਮ_Strictly_Greater_Eq(FPClassTest LHS, FPClassTest RHS,
                                        ਤਾਰਕਿਕ OrderedZeroSign) {
  ਵਾਪਸ cannot_ਕ੍ਰਮ_Strictly_Greater_Impl(LHS, RHS, ਸੱਚ, OrderedZeroSign);
}

ਤਾਰਕਿਕ llvm::cannot_ਕ੍ਰਮ_Strictly_Less(FPClassTest LHS, FPClassTest RHS,
                                   ਤਾਰਕਿਕ OrderedZeroSign) {
  ਵਾਪਸ cannot_ਕ੍ਰਮ_Strictly_Greater_Impl(RHS, LHS, ਝੂਠ, OrderedZeroSign);
}

ਤਾਰਕਿਕ llvm::cannot_ਕ੍ਰਮ_Strictly_Less_Eq(FPClassTest LHS, FPClassTest RHS,
                                     ਤਾਰਕਿਕ OrderedZeroSign) {
  ਵਾਪਸ cannot_ਕ੍ਰਮ_Strictly_Greater_Impl(RHS, LHS, ਸੱਚ, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest ਨਕਾਬ, ਤਾਰਕਿਕ OrderedZeroSign) {
  // Ignores NaN
  ਨਕਾਬ &= ~fcNan;
  // Since the classes are ordered bits, we can get all classes which are
  // smaller than all classes in Known by setting all trailing 0 bits to 1,
  // and setting the other bits to 0.
  // This is done by counting the number of trailing 0 bits and creating a
  // mask based on that.
  // For example: 0b1111000000 = fcPositive
  //           -> 0b0000111111 = fcNegative | fcNan
  FPClassTest New_ਨਕਾਬ = static_cast<FPClassTest>(
      ਨਕਾਬ_Trailing_Ones<ਬਿਨਾਨਿਸ਼ਾਨੀ>(countr_zero<ਬਿਨਾਨਿਸ਼ਾਨੀ>(ਨਕਾਬ)) & fcAllFlags);
  // Remove NaNs
  New_ਨਕਾਬ &= ~fcNan;
  // We cannot conclude that only one zero is smaller
  // if the zeroes are not ordered
  ਜੇ (!OrderedZeroSign && ((New_ਨਕਾਬ & fcZero) != fcZero))
    New_ਨਕਾਬ &= ~fcZero;
  ਵਾਪਸ New_ਨਕਾਬ;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest ਨਕਾਬ,
                                         ਤਾਰਕਿਕ OrderedZeroSign) {
  // Counting the number of leading 0 bits and create a mask based on that,
  // removing the leading 1s that are outside the bitfield:
  // For example: 0b0000111100 = fcNegative
  //           -> 0b1111000000 = fcPositive
  FPClassTest New_ਨਕਾਬ = static_cast<FPClassTest>(
      ਨਕਾਬ_Leading_Ones<ਬਿਨਾਨਿਸ਼ਾਨੀ>(countl_zero<ਬਿਨਾਨਿਸ਼ਾਨੀ>(ਨਕਾਬ)) & fcAllFlags);
  // Remove NaNs
  New_ਨਕਾਬ &= ~fcNan;
  // We cannot conclude that only one zero is greater
  // if the zeroes are not ordered
  ਜੇ (!OrderedZeroSign && ((New_ਨਕਾਬ & fcZero) != fcZero))
    New_ਨਕਾਬ &= ~fcZero;
  ਵਾਪਸ New_ਨਕਾਬ;
}
