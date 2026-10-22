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
  जर (Mask & fcNegInf)
    NewMask |= fcPosInf;
  जर (Mask & fcNegNormal)
    NewMask |= fcPosNormal;
  जर (Mask & fcNegSubnormal)
    NewMask |= fcPosSubnormal;
  जर (Mask & fcNegZero)
    NewMask |= fcPosZero;
  जर (Mask & fcPosZero)
    NewMask |= fcNegZero;
  जर (Mask & fcPosSubnormal)
    NewMask |= fcNegSubnormal;
  जर (Mask & fcPosNormal)
    NewMask |= fcNegNormal;
  जर (Mask & fcPosInf)
    NewMask |= fcNegInf;
  परत NewMask;
}

FPClassTest llvm::inverse_fabs(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  जर (Mask & fcPosZero)
    NewMask |= fcZero;
  जर (Mask & fcPosSubnormal)
    NewMask |= fcSubnormal;
  जर (Mask & fcPosNormal)
    NewMask |= fcNormal;
  जर (Mask & fcPosInf)
    NewMask |= fcInf;
  परत NewMask;
}

FPClassTest llvm::unknown_sign(FPClassTest Mask) {
  FPClassTest NewMask = Mask & fcNan;
  जर (Mask & fcZero)
    NewMask |= fcZero;
  जर (Mask & fcSubnormal)
    NewMask |= fcSubnormal;
  जर (Mask & fcNormal)
    NewMask |= fcNormal;
  जर (Mask & fcInf)
    NewMask |= fcInf;
  परत NewMask;
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

raw_ostream &llvm::चालक<<(raw_ostream &OS, FPClassTest Mask) {
  OS << '(';

  जर (Mask == fcNone) {
    OS << "none)";
    परत OS;
  }

  ListSeparator LS(" ");
  साठी (स्वतः [BitTest, Name] : NoFPClassName) {
    जर ((Mask & BitTest) == BitTest) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      Mask &= ~BitTest;
    }
  }

  प्रतिज्ञा(Mask == 0 && "didn't print some mask bits");

  OS << ')';
  परत OS;
}

शून्य DenormalFPEnv::print(raw_ostream &OS, तार्किक OmitIfSame) स्थिरांक {
  जर (F32Mode == DefaultMode) {
    DefaultMode.print(OS, /*Legacy=*/असत्य, OmitIfSame);
    परत;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  जर (DefaultMode != DenormalMode::getDefault()) {
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

  जर (LHS == fcNone || RHS == fcNone)
    परत सत्य;

  FPClassTest LowestBitRHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest HighestBitLHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  जर (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    जर (LowestBitRHS == fcNegZero)
      LowestBitRHS = fcPosZero;
    जर (HighestBitLHS == fcNegZero)
      HighestBitLHS = fcPosZero;
  }

  जर (LowestBitRHS > HighestBitLHS) {
    प्रतिज्ञा((LHS & RHS) == fcNone && "no bits should intersect");
    परत सत्य;
  }

  जर (LowestBitRHS < HighestBitLHS)
    परत असत्य;

  constexpr FPClassTest Exact_मूल्ये_Mask = fcZero | fcInf;
  परत !OrEqual && (LowestBitRHS & Exact_मूल्ये_Mask) != fcNone;
}

तार्किक llvm::cannotOrderStrictlyGreater(FPClassTest LHS, FPClassTest RHS,
                                      तार्किक OrderedZeroSign) {
  परत cannotOrderStrictlyGreaterImpl(LHS, RHS, असत्य, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyGreaterEq(FPClassTest LHS, FPClassTest RHS,
                                        तार्किक OrderedZeroSign) {
  परत cannotOrderStrictlyGreaterImpl(LHS, RHS, सत्य, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyLess(FPClassTest LHS, FPClassTest RHS,
                                   तार्किक OrderedZeroSign) {
  परत cannotOrderStrictlyGreaterImpl(RHS, LHS, असत्य, OrderedZeroSign);
}

तार्किक llvm::cannotOrderStrictlyLessEq(FPClassTest LHS, FPClassTest RHS,
                                     तार्किक OrderedZeroSign) {
  परत cannotOrderStrictlyGreaterImpl(RHS, LHS, सत्य, OrderedZeroSign);
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
      maskTrailingOnes<अचिन्हित>(countr_zero<अचिन्हित>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is smaller
  // if the zeroes are not ordered
  जर (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  परत NewMask;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest Mask,
                                         तार्किक OrderedZeroSign) {
  // Counting the number of leading 0 bits and create a mask based on that,
  // removing the leading 1s that are outside the bitfield:
  // For example: 0b0000111100 = fcNegative
  //           -> 0b1111000000 = fcPositive
  FPClassTest NewMask = static_cast<FPClassTest>(
      maskLeadingOnes<अचिन्हित>(countl_zero<अचिन्हित>(Mask)) & fcAllFlags);
  // Remove NaNs
  NewMask &= ~fcNan;
  // We cannot conclude that only one zero is greater
  // if the zeroes are not ordered
  जर (!OrderedZeroSign && ((NewMask & fcZero) != fcZero))
    NewMask &= ~fcZero;
  परत NewMask;
}
