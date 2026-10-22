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

#அந்தர்பாவ "llvm/ADT/APFixedPoint.h"
#அந்தர்பாவ "llvm/ADT/APFloat.h"

#அந்தர்பாவ <cmath>

நாமாகாஶ llvm {

வெறுமை FixedPointSemantics::print(llvm::raw_ostream &OS) மாறிலி {
  OS << "width=" << getWidth() << ", ";
  எனில் (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() மாறிலி {
  திரும்பு llvm::bit_cast<uint32_t>(*இது);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, பொய், பொய், பொய்);
  std::memcpy(&F, &I, அளவு(F));
  திரும்பு F;
}

APFixedPoint APFixedPoint::convert(மாறிலி FixedPointSemantics &DstSema,
                                   தருக்க *Overflow) மாறிலி {
  APSInt NewVal = Val;
  முழு RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  எனில் (Overflow)
    *Overflow = பொய்;

  எனில் (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  எனில் (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    எனில் (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    இல்லையெனில் எனில் (Overflow)
      *Overflow = உண்மை;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  எனில் (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    எனில் (DstSema.isSaturated())
      NewVal = 0;
    இல்லையெனில் எனில் (Overflow)
      *Overflow = உண்மை;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  திரும்பு APFixedPoint(NewVal, DstSema);
}

முழு APFixedPoint::compare(மாறிலி APFixedPoint &Other) மாறிலி {
  APSInt ThisVal = get_மதிப்பு();
  APSInt OtherVal = Other.get_மதிப்பு();
  தருக்க ThisSigned = Val.isSigned();
  தருக்க OtherSigned = OtherVal.isSigned();

  முழு CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  முழு CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  எனில் (ThisSigned && OtherSigned) {
    எனில் (ThisVal.sgt(OtherVal))
      திரும்பு 1;
    இல்லையெனில் எனில் (ThisVal.slt(OtherVal))
      திரும்பு -1;
  } இல்லையெனில் எனில் (!ThisSigned && !OtherSigned) {
    எனில் (ThisVal.ugt(OtherVal))
      திரும்பு 1;
    இல்லையெனில் எனில் (ThisVal.ult(OtherVal))
      திரும்பு -1;
  } இல்லையெனில் எனில் (ThisSigned && !OtherSigned) {
    எனில் (ThisVal.isSignBitSet())
      திரும்பு -1;
    இல்லையெனில் எனில் (ThisVal.ugt(OtherVal))
      திரும்பு 1;
    இல்லையெனில் எனில் (ThisVal.ult(OtherVal))
      திரும்பு -1;
  } இல்லையெனில் {
    // !ThisSigned && OtherSigned
    எனில் (OtherVal.isSignBitSet())
      திரும்பு 1;
    இல்லையெனில் எனில் (ThisVal.ugt(OtherVal))
      திரும்பு 1;
    இல்லையெனில் எனில் (ThisVal.ult(OtherVal))
      திரும்பு -1;
  }

  திரும்பு 0;
}

APFixedPoint APFixedPoint::getMax(மாறிலி FixedPointSemantics &Sema) {
  தருக்க IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_மதிப்பு(Sema.getWidth(), IsUnsigned);
  எனில் (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  திரும்பு APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(மாறிலி FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_மதிப்பு(Sema.getWidth(), !Sema.isSigned());
  திரும்பு APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(மாறிலி FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  திரும்பு APFixedPoint(Val, Sema);
}

தருக்க FixedPointSemantics::fitsInFloatSemantics(
    மாறிலி fltSemantics &FloatSema) மாறிலி {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*இது).get_மதிப்பு();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  எனில் ((Status & APFloat::opOverflow) || !isSigned())
    திரும்பு !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*இது).get_மதிப்பு();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  திரும்பு !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    மாறிலி FixedPointSemantics &Other) மாறிலி {
  முழு CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  முழு CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  தருக்க ResultIsSigned = isSigned() || Other.isSigned();
  தருக்க ResultIsSaturated = isSaturated() || Other.isSaturated();
  தருக்க ResultHasUnsignedPadding = பொய்;
  எனில் (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  எனில் (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  திரும்பு FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(மாறிலி APFixedPoint &Other,
                               தருக்க *Overflow) மாறிலி {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_மதிப்பு();
  APSInt OtherVal = ConvertedOther.get_மதிப்பு();
  தருக்க Overflowed = பொய்;

  APSInt Result;
  எனில் (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } இல்லையெனில் {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  எனில் (Overflow)
    *Overflow = Overflowed;

  திரும்பு APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(மாறிலி APFixedPoint &Other,
                               தருக்க *Overflow) மாறிலி {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_மதிப்பு();
  APSInt OtherVal = ConvertedOther.get_மதிப்பு();
  தருக்க Overflowed = பொய்;

  APSInt Result;
  எனில் (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } இல்லையெனில் {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  எனில் (Overflow)
    *Overflow = Overflowed;

  திரும்பு APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(மாறிலி APFixedPoint &Other,
                               தருக்க *Overflow) மாறிலி {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_மதிப்பு();
  APSInt OtherVal = ConvertedOther.get_மதிப்பு();
  தருக்க Overflowed = பொய்;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  எனில் (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } இல்லையெனில் {
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
  எனில் (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  இல்லையெனில்
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  ப்ரதிஜ்ஞா(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_மதிப்பு()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_மதிப்பு()
                                                 .extOrTrunc(Wide);
  எனில் (CommonFXSema.isSaturated()) {
    எனில் (Result < Min)
      Result = std::move(Min);
    இல்லையெனில் எனில் (Result > Max)
      Result = std::move(Max);
  } இல்லையெனில் {
    Overflowed = Result < Min || Result > Max;
  }

  எனில் (Overflow)
    *Overflow = Overflowed;

  திரும்பு APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(மாறிலி APFixedPoint &Other,
                               தருக்க *Overflow) மாறிலி {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_மதிப்பு();
  APSInt OtherVal = ConvertedOther.get_மதிப்பு();
  தருக்க Overflowed = பொய்;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  எனில் (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } இல்லையெனில் {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  எனில் (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  இல்லையெனில் எனில் (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  எனில் (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    எனில் (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } இல்லையெனில் {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_மதிப்பு()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_மதிப்பு()
                                                 .extOrTrunc(Wide);
  எனில் (CommonFXSema.isSaturated()) {
    எனில் (Result < Min)
      Result = std::move(Min);
    இல்லையெனில் எனில் (Result > Max)
      Result = std::move(Max);
  } இல்லையெனில் {
    Overflowed = Result < Min || Result > Max;
  }

  எனில் (Overflow)
    *Overflow = Overflowed;

  திரும்பு APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, தருக்க *Overflow) மாறிலி {
  APSInt ThisVal = Val;
  தருக்க Overflowed = பொய்;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  எனில் (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  இல்லையெனில்
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_மதிப்பு().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_மதிப்பு().extOrTrunc(Wide);
  எனில் (Sema.isSaturated()) {
    எனில் (Result < Min)
      Result = std::move(Min);
    இல்லையெனில் எனில் (Result > Max)
      Result = std::move(Max);
  } இல்லையெனில் {
    Overflowed = Result < Min || Result > Max;
  }

  எனில் (Overflow)
    *Overflow = Overflowed;

  திரும்பு APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

வெறுமை APFixedPoint::toString(SmallVectorImpl<எழுத்து> &Str) மாறிலி {
  APSInt Val = get_மதிப்பு();
  முழு Lsb = getLsbWeight();
  முழு OrigWidth = getWidth();

  எனில் (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.தள்ளு_back('.');
    Str.தள்ளு_back('0');
    திரும்பு;
  }

  எனில் (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(உண்மை);
    Str.தள்ளு_back('-');
  }

  முழு Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.தள்ளு_back('.');
  செய் {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } போது (FractPart != 0);
}

வெறுமை APFixedPoint::print(raw_ostream &OS) மாறிலி {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#எனில் !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD வெறுமை APFixedPoint::dump() மாறிலி { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(தருக்க *Overflow) மாறிலி {
  எனில் (!isSaturated()) {
    எனில் (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_மதிப்பு());
    திரும்பு APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  எனில் (Overflow)
    *Overflow = பொய்;

  எனில் (isSigned())
    திரும்பு Val.is_Min_Signed_மதிப்பு() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  இல்லையெனில்
    திரும்பு APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, தருக்க DstSign,
                                  தருக்க *Overflow) மாறிலி {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_மதிப்பு(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_மதிப்பு(DstWidth, !DstSign);

  எனில் (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } இல்லையெனில் எனில் (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  எனில் (Overflow) {
    எனில் (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } இல்லையெனில் எனில் (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } இல்லையெனில் {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  திரும்பு Result.extOrTrunc(DstWidth);
}

மாறிலி fltSemantics *APFixedPoint::promoteFloatSemantics(மாறிலி fltSemantics *S) {
  எனில் (S == &APFloat::BFloat())
    திரும்பு &APFloat::IEEEdouble();
  இல்லையெனில் எனில் (S == &APFloat::IEEEhalf())
    திரும்பு &APFloat::IEEEsingle();
  இல்லையெனில் எனில் (S == &APFloat::IEEEsingle())
    திரும்பு &APFloat::IEEEdouble();
  இல்லையெனில் எனில் (S == &APFloat::IEEEdouble())
    திரும்பு &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(மாறிலி fltSemantics &FloatSema) மாறிலி {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  மாறிலி fltSemantics *OpSema = &FloatSema;
  போது (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (வெறுமை)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  தருக்க Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  எனில் (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  திரும்பு Flt;
}

APFixedPoint APFixedPoint::get_From_Int_மதிப்பு(மாறிலி APSInt &மதிப்பு,
                                           மாறிலி FixedPointSemantics &DstFXSema,
                                           தருக்க *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      மதிப்பு.getBitWidth(), மதிப்பு.isSigned());
  திரும்பு APFixedPoint(மதிப்பு, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_மதிப்பு(மாறிலி APFloat &மதிப்பு,
                                மாறிலி FixedPointSemantics &DstFXSema,
                                தருக்க *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  மாறிலி fltSemantics &FloatSema = மதிப்பு.getSemantics();

  எனில் (மதிப்பு.isNaN()) {
    // Handle NaN immediately.
    எனில் (Overflow)
      *Overflow = உண்மை;
    திரும்பு APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  மாறிலி fltSemantics *OpSema = &FloatSema;
  போது (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = மதிப்பு;

  தருக்க Ignored;
  எனில் (&FloatSema != OpSema)
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
  தருக்க Overflowed = பொய்;
  எனில் (DstFXSema.isSaturated()) {
    எனில் (Val > FloatMax)
      Res = getMax(DstFXSema).get_மதிப்பு();
    இல்லையெனில் எனில் (Val < FloatMin)
      Res = getMin(DstFXSema).get_மதிப்பு();
  } இல்லையெனில் {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  எனில் (Overflow)
    *Overflow = Overflowed;

  திரும்பு APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
