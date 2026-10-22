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

शून्य FixedPointSemantics::print(llvm::raw_ostream &OS) स्थिरांक {
  OS << "width=" << getWidth() << ", ";
  जर (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() स्थिरांक {
  परत llvm::bit_cast<uint32_t>(*हे);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, असत्य, असत्य, असत्य);
  std::स_नकल(&F, &I, आकार(F));
  परत F;
}

APFixedPoint APFixedPoint::convert(स्थिरांक FixedPointSemantics &DstSema,
                                   तार्किक *Overflow) स्थिरांक {
  APSInt NewVal = Val;
  पूर्णांक RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  जर (Overflow)
    *Overflow = असत्य;

  जर (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  स्वतः Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  जर (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    जर (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    अन्यथा जर (Overflow)
      *Overflow = सत्य;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  जर (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    जर (DstSema.isSaturated())
      NewVal = 0;
    अन्यथा जर (Overflow)
      *Overflow = सत्य;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  परत APFixedPoint(NewVal, DstSema);
}

पूर्णांक APFixedPoint::compare(स्थिरांक APFixedPoint &Other) स्थिरांक {
  APSInt ThisVal = get_मूल्य();
  APSInt OtherVal = Other.get_मूल्य();
  तार्किक ThisSigned = Val.isSigned();
  तार्किक OtherSigned = OtherVal.isSigned();

  पूर्णांक CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णांक CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  अचिन्हित CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  जर (ThisSigned && OtherSigned) {
    जर (ThisVal.sgt(OtherVal))
      परत 1;
    अन्यथा जर (ThisVal.slt(OtherVal))
      परत -1;
  } अन्यथा जर (!ThisSigned && !OtherSigned) {
    जर (ThisVal.ugt(OtherVal))
      परत 1;
    अन्यथा जर (ThisVal.ult(OtherVal))
      परत -1;
  } अन्यथा जर (ThisSigned && !OtherSigned) {
    जर (ThisVal.isSignBitSet())
      परत -1;
    अन्यथा जर (ThisVal.ugt(OtherVal))
      परत 1;
    अन्यथा जर (ThisVal.ult(OtherVal))
      परत -1;
  } अन्यथा {
    // !ThisSigned && OtherSigned
    जर (OtherVal.isSignBitSet())
      परत 1;
    अन्यथा जर (ThisVal.ugt(OtherVal))
      परत 1;
    अन्यथा जर (ThisVal.ult(OtherVal))
      परत -1;
  }

  परत 0;
}

APFixedPoint APFixedPoint::getMax(स्थिरांक FixedPointSemantics &Sema) {
  तार्किक IsUnsigned = !Sema.isSigned();
  स्वतः Val = APSInt::get_Max_मूल्य(Sema.getWidth(), IsUnsigned);
  जर (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  परत APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(स्थिरांक FixedPointSemantics &Sema) {
  स्वतः Val = APSInt::get_Min_मूल्य(Sema.getWidth(), !Sema.isSigned());
  परत APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(स्थिरांक FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  परत APFixedPoint(Val, Sema);
}

तार्किक FixedPointSemantics::fitsInFloatSemantics(
    स्थिरांक fltSemantics &FloatSema) स्थिरांक {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*हे).get_मूल्य();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  जर ((Status & APFloat::opOverflow) || !isSigned())
    परत !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*हे).get_मूल्य();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  परत !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    स्थिरांक FixedPointSemantics &Other) स्थिरांक {
  पूर्णांक CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  पूर्णांक CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  अचिन्हित CommonWidth = CommonMSb - CommonLsb + 1;

  तार्किक ResultIsSigned = isSigned() || Other.isSigned();
  तार्किक ResultIsSaturated = isSaturated() || Other.isSaturated();
  तार्किक ResultHasUnsignedPadding = असत्य;
  जर (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  जर (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  परत FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(स्थिरांक APFixedPoint &Other,
                               तार्किक *Overflow) स्थिरांक {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  APSInt Result;
  जर (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } अन्यथा {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  जर (Overflow)
    *Overflow = Overflowed;

  परत APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(स्थिरांक APFixedPoint &Other,
                               तार्किक *Overflow) स्थिरांक {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  APSInt Result;
  जर (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } अन्यथा {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  जर (Overflow)
    *Overflow = Overflowed;

  परत APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(स्थिरांक APFixedPoint &Other,
                               तार्किक *Overflow) स्थिरांक {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  // Widen the LHS and RHS so we can perform a full multiplication.
  अचिन्हित Wide = CommonFXSema.getWidth() * 2;
  जर (CommonFXSema.isSigned()) {
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
  जर (CommonFXSema.isSigned())
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
  जर (CommonFXSema.isSaturated()) {
    जर (Result < Min)
      Result = std::move(Min);
    अन्यथा जर (Result > Max)
      Result = std::move(Max);
  } अन्यथा {
    Overflowed = Result < Min || Result > Max;
  }

  जर (Overflow)
    *Overflow = Overflowed;

  परत APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::भाग(स्थिरांक APFixedPoint &Other,
                               तार्किक *Overflow) स्थिरांक {
  स्वतः CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_मूल्य();
  APSInt OtherVal = ConvertedOther.get_मूल्य();
  तार्किक Overflowed = असत्य;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  अचिन्हित Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  जर (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } अन्यथा {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  जर (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  अन्यथा जर (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  जर (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    जर (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
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
  जर (CommonFXSema.isSaturated()) {
    जर (Result < Min)
      Result = std::move(Min);
    अन्यथा जर (Result > Max)
      Result = std::move(Max);
  } अन्यथा {
    Overflowed = Result < Min || Result > Max;
  }

  जर (Overflow)
    *Overflow = Overflowed;

  परत APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(अचिन्हित Amt, तार्किक *Overflow) स्थिरांक {
  APSInt ThisVal = Val;
  तार्किक Overflowed = असत्य;

  // Widen the LHS.
  अचिन्हित Wide = Sema.getWidth() * 2;
  जर (Sema.isSigned())
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
  जर (Sema.isSaturated()) {
    जर (Result < Min)
      Result = std::move(Min);
    अन्यथा जर (Result > Max)
      Result = std::move(Max);
  } अन्यथा {
    Overflowed = Result < Min || Result > Max;
  }

  जर (Overflow)
    *Overflow = Overflowed;

  परत APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

शून्य APFixedPoint::toString(SmallVectorImpl<अक्षर> &Str) स्थिरांक {
  APSInt Val = get_मूल्य();
  पूर्णांक Lsb = getLsbWeight();
  पूर्णांक OrigWidth = getWidth();

  जर (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ढकला_back('.');
    Str.ढकला_back('0');
    परत;
  }

  जर (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(सत्य);
    Str.ढकला_back('-');
  }

  पूर्णांक Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  अचिन्हित Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ढकला_back('.');
  करा {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } पर्यंत (FractPart != 0);
}

शून्य APFixedPoint::print(raw_ostream &OS) स्थिरांक {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#जर !defined(न_संशोधन) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD शून्य APFixedPoint::dump() स्थिरांक { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(तार्किक *Overflow) स्थिरांक {
  जर (!isSaturated()) {
    जर (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_मूल्य());
    परत APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  जर (Overflow)
    *Overflow = असत्य;

  जर (isSigned())
    परत Val.is_Min_Signed_मूल्य() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  अन्यथा
    परत APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(अचिन्हित DstWidth, तार्किक DstSign,
                                  तार्किक *Overflow) स्थिरांक {
  APSInt Result = getIntPart();
  अचिन्हित SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_मूल्य(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_मूल्य(DstWidth, !DstSign);

  जर (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } अन्यथा जर (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  जर (Overflow) {
    जर (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } अन्यथा जर (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } अन्यथा {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  परत Result.extOrTrunc(DstWidth);
}

स्थिरांक fltSemantics *APFixedPoint::promoteFloatSemantics(स्थिरांक fltSemantics *S) {
  जर (S == &APFloat::BFloat())
    परत &APFloat::IEEEdouble();
  अन्यथा जर (S == &APFloat::IEEEhalf())
    परत &APFloat::IEEEsingle();
  अन्यथा जर (S == &APFloat::IEEEsingle())
    परत &APFloat::IEEEdouble();
  अन्यथा जर (S == &APFloat::IEEEdouble())
    परत &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(स्थिरांक fltSemantics &FloatSema) स्थिरांक {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  स्थिरांक fltSemantics *OpSema = &FloatSema;
  पर्यंत (!Sema.fitsInFloatSemantics(*OpSema))
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
  APFloat ScaleFactor(std::घात(2, Sema.getLsbWeight()));
  तार्किक Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  जर (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  परत Flt;
}

APFixedPoint APFixedPoint::get_From_Int_मूल्य(स्थिरांक APSInt &मूल्य,
                                           स्थिरांक FixedPointSemantics &DstFXSema,
                                           तार्किक *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      मूल्य.getBitWidth(), मूल्य.isSigned());
  परत APFixedPoint(मूल्य, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_मूल्य(स्थिरांक APFloat &मूल्य,
                                स्थिरांक FixedPointSemantics &DstFXSema,
                                तार्किक *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  स्थिरांक fltSemantics &FloatSema = मूल्य.getSemantics();

  जर (मूल्य.isNaN()) {
    // Handle NaN immediately.
    जर (Overflow)
      *Overflow = सत्य;
    परत APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  स्थिरांक fltSemantics *OpSema = &FloatSema;
  पर्यंत (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = मूल्य;

  तार्किक Ignored;
  जर (&FloatSema != OpSema)
    Val.convert(*OpSema, LosslessRM, &Ignored);

  // Scale up the float so that the 'fractional' part of the mantissa ends up in
  // the integer range instead. Rounding mode is irrelevant here.
  // It is fine if this overflows to infinity even for saturating types,
  // since we will use floating point comparisons to check for saturation.
  APFloat ScaleFactor(std::घात(2, -DstFXSema.getLsbWeight()));
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
  ScaleFactor = APFloat(std::घात(2, DstFXSema.getLsbWeight()));
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Val.roundToIntegral(RM);
  Val.multiply(ScaleFactor, LosslessRM);

  // Check for overflow/saturation by checking if the floating point value
  // is outside the range representable by the fixed-point value.
  APFloat FloatMax = getMax(DstFXSema).convertToFloat(*OpSema);
  APFloat FloatMin = getMin(DstFXSema).convertToFloat(*OpSema);
  तार्किक Overflowed = असत्य;
  जर (DstFXSema.isSaturated()) {
    जर (Val > FloatMax)
      Res = getMax(DstFXSema).get_मूल्य();
    अन्यथा जर (Val < FloatMin)
      Res = getMin(DstFXSema).get_मूल्य();
  } अन्यथा {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  जर (Overflow)
    *Overflow = Overflowed;

  परत APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
