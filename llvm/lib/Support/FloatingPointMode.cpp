//===- FloatingPointMode.cpp ------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/FloatingPointMode.h"
#ضمن "llvm/ADT/StringExtras.h"

استفاده نطاق llvm;

FPClassTest llvm::fneg(FPClassTest ماسک) {
  FPClassTest New_ماسک = ماسک & fcNan;
  إذا (ماسک & fcNegInf)
    New_ماسک |= fcPosInf;
  إذا (ماسک & fc_Neg_نرمال)
    New_ماسک |= fc_Pos_نرمال;
  إذا (ماسک & fcNegSubnormal)
    New_ماسک |= fcPosSubnormal;
  إذا (ماسک & fcNegZero)
    New_ماسک |= fcPosZero;
  إذا (ماسک & fcPosZero)
    New_ماسک |= fcNegZero;
  إذا (ماسک & fcPosSubnormal)
    New_ماسک |= fcNegSubnormal;
  إذا (ماسک & fc_Pos_نرمال)
    New_ماسک |= fc_Neg_نرمال;
  إذا (ماسک & fcPosInf)
    New_ماسک |= fcNegInf;
  أرجع New_ماسک;
}

FPClassTest llvm::inverse_fabs(FPClassTest ماسک) {
  FPClassTest New_ماسک = ماسک & fcNan;
  إذا (ماسک & fcPosZero)
    New_ماسک |= fcZero;
  إذا (ماسک & fcPosSubnormal)
    New_ماسک |= fcSubnormal;
  إذا (ماسک & fc_Pos_نرمال)
    New_ماسک |= fc_نرمال;
  إذا (ماسک & fcPosInf)
    New_ماسک |= fcInf;
  أرجع New_ماسک;
}

FPClassTest llvm::unknown_sign(FPClassTest ماسک) {
  FPClassTest New_ماسک = ماسک & fcNan;
  إذا (ماسک & fcZero)
    New_ماسک |= fcZero;
  إذا (ماسک & fcSubnormal)
    New_ماسک |= fcSubnormal;
  إذا (ماسک & fc_نرمال)
    New_ماسک |= fc_نرمال;
  إذا (ماسک & fcInf)
    New_ماسک |= fcInf;
  أرجع New_ماسک;
}

// Every bitfield has a unique name and one or more aliasing names that cover
// multiple bits. Names should be listed in order of preference, with higher
// popcounts listed first.
//
// Bits are consumed as printed. Each field should only be represented in one
// printed field.
ساكن constexpr std::pair<FPClassTest, StringLiteral> NoFPClassName[] = {
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
  {fc_نرمال, "norm"},
  {fc_Neg_نرمال, "nnorm"},
  {fc_Pos_نرمال, "pnorm"}
};

raw_ostream &llvm::عملگر<<(raw_ostream &OS, FPClassTest ماسک) {
  OS << '(';

  إذا (ماسک == fcNone) {
    OS << "none)";
    أرجع OS;
  }

  ListSeparator LS(" ");
  لكل (خودکار [بیت_Test, Name] : NoFPClassName) {
    إذا ((ماسک & بیت_Test) == بیت_Test) {
      OS << LS << Name;

      // Clear the bits so we don't print any aliased names later.
      ماسک &= ~بیت_Test;
    }
  }

  أكد(ماسک == 0 && "didn't print some mask bits");

  OS << ')';
  أرجع OS;
}

فراغ DenormalFPEnv::print(raw_ostream &OS, منطقي OmitIfSame) ثابت {
  إذا (F32_نما == Default_نما) {
    Default_نما.print(OS, /*Legacy=*/خطأ, OmitIfSame);
    أرجع;
  }

  // Omit printing the base mode if only the f32 mode isn't the default.
  إذا (Default_نما != Denormal_نما::getDefault()) {
    Default_نما.print(OS, /*Legacy=*/خطأ, OmitIfSame);
    OS << ", ";
  }

  OS << "float: ";
  F32_نما.print(OS, /*Legacy=*/خطأ, OmitIfSame);
}

