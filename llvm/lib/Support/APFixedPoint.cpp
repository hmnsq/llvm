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

#অন্তর্ভাব "llvm/ADT/APFixedPoint.h"
#অন্তর্ভাব "llvm/ADT/APFloat.h"

#অন্তর্ভাব <cmath>

নামাকাশ llvm {

শূন্য FixedPointSemantics::print(llvm::raw_ostream &OS) ধ্রুবক {
  OS << "width=" << getWidth() << ", ";
  যদি (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ধ্রুবক {
  ফেরত llvm::bit_cast<uint32_t>(*এই);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, মিথ্যা, মিথ্যা, মিথ্যা);
  std::memcpy(&F, &I, আকার(F));
  ফেরত F;
}

APFixedPoint APFixedPoint::convert(ধ্রুবক FixedPointSemantics &DstSema,
                                   যৌক্তিক *Overflow) ধ্রুবক {
  APSInt NewVal = Val;
  পূর্ণসংখ্যা RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  যদি (Overflow)
    *Overflow = মিথ্যা;

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
      *Overflow = সত্য;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  যদি (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    যদি (DstSema.isSaturated())
      NewVal = 0;
    অন্যথা যদি (Overflow)
      *Overflow = সত্য;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  ফেরত APFixedPoint(NewVal, DstSema);
}

পূর্ণসংখ্যা APFixedPoint::compare(ধ্রুবক APFixedPoint &Other) ধ্রুবক {
  APSInt ThisVal = get_মান();
  APSInt OtherVal = Other.get_মান();
  যৌক্তিক ThisSigned = Val.isSigned();
  যৌক্তিক OtherSigned = OtherVal.isSigned();

  পূর্ণসংখ্যা CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  পূর্ণসংখ্যা CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  যদি (ThisSigned && OtherSigned) {
    যদি (ThisVal.sgt(OtherVal))
      ফেরত 1;
    অন্যথা যদি (ThisVal.slt(OtherVal))
      ফেরত -1;
  } অন্যথা যদি (!ThisSigned && !OtherSigned) {
    যদি (ThisVal.ugt(OtherVal))
      ফেরত 1;
    অন্যথা যদি (ThisVal.ult(OtherVal))
      ফেরত -1;
  } অন্যথা যদি (ThisSigned && !OtherSigned) {
    যদি (ThisVal.isSignBitSet())
      ফেরত -1;
    অন্যথা যদি (ThisVal.ugt(OtherVal))
      ফেরত 1;
    অন্যথা যদি (ThisVal.ult(OtherVal))
      ফেরত -1;
  } অন্যথা {
    // !ThisSigned && OtherSigned
    যদি (OtherVal.isSignBitSet())
      ফেরত 1;
    অন্যথা যদি (ThisVal.ugt(OtherVal))
      ফেরত 1;
    অন্যথা যদি (ThisVal.ult(OtherVal))
      ফেরত -1;
  }

  ফেরত 0;
}

APFixedPoint APFixedPoint::getMax(ধ্রুবক FixedPointSemantics &Sema) {
  যৌক্তিক IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_মান(Sema.getWidth(), IsUnsigned);
  যদি (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ফেরত APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ধ্রুবক FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_মান(Sema.getWidth(), !Sema.isSigned());
  ফেরত APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ধ্রুবক FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  ফেরত APFixedPoint(Val, Sema);
}

যৌক্তিক FixedPointSemantics::fitsInFloatSemantics(
    ধ্রুবক fltSemantics &FloatSema) ধ্রুবক {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*এই).get_মান();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  যদি ((Status & APFloat::opOverflow) || !isSigned())
    ফেরত !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*এই).get_মান();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  ফেরত !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ধ্রুবক FixedPointSemantics &Other) ধ্রুবক {
  পূর্ণসংখ্যা CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  পূর্ণসংখ্যা CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  যৌক্তিক ফলাফল_Is_Signed = isSigned() || Other.isSigned();
  যৌক্তিক ফলাফল_Is_Saturated = isSaturated() || Other.isSaturated();
  যৌক্তিক ফলাফল_Has_Unsigned_Padding = মিথ্যা;
  যদি (!ফলাফল_Is_Signed) {
    // Both are unsigned.
    ফলাফল_Has_Unsigned_Padding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ফলাফল_Is_Saturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  যদি (ফলাফল_Is_Signed || ফলাফল_Has_Unsigned_Padding)
    CommonWidth++;

  ফেরত FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ফলাফল_Is_Signed,
                             ফলাফল_Is_Saturated, ফলাফল_Has_Unsigned_Padding);
}

APFixedPoint APFixedPoint::add(ধ্রুবক APFixedPoint &Other,
                               যৌক্তিক *Overflow) ধ্রুবক {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  যৌক্তিক Overflowed = মিথ্যা;

  APSInt ফলাফল;
  যদি (CommonFXSema.isSaturated()) {
    ফলাফল = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } অন্যথা {
    ফলাফল = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ফেরত APFixedPoint(ফলাফল, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ধ্রুবক APFixedPoint &Other,
                               যৌক্তিক *Overflow) ধ্রুবক {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  যৌক্তিক Overflowed = মিথ্যা;

  APSInt ফলাফল;
  যদি (CommonFXSema.isSaturated()) {
    ফলাফল = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } অন্যথা {
    ফলাফল = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ফেরত APFixedPoint(ফলাফল, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ধ্রুবক APFixedPoint &Other,
                               যৌক্তিক *Overflow) ধ্রুবক {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  যৌক্তিক Overflowed = মিথ্যা;

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
  APSInt ফলাফল;
  যদি (CommonFXSema.isSigned())
    ফলাফল = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  অন্যথা
    ফলাফল = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  প্রতিজ্ঞা(!Overflowed && "Full multiplication cannot overflow!");
  ফলাফল.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  যদি (CommonFXSema.isSaturated()) {
    যদি (ফলাফল < Min)
      ফলাফল = std::move(Min);
    অন্যথা যদি (ফলাফল > Max)
      ফলাফল = std::move(Max);
  } অন্যথা {
    Overflowed = ফলাফল < Min || ফলাফল > Max;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ফেরত APFixedPoint(ফলাফল.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ধ্রুবক APFixedPoint &Other,
                               যৌক্তিক *Overflow) ধ্রুবক {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_মান();
  APSInt OtherVal = ConvertedOther.get_মান();
  যৌক্তিক Overflowed = মিথ্যা;

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
  APSInt ফলাফল;
  যদি (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, ফলাফল, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    যদি (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      ফলাফল = ফলাফল - 1;
  } অন্যথা {
    ফলাফল = ThisVal.udiv(OtherVal);
  }
  ফলাফল.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_মান()
                                                 .extOrTrunc(Wide);
  যদি (CommonFXSema.isSaturated()) {
    যদি (ফলাফল < Min)
      ফলাফল = std::move(Min);
    অন্যথা যদি (ফলাফল > Max)
      ফলাফল = std::move(Max);
  } অন্যথা {
    Overflowed = ফলাফল < Min || ফলাফল > Max;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ফেরত APFixedPoint(ফলাফল.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, যৌক্তিক *Overflow) ধ্রুবক {
  APSInt ThisVal = Val;
  যৌক্তিক Overflowed = মিথ্যা;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  যদি (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  অন্যথা
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt ফলাফল = ThisVal << Amt;
  ফলাফল.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_মান().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_মান().extOrTrunc(Wide);
  যদি (Sema.isSaturated()) {
    যদি (ফলাফল < Min)
      ফলাফল = std::move(Min);
    অন্যথা যদি (ফলাফল > Max)
      ফলাফল = std::move(Max);
  } অন্যথা {
    Overflowed = ফলাফল < Min || ফলাফল > Max;
  }

  যদি (Overflow)
    *Overflow = Overflowed;

  ফেরত APFixedPoint(ফলাফল.sextOrTrunc(Sema.getWidth()), Sema);
}

শূন্য APFixedPoint::toString(SmallVectorImpl<অক্ষর> &Str) ধ্রুবক {
  APSInt Val = get_মান();
  পূর্ণসংখ্যা Lsb = getLsbWeight();
  পূর্ণসংখ্যা OrigWidth = getWidth();

  যদি (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ঠেলো_back('.');
    Str.ঠেলো_back('0');
    ফেরত;
  }

  যদি (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(সত্য);
    Str.ঠেলো_back('-');
  }

  পূর্ণসংখ্যা Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ঠেলো_back('.');
  করো {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } যখন (FractPart != 0);
}

শূন্য APFixedPoint::print(raw_ostream &OS) ধ্রুবক {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#যদি !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD শূন্য APFixedPoint::dump() ধ্রুবক { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(যৌক্তিক *Overflow) ধ্রুবক {
  যদি (!isSaturated()) {
    যদি (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_মান());
    ফেরত APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  যদি (Overflow)
    *Overflow = মিথ্যা;

  যদি (isSigned())
    ফেরত Val.is_Min_Signed_মান() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  অন্যথা
    ফেরত APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, যৌক্তিক DstSign,
                                  যৌক্তিক *Overflow) ধ্রুবক {
  APSInt ফলাফল = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_মান(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_মান(DstWidth, !DstSign);

  যদি (SrcWidth < DstWidth) {
    ফলাফল = ফলাফল.extend(DstWidth);
  } অন্যথা যদি (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  যদি (Overflow) {
    যদি (ফলাফল.isSigned() && !DstSign) {
      *Overflow = ফলাফল.isNegative() || ফলাফল.ugt(DstMax);
    } অন্যথা যদি (ফলাফল.isUnsigned() && DstSign) {
      *Overflow = ফলাফল.ugt(DstMax);
    } অন্যথা {
      *Overflow = ফলাফল < DstMin || ফলাফল > DstMax;
    }
  }

  ফলাফল.setIsSigned(DstSign);
  ফেরত ফলাফল.extOrTrunc(DstWidth);
}

ধ্রুবক fltSemantics *APFixedPoint::promoteFloatSemantics(ধ্রুবক fltSemantics *S) {
  যদি (S == &APFloat::BFloat())
    ফেরত &APFloat::IEEEdouble();
  অন্যথা যদি (S == &APFloat::IEEEhalf())
    ফেরত &APFloat::IEEEsingle();
  অন্যথা যদি (S == &APFloat::IEEEsingle())
    ফেরত &APFloat::IEEEdouble();
  অন্যথা যদি (S == &APFloat::IEEEdouble())
    ফেরত &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(ধ্রুবক fltSemantics &FloatSema) ধ্রুবক {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ধ্রুবক fltSemantics *OpSema = &FloatSema;
  যখন (!Sema.fitsInFloatSemantics(*OpSema))
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
  যৌক্তিক Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  যদি (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ফেরত Flt;
}

APFixedPoint APFixedPoint::get_From_Int_মান(ধ্রুবক APSInt &মান,
                                           ধ্রুবক FixedPointSemantics &DstFXSema,
                                           যৌক্তিক *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      মান.getBitWidth(), মান.isSigned());
  ফেরত APFixedPoint(মান, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_মান(ধ্রুবক APFloat &মান,
                                ধ্রুবক FixedPointSemantics &DstFXSema,
                                যৌক্তিক *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ধ্রুবক fltSemantics &FloatSema = মান.getSemantics();

  যদি (মান.isNaN()) {
    // Handle NaN immediately.
    যদি (Overflow)
      *Overflow = সত্য;
    ফেরত APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ধ্রুবক fltSemantics *OpSema = &FloatSema;
  যখন (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = মান;

  যৌক্তিক Ignored;
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
  যৌক্তিক Overflowed = মিথ্যা;
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

  ফেরত APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
