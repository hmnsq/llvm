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

#شامل "llvm/ADT/APFixedPoint.h"
#شامل "llvm/ADT/APFloat.h"

#شامل <cmath>

نامخلا llvm {

خالی Fixed_نقطہ_Semantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << get_چوڑائی() << ", ";
  اگر (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << get_Msb_وزن() << ", ";
  OS << "lsb=" << get_Lsb_وزن() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t Fixed_نقطہ_Semantics::مقصد_Opaque_Int() ثابت {
  واپس llvm::ثنائی_ہندسہ_cast<uint32_t>(*یہ);
}

Fixed_نقطہ_Semantics Fixed_نقطہ_Semantics::getFromOpaqueInt(uint32_t I) {
  Fixed_نقطہ_Semantics F(0, 0, جھوٹ, جھوٹ, جھوٹ);
  std::حافظہ_نقل(&F, &I, حجم(F));
  واپس F;
}

APFixed_نقطہ APFixed_نقطہ::convert(ثابت Fixed_نقطہ_Semantics &DstSema,
                                   منطقی *حد_سے_تجاوز) ثابت {
  APSInt NewVal = Val;
  صحیح RelativeUpscale = get_Lsb_وزن() - DstSema.get_Lsb_وزن();
  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = جھوٹ;

  اگر (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_ثنائی_ہندسہ_چوڑائی() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  خودکار نقاب = APInt::get_بٹ_سیٹ_From(
      NewVal.get_ثنائی_ہندسہ_چوڑائی(),
      std::min(DstSema.get_Integral_بٹ() - DstSema.get_Lsb_وزن(),
               NewVal.get_ثنائی_ہندسہ_چوڑائی()));
  APInt Masked(NewVal & نقاب);

  // Change in the bits above the sign
  اگر (!(Masked == نقاب || Masked == 0)) {
    // Found overflow in the bits above the sign
    اگر (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? نقاب : ~نقاب;
    ورنہ اگر (حد_سے_تجاوز)
      *حد_سے_تجاوز = سچ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  اگر (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    اگر (DstSema.isSaturated())
      NewVal = 0;
    ورنہ اگر (حد_سے_تجاوز)
      *حد_سے_تجاوز = سچ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_چوڑائی());
  NewVal.سیٹ_Is_Signed(DstSema.isSigned());
  واپس APFixed_نقطہ(NewVal, DstSema);
}

صحیح APFixed_نقطہ::compare(ثابت APFixed_نقطہ &Other) ثابت {
  APSInt ThisVal = get_قدر();
  APSInt OtherVal = Other.get_قدر();
  منطقی ThisSigned = Val.isSigned();
  منطقی OtherSigned = OtherVal.isSigned();

  صحیح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحیح CommonMsb = std::max(get_Msb_وزن(), Other.get_Msb_وزن());
  غیرعلامتی Common_چوڑائی = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_چوڑائی);
  OtherVal = OtherVal.extOrTrunc(Common_چوڑائی);

  ThisVal = ThisVal.shl(get_Lsb_وزن() - CommonLsb);
  OtherVal = OtherVal.shl(Other.get_Lsb_وزن() - CommonLsb);

  اگر (ThisSigned && OtherSigned) {
    اگر (ThisVal.sgt(OtherVal))
      واپس 1;
    ورنہ اگر (ThisVal.slt(OtherVal))
      واپس -1;
  } ورنہ اگر (!ThisSigned && !OtherSigned) {
    اگر (ThisVal.ugt(OtherVal))
      واپس 1;
    ورنہ اگر (ThisVal.ult(OtherVal))
      واپس -1;
  } ورنہ اگر (ThisSigned && !OtherSigned) {
    اگر (ThisVal.is_Sign_ثنائی_ہندسہ_سیٹ())
      واپس -1;
    ورنہ اگر (ThisVal.ugt(OtherVal))
      واپس 1;
    ورنہ اگر (ThisVal.ult(OtherVal))
      واپس -1;
  } ورنہ {
    // !ThisSigned && OtherSigned
    اگر (OtherVal.is_Sign_ثنائی_ہندسہ_سیٹ())
      واپس 1;
    ورنہ اگر (ThisVal.ugt(OtherVal))
      واپس 1;
    ورنہ اگر (ThisVal.ult(OtherVal))
      واپس -1;
  }

  واپس 0;
}

APFixed_نقطہ APFixed_نقطہ::getMax(ثابت Fixed_نقطہ_Semantics &Sema) {
  منطقی IsUnsigned = !Sema.isSigned();
  خودکار Val = APSInt::get_Max_قدر(Sema.get_چوڑائی(), IsUnsigned);
  اگر (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  واپس APFixed_نقطہ(Val, Sema);
}

APFixed_نقطہ APFixed_نقطہ::getMin(ثابت Fixed_نقطہ_Semantics &Sema) {
  خودکار Val = APSInt::get_Min_قدر(Sema.get_چوڑائی(), !Sema.isSigned());
  واپس APFixed_نقطہ(Val, Sema);
}

APFixed_نقطہ APFixed_نقطہ::getEpsilon(ثابت Fixed_نقطہ_Semantics &Sema) {
  APSInt Val(Sema.get_چوڑائی(), !Sema.isSigned());
  Val.سیٹ_ثنائی_ہندسہ(/*BitPosition=*/0);
  واپس APFixed_نقطہ(Val, Sema);
}

منطقی Fixed_نقطہ_Semantics::سماتا_اندر_Float_Semantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixed_نقطہ::getMax(*یہ).get_قدر();
  APFloat F(FloatSema);
  APFloat::op_حالت حالت_2 = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_قریب_ترین_Ties_مقصد_Away);
  اگر ((حالت_2 & APFloat::op_حد_سے_تجاوز) || !isSigned())
    واپس !(حالت_2 & APFloat::op_حد_سے_تجاوز);

  APSInt MinInt = APFixed_نقطہ::getMin(*یہ).get_قدر();
  حالت_2 = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_قریب_ترین_Ties_مقصد_Away);
  واپس !(حالت_2 & APFloat::op_حد_سے_تجاوز);
}

Fixed_نقطہ_Semantics Fixed_نقطہ_Semantics::getCommonSemantics(
    ثابت Fixed_نقطہ_Semantics &Other) ثابت {
  صحیح CommonLsb = std::min(get_Lsb_وزن(), Other.get_Lsb_وزن());
  صحیح CommonMSb = std::max(get_Msb_وزن() - has_Sign_Or_Padding_ثنائی_ہندسہ(),
                           Other.get_Msb_وزن() - Other.has_Sign_Or_Padding_ثنائی_ہندسہ());
  غیرعلامتی Common_چوڑائی = CommonMSb - CommonLsb + 1;

  منطقی نتیجہ_Is_Signed = isSigned() || Other.isSigned();
  منطقی نتیجہ_Is_Saturated = isSaturated() || Other.isSaturated();
  منطقی نتیجہ_Has_Unsigned_Padding = جھوٹ;
  اگر (!نتیجہ_Is_Signed) {
    // Both are unsigned.
    نتیجہ_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !نتیجہ_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  اگر (نتیجہ_Is_Signed || نتیجہ_Has_Unsigned_Padding)
    Common_چوڑائی++;

  واپس Fixed_نقطہ_Semantics(Common_چوڑائی, Lsb{CommonLsb}, نتیجہ_Is_Signed,
                             نتیجہ_Is_Saturated, نتیجہ_Has_Unsigned_Padding);
}

APFixed_نقطہ APFixed_نقطہ::جوڑو(ثابت APFixed_نقطہ &Other,
                               منطقی *حد_سے_تجاوز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقی Overflowed = جھوٹ;

  APSInt نتیجہ;
  اگر (CommonFXSema.isSaturated()) {
    نتیجہ = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ورنہ {
    نتیجہ = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = Overflowed;

  واپس APFixed_نقطہ(نتیجہ, CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::sub(ثابت APFixed_نقطہ &Other,
                               منطقی *حد_سے_تجاوز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقی Overflowed = جھوٹ;

  APSInt نتیجہ;
  اگر (CommonFXSema.isSaturated()) {
    نتیجہ = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ورنہ {
    نتیجہ = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = Overflowed;

  واپس APFixed_نقطہ(نتیجہ, CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::mul(ثابت APFixed_نقطہ &Other,
                               منطقی *حد_سے_تجاوز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقی Overflowed = جھوٹ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  غیرعلامتی Wide = CommonFXSema.get_چوڑائی() * 2;
  اگر (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ورنہ {
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
  APSInt نتیجہ;
  اگر (CommonFXSema.isSigned())
    نتیجہ = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.get_Lsb_وزن());
  ورنہ
    نتیجہ = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.get_Lsb_وزن());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  نتیجہ.سیٹ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطہ::getMax(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطہ::getMin(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  اگر (CommonFXSema.isSaturated()) {
    اگر (نتیجہ < Min)
      نتیجہ = std::move(Min);
    ورنہ اگر (نتیجہ > Max)
      نتیجہ = std::move(Max);
  } ورنہ {
    Overflowed = نتیجہ < Min || نتیجہ > Max;
  }

  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = Overflowed;

  واپس APFixed_نقطہ(نتیجہ.sextOrTrunc(CommonFXSema.get_چوڑائی()),
                      CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::div(ثابت APFixed_نقطہ &Other,
                               منطقی *حد_سے_تجاوز) ثابت {
  خودکار CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixed_نقطہ ConvertedThis = convert(CommonFXSema);
  APFixed_نقطہ ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_قدر();
  APSInt OtherVal = ConvertedOther.get_قدر();
  منطقی Overflowed = جھوٹ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //حد_سے_تجاوز
  غیرعلامتی Wide =
      CommonFXSema.get_چوڑائی() * 2 + std::max(-CommonFXSema.get_Msb_وزن(), 0);
  اگر (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ورنہ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  اگر (CommonFXSema.get_Lsb_وزن() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.get_Lsb_وزن());
  ورنہ اگر (CommonFXSema.get_Lsb_وزن() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.get_Lsb_وزن());
  APSInt نتیجہ;
  اگر (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, نتیجہ, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    اگر (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      نتیجہ = نتیجہ - 1;
  } ورنہ {
    نتیجہ = ThisVal.udiv(OtherVal);
  }
  نتیجہ.سیٹ_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطہ::getMax(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixed_نقطہ::getMin(CommonFXSema).get_قدر()
                                                 .extOrTrunc(Wide);
  اگر (CommonFXSema.isSaturated()) {
    اگر (نتیجہ < Min)
      نتیجہ = std::move(Min);
    ورنہ اگر (نتیجہ > Max)
      نتیجہ = std::move(Max);
  } ورنہ {
    Overflowed = نتیجہ < Min || نتیجہ > Max;
  }

  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = Overflowed;

  واپس APFixed_نقطہ(نتیجہ.sextOrTrunc(CommonFXSema.get_چوڑائی()),
                      CommonFXSema);
}

APFixed_نقطہ APFixed_نقطہ::shl(غیرعلامتی Amt, منطقی *حد_سے_تجاوز) ثابت {
  APSInt ThisVal = Val;
  منطقی Overflowed = جھوٹ;

  // Widen the LHS.
  غیرعلامتی Wide = Sema.get_چوڑائی() * 2;
  اگر (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ورنہ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_ثنائی_ہندسہ_چوڑائی());
  APSInt نتیجہ = ThisVal << Amt;
  نتیجہ.سیٹ_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixed_نقطہ::getMax(Sema).get_قدر().extOrTrunc(Wide);
  APSInt Min = APFixed_نقطہ::getMin(Sema).get_قدر().extOrTrunc(Wide);
  اگر (Sema.isSaturated()) {
    اگر (نتیجہ < Min)
      نتیجہ = std::move(Min);
    ورنہ اگر (نتیجہ > Max)
      نتیجہ = std::move(Max);
  } ورنہ {
    Overflowed = نتیجہ < Min || نتیجہ > Max;
  }

  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = Overflowed;

  واپس APFixed_نقطہ(نتیجہ.sextOrTrunc(Sema.get_چوڑائی()), Sema);
}

خالی APFixed_نقطہ::مقصد_String(Small_سمتیہ_Impl<حرف> &Str) ثابت {
  APSInt Val = get_قدر();
  صحیح Lsb = get_Lsb_وزن();
  صحیح Orig_چوڑائی = get_چوڑائی();

  اگر (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_ثنائی_ہندسہ_چوڑائی() + Lsb);
    IntPart <<= Lsb;
    IntPart.مقصد_String(Str, /*Radix=*/10);
    Str.ڈالو_عقب('.');
    Str.ڈالو_عقب('0');
    واپس;
  }

  اگر (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.سیٹ_Is_Unsigned(سچ);
    Str.ڈالو_عقب('-');
  }

  صحیح Scale = -get_Lsb_وزن();
  APSInt IntPart = (Orig_چوڑائی > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  غیرعلامتی چوڑائی = std::max(Orig_چوڑائی, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(چوڑائی);
  APInt Fract_Part_نقاب = APInt::getAllOnes(Scale).zext(چوڑائی);
  APInt RadixInt = APInt(چوڑائی, 10);

  IntPart.مقصد_String(Str, /*Radix=*/10);
  Str.ڈالو_عقب('.');
  کرو {
    (FractPart * RadixInt)
        .lshr(Scale)
        .مقصد_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_نقاب;
  } جب (FractPart != 0);
}

خالی APFixed_نقطہ::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << مقصد_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_طریقہ خالی APFixed_نقطہ::dump() ثابت { print(llvm::errs()); }
#اگر_ختم

APFixed_نقطہ APFixed_نقطہ::negate(منطقی *حد_سے_تجاوز) ثابت {
  اگر (!isSaturated()) {
    اگر (حد_سے_تجاوز)
      *حد_سے_تجاوز =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_قدر());
    واپس APFixed_نقطہ(-Val, Sema);
  }

  // We never overflow for saturation
  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = جھوٹ;

  اگر (isSigned())
    واپس Val.is_Min_Signed_قدر() ? getMax(Sema) : APFixed_نقطہ(-Val, Sema);
  ورنہ
    واپس APFixed_نقطہ(Sema);
}

APSInt APFixed_نقطہ::convert_مقصد_Int(غیرعلامتی Dst_چوڑائی, منطقی DstSign,
                                  منطقی *حد_سے_تجاوز) ثابت {
  APSInt نتیجہ = getIntPart();
  غیرعلامتی Src_چوڑائی = get_چوڑائی();

  APSInt DstMin = APSInt::get_Min_قدر(Dst_چوڑائی, !DstSign);
  APSInt DstMax = APSInt::get_Max_قدر(Dst_چوڑائی, !DstSign);

  اگر (Src_چوڑائی < Dst_چوڑائی) {
    نتیجہ = نتیجہ.extend(Dst_چوڑائی);
  } ورنہ اگر (Src_چوڑائی > Dst_چوڑائی) {
    DstMin = DstMin.extend(Src_چوڑائی);
    DstMax = DstMax.extend(Src_چوڑائی);
  }

  اگر (حد_سے_تجاوز) {
    اگر (نتیجہ.isSigned() && !DstSign) {
      *حد_سے_تجاوز = نتیجہ.isNegative() || نتیجہ.ugt(DstMax);
    } ورنہ اگر (نتیجہ.isUnsigned() && DstSign) {
      *حد_سے_تجاوز = نتیجہ.ugt(DstMax);
    } ورنہ {
      *حد_سے_تجاوز = نتیجہ < DstMin || نتیجہ > DstMax;
    }
  }

  نتیجہ.سیٹ_Is_Signed(DstSign);
  واپس نتیجہ.extOrTrunc(Dst_چوڑائی);
}

ثابت fltSemantics *APFixed_نقطہ::promoteFloatSemantics(ثابت fltSemantics *S) {
  اگر (S == &APFloat::BFloat())
    واپس &APFloat::IEEEdouble();
  ورنہ اگر (S == &APFloat::IEEEhalf())
    واپس &APFloat::IEEEsingle();
  ورنہ اگر (S == &APFloat::IEEEsingle())
    واپس &APFloat::IEEEdouble();
  ورنہ اگر (S == &APFloat::IEEEdouble())
    واپس &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixed_نقطہ::convert_مقصد_Float(ثابت fltSemantics &FloatSema) ثابت {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_منوال RM = APFloat::rm_قریب_ترین_Ties_مقصد_Even;
  APFloat::rounding_منوال LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  جب (!Sema.سماتا_اندر_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_حالت S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (خالی)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_ضربی_عدد(std::قوت(2, Sema.get_Lsb_وزن()));
  منطقی Ignored;
  Scale_ضربی_عدد.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_ضربی_عدد, LosslessRM);

  اگر (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  واپس Flt;
}

APFixed_نقطہ APFixed_نقطہ::get_From_Int_قدر(ثابت APSInt &قدر,
                                           ثابت Fixed_نقطہ_Semantics &DstFXSema,
                                           منطقی *حد_سے_تجاوز) {
  Fixed_نقطہ_Semantics IntFXSema = Fixed_نقطہ_Semantics::GetIntegerSemantics(
      قدر.get_ثنائی_ہندسہ_چوڑائی(), قدر.isSigned());
  واپس APFixed_نقطہ(قدر, IntFXSema).convert(DstFXSema, حد_سے_تجاوز);
}

APFixed_نقطہ
APFixed_نقطہ::get_From_Float_قدر(ثابت APFloat &قدر,
                                ثابت Fixed_نقطہ_Semantics &DstFXSema,
                                منطقی *حد_سے_تجاوز) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_منوال RM = APFloat::rmTowardZero;
  APFloat::rounding_منوال LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = قدر.getSemantics();

  اگر (قدر.isNaN()) {
    // Handle NaN immediately.
    اگر (حد_سے_تجاوز)
      *حد_سے_تجاوز = سچ;
    واپس APFixed_نقطہ(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  جب (!DstFXSema.سماتا_اندر_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = قدر;

  منطقی Ignored;
  اگر (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_ضربی_عدد(std::قوت(2, -DstFXSema.get_Lsb_وزن()));
  Scale_ضربی_عدد.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_ضربی_عدد, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_چوڑائی(), !DstFXSema.isSigned());
  Val.convert_مقصد_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_ضربی_عدد = APFloat(std::قوت(2, DstFXSema.get_Lsb_وزن()));
  Scale_ضربی_عدد.convert(*OpSema, LosslessRM, &Ignored);
  Val.باری_مقصد_Integral(RM);
  Val.multiply(Scale_ضربی_عدد, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_مقصد_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_مقصد_Float(*OpSema);
  منطقی Overflowed = جھوٹ;
  اگر (DstFXSema.isSaturated()) {
    اگر (Val > FloatMax)
      Res = getMax(DstFXSema).get_قدر();
    ورنہ اگر (Val < FloatMin)
      Res = getMin(DstFXSema).get_قدر();
  } ورنہ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  اگر (حد_سے_تجاوز)
    *حد_سے_تجاوز = Overflowed;

  واپس APFixed_نقطہ(Res, DstFXSema);
}

} // namespace llvm
