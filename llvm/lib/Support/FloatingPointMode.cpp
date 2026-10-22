//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#अन्तर्भाव "llvm/ADT/FloatingPointMode.h"
#अन्तर्भाव "llvm/ADT/StringExtras.h"

using नामाकाश llvm;

FPClassTest llvm::fneg(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  यदि (Mask & fcNegInf)
    NewMask |= fcPosInf;
  यदि (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  यदि (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  यदि (Mask & fcNegZero)
    NewMask |= fcPosZero;
  यदि (Mask & fcPosZero)
    NewMask |= fcNegZero;
  यदि (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  यदि (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  यदि (Mask & fcPosInf)
    NewMask |= fcNegInf;
  प्रत्यागम NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  यदि (Mask & fcPosZero)
    NewMask |= fcZero;
  यदि (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  यदि (Mask & fcPosNormal)
    NewMask |= fcNormal;
  यदि (Mask & fcPosInf)
    NewMask |= fcInf;
  प्रत्यागम NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  यदि (Mask & fcZero)
    NewMask |= fcZero;
  यदि (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  यदि (Mask & fcNormal)
    NewMask |= fcNormal;
  यदि (Mask & fcInf)
    NewMask |= fcInf;
  प्रत्यागम NewMask;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
स्थिर constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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

  यदि (Mask == fcNone) {
    OS << "none)";
    प्रत्यागम OS;
  }

  ListSeparator LS(" ");
  प्रति (auto [BitTest, Name] : NoFPClassName) {
    यदि ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  प्रतिज्ञा(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  प्रत्यागम OS;
}

शून्य DenormalFPEnv::print(raw_ostream &OS, तार्किक OmitIfSame) ध्रुव {
  यदि (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/असत्य, OmitIfSame);
    प्रत्यागम;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  यदि (DefaultMode != DenormalMode::getDefault()) {
    DefaultMode.print(OS, /*Legacy=*/असत्य, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32Mode.print(OS, /*Legacy=*/असत्य, OmitIfSame);
}

स्थिर तार्किक cannotOrderStrictlyGreaterImpl(FPClassTest LHS, FPClassTest RHS,
                                           तार्किक OrEqual, तार्किक OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  यदि (LHS == fcNone || RHS == fcNone)
    प्रत्यागम सत्य;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  यदि (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    यदि (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    यदि (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  यदि (LowestBitRHS > HighestBitLHS) {
    प्रतिज्ञा((LHS & RHS) == fcNone && "no bits should intersect");
    प्रत्यागम सत्य;
  }

  यदि (LowestBitRHS < HighestBitLHS)
    प्रत्यागम असत्य;

  constexpr FPClassTest Exact_मूल्ये_Mask = fcZero | fcInf;
  प्रत्यागम !OrEqual && (LowestBitRHS & Exact_मूल्ये_Mask) != fcNone;
}

तार्किक llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      तार्किक OrderedZeroSign) {
  प्रत्यागम cannotOrderStrictlyGreaterImpl(LHS, RHS, असत्य, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        तार्किक OrderedZeroSign) {
  प्रत्यागम cannotOrderStrictlyGreaterImpl(LHS, RHS, सत्य, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   तार्किक OrderedZeroSign) {
  प्रत्यागम cannotOrderStrictlyGreaterImpl(RHS, LHS, असत्य, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     तार्किक OrderedZeroSign) {
  प्रत्यागम cannotOrderStrictlyGreaterImpl(RHS, LHS, सत्य, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest Mask, तार्किक OrderedZeroSign) {
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
  यदि (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  प्रत्यागम NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         तार्किक OrderedZeroSign) {
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
  यदि (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  प्रत्यागम NewMask;
}