ساكن منطقي cannot_ترتیب_Strictly_Greater_Impl(FPClassTest LHS, FPClassTest RHS,
                                           منطقي OrEqual, منطقي OrderedZero) {
  LHS &= ~fcNan;
  RHS &= ~fcNan;

  إذا (LHS == fcNone || RHS == fcNone)
    أرجع درست;

  FPClassTest پایینترین_بیت_RHS = static_cast<FPClassTest>(RHS & -RHS);
  FPClassTest Highest_بیت_LHS = static_cast<FPClassTest>(1 << Log2_32(LHS));

  إذا (!OrderedZero) {
    // Introduce conflict in zero bits if we're treating them as equal.
    إذا (پایینترین_بیت_RHS == fcNegZero)
      پایینترین_بیت_RHS = fcPosZero;
    إذا (Highest_بیت_LHS == fcNegZero)
      Highest_بیت_LHS = fcPosZero;
  }

  إذا (پایینترین_بیت_RHS > Highest_بیت_LHS) {
    أكد((LHS & RHS) == fcNone && "no bits should intersect");
    أرجع درست;
  }

  إذا (پایینترین_بیت_RHS < Highest_بیت_LHS)
    أرجع خطأ;

  constexpr FPClassTest Exact_مقادیر_ماسک = fcZero | fcInf;
  أرجع !OrEqual && (پایینترین_بیت_RHS & Exact_مقادیر_ماسک) != fcNone;
}

منطقي llvm::cannot_ترتیب_Strictly_Greater(FPClassTest LHS, FPClassTest RHS,
                                      منطقي OrderedZeroSign) {
  أرجع cannot_ترتیب_Strictly_Greater_Impl(LHS, RHS, خطأ, OrderedZeroSign);
}

منطقي llvm::cannot_ترتیب_Strictly_Greater_Eq(FPClassTest LHS, FPClassTest RHS,
                                        منطقي OrderedZeroSign) {
  أرجع cannot_ترتیب_Strictly_Greater_Impl(LHS, RHS, درست, OrderedZeroSign);
}

منطقي llvm::cannot_ترتیب_Strictly_Less(FPClassTest LHS, FPClassTest RHS,
                                   منطقي OrderedZeroSign) {
  أرجع cannot_ترتیب_Strictly_Greater_Impl(RHS, LHS, خطأ, OrderedZeroSign);
}

منطقي llvm::cannot_ترتیب_Strictly_Less_Eq(FPClassTest LHS, FPClassTest RHS,
                                     منطقي OrderedZeroSign) {
  أرجع cannot_ترتیب_Strictly_Greater_Impl(RHS, LHS, درست, OrderedZeroSign);
}

FPClassTest llvm::orderedStrictlyLess(FPClassTest ماسک, منطقي OrderedZeroSign) {
  // Ignores NaN
  ماسک &= ~fcNan;
  // Since the classes are ordered bits, we can get all classes which are
  // smaller than all classes in Known by setting all trailing 0 bits to 1,
  // and setting the other bits to 0.
  // This is done by counting the number of trailing 0 bits and creating a
  // mask based on that.
  // For example: 0b1111000000 = fcPositive
  //           -> 0b0000111111 = fcNegative | fcNan
  FPClassTest New_ماسک = static_cast<FPClassTest>(
      ماسک_Trailing_Ones<بیعلامت>(countr_zero<بیعلامت>(ماسک)) & fcAllFlags);
  // Remove NaNs
  New_ماسک &= ~fcNan;
  // We cannot conclude that only one zero is smaller
  // if the zeroes are not ordered
  إذا (!OrderedZeroSign && ((New_ماسک & fcZero) != fcZero))
    New_ماسک &= ~fcZero;
  أرجع New_ماسک;
}

FPClassTest llvm::orderedStrictlyGreater(FPClassTest ماسک,
                                         منطقي OrderedZeroSign) {
  // Counting the number of leading 0 bits and create a mask based on that,
  // removing the leading 1s that are outside the bitfield:
  // For example: 0b0000111100 = fcNegative
  //           -> 0b1111000000 = fcPositive
  FPClassTest New_ماسک = static_cast<FPClassTest>(
      ماسک_Leading_Ones<بیعلامت>(countl_zero<بیعلامت>(ماسک)) & fcAllFlags);
  // Remove NaNs
  New_ماسک &= ~fcNan;
  // We cannot conclude that only one zero is greater
  // if the zeroes are not ordered
  إذا (!OrderedZeroSign && ((New_ماسک & fcZero) != fcZero))
    New_ماسک &= ~fcZero;
  أرجع New_ماسک;
}
