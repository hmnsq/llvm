//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/ADT/FloatingPointMode.h"
#include "llvm/ADT/StringExtras.h"

using namespace llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ᤀᤡ᤺ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ᤀᤡ᤺ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ᤀᤡ᤺ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ᤀᤡ᤺ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ᤀᤡ᤺ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ᤀᤡ᤺ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ᤀᤡ᤺ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ᤀᤡ᤺ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ᤀᤡᤴ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ᤀᤡ᤺ (Mask & fcPosZero)
    NewMask |= fcZero;
  ᤀᤡ᤺ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ᤀᤡ᤺ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ᤀᤡ᤺ (Mask & fcPosInf)
    NewMask |= fcInf;
  ᤀᤡᤴ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ᤀᤡ᤺ (Mask & fcZero)
    NewMask |= fcZero;
  ᤀᤡ᤺ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ᤀᤡ᤺ (Mask & fcNormal)
    NewMask |= fcNormal;
  ᤀᤡ᤺ (Mask & fcInf)
    NewMask |= fcInf;
  ᤀᤡᤴ NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
static constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

  ᤀᤡ᤺ (Mask == fcNone) {
    OS << "none)";
    ᤀᤡᤴ OS;
  }

  ListSeparator LS(" ");
  for (auto [BitTest, Name] : NoFPClassName) {
    ᤀᤡ᤺ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ᤀᤡᤴ OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  ᤀᤡ᤺ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ᤀᤡᤴ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ᤀᤡ᤺ (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/false, OmitIfSame);
}

static bool cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           bool OrEqual, bool OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  ᤀᤡ᤺ (LHS == fcNone || RHS == fcNone)
    ᤀᤡᤴ true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ᤀᤡ᤺ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ᤀᤡ᤺ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ᤀᤡ᤺ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ᤀᤡ᤺ (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ᤀᤡᤴ true;
  }

  ᤀᤡ᤺ (LowestBitRHS < HighestBitLHS)
    ᤀᤡᤴ false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ᤀᤡᤴ !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ᤀᤡᤴ cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ᤀᤡᤴ cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ᤀᤡᤴ cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ᤀᤡᤴ cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
      maskTrailingOnes<unsigned>(countr_zero<unsigned>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is smaller
  // if the zeroes are not ordered
  ᤀᤡ᤺ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ᤀᤡᤴ NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         bool OrderedZeroSign) {
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
  ᤀᤡ᤺ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ᤀᤡᤴ NewMask;
}
