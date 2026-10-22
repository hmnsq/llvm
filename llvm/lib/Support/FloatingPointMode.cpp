//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#અન્તર્ભાવ "llvm/ADT/FloatingPointMode.h"
#અન્તર્ભાવ "llvm/ADT/StringExtras.h"

using નામાકાશ llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  જો (Mask & fcNegInf)
    NewMask |= fcPosInf;
  જો (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  જો (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  જો (Mask & fcNegZero)
    NewMask |= fcPosZero;
  જો (Mask & fcPosZero)
    NewMask |= fcNegZero;
  જો (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  જો (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  જો (Mask & fcPosInf)
    NewMask |= fcNegInf;
  પાછા NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  જો (Mask & fcPosZero)
    NewMask |= fcZero;
  જો (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  જો (Mask & fcPosNormal)
    NewMask |= fcNormal;
  જો (Mask & fcPosInf)
    NewMask |= fcInf;
  પાછા NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  જો (Mask & fcZero)
    NewMask |= fcZero;
  જો (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  જો (Mask & fcNormal)
    NewMask |= fcNormal;
  જો (Mask & fcInf)
    NewMask |= fcInf;
  પાછા NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
સ્થિર constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

  જો (Mask == fcNone) {
    OS << "none)";
    પાછા OS;
  }

  ListSeparator LS(" ");
  માટે (auto [BitTest, Name] : NoFPClassName) {
    જો ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  પ્રતિજ્ઞા(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  પાછા OS;
}

શૂન્ય DenormalFPEnv::print(raw_ostream &OS, તાર્કિક OmitIfSame) અચલ {
  જો (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/ખોટું, OmitIfSame);
    પાછા;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  જો (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/ખોટું, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/ખોટું, OmitIfSame);
}

સ્થિર તાર્કિક cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           તાર્કિક OrEqual, તાર્કિક OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  જો (LHS == fcNone || RHS == fcNone)
    પાછા સાચું;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  જો (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    જો (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    જો (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  જો (LowestBitRHS > HighestBitLHS) {
    પ્રતિજ્ઞા((LHS & RHS) == fcNone && "no bits should intersect");
    પાછા સાચું;
  }

  જો (LowestBitRHS < HighestBitLHS)
    પાછા ખોટું;

  constexpr FPClassTest Exact_કિંમતો_Mask = fcZero | fcInf;
  પાછા !OrEqual && (LowestBitRHS & Exact_કિંમતો_Mask) != fcNone;
}

તાર્કિક llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      તાર્કિક OrderedZeroSign) {
  પાછા cannotOrderStrictlyGreaterImpl(LHS, RHS, ખોટું, OrderedZeroSign);
}

તાર્કિક llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        તાર્કિક OrderedZeroSign) {
  પાછા cannotOrderStrictlyGreaterImpl(LHS, RHS, સાચું, OrderedZeroSign);
}

તાર્કિક llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   તાર્કિક OrderedZeroSign) {
  પાછા cannotOrderStrictlyGreaterImpl(RHS, LHS, ખોટું, OrderedZeroSign);
}

તાર્કિક llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     તાર્કિક OrderedZeroSign) {
  પાછા cannotOrderStrictlyGreaterImpl(RHS, LHS, સાચું, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, તાર્કિક OrderedZeroSign) {
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
  જો (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  પાછા NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         તાર્કિક OrderedZeroSign) {
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
  જો (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  પાછા NewMask;
}
