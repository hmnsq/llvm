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
  もし (Mask & fcNegInf)
    NewMask |= fcPosInf;
  もし (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  もし (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  もし (Mask & fcNegZero)
    NewMask |= fcPosZero;
  もし (Mask & fcPosZero)
    NewMask |= fcNegZero;
  もし (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  もし (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  もし (Mask & fcPosInf)
    NewMask |= fcNegInf;
  戻る NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  もし (Mask & fcPosZero)
    NewMask |= fcZero;
  もし (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  もし (Mask & fcPosNormal)
    NewMask |= fcNormal;
  もし (Mask & fcPosInf)
    NewMask |= fcInf;
  戻る NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  もし (Mask & fcZero)
    NewMask |= fcZero;
  もし (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  もし (Mask & fcNormal)
    NewMask |= fcNormal;
  もし (Mask & fcInf)
    NewMask |= fcInf;
  戻る NewMask;
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

  もし (Mask == fcNone) {
    OS << "none)";
    戻る OS;
  }

  ListSeparator LS(" ");
  ために (auto [BitTest, Name] : NoFPClassName) {
    もし ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  戻る OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  もし (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    戻る;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  もし (DefaultMode != DenormalMode::getDefault()) {
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

  もし (LHS == fcNone || RHS == fcNone)
    戻る true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  もし (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    もし (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    もし (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  もし (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    戻る true;
  }

  もし (LowestBitRHS < HighestBitLHS)
    戻る false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  戻る !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  戻る cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  戻る cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  戻る cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  戻る cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  もし (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  戻る NewMask;
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
  もし (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  戻る NewMask;
}
