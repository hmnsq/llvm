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

#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFixedPoint.h"
#ಅನ್ತರ್ಭಾವ "llvm/ADT/APFloat.h"

#ಅನ್ತರ್ಭಾವ <cmath>

ನಾಮಾಕಾಶ llvm {

ಶೂನ್ಯ FixedPointSemantics::print(llvm::raw_ostream &OS) ಧ್ರುವ {
  OS << "width=" << getWidth() << ", ";
  ಯದಿ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() ಧ್ರುವ {
  ಪ್ರತ್ಯಾಗಮ llvm::bit_cast<uint32_t>(*ಏತತ್);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, ಅಸತ್ಯ, ಅಸತ್ಯ, ಅಸತ್ಯ);
  std::memcpy(&F, &I, ಪರಿಮಾಣ(F));
  ಪ್ರತ್ಯಾಗಮ F;
}

APFixedPoint APFixedPoint::convert(ಧ್ರುವ FixedPointSemantics &DstSema,
                                   ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  APSInt NewVal = Val;
  ಪೂರ್ಣಾಙ್ಕ RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  ಯದಿ (Overflow)
    *Overflow = ಅಸತ್ಯ;

  ಯದಿ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  ಯದಿ (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    ಯದಿ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    ಅನ್ಯಥಾ ಯದಿ (Overflow)
      *Overflow = ಸತ್ಯ;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ಯದಿ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ಯದಿ (DstSema.isSaturated())
      NewVal = 0;
    ಅನ್ಯಥಾ ಯದಿ (Overflow)
      *Overflow = ಸತ್ಯ;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  ಪ್ರತ್ಯಾಗಮ APFixedPoint(NewVal, DstSema);
}

ಪೂರ್ಣಾಙ್ಕ APFixedPoint::compare(ಧ್ರುವ APFixedPoint &Other) ಧ್ರುವ {
  APSInt ThisVal = get_ಮೌಲ್ಯ();
  APSInt OtherVal = Other.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ ThisSigned = Val.isSigned();
  ತಾರ್ಕಿಕ OtherSigned = OtherVal.isSigned();

  ಪೂರ್ಣಾಙ್ಕ CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  ಪೂರ್ಣಾಙ್ಕ CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  ಯದಿ (ThisSigned && OtherSigned) {
    ಯದಿ (ThisVal.sgt(OtherVal))
      ಪ್ರತ್ಯಾಗಮ 1;
    ಅನ್ಯಥಾ ಯದಿ (ThisVal.slt(OtherVal))
      ಪ್ರತ್ಯಾಗಮ -1;
  } ಅನ್ಯಥಾ ಯದಿ (!ThisSigned && !OtherSigned) {
    ಯದಿ (ThisVal.ugt(OtherVal))
      ಪ್ರತ್ಯಾಗಮ 1;
    ಅನ್ಯಥಾ ಯದಿ (ThisVal.ult(OtherVal))
      ಪ್ರತ್ಯಾಗಮ -1;
  } ಅನ್ಯಥಾ ಯದಿ (ThisSigned && !OtherSigned) {
    ಯದಿ (ThisVal.isSignBitSet())
      ಪ್ರತ್ಯಾಗಮ -1;
    ಅನ್ಯಥಾ ಯದಿ (ThisVal.ugt(OtherVal))
      ಪ್ರತ್ಯಾಗಮ 1;
    ಅನ್ಯಥಾ ಯದಿ (ThisVal.ult(OtherVal))
      ಪ್ರತ್ಯಾಗಮ -1;
  } ಅನ್ಯಥಾ {
    // !ThisSigned && OtherSigned
    ಯದಿ (OtherVal.isSignBitSet())
      ಪ್ರತ್ಯಾಗಮ 1;
    ಅನ್ಯಥಾ ಯದಿ (ThisVal.ugt(OtherVal))
      ಪ್ರತ್ಯಾಗಮ 1;
    ಅನ್ಯಥಾ ಯದಿ (ThisVal.ult(OtherVal))
      ಪ್ರತ್ಯಾಗಮ -1;
  }

  ಪ್ರತ್ಯಾಗಮ 0;
}

APFixedPoint APFixedPoint::getMax(ಧ್ರುವ FixedPointSemantics &Sema) {
  ತಾರ್ಕಿಕ IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_ಮೌಲ್ಯ(Sema.getWidth(), IsUnsigned);
  ಯದಿ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(ಧ್ರುವ FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_ಮೌಲ್ಯ(Sema.getWidth(), !Sema.isSigned());
  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(ಧ್ರುವ FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Val, Sema);
}

ತಾರ್ಕಿಕ FixedPointSemantics::fitsInFloatSemantics(
    ಧ್ರುವ fltSemantics &FloatSema) ಧ್ರುವ {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*ಏತತ್).get_ಮೌಲ್ಯ();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  ಯದಿ ((Status & APFloat::opOverflow) || !isSigned())
    ಪ್ರತ್ಯಾಗಮ !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*ಏತತ್).get_ಮೌಲ್ಯ();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  ಪ್ರತ್ಯಾಗಮ !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    ಧ್ರುವ FixedPointSemantics &Other) ಧ್ರುವ {
  ಪೂರ್ಣಾಙ್ಕ CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  ಪೂರ್ಣಾಙ್ಕ CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  ತಾರ್ಕಿಕ ResultIsSigned = isSigned() || Other.isSigned();
  ತಾರ್ಕಿಕ ResultIsSaturated = isSaturated() || Other.isSaturated();
  ತಾರ್ಕಿಕ ResultHasUnsignedPadding = ಅಸತ್ಯ;
  ಯದಿ (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ಯದಿ (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  ಪ್ರತ್ಯಾಗಮ FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(ಧ್ರುವ APFixedPoint &Other,
                               ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಅಸತ್ಯ;

  APSInt Result;
  ಯದಿ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ಅನ್ಯಥಾ {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ಯದಿ (Overflow)
    *Overflow = Overflowed;

  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(ಧ್ರುವ APFixedPoint &Other,
                               ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಅಸತ್ಯ;

  APSInt Result;
  ಯದಿ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ಅನ್ಯಥಾ {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ಯದಿ (Overflow)
    *Overflow = Overflowed;

  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(ಧ್ರುವ APFixedPoint &Other,
                               ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಅಸತ್ಯ;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  ಯದಿ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ಅನ್ಯಥಾ {
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
  ಯದಿ (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  ಅನ್ಯಥಾ
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  ಪ್ರತಿಜ್ಞಾ(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  ಯದಿ (CommonFXSema.isSaturated()) {
    ಯದಿ (Result < Min)
      Result = std::move(Min);
    ಅನ್ಯಥಾ ಯದಿ (Result > Max)
      Result = std::move(Max);
  } ಅನ್ಯಥಾ {
    Overflowed = Result < Min || Result > Max;
  }

  ಯದಿ (Overflow)
    *Overflow = Overflowed;

  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(ಧ್ರುವ APFixedPoint &Other,
                               ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_ಮೌಲ್ಯ();
  APSInt OtherVal = ConvertedOther.get_ಮೌಲ್ಯ();
  ತಾರ್ಕಿಕ Overflowed = ಅಸತ್ಯ;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  ಯದಿ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ಅನ್ಯಥಾ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ಯದಿ (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  ಅನ್ಯಥಾ ಯದಿ (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  ಯದಿ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ಯದಿ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } ಅನ್ಯಥಾ {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_ಮೌಲ್ಯ()
                                                 .extOrTrunc(Wide);
  ಯದಿ (CommonFXSema.isSaturated()) {
    ಯದಿ (Result < Min)
      Result = std::move(Min);
    ಅನ್ಯಥಾ ಯದಿ (Result > Max)
      Result = std::move(Max);
  } ಅನ್ಯಥಾ {
    Overflowed = Result < Min || Result > Max;
  }

  ಯದಿ (Overflow)
    *Overflow = Overflowed;

  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  APSInt ThisVal = Val;
  ತಾರ್ಕಿಕ Overflowed = ಅಸತ್ಯ;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  ಯದಿ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ಅನ್ಯಥಾ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_ಮೌಲ್ಯ().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_ಮೌಲ್ಯ().extOrTrunc(Wide);
  ಯದಿ (Sema.isSaturated()) {
    ಯದಿ (Result < Min)
      Result = std::move(Min);
    ಅನ್ಯಥಾ ಯದಿ (Result > Max)
      Result = std::move(Max);
  } ಅನ್ಯಥಾ {
    Overflowed = Result < Min || Result > Max;
  }

  ಯದಿ (Overflow)
    *Overflow = Overflowed;

  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

ಶೂನ್ಯ APFixedPoint::toString(SmallVectorImpl<ವರ್ಣ> &Str) ಧ್ರುವ {
  APSInt Val = get_ಮೌಲ್ಯ();
  ಪೂರ್ಣಾಙ್ಕ Lsb = getLsbWeight();
  ಪೂರ್ಣಾಙ್ಕ OrigWidth = getWidth();

  ಯದಿ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.ತಳ್ಳು_back('.');
    Str.ತಳ್ಳು_back('0');
    ಪ್ರತ್ಯಾಗಮ;
  }

  ಯದಿ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(ಸತ್ಯ);
    Str.ತಳ್ಳು_back('-');
  }

  ಪೂರ್ಣಾಙ್ಕ Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.ತಳ್ಳು_back('.');
  ಕುರು {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } ಯಾವತ್ (FractPart != 0);
}

ಶೂನ್ಯ APFixedPoint::print(raw_ostream &OS) ಧ್ರುವ {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ಯದಿ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ಶೂನ್ಯ APFixedPoint::dump() ಧ್ರುವ { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  ಯದಿ (!isSaturated()) {
    ಯದಿ (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_ಮೌಲ್ಯ());
    ಪ್ರತ್ಯಾಗಮ APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  ಯದಿ (Overflow)
    *Overflow = ಅಸತ್ಯ;

  ಯದಿ (isSigned())
    ಪ್ರತ್ಯಾಗಮ Val.is_Min_Signed_ಮೌಲ್ಯ() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  ಅನ್ಯಥಾ
    ಪ್ರತ್ಯಾಗಮ APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, ತಾರ್ಕಿಕ DstSign,
                                  ತಾರ್ಕಿಕ *Overflow) ಧ್ರುವ {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_ಮೌಲ್ಯ(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_ಮೌಲ್ಯ(DstWidth, !DstSign);

  ಯದಿ (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } ಅನ್ಯಥಾ ಯದಿ (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  ಯದಿ (Overflow) {
    ಯದಿ (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } ಅನ್ಯಥಾ ಯದಿ (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } ಅನ್ಯಥಾ {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  ಪ್ರತ್ಯಾಗಮ Result.extOrTrunc(DstWidth);
}

ಧ್ರುವ fltSemantics *APFixedPoint::promoteFloatSemantics(ಧ್ರುವ fltSemantics *S) {
  ಯದಿ (S == &APFloat::BFloat())
    ಪ್ರತ್ಯಾಗಮ &APFloat::IEEEdouble();
  ಅನ್ಯಥಾ ಯದಿ (S == &APFloat::IEEEhalf())
    ಪ್ರತ್ಯಾಗಮ &APFloat::IEEEsingle();
  ಅನ್ಯಥಾ ಯದಿ (S == &APFloat::IEEEsingle())
    ಪ್ರತ್ಯಾಗಮ &APFloat::IEEEdouble();
  ಅನ್ಯಥಾ ಯದಿ (S == &APFloat::IEEEdouble())
    ಪ್ರತ್ಯಾಗಮ &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(ಧ್ರುವ fltSemantics &FloatSema) ಧ್ರುವ {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ಧ್ರುವ fltSemantics *OpSema = &FloatSema;
  ಯಾವತ್ (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (ಶೂನ್ಯ)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  ತಾರ್ಕಿಕ Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  ಯದಿ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ಪ್ರತ್ಯಾಗಮ Flt;
}

APFixedPoint APFixedPoint::get_From_Int_ಮೌಲ್ಯ(ಧ್ರುವ APSInt &ಮೌಲ್ಯ,
                                           ಧ್ರುವ FixedPointSemantics &DstFXSema,
                                           ತಾರ್ಕಿಕ *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      ಮೌಲ್ಯ.getBitWidth(), ಮೌಲ್ಯ.isSigned());
  ಪ್ರತ್ಯಾಗಮ APFixedPoint(ಮೌಲ್ಯ, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_ಮೌಲ್ಯ(ಧ್ರುವ APFloat &ಮೌಲ್ಯ,
                                ಧ್ರುವ FixedPointSemantics &DstFXSema,
                                ತಾರ್ಕಿಕ *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  ಧ್ರುವ fltSemantics &FloatSema = ಮೌಲ್ಯ.getSemantics();

  ಯದಿ (ಮೌಲ್ಯ.isNaN()) {
    // Handle NaN immediately.
    ಯದಿ (Overflow)
      *Overflow = ಸತ್ಯ;
    ಪ್ರತ್ಯಾಗಮ APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  ಧ್ರುವ fltSemantics *OpSema = &FloatSema;
  ಯಾವತ್ (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = ಮೌಲ್ಯ;

  ತಾರ್ಕಿಕ Ignored;
  ಯದಿ (&FloatSema != OpSema)
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
  ತಾರ್ಕಿಕ Overflowed = ಅಸತ್ಯ;
  ಯದಿ (DstFXSema.isSaturated()) {
    ಯದಿ (Val > FloatMax)
      Res = getMax(DstFXSema).get_ಮೌಲ್ಯ();
    ಅನ್ಯಥಾ ಯದಿ (Val < FloatMin)
      Res = getMin(DstFXSema).get_ಮೌಲ್ಯ();
  } ಅನ್ಯಥಾ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ಯದಿ (Overflow)
    *Overflow = Overflowed;

  ಪ್ರತ್ಯಾಗಮ APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
