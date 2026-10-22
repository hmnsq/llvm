//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/FloatingPointMode.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/StringExtras.h"

using ନାମାକାଶ llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ଯଦି (Mask & fcNegInf)
    NewMask |= fcPosInf;
  ଯଦି (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  ଯଦି (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  ଯଦି (Mask & fcNegZero)
    NewMask |= fcPosZero;
  ଯଦି (Mask & fcPosZero)
    NewMask |= fcNegZero;
  ଯଦି (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  ଯଦି (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  ଯଦି (Mask & fcPosInf)
    NewMask |= fcNegInf;
  ଫେରନ୍ତୁ NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ଯଦି (Mask & fcPosZero)
    NewMask |= fcZero;
  ଯଦି (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  ଯଦି (Mask & fcPosNormal)
    NewMask |= fcNormal;
  ଯଦି (Mask & fcPosInf)
    NewMask |= fcInf;
  ଫେରନ୍ତୁ NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  ଯଦି (Mask & fcZero)
    NewMask |= fcZero;
  ଯଦି (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  ଯଦି (Mask & fcNormal)
    NewMask |= fcNormal;
  ଯଦି (Mask & fcInf)
    NewMask |= fcInf;
  ଫେରନ୍ତୁ NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
ସ୍ଥିର constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

  ଯଦି (Mask == fcNone) {
    OS << "none)";
    ଫେରନ୍ତୁ OS;
  }

  ListSeparator LS(" ");
  ପାଇଁ (auto [BitTest, Name] : NoFPClassName) {
    ଯଦି ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  ପ୍ରତିଜ୍ଞା(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  ଫେରନ୍ତୁ OS;
}

ଶୂନ୍ୟ DenormalFPEnv::print(raw_ostream &OS, ତାର୍କିକ OmitIfSame) ଅଚଳ {
  ଯଦି (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/ମିଥ୍ୟା, OmitIfSame);
    ଫେରନ୍ତୁ;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  ଯଦି (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/ମିଥ୍ୟା, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/ମିଥ୍ୟା, OmitIfSame);
}

ସ୍ଥିର ତାର୍କିକ cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           ତାର୍କିକ OrEqual, ତାର୍କିକ OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  ଯଦି (LHS == fcNone || RHS == fcNone)
    ଫେରନ୍ତୁ ସତ୍ୟ;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  ଯଦି (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    ଯଦି (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    ଯଦି (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  ଯଦି (LowestBitRHS > HighestBitLHS) {
    ପ୍ରତିଜ୍ଞା((LHS & RHS) == fcNone && "no bits should intersect");
    ଫେରନ୍ତୁ ସତ୍ୟ;
  }

  ଯଦି (LowestBitRHS < HighestBitLHS)
    ଫେରନ୍ତୁ ମିଥ୍ୟା;

  constexpr FPClassTest Exact_ମୂଲ୍ୟଗୁଡ଼ିକ_Mask = fcZero | fcInf;
  ଫେରନ୍ତୁ !OrEqual && (LowestBitRHS & Exact_ମୂଲ୍ୟଗୁଡ଼ିକ_Mask) != fcNone;
}

ତାର୍କିକ llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      ତାର୍କିକ OrderedZeroSign) {
  ଫେରନ୍ତୁ cannotOrderStrictlyGreaterImpl(LHS, RHS, ମିଥ୍ୟା, OrderedZeroSign);
}

ତାର୍କିକ llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        ତାର୍କିକ OrderedZeroSign) {
  ଫେରନ୍ତୁ cannotOrderStrictlyGreaterImpl(LHS, RHS, ସତ୍ୟ, OrderedZeroSign);
}

ତାର୍କିକ llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   ତାର୍କିକ OrderedZeroSign) {
  ଫେରନ୍ତୁ cannotOrderStrictlyGreaterImpl(RHS, LHS, ମିଥ୍ୟା, OrderedZeroSign);
}

ତାର୍କିକ llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     ତାର୍କିକ OrderedZeroSign) {
  ଫେରନ୍ତୁ cannotOrderStrictlyGreaterImpl(RHS, LHS, ସତ୍ୟ, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, ତାର୍କିକ OrderedZeroSign) {
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
  ଯଦି (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ଫେରନ୍ତୁ NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         ତାର୍କିକ OrderedZeroSign) {
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
  ଯଦି (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  ଫେରନ୍ତୁ NewMask;
}
