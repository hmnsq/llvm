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

فضاینام llvm {

تهی FixedPointSemantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << getWidth() << ", ";
  اگر (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ثابت {
  بازگشت llvm::bit_cast<uint32_t>(*این);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, نادرست, نادرست, نادرست);
  std::memcpy(&F, &I, اندازه(F));
  بازگشت F;
}

APFixedPoint APFixedPoint::convert(ثابت FixedPointSemantics &DstSema,
                                   منطقی *Overflow) ثابت {
  APSInt NewVal = Val;
  صحیح RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  اگر (Overflow)
    *Overflow = نادرست;

  اگر (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  اگر (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    اگر (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    وگرنه اگر (Overflow)
      *Overflow = درست;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  اگر (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    اگر (DstSema.isSaturated())
      NewVal = 0;
    وگرنه اگر (Overflow)
      *Overflow = درست;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  بازگشت APFixedPoint(NewVal, DstSema);
}

صحیح APFixedPoint::compare(ثابت APFixedPoint &Other) ثابت {
  APSInt ThisVal = get_مقدار();
  APSInt OtherVal = Other.get_مقدار();
  منطقی ThisSigned = Val.isSigned();
  منطقی OtherSigned = OtherVal.isSigned();

  صحیح CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  صحیح CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  اگر (ThisSigned && OtherSigned) {
    اگر (ThisVal.sgt(OtherVal))
      بازگشت 1;
    وگرنه اگر (ThisVal.slt(OtherVal))
      بازگشت -1;
  } وگرنه اگر (!ThisSigned && !OtherSigned) {
    اگر (ThisVal.ugt(OtherVal))
      بازگشت 1;
    وگرنه اگر (ThisVal.ult(OtherVal))
      بازگشت -1;
  } وگرنه اگر (ThisSigned && !OtherSigned) {
    اگر (ThisVal.isSignBitSet())
      بازگشت -1;
    وگرنه اگر (ThisVal.ugt(OtherVal))
      بازگشت 1;
    وگرنه اگر (ThisVal.ult(OtherVal))
      بازگشت -1;
  } وگرنه {
    // !ThisSigned && OtherSigned
    اگر (OtherVal.isSignBitSet())
      بازگشت 1;
    وگرنه اگر (ThisVal.ugt(OtherVal))
      بازگشت 1;
    وگرنه اگر (ThisVal.ult(OtherVal))
      بازگشت -1;
  }

  بازگشت 0;
}

APFixedPoint APFixedPoint::getMax(ثابت FixedPointSemantics &Sema) {
  منطقی IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_مقدار(Sema.getWidth(), IsUnsigned);
  اگر (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  بازگشت APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ثابت FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_مقدار(Sema.getWidth(), !Sema.isSigned());
  بازگشت APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ثابت FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  بازگشت APFixedPoint(Val, Sema);
}

منطقی FixedPointSemantics::fitsInFloatSemantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*این).get_مقدار();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  اگر ((Status & APFloat::opOverflow) || !isSigned())
    بازگشت !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*این).get_مقدار();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  بازگشت !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ثابت FixedPointSemantics &Other) ثابت {
  صحیح CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  صحیح CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  منطقی ResultIsSigned = isSigned() || Other.isSigned();
  منطقی ResultIsSaturated = isSaturated() || Other.isSaturated();
  منطقی ResultHasUnsignedPadding = نادرست;
  اگر (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  اگر (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  بازگشت FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ثابت APFixedPoint &Other,
                               منطقی *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقی Overflowed = نادرست;

  APSInt Result;
  اگر (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } وگرنه {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  اگر (Overflow)
    *Overflow = Overflowed;

  بازگشت APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ثابت APFixedPoint &Other,
                               منطقی *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقی Overflowed = نادرست;

  APSInt Result;
  اگر (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } وگرنه {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  اگر (Overflow)
    *Overflow = Overflowed;

  بازگشت APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ثابت APFixedPoint &Other,
                               منطقی *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقی Overflowed = نادرست;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  اگر (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وگرنه {
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
  اگر (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  وگرنه
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  اگر (CommonFXSema.isSaturated()) {
    اگر (Result < Min)
      Result = std::move(Min);
    وگرنه اگر (Result > Max)
      Result = std::move(Max);
  } وگرنه {
    Overflowed = Result < Min || Result > Max;
  }

  اگر (Overflow)
    *Overflow = Overflowed;

  بازگشت APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ثابت APFixedPoint &Other,
                               منطقی *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_مقدار();
  APSInt OtherVal = ConvertedOther.get_مقدار();
  منطقی Overflowed = نادرست;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  اگر (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وگرنه {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  اگر (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  وگرنه اگر (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  اگر (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    اگر (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } وگرنه {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_مقدار()
                                                 .extOrTrunc(Wide);
  اگر (CommonFXSema.isSaturated()) {
    اگر (Result < Min)
      Result = std::move(Min);
    وگرنه اگر (Result > Max)
      Result = std::move(Max);
  } وگرنه {
    Overflowed = Result < Min || Result > Max;
  }

  اگر (Overflow)
    *Overflow = Overflowed;

  بازگشت APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, منطقی *Overflow) ثابت {
  APSInt ThisVal = Val;
  منطقی Overflowed = نادرست;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  اگر (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  وگرنه
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_مقدار().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_مقدار().extOrTrunc(Wide);
  اگر (Sema.isSaturated()) {
    اگر (Result < Min)
      Result = std::move(Min);
    وگرنه اگر (Result > Max)
      Result = std::move(Max);
  } وگرنه {
    Overflowed = Result < Min || Result > Max;
  }

  اگر (Overflow)
    *Overflow = Overflowed;

  بازگشت APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

تهی APFixedPoint::toString(SmallVectorImpl<نویسه> &Str) ثابت {
  APSInt Val = get_مقدار();
  صحیح Lsb = getLsbWeight();
  صحیح OrigWidth = getWidth();

  اگر (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.افزودن_back('.');
    Str.افزودن_back('0');
    بازگشت;
  }

  اگر (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(درست);
    Str.افزودن_back('-');
  }

  صحیح Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.افزودن_back('.');
  انجامبده {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } وقتی (FractPart != 0);
}

تهی APFixedPoint::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#اگر !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD تهی APFixedPoint::dump() ثابت { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(منطقی *Overflow) ثابت {
  اگر (!isSaturated()) {
    اگر (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_مقدار());
    بازگشت APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  اگر (Overflow)
    *Overflow = نادرست;

  اگر (isSigned())
    بازگشت Val.is_Min_Signed_مقدار() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  وگرنه
    بازگشت APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, منطقی DstSign,
                                  منطقی *Overflow) ثابت {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_مقدار(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_مقدار(DstWidth, !DstSign);

  اگر (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } وگرنه اگر (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  اگر (Overflow) {
    اگر (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } وگرنه اگر (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } وگرنه {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  بازگشت Result.extOrTrunc(DstWidth);
}

ثابت fltSemantics *APFixedPoint::promoteFloatSemantics(ثابت fltSemantics *S) {
  اگر (S == &APFloat::BFloat())
    بازگشت &APFloat::IEEEdouble();
  وگرنه اگر (S == &APFloat::IEEEhalf())
    بازگشت &APFloat::IEEEsingle();
  وگرنه اگر (S == &APFloat::IEEEsingle())
    بازگشت &APFloat::IEEEdouble();
  وگرنه اگر (S == &APFloat::IEEEdouble())
    بازگشت &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(ثابت fltSemantics &FloatSema) ثابت {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  وقتی (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (تهی)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  منطقی Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  اگر (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  بازگشت Flt;
}

APFixedPoint APFixedPoint::get_From_Int_مقدار(ثابت APSInt &مقدار,
                                           ثابت FixedPointSemantics &DstFXSema,
                                           منطقی *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      مقدار.getBitWidth(), مقدار.isSigned());
  بازگشت APFixedPoint(مقدار, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_مقدار(ثابت APFloat &مقدار,
                                ثابت FixedPointSemantics &DstFXSema,
                                منطقی *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = مقدار.getSemantics();

  اگر (مقدار.isNaN()) {
    // Handle NaN immediately.
    اگر (Overflow)
      *Overflow = درست;
    بازگشت APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  وقتی (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = مقدار;

  منطقی Ignored;
  اگر (&FloatSema != OpSema)
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
  منطقی Overflowed = نادرست;
  اگر (DstFXSema.isSaturated()) {
    اگر (Val > FloatMax)
      Res = getMax(DstFXSema).get_مقدار();
    وگرنه اگر (Val < FloatMin)
      Res = getMin(DstFXSema).get_مقدار();
  } وگرنه {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  اگر (Overflow)
    *Overflow = Overflowed;

  بازگشت APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
