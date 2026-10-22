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
  सचे (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ध्रुव {
  पटिनिवत्त llvm::bit_cast<uint32_t>(*एतत्);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, मिच्छा, मिच्छा, मिच्छा);
  std::memcpy(&F, &I, परिमाण(F));
  पटिनिवत्त F;
}

APFixedPoint APFixedPoint::convert(ध्रुव FixedPointSemantics &DstSema,
                                   तार्किक *Overflow) ध्रुव {
  APSInt NewVal = Val;
  पूर्णाङ्क RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  सचे (Overflow)
    *Overflow = मिच्छा;

  सचे (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  सचे (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    सचे (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    अञ्ञथा सचे (Overflow)
      *Overflow = सच्च;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  सचे (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    सचे (DstSema.isSaturated())
      NewVal = 0;
    अञ्ञथा सचे (Overflow)
      *Overflow = सच्च;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  पटिनिवत्त APFixedPoint(NewVal, DstSema);
}

पूर्णाङ्क APFixedPoint::compare(ध्रुव APFixedPoint &Other) ध्रुव {
  APSInt ThisVal = get_मानम्();
  APSInt OtherVal = Other.get_मानम्();
  तार्किक ThisSigned = Val.isSigned();
  तार्किक OtherSigned = OtherVal.isSigned();

  पूर्णाङ्क CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णाङ्क CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  सचे (ThisSigned && OtherSigned) {
    सचे (ThisVal.sgt(OtherVal))
      पटिनिवत्त 1;
    अञ्ञथा सचे (ThisVal.slt(OtherVal))
      पटिनिवत्त -1;
  } अञ्ञथा सचे (!ThisSigned && !OtherSigned) {
    सचे (ThisVal.ugt(OtherVal))
      पटिनिवत्त 1;
    अञ्ञथा सचे (ThisVal.ult(OtherVal))
      पटिनिवत्त -1;
  } अञ्ञथा सचे (ThisSigned && !OtherSigned) {
    सचे (ThisVal.isSignBitSet())
      पटिनिवत्त -1;
    अञ्ञथा सचे (ThisVal.ugt(OtherVal))
      पटिनिवत्त 1;
    अञ्ञथा सचे (ThisVal.ult(OtherVal))
      पटिनिवत्त -1;
  } अञ्ञथा {
    // !ThisSigned && OtherSigned
    सचे (OtherVal.isSignBitSet())
      पटिनिवत्त 1;
    अञ्ञथा सचे (ThisVal.ugt(OtherVal))
      पटिनिवत्त 1;
    अञ्ञथा सचे (ThisVal.ult(OtherVal))
      पटिनिवत्त -1;
  }

  पटिनिवत्त 0;
}

APFixedPoint APFixedPoint::getMax(ध्रुव FixedPointSemantics &Sema) {
  तार्किक IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_मानम्(Sema.getWidth(), IsUnsigned);
  सचे (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  पटिनिवत्त APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ध्रुव FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_मानम्(Sema.getWidth(), !Sema.isSigned());
  पटिनिवत्त APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ध्रुव FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  पटिनिवत्त APFixedPoint(Val, Sema);
}

तार्किक FixedPointSemantics::fitsInFloatSemantics(
    ध्रुव fltSemantics &FloatSema) ध्रुव {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*एतत्).get_मानम्();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  सचे ((Status & APFloat::opOverflow) || !isSigned())
    पटिनिवत्त !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*एतत्).get_मानम्();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  पटिनिवत्त !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ध्रुव FixedPointSemantics &Other) ध्रुव {
  पूर्णाङ्क CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णाङ्क CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  तार्किक ResultIsSigned = isSigned() || Other.isSigned();
  तार्किक ResultIsSaturated = isSaturated() || Other.isSaturated();
  तार्किक ResultHasUnsignedPadding = मिच्छा;
  सचे (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  सचे (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  पटिनिवत्त FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = मिच्छा;

  APSInt Result;
  सचे (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } अञ्ञथा {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  सचे (Overflow)
    *Overflow = Overflowed;

  पटिनिवत्त APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = मिच्छा;

  APSInt Result;
  सचे (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } अञ्ञथा {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  सचे (Overflow)
    *Overflow = Overflowed;

  पटिनिवत्त APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = मिच्छा;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  सचे (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अञ्ञथा {
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
  सचे (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  अञ्ञथा
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  प्रतिज्ञा(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  सचे (CommonFXSema.isSaturated()) {
    सचे (Result < Min)
      Result = std::move(Min);
    अञ्ञथा सचे (Result > Max)
      Result = std::move(Max);
  } अञ्ञथा {
    Overflowed = Result < Min || Result > Max;
  }

  सचे (Overflow)
    *Overflow = Overflowed;

  पटिनिवत्त APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ध्रुव APFixedPoint &Other,
                               तार्किक *Overflow) ध्रुव {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मानम्();
  APSInt OtherVal = ConvertedOther.get_मानम्();
  तार्किक Overflowed = मिच्छा;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  सचे (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अञ्ञथा {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  सचे (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  अञ्ञथा सचे (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  सचे (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    सचे (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } अञ्ञथा {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_मानम्()
                                                 .extOrTrunc(Wide);
  सचे (CommonFXSema.isSaturated()) {
    सचे (Result < Min)
      Result = std::move(Min);
    अञ्ञथा सचे (Result > Max)
      Result = std::move(Max);
  } अञ्ञथा {
    Overflowed = Result < Min || Result > Max;
  }

  सचे (Overflow)
    *Overflow = Overflowed;

  पटिनिवत्त APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, तार्किक *Overflow) ध्रुव {
  APSInt ThisVal = Val;
  तार्किक Overflowed = मिच्छा;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  सचे (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  अञ्ञथा
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_मानम्().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_मानम्().extOrTrunc(Wide);
  सचे (Sema.isSaturated()) {
    सचे (Result < Min)
      Result = std::move(Min);
    अञ्ञथा सचे (Result > Max)
      Result = std::move(Max);
  } अञ्ञथा {
    Overflowed = Result < Min || Result > Max;
  }

  सचे (Overflow)
    *Overflow = Overflowed;

  पटिनिवत्त APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

शून्य APFixedPoint::toString(SmallVectorImpl<वर्ण> &Str) ध्रुव {
  APSInt Val = get_मानम्();
  पूर्णाङ्क Lsb = getLsbWeight();
  पूर्णाङ्क OrigWidth = getWidth();

  सचे (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.प्रक्षिप_back('.');
    Str.प्रक्षिप_back('0');
    पटिनिवत्त;
  }

  सचे (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(सच्च);
    Str.प्रक्षिप_back('-');
  }

  पूर्णाङ्क Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.प्रक्षिप_back('.');
  करोहि {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } याव (FractPart != 0);
}

शून्य APFixedPoint::print(raw_ostream &OS) ध्रुव {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#सचे !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APFixedPoint::dump() ध्रुव { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(तार्किक *Overflow) ध्रुव {
  सचे (!isSaturated()) {
    सचे (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_मानम्());
    पटिनिवत्त APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  सचे (Overflow)
    *Overflow = मिच्छा;

  सचे (isSigned())
    पटिनिवत्त Val.is_Min_Signed_मानम्() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  अञ्ञथा
    पटिनिवत्त APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, तार्किक DstSign,
                                  तार्किक *Overflow) ध्रुव {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_मानम्(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_मानम्(DstWidth, !DstSign);

  सचे (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } अञ्ञथा सचे (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  सचे (Overflow) {
    सचे (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } अञ्ञथा सचे (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } अञ्ञथा {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  पटिनिवत्त Result.extOrTrunc(DstWidth);
}

ध्रुव fltSemantics *APFixedPoint::promoteFloatSemantics(ध्रुव fltSemantics *S) {
  सचे (S == &APFloat::BFloat())
    पटिनिवत्त &APFloat::IEEEdouble();
  अञ्ञथा सचे (S == &APFloat::IEEEhalf())
    पटिनिवत्त &APFloat::IEEEsingle();
  अञ्ञथा सचे (S == &APFloat::IEEEsingle())
    पटिनिवत्त &APFloat::IEEEdouble();
  अञ्ञथा सचे (S == &APFloat::IEEEdouble())
    पटिनिवत्त &APFloat::IEEEquad();
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
  याव (!Sema.fitsInFloatSemantics(*OpSema))
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

  सचे (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  पटिनिवत्त Flt;
}

APFixedPoint APFixedPoint::get_From_Int_मानम्(ध्रुव APSInt &मानम्,
                                           ध्रुव FixedPointSemantics &DstFXSema,
                                           तार्किक *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      मानम्.getBitWidth(), मानम्.isSigned());
  पटिनिवत्त APFixedPoint(मानम्, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_मानम्(ध्रुव APFloat &मानम्,
                                ध्रुव FixedPointSemantics &DstFXSema,
                                तार्किक *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ध्रुव fltSemantics &FloatSema = मानम्.getSemantics();

  सचे (मानम्.isNaN()) {
    // Handle NaN immediately.
    सचे (Overflow)
      *Overflow = सच्च;
    पटिनिवत्त APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ध्रुव fltSemantics *OpSema = &FloatSema;
  याव (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = मानम्;

  तार्किक Ignored;
  सचे (&FloatSema != OpSema)
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
  तार्किक Overflowed = मिच्छा;
  सचे (DstFXSema.isSaturated()) {
    सचे (Val > FloatMax)
      Res = getMax(DstFXSema).get_मानम्();
    अञ्ञथा सचे (Val < FloatMin)
      Res = getMin(DstFXSema).get_मानम्();
  } अञ्ञथा {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  सचे (Overflow)
    *Overflow = Overflowed;

  पटिनिवत्त APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
