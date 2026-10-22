//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/FloatingPointMode.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"

using नामाकाश llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  सचे (Mask & fcNegInf)
    NewMask |= fcPosInf;
  सचे (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  सचे (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  सचे (Mask & fcNegZero)
    NewMask |= fcPosZero;
  सचे (Mask & fcPosZero)
    NewMask |= fcNegZero;
  सचे (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  सचे (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  सचे (Mask & fcPosInf)
    NewMask |= fcNegInf;
  पटिनिवत्त NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  सचे (Mask & fcPosZero)
    NewMask |= fcZero;
  सचे (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  सचे (Mask & fcPosNormal)
    NewMask |= fcNormal;
  सचे (Mask & fcPosInf)
    NewMask |= fcInf;
  पटिनिवत्त NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  सचे (Mask & fcZero)
    NewMask |= fcZero;
  सचे (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  सचे (Mask & fcNormal)
    NewMask |= fcNormal;
  सचे (Mask & fcInf)
    NewMask |= fcInf;
  पटिनिवत्त NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
स्थिर constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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
  {fcNormal, "norm"},
  {fcNegNormal, "nnorm"},
  {fcPosNormal, "pnorm"}
};

raw_ostream &llvm::operator<<(raw_ostream &OS, FPClassTest Mask) {
  OS << '(';

  सचे (Mask == fcNone) {
    OS << "none)";
    पटिनिवत्त OS;
  }

  ListSeparator LS(" ");
  अत्थाय (auto [BitTest, Name] : NoFPClassName) {
    सचे ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  प्रतिज्ञा(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  पटिनिवत्त OS;
}

शून्य DenormalFPEnv::print(raw_ostream &OS, तार्किक OmitIfSame) ध्रुव {
  सचे (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/मिच्छा, OmitIfSame);
    पटिनिवत्त;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  सचे (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/मिच्छा, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/मिच्छा, OmitIfSame);
}

स्थिर तार्किक cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           तार्किक OrEqual, तार्किक OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  सचे (LHS == fcNone || RHS == fcNone)
    पटिनिवत्त सच्च;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  सचे (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    सचे (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    सचे (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  सचे (LowestBitRHS > HighestBitLHS) {
    प्रतिज्ञा((LHS & RHS) == fcNone && "no bits should intersect");
    पटिनिवत्त सच्च;
  }

  सचे (LowestBitRHS < HighestBitLHS)
    पटिनिवत्त मिच्छा;

  constexpr FPClassTest Exact_मानानि_Mask = fcZero | fcInf;
  पटिनिवत्त !OrEqual && (LowestBitRHS & Exact_मानानि_Mask) != fcNone;
}

तार्किक llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      तार्किक OrderedZeroSign) {
  पटिनिवत्त cannotOrderStrictlyGreaterImpl(LHS, RHS, मिच्छा, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        तार्किक OrderedZeroSign) {
  पटिनिवत्त cannotOrderStrictlyGreaterImpl(LHS, RHS, सच्च, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   तार्किक OrderedZeroSign) {
  पटिनिवत्त cannotOrderStrictlyGreaterImpl(RHS, LHS, मिच्छा, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     तार्किक OrderedZeroSign) {
  पटिनिवत्त cannotOrderStrictlyGreaterImpl(RHS, LHS, सच्च, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, तार्किक OrderedZeroSign) {
  // Ignores NaN
  Mask &= ~fcNan;
  // Since the classes are ordered bits, we can get all classes which are
  // smaller than all classes in Known by setting all trailing 0 bits to 1,
  // and setting the other bits to 0.
  // This is done by counting the number of trailing 0 bits and creating a
  // mask based on that.
  // For example: 0b1111000000 = fcPositive
  //           -> 0b0000111111 = fcNegative | fcNan
  FPClassTest NewMask = static_cast<FPClassTest>(
      maskTrailingOnes<unsigned>(countr_zero<unsigned>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is smaller
  // if the zeroes are not ordered
  सचे (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  पटिनिवत्त NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         तार्किक OrderedZeroSign) {
  // Counting the number of leading 0 bits and create a mask based on that,
  // removing the leading 1s that are outside the bitfield:
  // For example: 0b0000111100 = fcNegative
  //           -> 0b1111000000 = fcPositive
  FPClassTest NewMask = static_cast<FPClassTest>(
      maskLeadingOnes<unsigned>(countl_zero<unsigned>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is greater
  // if the zeroes are not ordered
  सचे (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  पटिनिवत्त NewMask;
}
