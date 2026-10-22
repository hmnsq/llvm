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
  OS << "width=" << getWidth() << ", ";
  यदि (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ध्रुव {
  प्रत्यागम llvm::bit_cast<uint32_t>(*एतत्);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, असत्य, असत्य, असत्य);
  std::memcpy(&F, &I, परिमाण(F));
  प्रत्यागम F;
}

APFixedPoint APFixedPoint::convert(ध्रुव FixedPointSemantics &DstSema,
                                   तार्किक *Overflow) ध्रुव {
  APSInt NewVal = Val;
  पूर्णाङ्क RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  यदि (Overflow)
    *Overflow = असत्य;

  यदि (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  यदि (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    यदि (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    अन्यथा यदि (Overflow)
      *Overflow = सत्य;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  यदि (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    यदि (DstSema.isSaturated())
      NewVal = 0;
    अन्यथा यदि (Overflow)
      *Overflow = सत्य;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  प्रत्यागम APFixedPoint(NewVal, DstSema);
}

पूर्णाङ्क APFixedPoint::compare(ध्रुव APFixedPoint &Other) ध्रुव {
  APSInt ThisVal = get_मूल्य();
  APSInt OtherVal = Other.get_मूल्य();
  तार्किक ThisSigned = Val.isSigned();
  तार्किक OtherSigned = OtherVal.isSigned();

  पूर्णाङ्क CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णाङ्क CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

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
    यदि (ThisVal.isSignBitSet())
      प्रत्यागम -1;
    अन्यथा यदि (ThisVal.ugt(OtherVal))
      प्रत्यागम 1;
    अन्यथा यदि (ThisVal.ult(OtherVal))
      प्रत्यागम -1;
  } अन्यथा {
    // !ThisSigned && OtherSigned
    यदि (OtherVal.isSignBitSet())
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
  auto Val = APSInt::get_Max_मूल्य(Sema.getWidth(), IsUnsigned);
  यदि (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  प्रत्यागम APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ध्रुव FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_मूल्य(Sema.getWidth(), !Sema.isSigned());
  प्रत्यागम APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ध्रुव FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  प्रत्यागम APFixedPoint(Val, Sema);
}

तार्किक FixedPointSemantics::fitsInFloatSemantics(
    ध्रुव fltSemantics &FloatSema) ध्रुव {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*एतत्).get_मूल्य();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  यदि ((Status & APFloat::opOverflow) || !isSigned())
    प्रत्यागम !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*एतत्).get_मूल्य();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  प्रत्यागम !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ध्रुव FixedPointSemantics &Other) ध्रुव {
  पूर्णाङ्क CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णाङ्क CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  तार्किक ResultIsSigned = isSigned() || Other.isSigned();
  तार्किक ResultIsSaturated = isSaturated() || Other.isSaturated();
  तार्किक ResultHasUnsignedPadding = असत्य;
  यदि (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  यदि (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  प्रत्यागम FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  APSInt Result;
  यदि (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } अन्यथा {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  यदि (Overflow)
    *Overflow = Overflowed;

  प्रत्यागम APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  APSInt Result;
  यदि (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } अन्यथा {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  यदि (Overflow)
    *Overflow = Overflowed;

  प्रत्यागम APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
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
  APSInt Result;
  यदि (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  अन्यथा
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  प्रतिज्ञा(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  यदि (CommonFXSema.isSaturated()) {
    यदि (Result < Min)
      Result = std::move(Min);
    अन्यथा यदि (Result > Max)
      Result = std::move(Max);
  } अन्यथा {
    Overflowed = Result < Min || Result > Max;
  }

  यदि (Overflow)
    *Overflow = Overflowed;

  प्रत्यागम APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
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
  APSInt Result;
  यदि (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    यदि (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } अन्यथा {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मूल्य()
                                                 .extOrTrunc(Wide);
  यदि (CommonFXSema.isSaturated()) {
    यदि (Result < Min)
      Result = std::move(Min);
    अन्यथा यदि (Result > Max)
      Result = std::move(Max);
  } अन्यथा {
    Overflowed = Result < Min || Result > Max;
  }

  यदि (Overflow)
    *Overflow = Overflowed;

  प्रत्यागम APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, तार्किक *Overflow) ध्रुव {
  APSInt ThisVal = Val;
  तार्किक Overflowed = असत्य;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  यदि (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  अन्यथा
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_मूल्य().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_मूल्य().extOrTrunc(Wide);
  यदि (Sema.isSaturated()) {
    यदि (Result < Min)
      Result = std::move(Min);
    अन्यथा यदि (Result > Max)
      Result = std::move(Max);
  } अन्यथा {
    Overflowed = Result < Min || Result > Max;
  }

  यदि (Overflow)
    *Overflow = Overflowed;

  प्रत्यागम APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

शून्य APFixedPoint::toString(SmallVectorImpl<वर्ण> &Str) ध्रुव {
  APSInt Val = get_मूल्य();
  पूर्णाङ्क Lsb = getLsbWeight();
  पूर्णाङ्क OrigWidth = getWidth();

  यदि (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ढकला_back('.');
    Str.ढकला_back('0');
    प्रत्यागम;
  }

  यदि (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(सत्य);
    Str.ढकला_back('-');
  }

  पूर्णाङ्क Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ढकला_back('.');
  कुरु {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } यावत् (FractPart != 0);
}

शून्य APFixedPoint::print(raw_ostream &OS) ध्रुव {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#यदि !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APFixedPoint::dump() ध्रुव { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(तार्किक *Overflow) ध्रुव {
  यदि (!isSaturated()) {
    यदि (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_मूल्य());
    प्रत्यागम APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  यदि (Overflow)
    *Overflow = असत्य;

  यदि (isSigned())
    प्रत्यागम Val.is_Min_Signed_मूल्य() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  अन्यथा
    प्रत्यागम APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, तार्किक DstSign,
                                  तार्किक *Overflow) ध्रुव {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_मूल्य(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_मूल्य(DstWidth, !DstSign);

  यदि (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } अन्यथा यदि (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  यदि (Overflow) {
    यदि (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } अन्यथा यदि (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } अन्यथा {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  प्रत्यागम Result.extOrTrunc(DstWidth);
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

APFloat APFixedPoint::convertToFloat(ध्रुव fltSemantics &FloatSema) ध्रुव {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ध्रुव fltSemantics *OpSema = &FloatSema;
  यावत् (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (शून्य)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  तार्किक Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  यदि (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  प्रत्यागम Flt;
}

APFixedPoint APFixedPoint::get_From_Int_मूल्य(ध्रुव APSInt &मूल्य,
                                           ध्रुव FixedPointSemantics &DstFXSema,
                                           तार्किक *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      मूल्य.getBitWidth(), मूल्य.isSigned());
  प्रत्यागम APFixedPoint(मूल्य, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_मूल्य(ध्रुव APFloat &मूल्य,
                                ध्रुव FixedPointSemantics &DstFXSema,
                                तार्किक *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ध्रुव fltSemantics &FloatSema = मूल्य.getSemantics();

  यदि (मूल्य.isNaN()) {
    // Handle NaN immediately.
    यदि (Overflow)
      *Overflow = सत्य;
    प्रत्यागम APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ध्रुव fltSemantics *OpSema = &FloatSema;
  यावत् (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = मूल्य;

  तार्किक Ignored;
  यदि (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat ScaleFactor(std::pow(2, -DstFXSema.getLsbWeight()));
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Val.multiply(ScaleFactor, LosslessRM);

  // Convert to the integral representation of the value. This rounding mode
  // is significant.
  APSInt Res(DstFXSema.getWidth(), !DstFXSema.isSigned());
  Val.convertToInteger(Res, RM, &Ignored);

  // Round the integral value and scale back. This makes the
  // overflow calculations below work properly. If we do not round here,
  // we risk checking for overflow with a value that is outside the
  // representable range of the fixed-point semantic even though no overflow
  // would occur had we rounded first.
  ScaleFactor = APFloat(std::pow(2, DstFXSema.getLsbWeight()));
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Val.roundToIntegral(RM);
  Val.multiply(ScaleFactor, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convertToFloat(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convertToFloat(*OpSema);
  तार्किक Overflowed = असत्य;
  यदि (DstFXSema.isSaturated()) {
    यदि (Val > FloatMax)
      Res = getMax(DstFXSema).get_मूल्य();
    अन्यथा यदि (Val < FloatMin)
      Res = getMin(DstFXSema).get_मूल्य();
  } अन्यथा {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  यदि (Overflow)
    *Overflow = Overflowed;

  प्रत्यागम APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
