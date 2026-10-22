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
  ᨀᨘ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ᨀᨘ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ᨀᨘ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ᨀᨘ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ᨀᨘ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ᨀᨘ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ᨀᨘ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ᨀᨘ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ᨀᨘᨒ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ᨀᨘ (Mask & fcPosZero)
    NewMask |= fcZero;
  ᨀᨘ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ᨀᨘ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ᨀᨘ (Mask & fcPosInf)
    NewMask |= fcInf;
  ᨀᨘᨒ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ᨀᨘ (Mask & fcZero)
    NewMask |= fcZero;
  ᨀᨘ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ᨀᨘ (Mask & fcNormal)
    NewMask |= fcNormal;
  ᨀᨘ (Mask & fcInf)
    NewMask |= fcInf;
  ᨀᨘᨒ NewMask;
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

  ᨀᨘ (Mask == fcNone) {
    OS << "none)";
    ᨀᨘᨒ OS;
  }

  ListSeparator LS(" ");
  for (auto [BitTest, Name] : NoFPClassName) {
    ᨀᨘ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ᨀᨘᨒ OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  ᨀᨘ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ᨀᨘᨒ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ᨀᨘ (DefaultMode != DenormalMode::getDefault()) {
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

  ᨀᨘ (LHS == fcNone || RHS == fcNone)
    ᨀᨘᨒ true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ᨀᨘ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ᨀᨘ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ᨀᨘ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ᨀᨘ (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ᨀᨘᨒ true;
  }

  ᨀᨘ (LowestBitRHS < HighestBitLHS)
    ᨀᨘᨒ false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ᨀᨘᨒ !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ᨀᨘᨒ cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ᨀᨘᨒ cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ᨀᨘᨒ cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ᨀᨘᨒ cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  ᨀᨘ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ᨀᨘᨒ NewMask;
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
  ᨀᨘ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ᨀᨘᨒ NewMask;
}
