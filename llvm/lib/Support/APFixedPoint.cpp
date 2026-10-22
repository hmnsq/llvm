//===- APFixedPoint.cpp - Fixed point constant handling ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the implementation for the fixed point number interface.
//
//===----------------------------------------------------------------------===//

#ضمن "llvm/ADT/APFixedPoint.h"
#ضمن "llvm/ADT/APFloat.h"

#ضمن <cmath>

نطاق llvm {

فراغ Fixed_ټکی_Semantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << get_پلنوالی() << ", ";
  إذا (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_وزن() << ", ";
  OS << "lsb=" << get_Lsb_وزن() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_ټکی_Semantics::منزل_Opaque_Int() ثابت {
  أرجع llvm::بټ_cast<uint32_t>(*هذا);
}

Fixed_ټکی_Semantics Fixed_ټکی_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_ټکی_Semantics F(0, 0, خطأ, خطأ, خطأ);
  std::حافظه_نقل(&F, &I, حجم(F));
  أرجع F;
}

APFixed_ټکی APFixed_ټکی::convert(ثابت Fixed_ټکی_Semantics &DstSema,
                                   منطقي *ډکېدنه) ثابت {
  APSInt NewVal = Val;
  صحيح RelativeUpscale = get_Lsb_وزن() - DstSema.get_Lsb_وزن();
  إذا (ډکېدنه)
    *ډکېدنه = خطأ;

  إذا (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_بټ_پلنوالی() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  خپلکاری ماسک = APInt::get_بټونه_سېټ_From(
      NewVal.get_بټ_پلنوالی(),
      std::min(DstSema.get_Integral_بټونه() - DstSema.get_Lsb_وزن(),
               NewVal.get_بټ_پلنوالی()));
  APInt Masked(NewVal & ماسک);

  // Change in the bits above the sign
  إذا (!(Masked == ماسک || Masked == 0)) {
    // Found overflow in the bits above the sign
    إذا (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? ماسک : ~ماسک;
    وإلا إذا (ډکېدنه)
      *ډکېدنه = ریښتیا;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  إذا (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    إذا (DstSema.isSaturated())
      NewVal = 0;
    وإلا إذا (ډکېدنه)
      *ډکېدنه = ریښتیا;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_پلنوالی());
  NewVal.سېټ_Is_Signed(DstSema.isSigned());
  أرجع APFixed_ټکی(NewVal, DstSema);
}

صحيح APFixed_ټکی::compare(ثابت APFixed_ټکی &Other) ثابت {
  APSInt ThisVal = get_ارزښت();
  APSInt OtherVal = Other.get_ارزښت();
  منطقي ThisSigned = Val.isSigned();
  منطقي OtherSigned = OtherVal.isSigned();

  صحيح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحيح CommonMsb = std::max(get_Msb_وزن(), Other.get_Msb_وزن());
  بینښه Common_پلنوالی = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_پلنوالی);
  OtherVal = OtherVal.extOrTrunc(Common_پلنوالی);

  ThisVal = ThisVal.shl(get_Lsb_وزن() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_وزن() - CommonLsb);

  إذا (ThisSigned && OtherSigned) {
    إذا (ThisVal.sgt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.slt(OtherVal))
      أرجع -1;
  } وإلا إذا (!ThisSigned && !OtherSigned) {
    إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا إذا (ThisSigned && !OtherSigned) {
    إذا (ThisVal.is_Sign_بټ_سېټ())
      أرجع -1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا {
    // !ThisSigned && OtherSigned
    إذا (OtherVal.is_Sign_بټ_سېټ())
      أرجع 1;
    وإلا إذا (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا إذا (ThisVal.ult(OtherVal))
      أرجع -1;
  }

  أرجع 0;
}

APFixed_ټکی APFixed_ټکی::getMax(ثابت Fixed_ټکی_Semantics &Sema) {
  منطقي IsUnsigned = !Sema.isSigned();
  خپلکاری Val = APSInt::get_Max_ارزښت(Sema.get_پلنوالی(), IsUnsigned);
  إذا (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  أرجع APFixed_ټکی(Val, Sema);
}

APFixed_ټکی APFixed_ټکی::getMin(ثابت Fixed_ټکی_Semantics &Sema) {
  خپلکاری Val = APSInt::get_Min_ارزښت(Sema.get_پلنوالی(), !Sema.isSigned());
  أرجع APFixed_ټکی(Val, Sema);
}

APFixed_ټکی APFixed_ټکی::getEpsilon(ثابت Fixed_ټکی_Semantics &Sema) {
  APSInt Val(Sema.get_پلنوالی(), !Sema.isSigned());
  Val.سېټ_بټ(/*BitPosition=*/0);
  أرجع APFixed_ټکی(Val, Sema);
}

منطقي Fixed_ټکی_Semantics::ځاییږي_دننه_Float_Semantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_ټکی::getMax(*هذا).get_ارزښت();
  APFloat F(FloatSema);
  APFloat::op_حالت حالت = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_تر_ټولو_نږدې_Ties_منزل_Away);
  إذا ((حالت & APFloat::op_ډکېدنه) || !isSigned())
    أرجع !(حالت & APFloat::op_ډکېدنه);

  APSInt MinInt = APFixed_ټکی::getMin(*هذا).get_ارزښت();
  حالت = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_تر_ټولو_نږدې_Ties_منزل_Away);
  أرجع !(حالت & APFloat::op_ډکېدنه);
}

Fixed_ټکی_Semantics Fixed_ټکی_Semantics::getCommonSemantics(
    ثابت Fixed_ټکی_Semantics &Other) ثابت {
  صحيح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحيح CommonMSb = std::max(get_Msb_وزن() - has_Sign_Or_Padding_بټ(),
                           Other.get_Msb_وزن() - Other.has_Sign_Or_Padding_بټ());
  بینښه Common_پلنوالی = CommonMSb - CommonLsb + 1;

  منطقي پایله_Is_Signed = isSigned() || Other.isSigned();
  منطقي پایله_Is_Saturated = isSaturated() || Other.isSaturated();
  منطقي پایله_Has_Unsigned_Padding = خطأ;
  إذا (!پایله_Is_Signed) {
    // Both are unsigned.
    پایله_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !پایله_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  إذا (پایله_Is_Signed || پایله_Has_Unsigned_Padding)
    Common_پلنوالی++;

  أرجع Fixed_ټکی_Semantics(Common_پلنوالی, Lsb{CommonLsb}, پایله_Is_Signed,
                             پایله_Is_Saturated, پایله_Has_Unsigned_Padding);
}

APFixed_ټکی APFixed_ټکی::جمع_کړه(ثابت APFixed_ټکی &Other,
                               منطقي *ډکېدنه) ثابت {
  خپلکاری CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ټکی ConvertedThis = convert(CommonFXSema);
  APFixed_ټکی ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ارزښت();
  APSInt OtherVal = ConvertedOther.get_ارزښت();
  منطقي Overflowed = خطأ;

  APSInt پایله;
  إذا (CommonFXSema.isSaturated()) {
    پایله = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } وإلا {
    پایله = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  إذا (ډکېدنه)
    *ډکېدنه = Overflowed;

  أرجع APFixed_ټکی(پایله, CommonFXSema);
}

APFixed_ټکی APFixed_ټکی::sub(ثابت APFixed_ټکی &Other,
                               منطقي *ډکېدنه) ثابت {
  خپلکاری CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ټکی ConvertedThis = convert(CommonFXSema);
  APFixed_ټکی ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ارزښت();
  APSInt OtherVal = ConvertedOther.get_ارزښت();
  منطقي Overflowed = خطأ;

  APSInt پایله;
  إذا (CommonFXSema.isSaturated()) {
    پایله = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } وإلا {
    پایله = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  إذا (ډکېدنه)
    *ډکېدنه = Overflowed;

  أرجع APFixed_ټکی(پایله, CommonFXSema);
}

APFixed_ټکی APFixed_ټکی::mul(ثابت APFixed_ټکی &Other,
                               منطقي *ډکېدنه) ثابت {
  خپلکاری CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ټکی ConvertedThis = convert(CommonFXSema);
  APFixed_ټکی ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ارزښت();
  APSInt OtherVal = ConvertedOther.get_ارزښت();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  بینښه Wide = CommonFXSema.get_پلنوالی() * 2;
  إذا (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وإلا {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Perform the full multiplication and downscale to get the same scale.
  //
  // Note that the right shifts here perform an implicit downwards rounding.
  // This rounding could discard bits that would technically place the result
  // outside the representable range. We interpret the spec as allowing us to
  // perform the rounding step first, avoiding the overflow case that would
  // arise.
  APSInt پایله;
  إذا (CommonFXSema.isSigned())
    پایله = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_وزن());
  وإلا
    پایله = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_وزن());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  پایله.سېټ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ټکی::getMax(CommonFXSema).get_ارزښت()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_ټکی::getMin(CommonFXSema).get_ارزښت()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (پایله < Min)
      پایله = std::move(Min);
    وإلا إذا (پایله > Max)
      پایله = std::move(Max);
  } وإلا {
    Overflowed = پایله < Min || پایله > Max;
  }

  إذا (ډکېدنه)
    *ډکېدنه = Overflowed;

  أرجع APFixed_ټکی(پایله.sextOrTrunc(CommonFXSema.get_پلنوالی()),
                      CommonFXSema);
}

APFixed_ټکی APFixed_ټکی::div(ثابت APFixed_ټکی &Other,
                               منطقي *ډکېدنه) ثابت {
  خپلکاری CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_ټکی ConvertedThis = convert(CommonFXSema);
  APFixed_ټکی ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ارزښت();
  APSInt OtherVal = ConvertedOther.get_ارزښت();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //ډکېدنه
  بینښه Wide =
      CommonFXSema.get_پلنوالی() * 2 + std::max(-CommonFXSema.get_Msb_وزن(), 0);
  إذا (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وإلا {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  إذا (CommonFXSema.get_Lsb_وزن() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_وزن());
  وإلا إذا (CommonFXSema.get_Lsb_وزن() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_وزن());
  APSInt پایله;
  إذا (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, پایله, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    إذا (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      پایله = پایله - 1;
  } وإلا {
    پایله = ThisVal.udiv(OtherVal);
  }
  پایله.سېټ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ټکی::getMax(CommonFXSema).get_ارزښت()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_ټکی::getMin(CommonFXSema).get_ارزښت()
                                                 .extOrTrunc(Wide);
  إذا (CommonFXSema.isSaturated()) {
    إذا (پایله < Min)
      پایله = std::move(Min);
    وإلا إذا (پایله > Max)
      پایله = std::move(Max);
  } وإلا {
    Overflowed = پایله < Min || پایله > Max;
  }

  إذا (ډکېدنه)
    *ډکېدنه = Overflowed;

  أرجع APFixed_ټکی(پایله.sextOrTrunc(CommonFXSema.get_پلنوالی()),
                      CommonFXSema);
}

APFixed_ټکی APFixed_ټکی::shl(بینښه Amt, منطقي *ډکېدنه) ثابت {
  APSInt ThisVal = Val;
  منطقي Overflowed = خطأ;

  // Widen the LHS.
  بینښه Wide = Sema.get_پلنوالی() * 2;
  إذا (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  وإلا
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_بټ_پلنوالی());
  APSInt پایله = ThisVal << Amt;
  پایله.سېټ_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_ټکی::getMax(Sema).get_ارزښت().extOrTrunc(Wide);
  APSInt Min = APFixed_ټکی::getMin(Sema).get_ارزښت().extOrTrunc(Wide);
  إذا (Sema.isSaturated()) {
    إذا (پایله < Min)
      پایله = std::move(Min);
    وإلا إذا (پایله > Max)
      پایله = std::move(Max);
  } وإلا {
    Overflowed = پایله < Min || پایله > Max;
  }

  إذا (ډکېدنه)
    *ډکېدنه = Overflowed;

  أرجع APFixed_ټکی(پایله.sextOrTrunc(Sema.get_پلنوالی()), Sema);
}

فراغ APFixed_ټکی::منزل_String(Small_ویکټور_Impl<حرف> &Str) ثابت {
  APSInt Val = get_ارزښت();
  صحيح Lsb = get_Lsb_وزن();
  صحيح Orig_پلنوالی = get_پلنوالی();

  إذا (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_بټ_پلنوالی() + Lsb);
    IntPart <<= Lsb;
    IntPart.منزل_String(Str, /*Radix=*/10);
    Str.ټېل_شا('.');
    Str.ټېل_شا('0');
    أرجع;
  }

  إذا (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.سېټ_Is_Unsigned(ریښتیا);
    Str.ټېل_شا('-');
  }

  صحيح Scale = -get_Lsb_وزن();
  APSInt IntPart = (Orig_پلنوالی > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  بینښه پلنوالی = std::max(Orig_پلنوالی, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(پلنوالی);
  APInt Fract_Part_ماسک = APInt::getAllOnes(Scale).zext(پلنوالی);
  APInt RadixInt = APInt(پلنوالی, 10);

  IntPart.منزل_String(Str, /*Radix=*/10);
  Str.ټېل_شا('.');
  افعل {
    (FractPart * RadixInt)
        .lshr(Scale)
        .منزل_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_ماسک;
  } طالما (FractPart != 0);
}

فراغ APFixed_ټکی::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << منزل_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#إذا !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_لاره فراغ APFixed_ټکی::dump() ثابت { print(llvm::errs()); }
#د_که_پای

APFixed_ټکی APFixed_ټکی::negate(منطقي *ډکېدنه) ثابت {
  إذا (!isSaturated()) {
    إذا (ډکېدنه)
      *ډکېدنه =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_ارزښت());
    أرجع APFixed_ټکی(-Val, Sema);
  }

  // We never overflow for saturation
  إذا (ډکېدنه)
    *ډکېدنه = خطأ;

  إذا (isSigned())
    أرجع Val.is_Min_Signed_ارزښت() ? getMax(Sema) : APFixed_ټکی(-Val, Sema);
  وإلا
    أرجع APFixed_ټکی(Sema);
}

APSInt APFixed_ټکی::convert_منزل_Int(بینښه Dst_پلنوالی, منطقي DstSign,
                                  منطقي *ډکېدنه) ثابت {
  APSInt پایله = getIntPart();
  بینښه Src_پلنوالی = get_پلنوالی();

  APSInt DstMin = APSInt::get_Min_ارزښت(Dst_پلنوالی, !DstSign);
  APSInt DstMax = APSInt::get_Max_ارزښت(Dst_پلنوالی, !DstSign);

  إذا (Src_پلنوالی < Dst_پلنوالی) {
    پایله = پایله.extend(Dst_پلنوالی);
  } وإلا إذا (Src_پلنوالی > Dst_پلنوالی) {
    DstMin = DstMin.extend(Src_پلنوالی);
    DstMax = DstMax.extend(Src_پلنوالی);
  }

  إذا (ډکېدنه) {
    إذا (پایله.isSigned() && !DstSign) {
      *ډکېدنه = پایله.isNegative() || پایله.ugt(DstMax);
    } وإلا إذا (پایله.isUnsigned() && DstSign) {
      *ډکېدنه = پایله.ugt(DstMax);
    } وإلا {
      *ډکېدنه = پایله < DstMin || پایله > DstMax;
    }
  }

  پایله.سېټ_Is_Signed(DstSign);
  أرجع پایله.extOrTrunc(Dst_پلنوالی);
}

ثابت fltSemantics *APFixed_ټکی::promoteFloatSemantics(ثابت fltSemantics *S) {
  إذا (S == &APFloat::BFloat())
    أرجع &APFloat::IEEEdouble();
  وإلا إذا (S == &APFloat::IEEEhalf())
    أرجع &APFloat::IEEEsingle();
  وإلا إذا (S == &APFloat::IEEEsingle())
    أرجع &APFloat::IEEEdouble();
  وإلا إذا (S == &APFloat::IEEEdouble())
    أرجع &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixed_ټکی::convert_منزل_Float(ثابت fltSemantics &FloatSema) ثابت {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_ډېرپېښ RM = APFloat::rm_تر_ټولو_نږدې_Ties_منزل_Even;
  APFloat::rounding_ډېرپېښ LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!Sema.ځاییږي_دننه_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_حالت S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (فراغ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_ضریب(std::توان(2, Sema.get_Lsb_وزن()));
  منطقي Ignored;
  Scale_ضریب.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ضریب, LosslessRM);

  إذا (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  أرجع Flt;
}

APFixed_ټکی APFixed_ټکی::get_From_Int_ارزښت(ثابت APSInt &ارزښت,
                                           ثابت Fixed_ټکی_Semantics &DstFXSema,
                                           منطقي *ډکېدنه) {
  Fixed_ټکی_Semantics IntFXSema = Fixed_ټکی_Semantics::GetIntegerSemantics(
      ارزښت.get_بټ_پلنوالی(), ارزښت.isSigned());
  أرجع APFixed_ټکی(ارزښت, IntFXSema).convert(DstFXSema, ډکېدنه);
}

APFixed_ټکی
APFixed_ټکی::get_From_Float_ارزښت(ثابت APFloat &ارزښت,
                                ثابت Fixed_ټکی_Semantics &DstFXSema,
                                منطقي *ډکېدنه) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_ډېرپېښ RM = APFloat::rmTowardZero;
  APFloat::rounding_ډېرپېښ LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = ارزښت.getSemantics();

  إذا (ارزښت.isNaN()) {
    // Handle NaN immediately.
    إذا (ډکېدنه)
      *ډکېدنه = ریښتیا;
    أرجع APFixed_ټکی(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!DstFXSema.ځاییږي_دننه_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = ارزښت;

  منطقي Ignored;
  إذا (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ضریب(std::توان(2, -DstFXSema.get_Lsb_وزن()));
  Scale_ضریب.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ضریب, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_پلنوالی(), !DstFXSema.isSigned());
  Val.convert_منزل_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ضریب = APFloat(std::توان(2, DstFXSema.get_Lsb_وزن()));
  Scale_ضریب.convert(*OpSema, LosslessRM, &Ignored);
  Val.پړاو_منزل_Integral(RM);
  Val.multiply(Scale_ضریب, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_منزل_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_منزل_Float(*OpSema);
  منطقي Overflowed = خطأ;
  إذا (DstFXSema.isSaturated()) {
    إذا (Val > FloatMax)
      Res = getMax(DstFXSema).get_ارزښت();
    وإلا إذا (Val < FloatMin)
      Res = getMin(DstFXSema).get_ارزښت();
  } وإلا {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  إذا (ډکېدنه)
    *ډکېدنه = Overflowed;

  أرجع APFixed_ټکی(Res, DstFXSema);
}

} // namespace llvm
