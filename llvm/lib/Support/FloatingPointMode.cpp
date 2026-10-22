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
  གལ་ཏེ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  གལ་ཏེ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  གལ་ཏེ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  གལ་ཏེ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  གལ་ཏེ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  གལ་ཏེ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  གལ་ཏེ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  གལ་ཏེ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ཕྱིར NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  གལ་ཏེ (Mask & fcPosZero)
    NewMask |= fcZero;
  གལ་ཏེ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  གལ་ཏེ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  གལ་ཏེ (Mask & fcPosInf)
    NewMask |= fcInf;
  ཕྱིར NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  གལ་ཏེ (Mask & fcZero)
    NewMask |= fcZero;
  གལ་ཏེ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  གལ་ཏེ (Mask & fcNormal)
    NewMask |= fcNormal;
  གལ་ཏེ (Mask & fcInf)
    NewMask |= fcInf;
  ཕྱིར NewMask;
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

  གལ་ཏེ (Mask == fcNone) {
    OS << "none)";
    ཕྱིར OS;
  }

  ListSeparator LS(" ");
  ཆེད་དུ (auto [BitTest, Name] : NoFPClassName) {
    གལ་ཏེ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ཕྱིར OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  གལ་ཏེ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ཕྱིར;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  གལ་ཏེ (DefaultMode != DenormalMode::getDefault()) {
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

  གལ་ཏེ (LHS == fcNone || RHS == fcNone)
    ཕྱིར true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  གལ་ཏེ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    གལ་ཏེ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    གལ་ཏེ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  གལ་ཏེ (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ཕྱིར true;
  }

  གལ་ཏེ (LowestBitRHS < HighestBitLHS)
    ཕྱིར false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ཕྱིར !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ཕྱིར cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ཕྱིར cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ཕྱིར cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ཕྱིར cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  གལ་ཏེ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ཕྱིར NewMask;
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
  གལ་ཏེ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ཕྱིར NewMask;
}
