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
  ຖ້າ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ຖ້າ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ຖ້າ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ຖ້າ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ຖ້າ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ຖ້າ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ຖ້າ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ຖ້າ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ສົ່ງຄືນ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ຖ້າ (Mask & fcPosZero)
    NewMask |= fcZero;
  ຖ້າ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ຖ້າ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ຖ້າ (Mask & fcPosInf)
    NewMask |= fcInf;
  ສົ່ງຄືນ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ຖ້າ (Mask & fcZero)
    NewMask |= fcZero;
  ຖ້າ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ຖ້າ (Mask & fcNormal)
    NewMask |= fcNormal;
  ຖ້າ (Mask & fcInf)
    NewMask |= fcInf;
  ສົ່ງຄືນ NewMask;
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

  ຖ້າ (Mask == fcNone) {
    OS << "none)";
    ສົ່ງຄືນ OS;
  }

  ListSeparator LS(" ");
  ສຳລັບ (auto [BitTest, Name] : NoFPClassName) {
    ຖ້າ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ສົ່ງຄືນ OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  ຖ້າ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ສົ່ງຄືນ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ຖ້າ (DefaultMode != DenormalMode::getDefault()) {
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

  ຖ້າ (LHS == fcNone || RHS == fcNone)
    ສົ່ງຄືນ true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ຖ້າ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ຖ້າ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ຖ້າ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ຖ້າ (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ສົ່ງຄືນ true;
  }

  ຖ້າ (LowestBitRHS < HighestBitLHS)
    ສົ່ງຄືນ false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ສົ່ງຄືນ !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ສົ່ງຄືນ cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ສົ່ງຄືນ cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ສົ່ງຄືນ cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ສົ່ງຄືນ cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  ຖ້າ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ສົ່ງຄືນ NewMask;
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
  ຖ້າ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ສົ່ງຄືນ NewMask;
}
