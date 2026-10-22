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
  ꦪꦼꦤ꧀ (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ꦪꦼꦤ꧀ (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ꦪꦼꦤ꧀ (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ꦪꦼꦤ꧀ (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ꦪꦼꦤ꧀ (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ꦪꦼꦤ꧀ (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ꦪꦼꦤ꧀ (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ꦪꦼꦤ꧀ (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ꦮꦭꦶ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ꦪꦼꦤ꧀ (Mask & fcPosZero)
    NewMask |= fcZero;
  ꦪꦼꦤ꧀ (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ꦪꦼꦤ꧀ (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ꦪꦼꦤ꧀ (Mask & fcPosInf)
    NewMask |= fcInf;
  ꦮꦭꦶ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ꦪꦼꦤ꧀ (Mask & fcZero)
    NewMask |= fcZero;
  ꦪꦼꦤ꧀ (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ꦪꦼꦤ꧀ (Mask & fcNormal)
    NewMask |= fcNormal;
  ꦪꦼꦤ꧀ (Mask & fcInf)
    NewMask |= fcInf;
  ꦮꦭꦶ NewMask;
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

  ꦪꦼꦤ꧀ (Mask == fcNone) {
    OS << "none)";
    ꦮꦭꦶ OS;
  }

  ListSeparator LS(" ");
  ꦏꦁꦒꦺꦴ (auto [BitTest, Name] : NoFPClassName) {
    ꦪꦼꦤ꧀ ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  assert(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ꦮꦭꦶ OS;
}

void DenormalFPEnv::print(raw_ostream &OS, bool OmitIfSame) const {
  ꦪꦼꦤ꧀ (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/false, OmitIfSame);
    ꦮꦭꦶ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ꦪꦼꦤ꧀ (DefaultMode != DenormalMode::getDefault()) {
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

  ꦪꦼꦤ꧀ (LHS == fcNone || RHS == fcNone)
    ꦮꦭꦶ true;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ꦪꦼꦤ꧀ (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ꦪꦼꦤ꧀ (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ꦪꦼꦤ꧀ (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ꦪꦼꦤ꧀ (LowestBitRHS > HighestBitLHS) {
    assert((LHS & RHS) == fcNone && "no bits should intersect");
    ꦮꦭꦶ true;
  }

  ꦪꦼꦤ꧀ (LowestBitRHS < HighestBitLHS)
    ꦮꦭꦶ false;

  constexpr FPClassTest ExactValuesMask = fcZero | fcInf;
  ꦮꦭꦶ !OrEqual && (LowestBitRHS & ExactValuesMask) != fcNone;
}

bool llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      bool OrderedZeroSign) {
  ꦮꦭꦶ cannotOrderStrictlyGreaterImpl(LHS, RHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        bool OrderedZeroSign) {
  ꦮꦭꦶ cannotOrderStrictlyGreaterImpl(LHS, RHS, true, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   bool OrderedZeroSign) {
  ꦮꦭꦶ cannotOrderStrictlyGreaterImpl(RHS, LHS, false, OrderedZeroSign);
}

bool llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     bool OrderedZeroSign) {
  ꦮꦭꦶ cannotOrderStrictlyGreaterImpl(RHS, LHS, true, OrderedZeroSign);
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
  ꦪꦼꦤ꧀ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ꦮꦭꦶ NewMask;
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
  ꦪꦼꦤ꧀ (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ꦮꦭꦶ NewMask;
}
