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

#অন্তর্ভাৱ "llvm/ADT/APFixedPoint.h"
#অন্তর্ভাৱ "llvm/ADT/APFloat.h"

#অন্তর্ভাৱ <cmath>

নামাকাশ llvm {

শূন্য FixedPointSemantics::print(llvm::raw_ostream &OS) ধ্রুৱ {
  OS << "width=" << getWidth() << ", ";
  যদি (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ধ্রুৱ {
  ঘূৰাই llvm::bit_cast<uint32_t>(*এতত্);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, মিছা, মিছা, মিছা);
  std::memcpy(&F, &I, পরিমাণ(F));
  ঘূৰাই F;
}

APFixedPoint APFixedPoint::convert(ধ্রুৱ FixedPointSemantics &DstSema,
                                   তার্কিক *Overflow) ধ্রুৱ {
  APSInt NewVal = Val;
  পূর্ণাঙ্ক RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  যদি (Overflow)
    *Overflow = মিছা;

  যদি (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  যদি (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    যদি (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    অন্যথা যদি (Overflow)
      *Overflow = সঁচা;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  যদি (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    যদি (DstSema.isSaturated())
      NewVal = 0;
    অন্যথা যদি (Overflow)
      *Overflow = সঁচা;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  ঘূৰাই APFixedPoint(NewVal, DstSema);
}

পূর্ণাঙ্ক APFixedPoint::compare(ধ্রুৱ APFixedPoint &Other) ধ্রুৱ {
  APSInt ThisVal = get_মান();
  APSInt OtherVal = Other.get_মান();
  তার্কিক ThisSigned = Val.isSigned();
  তার্কিক OtherSigned = OtherVal.isSigned();

  পূর্ণাঙ্ক CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  পূর্ণাঙ্ক CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  যদি (ThisSigned && OtherSigned) {
    যদি (ThisVal.sgt(OtherVal))
      ঘূৰাই 1;
    অন্যথা যদি (ThisVal.slt(OtherVal))
      ঘূৰাই -1;
  } অন্যথা যদি (!ThisSigned && !OtherSigned) {
    যদি (ThisVal.ugt(OtherVal))
      ঘূৰাই 1;
    অন্যথা যদি (ThisVal.ult(OtherVal))
      ঘূৰাই -1;
  } অন্যথা যদি (ThisSigned && !OtherSigned) {
    যদি (ThisVal.isSignBitSet())
      ঘূৰাই -1;
    অন্যথা যদি (ThisVal.ugt(OtherVal))
      ঘূৰাই 1;
    অন্যথা যদি (ThisVal.ult(OtherVal))
      ঘূৰাই -1;
  } অন্যথা {
    // !ThisSigned && OtherSigned
    যদি (OtherVal.isSignBitSet())
      ঘূৰাই 1;
    অন্যথা যদি (ThisVal.ugt(OtherVal))
      ঘূৰাই 1;
    অন্যথা যদি (ThisVal.ult(OtherVal))
      ঘূৰাই -1;
  }

  ঘূৰাই 0;
}

APFixedPoint APFixedPoint::getMax(ধ্রুৱ FixedPointSemantics &Sema) {
  তার্কিক IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_মান(Sema.getWidth(), IsUnsigned);
  যদি (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ঘূৰাই APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ধ্রুৱ FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_মান(Sema.getWidth(), !Sema.isSigned());
  ঘূৰাই APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ধ্রুৱ FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  ঘূৰাই APFixedPoint(Val, Sema);
}

তার্কিক FixedPointSemantics::fitsInFloatSemantics(
    ধ্রুৱ fltSemantics &FloatSema) ধ্রুৱ {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*এতত্).get_মান();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  যদি ((Status & APFloat::opOverflow) || !isSigned())
    ঘূৰাই !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*এতত্).get_মান();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  ঘূৰাই !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ধ্রুৱ FixedPointSemantics &Other) ধ্রুৱ {
  পূর্ণাঙ্ক CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  পূর্ণাঙ্ক CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  তার্কিক ResultIsSigned = isSigned() || Other.isSigned();
  তার্কিক ResultIsSaturated = isSaturated() || Other.isSaturated();
  তার্কিক ResultHasUnsignedPadding = মিছা;
  যদি (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  যদি (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  ঘূৰাই FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ধ্রুৱ APFixedPoint &Other,
                               তার্কিক *Overflow) ধ্রুৱ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  তার্কিক Overflowed = মিছা;

  APSInt Result;
  যদি (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } অন্যথা {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ঘূৰাই APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ধ্রুৱ APFixedPoint &Other,
                               তার্কিক *Overflow) ধ্রুৱ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  তার্কিক Overflowed = মিছা;

  APSInt Result;
  যদি (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } অন্যথা {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ঘূৰাই APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ধ্রুৱ APFixedPoint &Other,
                               তার্কিক *Overflow) ধ্রুৱ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  তার্কিক Overflowed = মিছা;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  যদি (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } অন্যথা {
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
  যদি (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  অন্যথা
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  প্রতিজ্ঞা(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  যদি (CommonFXSema.isSaturated()) {
    যদি (Result < Min)
      Result = std::move(Min);
    অন্যথা যদি (Result > Max)
      Result = std::move(Max);
  } অন্যথা {
    Overflowed = Result < Min || Result > Max;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ঘূৰাই APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ধ্রুৱ APFixedPoint &Other,
                               তার্কিক *Overflow) ধ্রুৱ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  তার্কিক Overflowed = মিছা;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  যদি (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } অন্যথা {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  যদি (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  অন্যথা যদি (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  যদি (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    যদি (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } অন্যথা {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  যদি (CommonFXSema.isSaturated()) {
    যদি (Result < Min)
      Result = std::move(Min);
    অন্যথা যদি (Result > Max)
      Result = std::move(Max);
  } অন্যথা {
    Overflowed = Result < Min || Result > Max;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ঘূৰাই APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, তার্কিক *Overflow) ধ্রুৱ {
  APSInt ThisVal = Val;
  তার্কিক Overflowed = মিছা;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  যদি (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  অন্যথা
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_মান().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_মান().extOrTrunc(Wide);
  যদি (Sema.isSaturated()) {
    যদি (Result < Min)
      Result = std::move(Min);
    অন্যথা যদি (Result > Max)
      Result = std::move(Max);
  } অন্যথা {
    Overflowed = Result < Min || Result > Max;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ঘূৰাই APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

শূন্য APFixedPoint::toString(SmallVectorImpl<ৱর্ণ> &Str) ধ্রুৱ {
  APSInt Val = get_মান();
  পূর্ণাঙ্ক Lsb = getLsbWeight();
  পূর্ণাঙ্ক OrigWidth = getWidth();

  যদি (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ঠেলো_back('.');
    Str.ঠেলো_back('0');
    ঘূৰাই;
  }

  যদি (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(সঁচা);
    Str.ঠেলো_back('-');
  }

  পূর্ণাঙ্ক Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ঠেলো_back('.');
  কৰা {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } যেতিয়ালৈ (FractPart != 0);
}

শূন্য APFixedPoint::print(raw_ostream &OS) ধ্রুৱ {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য APFixedPoint::dump() ধ্রুৱ { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(তার্কিক *Overflow) ধ্রুৱ {
  যদি (!isSaturated()) {
    যদি (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_মান());
    ঘূৰাই APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  যদি (Overflow)
    *Overflow = মিছা;

  যদি (isSigned())
    ঘূৰাই Val.is_Min_Signed_মান() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  অন্যথা
    ঘূৰাই APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, তার্কিক DstSign,
                                  তার্কিক *Overflow) ধ্রুৱ {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_মান(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_মান(DstWidth, !DstSign);

  যদি (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } অন্যথা যদি (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  যদি (Overflow) {
    যদি (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } অন্যথা যদি (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } অন্যথা {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  ঘূৰাই Result.extOrTrunc(DstWidth);
}

ধ্রুৱ fltSemantics *APFixedPoint::promoteFloatSemantics(ধ্রুৱ fltSemantics *S) {
  যদি (S == &APFloat::BFloat())
    ঘূৰাই &APFloat::IEEEdouble();
  অন্যথা যদি (S == &APFloat::IEEEhalf())
    ঘূৰাই &APFloat::IEEEsingle();
  অন্যথা যদি (S == &APFloat::IEEEsingle())
    ঘূৰাই &APFloat::IEEEdouble();
  অন্যথা যদি (S == &APFloat::IEEEdouble())
    ঘূৰাই &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(ধ্রুৱ fltSemantics &FloatSema) ধ্রুৱ {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ধ্রুৱ fltSemantics *OpSema = &FloatSema;
  যেতিয়ালৈ (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (শূন্য)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  তার্কিক Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  যদি (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ঘূৰাই Flt;
}

APFixedPoint APFixedPoint::get_From_Int_মান(ধ্রুৱ APSInt &মান,
                                           ধ্রুৱ FixedPointSemantics &DstFXSema,
                                           তার্কিক *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      মান.getBitWidth(), মান.isSigned());
  ঘূৰাই APFixedPoint(মান, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_মান(ধ্রুৱ APFloat &মান,
                                ধ্রুৱ FixedPointSemantics &DstFXSema,
                                তার্কিক *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ধ্রুৱ fltSemantics &FloatSema = মান.getSemantics();

  যদি (মান.isNaN()) {
    // Handle NaN immediately.
    যদি (Overflow)
      *Overflow = সঁচা;
    ঘূৰাই APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ধ্রুৱ fltSemantics *OpSema = &FloatSema;
  যেতিয়ালৈ (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = মান;

  তার্কিক Ignored;
  যদি (&FloatSema != OpSema)
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
  তার্কিক Overflowed = মিছা;
  যদি (DstFXSema.isSaturated()) {
    যদি (Val > FloatMax)
      Res = getMax(DstFXSema).get_মান();
    অন্যথা যদি (Val < FloatMin)
      Res = getMin(DstFXSema).get_মান();
  } অন্যথা {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ঘূৰাই APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
