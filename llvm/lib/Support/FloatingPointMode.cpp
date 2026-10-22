//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#समावेश "llvm/ADT/FloatingPointMode.h"
#समावेश "llvm/ADT/StringExtras.h"

using namespace llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  अगर (Mask & fcNegInf)
    NewMask |= fcPosInf;
  अगर (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  अगर (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  अगर (Mask & fcNegZero)
    NewMask |= fcPosZero;
  अगर (Mask & fcPosZero)
    NewMask |= fcNegZero;
  अगर (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  अगर (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  अगर (Mask & fcPosInf)
    NewMask |= fcNegInf;
  वापस NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  अगर (Mask & fcPosZero)
    NewMask |= fcZero;
  अगर (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  अगर (Mask & fcPosNormal)
    NewMask |= fcNormal;
  अगर (Mask & fcPosInf)
    NewMask |= fcInf;
  वापस NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  अगर (Mask & fcZero)
    NewMask |= fcZero;
  अगर (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  अगर (Mask & fcNormal)
    NewMask |= fcNormal;
  अगर (Mask & fcInf)
    NewMask |= fcInf;
  वापस NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
अटल constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

raw_ostream &llvm::चालक<<(raw_ostream &OS, FPClassTest Mask) {
  OS << '(';

  अगर (Mask == fcNone) {
    OS << "none)";
    वापस OS;
  }

  ListSeparator LS(" ");
  क्रम (स्वतः [BitTest, Name] : NoFPClassName) {
    अगर ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  निश्चित(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  वापस OS;
}

व्योम DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) स्थिर {
  अगर (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    वापस;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  अगर (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/false, OmitIfSame);
}

अटल bool cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           bool OrEqual, bool OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  अगर (LHS == fcNone || RHS == fcNone)
    वापस true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  अगर (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    अगर (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    अगर (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  अगर (LowestBitRHS > HighestBitLHS) {
    निश्चित((LHS & RHS) == fcNone && "no bits should intersect");
    वापस true;
  }

  अगर (LowestBitRHS < HighestBitLHS)
    वापस false;

  constexpr FPClassTest Exact_मूल्यावली_Mask = fcZero | fcInf;
  वापस !OrEqual && (LowestBitRHS & Exact_मूल्यावली_Mask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  वापस cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  वापस cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  वापस cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  वापस cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, bool OrderedZeroSign) {
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
      maskTrailingOnes<अचिन्हित>(countr_zero<अचिन्हित>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is smaller
  // if the zeroes are not ordered
  अगर (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  वापस NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         bool OrderedZeroSign) {
  // Counting the number of leading 0 bits and create a mask based on that,
  // removing the leading 1s that are outside the bitfield:
  // For example: 0b0000111100 = fcNegative
  //           -> 0b1111000000 = fcPositive
  FPClassTest NewMask = static_cast<FPClassTest>(
      maskLeadingOnes<अचिन्हित>(countl_zero<अचिन्हित>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is greater
  // if the zeroes are not ordered
  अगर (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  वापस NewMask;
}
