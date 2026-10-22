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
  ប្រសិនបើ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ប្រសិនបើ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ប្រសិនបើ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ប្រសិនបើ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ប្រសិនបើ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ប្រសិនបើ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ប្រសិនបើ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ប្រសិនបើ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ត្រឡប់ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ប្រសិនបើ (Mask & fcPosZero)
    NewMask |= fcZero;
  ប្រសិនបើ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ប្រសិនបើ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ប្រសិនបើ (Mask & fcPosInf)
    NewMask |= fcInf;
  ត្រឡប់ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ប្រសិនបើ (Mask & fcZero)
    NewMask |= fcZero;
  ប្រសិនបើ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ប្រសិនបើ (Mask & fcNormal)
    NewMask |= fcNormal;
  ប្រសិនបើ (Mask & fcInf)
    NewMask |= fcInf;
  ត្រឡប់ NewMask;
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

  ប្រសិនបើ (Mask == fcNone) {
    OS << "none)";
    ត្រឡប់ OS;
  }

  ListSeparator LS(" ");
  សម្រាប់ (auto [BitTest, Name] : NoFPClassName) {
    ប្រសិនបើ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ត្រឡប់ OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  ប្រសិនបើ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ត្រឡប់;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ប្រសិនបើ (DefaultMode != DenormalMode::getDefault()) {
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

  ប្រសិនបើ (LHS == fcNone || RHS == fcNone)
    ត្រឡប់ true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ប្រសិនបើ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ប្រសិនបើ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ប្រសិនបើ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ប្រសិនបើ (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ត្រឡប់ true;
  }

  ប្រសិនបើ (LowestBitRHS < HighestBitLHS)
    ត្រឡប់ false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ត្រឡប់ !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ត្រឡប់ cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ត្រឡប់ cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ត្រឡប់ cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ត្រឡប់ cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  ប្រសិនបើ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ត្រឡប់ NewMask;
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
  ប្រសិនបើ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ត្រឡប់ NewMask;
}
