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
  𐡄𐡍 (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ثابت {
  أرجع llvm::bit_cast<uint32_t>(*هذا);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, خطأ, خطأ, خطأ);
  std::memcpy(&F, &I, حجم(F));
  أرجع F;
}

APFixedPoint APFixedPoint::convert(ثابت FixedPointSemantics &DstSema,
                                   منطقي *Overflow) ثابت {
  APSInt NewVal = Val;
  صحيح RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  𐡄𐡍 (Overflow)
    *Overflow = خطأ;

  𐡄𐡍 (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  𐡄𐡍 (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    𐡄𐡍 (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    وإلا 𐡄𐡍 (Overflow)
      *Overflow = 𐡒𐡔𐡈;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  𐡄𐡍 (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    𐡄𐡍 (DstSema.isSaturated())
      NewVal = 0;
    وإلا 𐡄𐡍 (Overflow)
      *Overflow = 𐡒𐡔𐡈;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  أرجع APFixedPoint(NewVal, DstSema);
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

  𐡄𐡍 (ThisSigned && OtherSigned) {
    𐡄𐡍 (ThisVal.sgt(OtherVal))
      أرجع 1;
    وإلا 𐡄𐡍 (ThisVal.slt(OtherVal))
      أرجع -1;
  } وإلا 𐡄𐡍 (!ThisSigned && !OtherSigned) {
    𐡄𐡍 (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا 𐡄𐡍 (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا 𐡄𐡍 (ThisSigned && !OtherSigned) {
    𐡄𐡍 (ThisVal.isSignBitSet())
      أرجع -1;
    وإلا 𐡄𐡍 (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا 𐡄𐡍 (ThisVal.ult(OtherVal))
      أرجع -1;
  } وإلا {
    // !ThisSigned && OtherSigned
    𐡄𐡍 (OtherVal.isSignBitSet())
      أرجع 1;
    وإلا 𐡄𐡍 (ThisVal.ugt(OtherVal))
      أرجع 1;
    وإلا 𐡄𐡍 (ThisVal.ult(OtherVal))
      أرجع -1;
  }

  أرجع 0;
}

APFixedPoint APFixedPoint::getMax(ثابت FixedPointSemantics &Sema) {
  منطقي IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::getMaxValue(Sema.getWidth(), IsUnsigned);
  𐡄𐡍 (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  أرجع APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ثابت FixedPointSemantics &Sema) {
  auto Val = APSInt::getMinValue(Sema.getWidth(), !Sema.isSigned());
  أرجع APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ثابت FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  أرجع APFixedPoint(Val, Sema);
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
  𐡄𐡍 ((Status & APFloat::opOverflow) || !isSigned())
    أرجع !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*هذا).getValue();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  أرجع !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ثابت FixedPointSemantics &Other) ثابت {
  صحيح CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  صحيح CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  منطقي ResultIsSigned = isSigned() || Other.isSigned();
  منطقي ResultIsSaturated = isSaturated() || Other.isSaturated();
  منطقي ResultHasUnsignedPadding = خطأ;
  𐡄𐡍 (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  𐡄𐡍 (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  أرجع FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = خطأ;

  APSInt Result;
  𐡄𐡍 (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } وإلا {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  𐡄𐡍 (Overflow)
    *Overflow = Overflowed;

  أرجع APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = خطأ;

  APSInt Result;
  𐡄𐡍 (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } وإلا {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  𐡄𐡍 (Overflow)
    *Overflow = Overflowed;

  أرجع APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  𐡄𐡍 (CommonFXSema.isSigned()) {
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
  APSInt Result;
  𐡄𐡍 (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  وإلا
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
  𐡄𐡍 (CommonFXSema.isSaturated()) {
    𐡄𐡍 (Result < Min)
      Result = std::move(Min);
    وإلا 𐡄𐡍 (Result > Max)
      Result = std::move(Max);
  } وإلا {
    Overflowed = Result < Min || Result > Max;
  }

  𐡄𐡍 (Overflow)
    *Overflow = Overflowed;

  أرجع APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ثابت APFixedPoint &Other,
                               منطقي *Overflow) ثابت {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  منطقي Overflowed = خطأ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  𐡄𐡍 (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } وإلا {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  𐡄𐡍 (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  وإلا 𐡄𐡍 (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  𐡄𐡍 (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    𐡄𐡍 (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } وإلا {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  𐡄𐡍 (CommonFXSema.isSaturated()) {
    𐡄𐡍 (Result < Min)
      Result = std::move(Min);
    وإلا 𐡄𐡍 (Result > Max)
      Result = std::move(Max);
  } وإلا {
    Overflowed = Result < Min || Result > Max;
  }

  𐡄𐡍 (Overflow)
    *Overflow = Overflowed;

  أرجع APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, منطقي *Overflow) ثابت {
  APSInt ThisVal = Val;
  منطقي Overflowed = خطأ;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  𐡄𐡍 (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  وإلا
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).getValue().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).getValue().extOrTrunc(Wide);
  𐡄𐡍 (Sema.isSaturated()) {
    𐡄𐡍 (Result < Min)
      Result = std::move(Min);
    وإلا 𐡄𐡍 (Result > Max)
      Result = std::move(Max);
  } وإلا {
    Overflowed = Result < Min || Result > Max;
  }

  𐡄𐡍 (Overflow)
    *Overflow = Overflowed;

  أرجع APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

فراغ APFixedPoint::toString(SmallVectorImpl<حرف> &Str) ثابت {
  APSInt Val = getValue();
  صحيح Lsb = getLsbWeight();
  صحيح OrigWidth = getWidth();

  𐡄𐡍 (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.push_back('.');
    Str.push_back('0');
    أرجع;
  }

  𐡄𐡍 (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(𐡒𐡔𐡈);
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
  𐡏𐡁𐡃 {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } طالما (FractPart != 0);
}

فراغ APFixedPoint::print(raw_ostream &OS) ثابت {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#𐡄𐡍 !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD فراغ APFixedPoint::dump() ثابت { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(منطقي *Overflow) ثابت {
  𐡄𐡍 (!isSaturated()) {
    𐡄𐡍 (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.isMinSignedValue());
    أرجع APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  𐡄𐡍 (Overflow)
    *Overflow = خطأ;

  𐡄𐡍 (isSigned())
    أرجع Val.isMinSignedValue() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  وإلا
    أرجع APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, منطقي DstSign,
                                  منطقي *Overflow) ثابت {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::getMinValue(DstWidth, !DstSign);
  APSInt DstMax = APSInt::getMaxValue(DstWidth, !DstSign);

  𐡄𐡍 (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } وإلا 𐡄𐡍 (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  𐡄𐡍 (Overflow) {
    𐡄𐡍 (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } وإلا 𐡄𐡍 (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } وإلا {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  أرجع Result.extOrTrunc(DstWidth);
}

ثابت fltSemantics *APFixedPoint::promoteFloatSemantics(ثابت fltSemantics *S) {
  𐡄𐡍 (S == &APFloat::BFloat())
    أرجع &APFloat::IEEEdouble();
  وإلا 𐡄𐡍 (S == &APFloat::IEEEhalf())
    أرجع &APFloat::IEEEsingle();
  وإلا 𐡄𐡍 (S == &APFloat::IEEEsingle())
    أرجع &APFloat::IEEEdouble();
  وإلا 𐡄𐡍 (S == &APFloat::IEEEdouble())
    أرجع &APFloat::IEEEquad();
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
  طالما (!Sema.fitsInFloatSemantics(*OpSema))
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

  𐡄𐡍 (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  أرجع Flt;
}

APFixedPoint APFixedPoint::getFromIntValue(ثابت APSInt &Value,
                                           ثابت FixedPointSemantics &DstFXSema,
                                           منطقي *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      Value.getBitWidth(), Value.isSigned());
  أرجع APFixedPoint(Value, IntFXSema).convert(DstFXSema, Overflow);
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

  𐡄𐡍 (Value.isNaN()) {
    // Handle NaN immediately.
    𐡄𐡍 (Overflow)
      *Overflow = 𐡒𐡔𐡈;
    أرجع APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ثابت fltSemantics *OpSema = &FloatSema;
  طالما (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = Value;

  منطقي Ignored;
  𐡄𐡍 (&FloatSema != OpSema)
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
  منطقي Overflowed = خطأ;
  𐡄𐡍 (DstFXSema.isSaturated()) {
    𐡄𐡍 (Val > FloatMax)
      Res = getMax(DstFXSema).getValue();
    وإلا 𐡄𐡍 (Val < FloatMin)
      Res = getMin(DstFXSema).getValue();
  } وإلا {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  𐡄𐡍 (Overflow)
    *Overflow = Overflowed;

  أرجع APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
