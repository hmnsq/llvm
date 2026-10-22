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

#अन्तर्भाव "llvm/ADT/APFixedPoint.h"
#अन्तर्भाव "llvm/ADT/APFloat.h"

#अन्तर्भाव <cmath>

नामाकाश llvm {

शून्य FixedPointSemantics::print(llvm::raw_ostream &OS) ध्रुव {
  OS << "width=" << get_विस्तारः() << ", ";
  यदि (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::गन्तव्यम्_Opaque_Int() ध्रुव {
  प्रत्यागम llvm::bit_cast<uint32_t>(*एतत्);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, असत्य, असत्य, असत्य);
  std::स्मृतिं_प्रतिलिख(&F, &I, परिमाण(F));
  प्रत्यागम F;
}

APFixedPoint APFixedPoint::convert(ध्रुव FixedPointSemantics &DstSema,
                                   तार्किक *अतिप्रवाहः) ध्रुव {
  APSInt NewVal = Val;
  पूर्णाङ्क RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = असत्य;

  यदि (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.get_Bit_विस्तारः() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  स्वचालित आवरणम् = APInt::get_द्वयङ्काः_समुच्चयः_From(
      NewVal.get_Bit_विस्तारः(),
      std::min(DstSema.get_Integral_द्वयङ्काः() - DstSema.getLsbWeight(),
               NewVal.get_Bit_विस्तारः()));
  APInt Masked(NewVal & आवरणम्);

  // Change in the bits above the sign
  यदि (!(Masked == आवरणम् || Masked == 0)) {
    // Found overflow in the bits above the sign
    यदि (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? आवरणम् : ~आवरणम्;
    अन्यथा यदि (अतिप्रवाहः)
      *अतिप्रवाहः = सत्य;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  यदि (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    यदि (DstSema.isSaturated())
      NewVal = 0;
    अन्यथा यदि (अतिप्रवाहः)
      *अतिप्रवाहः = सत्य;
  }

  NewVal = NewVal.extOrTrunc(DstSema.get_विस्तारः());
  NewVal.समुच्चयः_Is_Signed(DstSema.isSigned());
  प्रत्यागम APFixedPoint(NewVal, DstSema);
}

पूर्णाङ्क APFixedPoint::compare(ध्रुव APFixedPoint &Other) ध्रुव {
  APSInt ThisVal = get_मानम्();
  APSInt OtherVal = Other.get_मानम्();
  तार्किक ThisSigned = Val.isSigned();
  तार्किक OtherSigned = OtherVal.isSigned();

  पूर्णाङ्क CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णाङ्क CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  अचिह्नित Common_विस्तारः = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(Common_विस्तारः);
  OtherVal = OtherVal.extOrTrunc(Common_विस्तारः);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  यदि (ThisSigned && OtherSigned) {
    यदि (ThisVal.sgt(OtherVal))
      प्रत्यागम 1;
    अन्यथा यदि (ThisVal.slt(OtherVal))
      प्रत्यागम -1;
  } अन्यथा यदि (!ThisSigned && !OtherSigned) {
    यदि (ThisVal.ugt(OtherVal))
      प्रत्यागम 1;
    अन्यथा यदि (ThisVal.ult(OtherVal))
      प्रत्यागम -1;
  } अन्यथा यदि (ThisSigned && !OtherSigned) {
    यदि (ThisVal.is_Sign_Bit_समुच्चयः())
      प्रत्यागम -1;
    अन्यथा यदि (ThisVal.ugt(OtherVal))
      प्रत्यागम 1;
    अन्यथा यदि (ThisVal.ult(OtherVal))
      प्रत्यागम -1;
  } अन्यथा {
    // !ThisSigned && OtherSigned
    यदि (OtherVal.is_Sign_Bit_समुच्चयः())
      प्रत्यागम 1;
    अन्यथा यदि (ThisVal.ugt(OtherVal))
      प्रत्यागम 1;
    अन्यथा यदि (ThisVal.ult(OtherVal))
      प्रत्यागम -1;
  }

  प्रत्यागम 0;
}

APFixedPoint APFixedPoint::getMax(ध्रुव FixedPointSemantics &Sema) {
  तार्किक IsUnsigned = !Sema.isSigned();
  स्वचालित Val = APSInt::get_Max_मानम्(Sema.get_विस्तारः(), IsUnsigned);
  यदि (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  प्रत्यागम APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ध्रुव FixedPointSemantics &Sema) {
  स्वचालित Val = APSInt::get_Min_मानम्(Sema.get_विस्तारः(), !Sema.isSigned());
  प्रत्यागम APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ध्रुव FixedPointSemantics &Sema) {
  APSInt Val(Sema.get_विस्तारः(), !Sema.isSigned());
  Val.समुच्चयः_Bit(/*BitPosition=*/0);
  प्रत्यागम APFixedPoint(Val, Sema);
}

तार्किक FixedPointSemantics::माति_अन्तर्_Float_Semantics(
    ध्रुव fltSemantics &FloatSema) ध्रुव {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*एतत्).get_मानम्();
  APFloat F(FloatSema);
  APFloat::op_स्थितिः स्थितिः = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Away);
  यदि ((स्थितिः & APFloat::op_अतिप्रवाहः) || !isSigned())
    प्रत्यागम !(स्थितिः & APFloat::op_अतिप्रवाहः);

  APSInt MinInt = APFixedPoint::getMin(*एतत्).get_मानम्();
  स्थितिः = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Away);
  प्रत्यागम !(स्थितिः & APFloat::op_अतिप्रवाहः);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ध्रुव FixedPointSemantics &Other) ध्रुव {
  पूर्णाङ्क CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णाङ्क CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  अचिह्नित Common_विस्तारः = CommonMSb - CommonLsb + 1;

  तार्किक परिणाम_Is_Signed = isSigned() || Other.isSigned();
  तार्किक परिणाम_Is_Saturated = isSaturated() || Other.isSaturated();
  तार्किक परिणाम_Has_Unsigned_Padding = असत्य;
  यदि (!परिणाम_Is_Signed) {
    // Both are unsigned.
    परिणाम_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !परिणाम_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  यदि (परिणाम_Is_Signed || परिणाम_Has_Unsigned_Padding)
    Common_विस्तारः++;

  प्रत्यागम FixedPointSemantics(Common_विस्तारः, Lsb{CommonLsb}, परिणाम_Is_Signed,
                             परिणाम_Is_Saturated, परिणाम_Has_Unsigned_Padding);
}

APFixedPoint APFixedPoint::सङ्कलय(ध्रुव APFixedPoint &Other,
                               तार्किक *अतिप्रवाहः) ध्रुव {
  स्वचालित CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = असत्य;

  APSInt परिणाम;
  यदि (CommonFXSema.isSaturated()) {
    परिणाम = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } अन्यथा {
    परिणाम = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = Overflowed;

  प्रत्यागम APFixedPoint(परिणाम, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ध्रुव APFixedPoint &Other,
                               तार्किक *अतिप्रवाहः) ध्रुव {
  स्वचालित CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = असत्य;

  APSInt परिणाम;
  यदि (CommonFXSema.isSaturated()) {
    परिणाम = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } अन्यथा {
    परिणाम = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = Overflowed;

  प्रत्यागम APFixedPoint(परिणाम, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ध्रुव APFixedPoint &Other,
                               तार्किक *अतिप्रवाहः) ध्रुव {
  स्वचालित CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = असत्य;

  // Widen the LHS and RHS so we can perform a full multiplication.
  अचिह्नित Wide = CommonFXSema.get_विस्तारः() * 2;
  यदि (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अन्यथा {
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
  APSInt परिणाम;
  यदि (CommonFXSema.isSigned())
    परिणाम = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  अन्यथा
    परिणाम = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  प्रतिज्ञा(!Overflowed && "Full multiplication cannot overflow!");
  परिणाम.समुच्चयः_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  यदि (CommonFXSema.isSaturated()) {
    यदि (परिणाम < Min)
      परिणाम = std::move(Min);
    अन्यथा यदि (परिणाम > Max)
      परिणाम = std::move(Max);
  } अन्यथा {
    Overflowed = परिणाम < Min || परिणाम > Max;
  }

  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = Overflowed;

  प्रत्यागम APFixedPoint(परिणाम.sextOrTrunc(CommonFXSema.get_विस्तारः()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::भाजय(ध्रुव APFixedPoint &Other,
                               तार्किक *अतिप्रवाहः) ध्रुव {
  स्वचालित CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = असत्य;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  //अतिप्रवाहः
  अचिह्नित Wide =
      CommonFXSema.get_विस्तारः() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  यदि (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अन्यथा {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  यदि (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  अन्यथा यदि (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt परिणाम;
  यदि (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, परिणाम, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    यदि (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      परिणाम = परिणाम - 1;
  } अन्यथा {
    परिणाम = ThisVal.udiv(OtherVal);
  }
  परिणाम.समुच्चयः_Is_Signed(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  यदि (CommonFXSema.isSaturated()) {
    यदि (परिणाम < Min)
      परिणाम = std::move(Min);
    अन्यथा यदि (परिणाम > Max)
      परिणाम = std::move(Max);
  } अन्यथा {
    Overflowed = परिणाम < Min || परिणाम > Max;
  }

  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = Overflowed;

  प्रत्यागम APFixedPoint(परिणाम.sextOrTrunc(CommonFXSema.get_विस्तारः()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(अचिह्नित Amt, तार्किक *अतिप्रवाहः) ध्रुव {
  APSInt ThisVal = Val;
  तार्किक Overflowed = असत्य;

  // Widen the LHS.
  अचिह्नित Wide = Sema.get_विस्तारः() * 2;
  यदि (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  अन्यथा
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.get_Bit_विस्तारः());
  APSInt परिणाम = ThisVal << Amt;
  परिणाम.समुच्चयः_Is_Signed(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_मानम्().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_मानम्().extOrTrunc(Wide);
  यदि (Sema.isSaturated()) {
    यदि (परिणाम < Min)
      परिणाम = std::move(Min);
    अन्यथा यदि (परिणाम > Max)
      परिणाम = std::move(Max);
  } अन्यथा {
    Overflowed = परिणाम < Min || परिणाम > Max;
  }

  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = Overflowed;

  प्रत्यागम APFixedPoint(परिणाम.sextOrTrunc(Sema.get_विस्तारः()), Sema);
}

शून्य APFixedPoint::गन्तव्यम्_String(Small_सदिशम्_Impl<वर्ण> &Str) ध्रुव {
  APSInt Val = get_मानम्();
  पूर्णाङ्क Lsb = getLsbWeight();
  पूर्णाङ्क Orig_विस्तारः = get_विस्तारः();

  यदि (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.get_Bit_विस्तारः() + Lsb);
    IntPart <<= Lsb;
    IntPart.गन्तव्यम्_String(Str, /*Radix=*/10);
    Str.प्रक्षिप_पृष्ठभागः('.');
    Str.प्रक्षिप_पृष्ठभागः('0');
    प्रत्यागम;
  }

  यदि (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.समुच्चयः_Is_Unsigned(सत्य);
    Str.प्रक्षिप_पृष्ठभागः('-');
  }

  पूर्णाङ्क Scale = -getLsbWeight();
  APSInt IntPart = (Orig_विस्तारः > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  अचिह्नित विस्तारः = std::max(Orig_विस्तारः, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(विस्तारः);
  APInt Fract_Part_आवरणम् = APInt::getAllOnes(Scale).zext(विस्तारः);
  APInt RadixInt = APInt(विस्तारः, 10);

  IntPart.गन्तव्यम्_String(Str, /*Radix=*/10);
  Str.प्रक्षिप_पृष्ठभागः('.');
  कुरु {
    (FractPart * RadixInt)
        .lshr(Scale)
        .गन्तव्यम्_String(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & Fract_Part_आवरणम्;
  } यावत् (FractPart != 0);
}

शून्य APFixedPoint::print(raw_ostream &OS) ध्रुव {
  OS << "APFixedPoint(" << गन्तव्यम्_String() << ", {";
  Sema.print(OS);
  OS << "})";
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_विधानम् शून्य APFixedPoint::dump() ध्रुव { print(llvm::errs()); }
#यदि_अन्तः

APFixedPoint APFixedPoint::negate(तार्किक *अतिप्रवाहः) ध्रुव {
  यदि (!isSaturated()) {
    यदि (अतिप्रवाहः)
      *अतिप्रवाहः =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_मानम्());
    प्रत्यागम APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = असत्य;

  यदि (isSigned())
    प्रत्यागम Val.is_Min_Signed_मानम्() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  अन्यथा
    प्रत्यागम APFixedPoint(Sema);
}

APSInt APFixedPoint::convert_गन्तव्यम्_Int(अचिह्नित Dst_विस्तारः, तार्किक DstSign,
                                  तार्किक *अतिप्रवाहः) ध्रुव {
  APSInt परिणाम = getIntPart();
  अचिह्नित Src_विस्तारः = get_विस्तारः();

  APSInt DstMin = APSInt::get_Min_मानम्(Dst_विस्तारः, !DstSign);
  APSInt DstMax = APSInt::get_Max_मानम्(Dst_विस्तारः, !DstSign);

  यदि (Src_विस्तारः < Dst_विस्तारः) {
    परिणाम = परिणाम.extend(Dst_विस्तारः);
  } अन्यथा यदि (Src_विस्तारः > Dst_विस्तारः) {
    DstMin = DstMin.extend(Src_विस्तारः);
    DstMax = DstMax.extend(Src_विस्तारः);
  }

  यदि (अतिप्रवाहः) {
    यदि (परिणाम.isSigned() && !DstSign) {
      *अतिप्रवाहः = परिणाम.isNegative() || परिणाम.ugt(DstMax);
    } अन्यथा यदि (परिणाम.isUnsigned() && DstSign) {
      *अतिप्रवाहः = परिणाम.ugt(DstMax);
    } अन्यथा {
      *अतिप्रवाहः = परिणाम < DstMin || परिणाम > DstMax;
    }
  }

  परिणाम.समुच्चयः_Is_Signed(DstSign);
  प्रत्यागम परिणाम.extOrTrunc(Dst_विस्तारः);
}

ध्रुव fltSemantics *APFixedPoint::promoteFloatSemantics(ध्रुव fltSemantics *S) {
  यदि (S == &APFloat::BFloat())
    प्रत्यागम &APFloat::IEEEdouble();
  अन्यथा यदि (S == &APFloat::IEEEhalf())
    प्रत्यागम &APFloat::IEEEsingle();
  अन्यथा यदि (S == &APFloat::IEEEsingle())
    प्रत्यागम &APFloat::IEEEdouble();
  अन्यथा यदि (S == &APFloat::IEEEdouble())
    प्रत्यागम &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convert_गन्तव्यम्_Float(ध्रुव fltSemantics &FloatSema) ध्रुव {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::rounding_बहुलकम् RM = APFloat::rm_निकटतमः_Ties_गन्तव्यम्_Even;
  APFloat::rounding_बहुलकम् LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ध्रुव fltSemantics *OpSema = &FloatSema;
  यावत् (!Sema.माति_अन्तर्_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::op_स्थितिः S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (शून्य)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat Scale_गुणकः(std::घातः(2, Sema.getLsbWeight()));
  तार्किक Ignored;
  Scale_गुणकः.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(Scale_गुणकः, LosslessRM);

  यदि (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  प्रत्यागम Flt;
}

APFixedPoint APFixedPoint::get_From_Int_मानम्(ध्रुव APSInt &मानम्,
                                           ध्रुव FixedPointSemantics &DstFXSema,
                                           तार्किक *अतिप्रवाहः) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      मानम्.get_Bit_विस्तारः(), मानम्.isSigned());
  प्रत्यागम APFixedPoint(मानम्, IntFXSema).convert(DstFXSema, अतिप्रवाहः);
}

APFixedPoint
APFixedPoint::get_From_Float_मानम्(ध्रुव APFloat &मानम्,
                                ध्रुव FixedPointSemantics &DstFXSema,
                                तार्किक *अतिप्रवाहः) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::rounding_बहुलकम् RM = APFloat::rmTowardZero;
  APFloat::rounding_बहुलकम् LosslessRM = APFloat::rmTowardZero;

  ध्रुव fltSemantics &FloatSema = मानम्.getSemantics();

  यदि (मानम्.isNaN()) {
    // Handle NaN immediately.
    यदि (अतिप्रवाहः)
      *अतिप्रवाहः = सत्य;
    प्रत्यागम APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ध्रुव fltSemantics *OpSema = &FloatSema;
  यावत् (!DstFXSema.माति_अन्तर्_Float_Semantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = मानम्;

  तार्किक Ignored;
  यदि (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat Scale_गुणकः(std::घातः(2, -DstFXSema.getLsbWeight()));
  Scale_गुणकः.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(Scale_गुणकः, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.get_विस्तारः(), !DstFXSema.isSigned());
  Val.convert_गन्तव्यम्_Integer(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  Scale_गुणकः = APFloat(std::घातः(2, DstFXSema.getLsbWeight()));
  Scale_गुणकः.convert(*OpSema, LosslessRM, &Ignored);
  Val.आवर्तनम्_गन्तव्यम्_Integral(RM);
  Val.multiply(Scale_गुणकः, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convert_गन्तव्यम्_Float(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convert_गन्तव्यम्_Float(*OpSema);
  तार्किक Overflowed = असत्य;
  यदि (DstFXSema.isSaturated()) {
    यदि (Val > FloatMax)
      Res = getMax(DstFXSema).get_मानम्();
    अन्यथा यदि (Val < FloatMin)
      Res = getMin(DstFXSema).get_मानम्();
  } अन्यथा {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  यदि (अतिप्रवाहः)
    *अतिप्रवाहः = Overflowed;

  प्रत्यागम APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
