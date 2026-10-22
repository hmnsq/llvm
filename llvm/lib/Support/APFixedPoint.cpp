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

#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APFixedPoint.h"
#ଅନ୍ତର୍ଭାଵ "llvm/ADT/APFloat.h"

#ଅନ୍ତର୍ଭାଵ <cmath>

ନାମାକାଶ llvm {

ଶୂନ୍ୟ FixedPointSemantics::print(llvm::raw_ostream &OS) ଅଚଳ {
  OS << "width=" << getWidth() << ", ";
  ଯଦି (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ଅଚଳ {
  ଫେରନ୍ତୁ llvm::bit_cast<uint32_t>(*ଏହି);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, ମିଥ୍ୟା, ମିଥ୍ୟା, ମିଥ୍ୟା);
  std::memcpy(&F, &I, ଆକାର(F));
  ଫେରନ୍ତୁ F;
}

APFixedPoint APFixedPoint::convert(ଅଚଳ FixedPointSemantics &DstSema,
                                   ତାର୍କିକ *Overflow) ଅଚଳ {
  APSInt NewVal = Val;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  ଯଦି (Overflow)
    *Overflow = ମିଥ୍ୟା;

  ଯଦି (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  ଯଦି (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    ଯଦି (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    ଅନ୍ୟଥା ଯଦି (Overflow)
      *Overflow = ସତ୍ୟ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ଯଦି (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ଯଦି (DstSema.isSaturated())
      NewVal = 0;
    ଅନ୍ୟଥା ଯଦି (Overflow)
      *Overflow = ସତ୍ୟ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  ଫେରନ୍ତୁ APFixedPoint(NewVal, DstSema);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା APFixedPoint::compare(ଅଚଳ APFixedPoint &Other) ଅଚଳ {
  APSInt ThisVal = get_ମୂଲ୍ୟ();
  APSInt OtherVal = Other.get_ମୂଲ୍ୟ();
  ତାର୍କିକ ThisSigned = Val.isSigned();
  ତାର୍କିକ OtherSigned = OtherVal.isSigned();

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  ଯଦି (ThisSigned && OtherSigned) {
    ଯଦି (ThisVal.sgt(OtherVal))
      ଫେରନ୍ତୁ 1;
    ଅନ୍ୟଥା ଯଦି (ThisVal.slt(OtherVal))
      ଫେରନ୍ତୁ -1;
  } ଅନ୍ୟଥା ଯଦି (!ThisSigned && !OtherSigned) {
    ଯଦି (ThisVal.ugt(OtherVal))
      ଫେରନ୍ତୁ 1;
    ଅନ୍ୟଥା ଯଦି (ThisVal.ult(OtherVal))
      ଫେରନ୍ତୁ -1;
  } ଅନ୍ୟଥା ଯଦି (ThisSigned && !OtherSigned) {
    ଯଦି (ThisVal.isSignBitSet())
      ଫେରନ୍ତୁ -1;
    ଅନ୍ୟଥା ଯଦି (ThisVal.ugt(OtherVal))
      ଫେରନ୍ତୁ 1;
    ଅନ୍ୟଥା ଯଦି (ThisVal.ult(OtherVal))
      ଫେରନ୍ତୁ -1;
  } ଅନ୍ୟଥା {
    // !ThisSigned && OtherSigned
    ଯଦି (OtherVal.isSignBitSet())
      ଫେରନ୍ତୁ 1;
    ଅନ୍ୟଥା ଯଦି (ThisVal.ugt(OtherVal))
      ଫେରନ୍ତୁ 1;
    ଅନ୍ୟଥା ଯଦି (ThisVal.ult(OtherVal))
      ଫେରନ୍ତୁ -1;
  }

  ଫେରନ୍ତୁ 0;
}

APFixedPoint APFixedPoint::getMax(ଅଚଳ FixedPointSemantics &Sema) {
  ତାର୍କିକ IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_ମୂଲ୍ୟ(Sema.getWidth(), IsUnsigned);
  ଯଦି (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ଫେରନ୍ତୁ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ଅଚଳ FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_ମୂଲ୍ୟ(Sema.getWidth(), !Sema.isSigned());
  ଫେରନ୍ତୁ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ଅଚଳ FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  ଫେରନ୍ତୁ APFixedPoint(Val, Sema);
}

ତାର୍କିକ FixedPointSemantics::fitsInFloatSemantics(
    ଅଚଳ fltSemantics &FloatSema) ଅଚଳ {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*ଏହି).get_ମୂଲ୍ୟ();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  ଯଦି ((Status & APFloat::opOverflow) || !isSigned())
    ଫେରନ୍ତୁ !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*ଏହି).get_ମୂଲ୍ୟ();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  ଫେରନ୍ତୁ !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ଅଚଳ FixedPointSemantics &Other) ଅଚଳ {
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  ତାର୍କିକ ResultIsSigned = isSigned() || Other.isSigned();
  ତାର୍କିକ ResultIsSaturated = isSaturated() || Other.isSaturated();
  ତାର୍କିକ ResultHasUnsignedPadding = ମିଥ୍ୟା;
  ଯଦି (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ଯଦି (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  ଫେରନ୍ତୁ FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ଅଚଳ APFixedPoint &Other,
                               ତାର୍କିକ *Overflow) ଅଚଳ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ମୂଲ୍ୟ();
  APSInt OtherVal = ConvertedOther.get_ମୂଲ୍ୟ();
  ତାର୍କିକ Overflowed = ମିଥ୍ୟା;

  APSInt Result;
  ଯଦି (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ଅନ୍ୟଥା {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ଯଦି (Overflow)
    *Overflow = Overflowed;

  ଫେରନ୍ତୁ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ଅଚଳ APFixedPoint &Other,
                               ତାର୍କିକ *Overflow) ଅଚଳ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ମୂଲ୍ୟ();
  APSInt OtherVal = ConvertedOther.get_ମୂଲ୍ୟ();
  ତାର୍କିକ Overflowed = ମିଥ୍ୟା;

  APSInt Result;
  ଯଦି (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ଅନ୍ୟଥା {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ଯଦି (Overflow)
    *Overflow = Overflowed;

  ଫେରନ୍ତୁ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ଅଚଳ APFixedPoint &Other,
                               ତାର୍କିକ *Overflow) ଅଚଳ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ମୂଲ୍ୟ();
  APSInt OtherVal = ConvertedOther.get_ମୂଲ୍ୟ();
  ତାର୍କିକ Overflowed = ମିଥ୍ୟା;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  ଯଦି (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ଅନ୍ୟଥା {
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
  ଯଦି (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  ଅନ୍ୟଥା
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  ପ୍ରତିଜ୍ଞା(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_ମୂଲ୍ୟ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_ମୂଲ୍ୟ()
                                                 .extOrTrunc(Wide);
  ଯଦି (CommonFXSema.isSaturated()) {
    ଯଦି (Result < Min)
      Result = std::move(Min);
    ଅନ୍ୟଥା ଯଦି (Result > Max)
      Result = std::move(Max);
  } ଅନ୍ୟଥା {
    Overflowed = Result < Min || Result > Max;
  }

  ଯଦି (Overflow)
    *Overflow = Overflowed;

  ଫେରନ୍ତୁ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ଅଚଳ APFixedPoint &Other,
                               ତାର୍କିକ *Overflow) ଅଚଳ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ମୂଲ୍ୟ();
  APSInt OtherVal = ConvertedOther.get_ମୂଲ୍ୟ();
  ତାର୍କିକ Overflowed = ମିଥ୍ୟା;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  ଯଦି (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ଅନ୍ୟଥା {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ଯଦି (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  ଅନ୍ୟଥା ଯଦି (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  ଯଦି (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ଯଦି (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } ଅନ୍ୟଥା {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_ମୂଲ୍ୟ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_ମୂଲ୍ୟ()
                                                 .extOrTrunc(Wide);
  ଯଦି (CommonFXSema.isSaturated()) {
    ଯଦି (Result < Min)
      Result = std::move(Min);
    ଅନ୍ୟଥା ଯଦି (Result > Max)
      Result = std::move(Max);
  } ଅନ୍ୟଥା {
    Overflowed = Result < Min || Result > Max;
  }

  ଯଦି (Overflow)
    *Overflow = Overflowed;

  ଫେରନ୍ତୁ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, ତାର୍କିକ *Overflow) ଅଚଳ {
  APSInt ThisVal = Val;
  ତାର୍କିକ Overflowed = ମିଥ୍ୟା;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  ଯଦି (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ଅନ୍ୟଥା
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_ମୂଲ୍ୟ().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_ମୂଲ୍ୟ().extOrTrunc(Wide);
  ଯଦି (Sema.isSaturated()) {
    ଯଦି (Result < Min)
      Result = std::move(Min);
    ଅନ୍ୟଥା ଯଦି (Result > Max)
      Result = std::move(Max);
  } ଅନ୍ୟଥା {
    Overflowed = Result < Min || Result > Max;
  }

  ଯଦି (Overflow)
    *Overflow = Overflowed;

  ଫେରନ୍ତୁ APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

ଶୂନ୍ୟ APFixedPoint::toString(SmallVectorImpl<ଅକ୍ଷର> &Str) ଅଚଳ {
  APSInt Val = get_ମୂଲ୍ୟ();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Lsb = getLsbWeight();
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା OrigWidth = getWidth();

  ଯଦି (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ଠେଲ_back('.');
    Str.ଠେଲ_back('0');
    ଫେରନ୍ତୁ;
  }

  ଯଦି (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(ସତ୍ୟ);
    Str.ଠେଲ_back('-');
  }

  ପୂର୍ଣ୍ଣସଂଖ୍ୟା Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ଠେଲ_back('.');
  କର {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } ଯେତେବେଳେ (FractPart != 0);
}

ଶୂନ୍ୟ APFixedPoint::print(raw_ostream &OS) ଅଚଳ {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ଯଦି !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ଶୂନ୍ୟ APFixedPoint::dump() ଅଚଳ { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(ତାର୍କିକ *Overflow) ଅଚଳ {
  ଯଦି (!isSaturated()) {
    ଯଦି (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_ମୂଲ୍ୟ());
    ଫେରନ୍ତୁ APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  ଯଦି (Overflow)
    *Overflow = ମିଥ୍ୟା;

  ଯଦି (isSigned())
    ଫେରନ୍ତୁ Val.is_Min_Signed_ମୂଲ୍ୟ() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  ଅନ୍ୟଥା
    ଫେରନ୍ତୁ APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, ତାର୍କିକ DstSign,
                                  ତାର୍କିକ *Overflow) ଅଚଳ {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_ମୂଲ୍ୟ(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_ମୂଲ୍ୟ(DstWidth, !DstSign);

  ଯଦି (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } ଅନ୍ୟଥା ଯଦି (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  ଯଦି (Overflow) {
    ଯଦି (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } ଅନ୍ୟଥା ଯଦି (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } ଅନ୍ୟଥା {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  ଫେରନ୍ତୁ Result.extOrTrunc(DstWidth);
}

ଅଚଳ fltSemantics *APFixedPoint::promoteFloatSemantics(ଅଚଳ fltSemantics *S) {
  ଯଦି (S == &APFloat::BFloat())
    ଫେରନ୍ତୁ &APFloat::IEEEdouble();
  ଅନ୍ୟଥା ଯଦି (S == &APFloat::IEEEhalf())
    ଫେରନ୍ତୁ &APFloat::IEEEsingle();
  ଅନ୍ୟଥା ଯଦି (S == &APFloat::IEEEsingle())
    ଫେରନ୍ତୁ &APFloat::IEEEdouble();
  ଅନ୍ୟଥା ଯଦି (S == &APFloat::IEEEdouble())
    ଫେରନ୍ତୁ &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(ଅଚଳ fltSemantics &FloatSema) ଅଚଳ {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ଅଚଳ fltSemantics *OpSema = &FloatSema;
  ଯେତେବେଳେ (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (ଶୂନ୍ୟ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  ତାର୍କିକ Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  ଯଦି (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ଫେରନ୍ତୁ Flt;
}

APFixedPoint APFixedPoint::get_From_Int_ମୂଲ୍ୟ(ଅଚଳ APSInt &ମୂଲ୍ୟ,
                                           ଅଚଳ FixedPointSemantics &DstFXSema,
                                           ତାର୍କିକ *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      ମୂଲ୍ୟ.getBitWidth(), ମୂଲ୍ୟ.isSigned());
  ଫେରନ୍ତୁ APFixedPoint(ମୂଲ୍ୟ, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_ମୂଲ୍ୟ(ଅଚଳ APFloat &ମୂଲ୍ୟ,
                                ଅଚଳ FixedPointSemantics &DstFXSema,
                                ତାର୍କିକ *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ଅଚଳ fltSemantics &FloatSema = ମୂଲ୍ୟ.getSemantics();

  ଯଦି (ମୂଲ୍ୟ.isNaN()) {
    // Handle NaN immediately.
    ଯଦି (Overflow)
      *Overflow = ସତ୍ୟ;
    ଫେରନ୍ତୁ APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ଅଚଳ fltSemantics *OpSema = &FloatSema;
  ଯେତେବେଳେ (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = ମୂଲ୍ୟ;

  ତାର୍କିକ Ignored;
  ଯଦି (&FloatSema != OpSema)
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
  ତାର୍କିକ Overflowed = ମିଥ୍ୟା;
  ଯଦି (DstFXSema.isSaturated()) {
    ଯଦି (Val > FloatMax)
      Res = getMax(DstFXSema).get_ମୂଲ୍ୟ();
    ଅନ୍ୟଥା ଯଦି (Val < FloatMin)
      Res = getMin(DstFXSema).get_ମୂଲ୍ୟ();
  } ଅନ୍ୟଥା {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ଯଦି (Overflow)
    *Overflow = Overflowed;

  ଫେରନ୍ତୁ APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
