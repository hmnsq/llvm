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

فراغ FixedPointSemantics::print(llvm::raw_ostream &OS) ثابت {
  OS << "width=" << getWidth() << ", ";
  ܐܢ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ثابت {
  ܦܢܐ llvm::bit_cast<uint32_t>(*هذا);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, ܕܓܠܐ, ܕܓܠܐ, ܕܓܠܐ);
  std::memcpy(&F, &I, حجم(F));
  ܦܢܐ F;
}

APFixedPoint APFixedPoint::convert(ثابت FixedPointSemantics &DstSema,
                                   منطقي *Overflow) ثابت {
  APSInt NewVal = Val;
  صحيح RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  ܐܢ (Overflow)
    *Overflow = ܕܓܠܐ;

  ܐܢ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  ܐܢ (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    ܐܢ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    ܐܚܪܢܐ ܐܢ (Overflow)
      *Overflow = ܫܪܪܐ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ܐܢ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ܐܢ (DstSema.isSaturated())
      NewVal = 0;
    ܐܚܪܢܐ ܐܢ (Overflow)
      *Overflow = ܫܪܪܐ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  ܦܢܐ APFixedPoint(NewVal, DstSema);
}

صحيح APFixedPoint::compare(ثابت APFixedPoint &Other) ثابت {
  APSInt ThisVal = getValue();
  APSInt OtherVal = Other.getValue();
  منطقي ThisSigned = Val.isSigned();
  منطقي OtherSigned = OtherVal.isSigned();

  صحيح CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  صحيح CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  ܐܢ (ThisSigned && OtherSigned) {
    ܐܢ (ThisVal.sgt(OtherVal))
      ܦܢܐ 1;
    ܐܚܪܢܐ ܐܢ (ThisVal.slt(OtherVal))
      ܦܢܐ -1;
  } ܐܚܪܢܐ ܐܢ (!ThisSigned && !OtherSigned) {
    ܐܢ (ThisVal.ugt(OtherVal))
      ܦܢܐ 1;
    ܐܚܪܢܐ ܐܢ (ThisVal.ult(OtherVal))
      ܦܢܐ -1;
  } ܐܚܪܢܐ ܐܢ (ThisSigned && !OtherSigned) {
    ܐܢ (ThisVal.isSignBitSet())
      ܦܢܐ -1;
    ܐܚܪܢܐ ܐܢ (ThisVal.ugt(OtherVal))
      ܦܢܐ 1;
    ܐܚܪܢܐ ܐܢ (ThisVal.ult(OtherVal))
      ܦܢܐ -1;
  } ܐܚܪܢܐ {
    // !ThisSigned && OtherSigned
    ܐܢ (OtherVal.isSignBitSet())
      ܦܢܐ 1;
    ܐܚܪܢܐ ܐܢ (ThisVal.ugt(OtherVal))
      ܦܢܐ 1;
    ܐܚܪܢܐ ܐܢ (ThisVal.ult(OtherVal))
      ܦܢܐ -1;
  }

  ܦܢܐ 0;
}

APFixedPoint APFixedPoint::getMax(ثابت FixedPointSemantics &Sema) {
  منطقي IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::getMaxValue(Sema.getWidth(), IsUnsigned);
  ܐܢ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ܦܢܐ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ثابت FixedPointSemantics &Sema) {
  auto Val = APSInt::getMinValue(Sema.getWidth(), !Sema.isSigned());
  ܦܢܐ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ثابت FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  ܦܢܐ APFixedPoint(Val, Sema);
}

منطقي FixedPointSemantics::fitsInFloatSemantics(
    ثابت fltSemantics &FloatSema) ثابت {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*هذا).getValue();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  ܐܢ ((Status & APFloat::opOverflow) || !isSigned())
    ܦܢܐ !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*هذا).getValue();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  ܦܢܐ !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ثابت FixedPointSemantics &Other) ثابت {
  صحيح CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  صحيح CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  منطقي ResultIsSigned = isSigned() || Other.isSigned();
  منطقي ResultIsSaturated = isSaturated() || Other.isSaturated();
  منطقي ResultHasUnsignedPadding = ܕܓܠܐ;
  ܐܢ (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ܐܢ (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  ܦܢܐ FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = ܕܓܠܐ;

  APSInt Result;
  ܐܢ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ܐܚܪܢܐ {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ܐܢ (Overflow)
    *Overflow = Overflowed;

  ܦܢܐ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = ܕܓܠܐ;

  APSInt Result;
  ܐܢ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ܐܚܪܢܐ {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ܐܢ (Overflow)
    *Overflow = Overflowed;

  ܦܢܐ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = ܕܓܠܐ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  ܐܢ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ܐܚܪܢܐ {
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
  ܐܢ (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  ܐܚܪܢܐ
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  أكد(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  ܐܢ (CommonFXSema.isSaturated()) {
    ܐܢ (Result < Min)
      Result = std::move(Min);
    ܐܚܪܢܐ ܐܢ (Result > Max)
      Result = std::move(Max);
  } ܐܚܪܢܐ {
    Overflowed = Result < Min || Result > Max;
  }

  ܐܢ (Overflow)
    *Overflow = Overflowed;

  ܦܢܐ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = ܕܓܠܐ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  ܐܢ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ܐܚܪܢܐ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ܐܢ (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  ܐܚܪܢܐ ܐܢ (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  ܐܢ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ܐܢ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } ܐܚܪܢܐ {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  ܐܢ (CommonFXSema.isSaturated()) {
    ܐܢ (Result < Min)
      Result = std::move(Min);
    ܐܚܪܢܐ ܐܢ (Result > Max)
      Result = std::move(Max);
  } ܐܚܪܢܐ {
    Overflowed = Result < Min || Result > Max;
  }

  ܐܢ (Overflow)
    *Overflow = Overflowed;

  ܦܢܐ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, منطقي *Overflow) ثابت {
  APSInt ThisVal = Val;
  منطقي Overflowed = ܕܓܠܐ;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  ܐܢ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ܐܚܪܢܐ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).getValue().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).getValue().extOrTrunc(Wide);
  ܐܢ (Sema.isSaturated()) {
    ܐܢ (Result < Min)
      Result = std::move(Min);
    ܐܚܪܢܐ ܐܢ (Result > Max)
      Result = std::move(Max);
  } ܐܚܪܢܐ {
    Overflowed = Result < Min || Result > Max;
  }

  ܐܢ (Overflow)
    *Overflow = Overflowed;

  ܦܢܐ APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

فراغ APFixedPoint::toString(SmallVectorImpl<حرف> &Str) ثابت {
  APSInt Val = getValue();
  صحيح Lsb = getLsbWeight();
  صحيح OrigWidth = getWidth();

  ܐܢ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.push_back('.');
    Str.push_back('0');
    ܦܢܐ;
  }

  ܐܢ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(ܫܪܪܐ);
    Str.push_back('-');
  }

  صحيح Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.push_back('.');
  ܥܒܕ {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } ܟܡܐ (FractPart != 0);
}

فراغ APFixedPoint::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ܐܢ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ APFixedPoint::dump() ثابت { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(منطقي *Overflow) ثابت {
  ܐܢ (!isSaturated()) {
    ܐܢ (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.isMinSignedValue());
    ܦܢܐ APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  ܐܢ (Overflow)
    *Overflow = ܕܓܠܐ;

  ܐܢ (isSigned())
    ܦܢܐ Val.isMinSignedValue() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  ܐܚܪܢܐ
    ܦܢܐ APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, منطقي DstSign,
                                  منطقي *Overflow) ثابت {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::getMinValue(DstWidth, !DstSign);
  APSInt DstMax = APSInt::getMaxValue(DstWidth, !DstSign);

  ܐܢ (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } ܐܚܪܢܐ ܐܢ (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  ܐܢ (Overflow) {
    ܐܢ (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } ܐܚܪܢܐ ܐܢ (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } ܐܚܪܢܐ {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  ܦܢܐ Result.extOrTrunc(DstWidth);
}

ثابت fltSemantics *APFixedPoint::promoteFloatSemantics(ثابت fltSemantics *S) {
  ܐܢ (S == &APFloat::BFloat())
    ܦܢܐ &APFloat::IEEEdouble();
  ܐܚܪܢܐ ܐܢ (S == &APFloat::IEEEhalf())
    ܦܢܐ &APFloat::IEEEsingle();
  ܐܚܪܢܐ ܐܢ (S == &APFloat::IEEEsingle())
    ܦܢܐ &APFloat::IEEEdouble();
  ܐܚܪܢܐ ܐܢ (S == &APFloat::IEEEdouble())
    ܦܢܐ &APFloat::IEEEquad();
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
  ܟܡܐ (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (فراغ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  منطقي Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  ܐܢ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ܦܢܐ Flt;
}

APFixedPoint APFixedPoint::getFromIntValue(ثابت APSInt &Value,
                                           ثابت FixedPointSemantics &DstFXSema,
                                           منطقي *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      Value.getBitWidth(), Value.isSigned());
  ܦܢܐ APFixedPoint(Value, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::getFromFloatValue(ثابت APFloat &Value,
                                ثابت FixedPointSemantics &DstFXSema,
                                منطقي *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ثابت fltSemantics &FloatSema = Value.getSemantics();

  ܐܢ (Value.isNaN()) {
    // Handle NaN immediately.
    ܐܢ (Overflow)
      *Overflow = ܫܪܪܐ;
    ܦܢܐ APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  ܟܡܐ (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = Value;

  منطقي Ignored;
  ܐܢ (&FloatSema != OpSema)
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
  منطقي Overflowed = ܕܓܠܐ;
  ܐܢ (DstFXSema.isSaturated()) {
    ܐܢ (Val > FloatMax)
      Res = getMax(DstFXSema).getValue();
    ܐܚܪܢܐ ܐܢ (Val < FloatMin)
      Res = getMin(DstFXSema).getValue();
  } ܐܚܪܢܐ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ܐܢ (Overflow)
    *Overflow = Overflowed;

  ܦܢܐ APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
