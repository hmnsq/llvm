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

#അന്തര്ഭാവ "llvm/ADT/APFixedPoint.h"
#അന്തര്ഭാവ "llvm/ADT/APFloat.h"

#അന്തര്ഭാവ <cmath>

നാമാകാശ llvm {

ശൂന്യം FixedPointSemantics::print(llvm::raw_ostream &OS) സ്ഥിരം {
  OS << "width=" << getWidth() << ", ";
  എങ്കിൽ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() സ്ഥിരം {
  തിരികെ llvm::bit_cast<uint32_t>(*ഇത്);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, അസത്യം, അസത്യം, അസത്യം);
  std::memcpy(&F, &I, വലുപ്പം(F));
  തിരികെ F;
}

APFixedPoint APFixedPoint::convert(സ്ഥിരം FixedPointSemantics &DstSema,
                                   തർക്കം *Overflow) സ്ഥിരം {
  APSInt NewVal = Val;
  പൂർണ്ണസംഖ്യ RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  എങ്കിൽ (Overflow)
    *Overflow = അസത്യം;

  എങ്കിൽ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  എങ്കിൽ (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    എങ്കിൽ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    അല്ലെങ്കിൽ എങ്കിൽ (Overflow)
      *Overflow = സത്യം;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  എങ്കിൽ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    എങ്കിൽ (DstSema.isSaturated())
      NewVal = 0;
    അല്ലെങ്കിൽ എങ്കിൽ (Overflow)
      *Overflow = സത്യം;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  തിരികെ APFixedPoint(NewVal, DstSema);
}

പൂർണ്ണസംഖ്യ APFixedPoint::compare(സ്ഥിരം APFixedPoint &Other) സ്ഥിരം {
  APSInt ThisVal = get_മൂല്യം();
  APSInt OtherVal = Other.get_മൂല്യം();
  തർക്കം ThisSigned = Val.isSigned();
  തർക്കം OtherSigned = OtherVal.isSigned();

  പൂർണ്ണസംഖ്യ CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  പൂർണ്ണസംഖ്യ CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  എങ്കിൽ (ThisSigned && OtherSigned) {
    എങ്കിൽ (ThisVal.sgt(OtherVal))
      തിരികെ 1;
    അല്ലെങ്കിൽ എങ്കിൽ (ThisVal.slt(OtherVal))
      തിരികെ -1;
  } അല്ലെങ്കിൽ എങ്കിൽ (!ThisSigned && !OtherSigned) {
    എങ്കിൽ (ThisVal.ugt(OtherVal))
      തിരികെ 1;
    അല്ലെങ്കിൽ എങ്കിൽ (ThisVal.ult(OtherVal))
      തിരികെ -1;
  } അല്ലെങ്കിൽ എങ്കിൽ (ThisSigned && !OtherSigned) {
    എങ്കിൽ (ThisVal.isSignBitSet())
      തിരികെ -1;
    അല്ലെങ്കിൽ എങ്കിൽ (ThisVal.ugt(OtherVal))
      തിരികെ 1;
    അല്ലെങ്കിൽ എങ്കിൽ (ThisVal.ult(OtherVal))
      തിരികെ -1;
  } അല്ലെങ്കിൽ {
    // !ThisSigned && OtherSigned
    എങ്കിൽ (OtherVal.isSignBitSet())
      തിരികെ 1;
    അല്ലെങ്കിൽ എങ്കിൽ (ThisVal.ugt(OtherVal))
      തിരികെ 1;
    അല്ലെങ്കിൽ എങ്കിൽ (ThisVal.ult(OtherVal))
      തിരികെ -1;
  }

  തിരികെ 0;
}

APFixedPoint APFixedPoint::getMax(സ്ഥിരം FixedPointSemantics &Sema) {
  തർക്കം IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::get_Max_മൂല്യം(Sema.getWidth(), IsUnsigned);
  എങ്കിൽ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  തിരികെ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(സ്ഥിരം FixedPointSemantics &Sema) {
  auto Val = APSInt::get_Min_മൂല്യം(Sema.getWidth(), !Sema.isSigned());
  തിരികെ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(സ്ഥിരം FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  തിരികെ APFixedPoint(Val, Sema);
}

തർക്കം FixedPointSemantics::fitsInFloatSemantics(
    സ്ഥിരം fltSemantics &FloatSema) സ്ഥിരം {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*ഇത്).get_മൂല്യം();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  എങ്കിൽ ((Status & APFloat::opOverflow) || !isSigned())
    തിരികെ !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*ഇത്).get_മൂല്യം();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  തിരികെ !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    സ്ഥിരം FixedPointSemantics &Other) സ്ഥിരം {
  പൂർണ്ണസംഖ്യ CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  പൂർണ്ണസംഖ്യ CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  തർക്കം ResultIsSigned = isSigned() || Other.isSigned();
  തർക്കം ResultIsSaturated = isSaturated() || Other.isSaturated();
  തർക്കം ResultHasUnsignedPadding = അസത്യം;
  എങ്കിൽ (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  എങ്കിൽ (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  തിരികെ FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(സ്ഥിരം APFixedPoint &Other,
                               തർക്കം *Overflow) സ്ഥിരം {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_മൂല്യം();
  APSInt OtherVal = ConvertedOther.get_മൂല്യം();
  തർക്കം Overflowed = അസത്യം;

  APSInt Result;
  എങ്കിൽ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } അല്ലെങ്കിൽ {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  എങ്കിൽ (Overflow)
    *Overflow = Overflowed;

  തിരികെ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(സ്ഥിരം APFixedPoint &Other,
                               തർക്കം *Overflow) സ്ഥിരം {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_മൂല്യം();
  APSInt OtherVal = ConvertedOther.get_മൂല്യം();
  തർക്കം Overflowed = അസത്യം;

  APSInt Result;
  എങ്കിൽ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } അല്ലെങ്കിൽ {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  എങ്കിൽ (Overflow)
    *Overflow = Overflowed;

  തിരികെ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(സ്ഥിരം APFixedPoint &Other,
                               തർക്കം *Overflow) സ്ഥിരം {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_മൂല്യം();
  APSInt OtherVal = ConvertedOther.get_മൂല്യം();
  തർക്കം Overflowed = അസത്യം;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  എങ്കിൽ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } അല്ലെങ്കിൽ {
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
  എങ്കിൽ (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  അല്ലെങ്കിൽ
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  പ്രതിജ്ഞാ(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_മൂല്യം()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_മൂല്യം()
                                                 .extOrTrunc(Wide);
  എങ്കിൽ (CommonFXSema.isSaturated()) {
    എങ്കിൽ (Result < Min)
      Result = std::move(Min);
    അല്ലെങ്കിൽ എങ്കിൽ (Result > Max)
      Result = std::move(Max);
  } അല്ലെങ്കിൽ {
    Overflowed = Result < Min || Result > Max;
  }

  എങ്കിൽ (Overflow)
    *Overflow = Overflowed;

  തിരികെ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(സ്ഥിരം APFixedPoint &Other,
                               തർക്കം *Overflow) സ്ഥിരം {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.get_മൂല്യം();
  APSInt OtherVal = ConvertedOther.get_മൂല്യം();
  തർക്കം Overflowed = അസത്യം;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  എങ്കിൽ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } അല്ലെങ്കിൽ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  എങ്കിൽ (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  അല്ലെങ്കിൽ എങ്കിൽ (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  എങ്കിൽ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    എങ്കിൽ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } അല്ലെങ്കിൽ {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).get_മൂല്യം()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).get_മൂല്യം()
                                                 .extOrTrunc(Wide);
  എങ്കിൽ (CommonFXSema.isSaturated()) {
    എങ്കിൽ (Result < Min)
      Result = std::move(Min);
    അല്ലെങ്കിൽ എങ്കിൽ (Result > Max)
      Result = std::move(Max);
  } അല്ലെങ്കിൽ {
    Overflowed = Result < Min || Result > Max;
  }

  എങ്കിൽ (Overflow)
    *Overflow = Overflowed;

  തിരികെ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, തർക്കം *Overflow) സ്ഥിരം {
  APSInt ThisVal = Val;
  തർക്കം Overflowed = അസത്യം;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  എങ്കിൽ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  അല്ലെങ്കിൽ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).get_മൂല്യം().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).get_മൂല്യം().extOrTrunc(Wide);
  എങ്കിൽ (Sema.isSaturated()) {
    എങ്കിൽ (Result < Min)
      Result = std::move(Min);
    അല്ലെങ്കിൽ എങ്കിൽ (Result > Max)
      Result = std::move(Max);
  } അല്ലെങ്കിൽ {
    Overflowed = Result < Min || Result > Max;
  }

  എങ്കിൽ (Overflow)
    *Overflow = Overflowed;

  തിരികെ APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

ശൂന്യം APFixedPoint::toString(SmallVectorImpl<അക്ഷരം> &Str) സ്ഥിരം {
  APSInt Val = get_മൂല്യം();
  പൂർണ്ണസംഖ്യ Lsb = getLsbWeight();
  പൂർണ്ണസംഖ്യ OrigWidth = getWidth();

  എങ്കിൽ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.തള്ളുക_back('.');
    Str.തള്ളുക_back('0');
    തിരികെ;
  }

  എങ്കിൽ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(സത്യം);
    Str.തള്ളുക_back('-');
  }

  പൂർണ്ണസംഖ്യ Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.തള്ളുക_back('.');
  ചെയ്യുക {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } ആകുമ്പോൾ (FractPart != 0);
}

ശൂന്യം APFixedPoint::print(raw_ostream &OS) സ്ഥിരം {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#എങ്കിൽ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD ശൂന്യം APFixedPoint::dump() സ്ഥിരം { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(തർക്കം *Overflow) സ്ഥിരം {
  എങ്കിൽ (!isSaturated()) {
    എങ്കിൽ (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.is_Min_Signed_മൂല്യം());
    തിരികെ APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  എങ്കിൽ (Overflow)
    *Overflow = അസത്യം;

  എങ്കിൽ (isSigned())
    തിരികെ Val.is_Min_Signed_മൂല്യം() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  അല്ലെങ്കിൽ
    തിരികെ APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, തർക്കം DstSign,
                                  തർക്കം *Overflow) സ്ഥിരം {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::get_Min_മൂല്യം(DstWidth, !DstSign);
  APSInt DstMax = APSInt::get_Max_മൂല്യം(DstWidth, !DstSign);

  എങ്കിൽ (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } അല്ലെങ്കിൽ എങ്കിൽ (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  എങ്കിൽ (Overflow) {
    എങ്കിൽ (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } അല്ലെങ്കിൽ എങ്കിൽ (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } അല്ലെങ്കിൽ {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  തിരികെ Result.extOrTrunc(DstWidth);
}

സ്ഥിരം fltSemantics *APFixedPoint::promoteFloatSemantics(സ്ഥിരം fltSemantics *S) {
  എങ്കിൽ (S == &APFloat::BFloat())
    തിരികെ &APFloat::IEEEdouble();
  അല്ലെങ്കിൽ എങ്കിൽ (S == &APFloat::IEEEhalf())
    തിരികെ &APFloat::IEEEsingle();
  അല്ലെങ്കിൽ എങ്കിൽ (S == &APFloat::IEEEsingle())
    തിരികെ &APFloat::IEEEdouble();
  അല്ലെങ്കിൽ എങ്കിൽ (S == &APFloat::IEEEdouble())
    തിരികെ &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(സ്ഥിരം fltSemantics &FloatSema) സ്ഥിരം {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  സ്ഥിരം fltSemantics *OpSema = &FloatSema;
  ആകുമ്പോൾ (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (ശൂന്യം)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  തർക്കം Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  എങ്കിൽ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  തിരികെ Flt;
}

APFixedPoint APFixedPoint::get_From_Int_മൂല്യം(സ്ഥിരം APSInt &മൂല്യം,
                                           സ്ഥിരം FixedPointSemantics &DstFXSema,
                                           തർക്കം *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      മൂല്യം.getBitWidth(), മൂല്യം.isSigned());
  തിരികെ APFixedPoint(മൂല്യം, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::get_From_Float_മൂല്യം(സ്ഥിരം APFloat &മൂല്യം,
                                സ്ഥിരം FixedPointSemantics &DstFXSema,
                                തർക്കം *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  സ്ഥിരം fltSemantics &FloatSema = മൂല്യം.getSemantics();

  എങ്കിൽ (മൂല്യം.isNaN()) {
    // Handle NaN immediately.
    എങ്കിൽ (Overflow)
      *Overflow = സത്യം;
    തിരികെ APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  സ്ഥിരം fltSemantics *OpSema = &FloatSema;
  ആകുമ്പോൾ (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = മൂല്യം;

  തർക്കം Ignored;
  എങ്കിൽ (&FloatSema != OpSema)
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
  തർക്കം Overflowed = അസത്യം;
  എങ്കിൽ (DstFXSema.isSaturated()) {
    എങ്കിൽ (Val > FloatMax)
      Res = getMax(DstFXSema).get_മൂല്യം();
    അല്ലെങ്കിൽ എങ്കിൽ (Val < FloatMin)
      Res = getMin(DstFXSema).get_മൂല്യം();
  } അല്ലെങ്കിൽ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  എങ്കിൽ (Overflow)
    *Overflow = Overflowed;

  തിരികെ APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
