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
  གལ་སྲིད (Mask & fcNegInf)
    NewMask |= fcPosInf;
  གལ་སྲིད (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  གལ་སྲིད (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  གལ་སྲིད (Mask & fcNegZero)
    NewMask |= fcPosZero;
  གལ་སྲིད (Mask & fcPosZero)
    NewMask |= fcNegZero;
  གལ་སྲིད (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  གལ་སྲིད (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  གལ་སྲིད (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ལོག NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  གལ་སྲིད (Mask & fcPosZero)
    NewMask |= fcZero;
  གལ་སྲིད (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  གལ་སྲིད (Mask & fcPosNormal)
    NewMask |= fcNormal;
  གལ་སྲིད (Mask & fcPosInf)
    NewMask |= fcInf;
  ལོག NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  གལ་སྲིད (Mask & fcZero)
    NewMask |= fcZero;
  གལ་སྲིད (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  གལ་སྲིད (Mask & fcNormal)
    NewMask |= fcNormal;
  གལ་སྲིད (Mask & fcInf)
    NewMask |= fcInf;
  ལོག NewMask;
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

  གལ་སྲིད (Mask == fcNone) {
    OS << "none)";
    ལོག OS;
  }

  ListSeparator LS(" ");
  དོན་ལུ (auto [BitTest, Name] : NoFPClassName) {
    གལ་སྲིད ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ལོག OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  གལ་སྲིད (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ལོག;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  གལ་སྲིད (DefaultMode != DenormalMode::getDefault()) {
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

  གལ་སྲིད (LHS == fcNone || RHS == fcNone)
    ལོག true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  གལ་སྲིད (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    གལ་སྲིད (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    གལ་སྲིད (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  གལ་སྲིད (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ལོག true;
  }

  གལ་སྲིད (LowestBitRHS < HighestBitLHS)
    ལོག false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ལོག !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ལོག cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ལོག cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ལོག cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ལོག cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  གལ་སྲིད (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ལོག NewMask;
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
  གལ་སྲིད (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ལོག NewMask;
}
