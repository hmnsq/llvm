//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#شامل "llvm/ADT/FloatingPointMode.h"
#شامل "llvm/ADT/StringExtras.h"

using فضاینام llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  اگر (Mask & fcNegInf)
    NewMask |= fcPosInf;
  اگر (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  اگر (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  اگر (Mask & fcNegZero)
    NewMask |= fcPosZero;
  اگر (Mask & fcPosZero)
    NewMask |= fcNegZero;
  اگر (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  اگر (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  اگر (Mask & fcPosInf)
    NewMask |= fcNegInf;
  بازگشت NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  اگر (Mask & fcPosZero)
    NewMask |= fcZero;
  اگر (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  اگر (Mask & fcPosNormal)
    NewMask |= fcNormal;
  اگر (Mask & fcPosInf)
    NewMask |= fcInf;
  بازگشت NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  اگر (Mask & fcZero)
    NewMask |= fcZero;
  اگر (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  اگر (Mask & fcNormal)
    NewMask |= fcNormal;
  اگر (Mask & fcInf)
    NewMask |= fcInf;
  بازگشت NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
ایستا constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

  اگر (Mask == fcNone) {
    OS << "none)";
    بازگشت OS;
  }

  ListSeparator LS(" ");
  برای (auto [BitTest, Name] : NoFPClassName) {
    اگر ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  أكد(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  بازگشت OS;
}

تهی DenormalFPEnv::print(raw_ostream &OS, منطقی OmitIfSame) ثابت {
  اگر (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/نادرست, OmitIfSame);
    بازگشت;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  اگر (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/نادرست, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/نادرست, OmitIfSame);
}

ایستا منطقی cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           منطقی OrEqual, منطقی OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  اگر (LHS == fcNone || RHS == fcNone)
    بازگشت درست;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  اگر (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    اگر (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    اگر (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  اگر (LowestBitRHS > HighestBitLHS) {
    أكد((LHS & RHS) == fcNone && "no bits should intersect");
    بازگشت درست;
  }

  اگر (LowestBitRHS < HighestBitLHS)
    بازگشت نادرست;

  constexpr FPClassTest Exact_مقادیر_Mask = fcZero | fcInf;
  بازگشت !OrEqual && (LowestBitRHS & Exact_مقادیر_Mask) != fcNone;
}

منطقی llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      منطقی OrderedZeroSign) {
  بازگشت cannotOrderStrictlyGreaterImpl(LHS, RHS, نادرست, OrderedZeroSign);
}

منطقی llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        منطقی OrderedZeroSign) {
  بازگشت cannotOrderStrictlyGreaterImpl(LHS, RHS, درست, OrderedZeroSign);
}

منطقی llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   منطقی OrderedZeroSign) {
  بازگشت cannotOrderStrictlyGreaterImpl(RHS, LHS, نادرست, OrderedZeroSign);
}

منطقی llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     منطقی OrderedZeroSign) {
  بازگشت cannotOrderStrictlyGreaterImpl(RHS, LHS, درست, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, منطقی OrderedZeroSign) {
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
  اگر (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  بازگشت NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         منطقی OrderedZeroSign) {
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
  اگر (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  بازگشت NewMask;
}
