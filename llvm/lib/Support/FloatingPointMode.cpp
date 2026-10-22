//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/FloatingPointMode.h"
#ضمن "llvm/ADT/StringExtras.h"

using نطاق llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ܐܢ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ܐܢ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ܐܢ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ܐܢ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ܐܢ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ܐܢ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ܐܢ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ܐܢ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ܦܢܐ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ܐܢ (Mask & fcPosZero)
    NewMask |= fcZero;
  ܐܢ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ܐܢ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ܐܢ (Mask & fcPosInf)
    NewMask |= fcInf;
  ܦܢܐ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ܐܢ (Mask & fcZero)
    NewMask |= fcZero;
  ܐܢ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ܐܢ (Mask & fcNormal)
    NewMask |= fcNormal;
  ܐܢ (Mask & fcInf)
    NewMask |= fcInf;
  ܦܢܐ NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
ساكن constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

  ܐܢ (Mask == fcNone) {
    OS << "none)";
    ܦܢܐ OS;
  }

  ListSeparator LS(" ");
  لكل (auto [BitTest, Name] : NoFPClassName) {
    ܐܢ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  أكد(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ܦܢܐ OS;
}

فراغ DenormalFPEnv::print(raw_ostream &OS, منطقي OmitIfSame) ثابت {
  ܐܢ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/ܕܓܠܐ, OmitIfSame);
    ܦܢܐ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ܐܢ (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/ܕܓܠܐ, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/ܕܓܠܐ, OmitIfSame);
}

ساكن منطقي cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           منطقي OrEqual, منطقي OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  ܐܢ (LHS == fcNone || RHS == fcNone)
    ܦܢܐ ܫܪܪܐ;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ܐܢ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ܐܢ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ܐܢ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ܐܢ (LowestBitRHS > HighestBitLHS) {
    أكد((LHS & RHS) == fcNone && "no bits should intersect");
    ܦܢܐ ܫܪܪܐ;
  }

  ܐܢ (LowestBitRHS < HighestBitLHS)
    ܦܢܐ ܕܓܠܐ;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ܦܢܐ !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

منطقي llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      منطقي OrderedZeroSign) {
  ܦܢܐ cannotOrderStrictlyGreaterImpl(LHS, RHS, ܕܓܠܐ, OrderedZeroSign);
}

منطقي llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        منطقي OrderedZeroSign) {
  ܦܢܐ cannotOrderStrictlyGreaterImpl(LHS, RHS, ܫܪܪܐ, OrderedZeroSign);
}

منطقي llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   منطقي OrderedZeroSign) {
  ܦܢܐ cannotOrderStrictlyGreaterImpl(RHS, LHS, ܕܓܠܐ, OrderedZeroSign);
}

منطقي llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     منطقي OrderedZeroSign) {
  ܦܢܐ cannotOrderStrictlyGreaterImpl(RHS, LHS, ܫܪܪܐ, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, منطقي OrderedZeroSign) {
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
  ܐܢ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ܦܢܐ NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         منطقي OrderedZeroSign) {
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
  ܐܢ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ܦܢܐ NewMask;
}
