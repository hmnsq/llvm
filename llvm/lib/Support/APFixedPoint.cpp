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

#include "llvm/ADT/APFixedPoint.h"
#include "llvm/ADT/APFloat.h"

#include <cmath>

namespace llvm {

void FixedPointSemantics::print(llvm::raw_ostream &OS) const {
  OS << "width=" << getWidth() << ", ";
  ᬬᬾᬦ᭄ (isValidLegacySema())
    OS << "scale=" << getScale() << ", ";
  OS << "msb=" << getMsbWeight() << ", ";
  OS << "lsb=" << getLsbWeight() << ", ";
  OS << "IsSigned=" << IsSigned << ", ";
  OS << "HasUnsignedPadding=" << HasUnsignedPadding << ", ";
  OS << "IsSaturated=" << IsSaturated;
}

uint32_t FixedPointSemantics::toOpaqueInt() const {
  ᬯᬮᬶ llvm::bit_cast<uint32_t>(*this);
}

FixedPointSemantics FixedPointSemantics::getFromOpaqueInt(uint32_t I) {
  FixedPointSemantics F(0, 0, false, false, false);
  std::memcpy(&F, &I, parimaa_na(F));
  ᬯᬮᬶ F;
}

APFixedPoint APFixedPoint::convert(const FixedPointSemantics &DstSema,
                                   bool *Overflow) const {
  APSInt NewVal = Val;
  puur_naa_nka RelativeUpscale = getLsbWeight() - DstSema.getLsbWeight();
  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = false;

  ᬬᬾᬦ᭄ (RelativeUpscale > 0)
    NewVal = NewVal.extend(NewVal.getBitWidth() + RelativeUpscale);
  NewVal = NewVal.relativeShl(RelativeUpscale);

  auto Mask = APInt::getBitsSetFrom(
      NewVal.getBitWidth(),
      std::min(DstSema.getIntegralBits() - DstSema.getLsbWeight(),
               NewVal.getBitWidth()));
  APInt Masked(NewVal & Mask);

  // Change in the bits above the sign
  ᬬᬾᬦ᭄ (!(Masked == Mask || Masked == 0)) {
    // Found overflow in the bits above the sign
    ᬬᬾᬦ᭄ (DstSema.isSaturated())
      NewVal = NewVal.isNegative() ? Mask : ~Mask;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (Overflow)
      *Overflow = true;
  }

  // If the dst semantics are unsigned, but our value is signed and negative, we
  // clamp to zero.
  ᬬᬾᬦ᭄ (!DstSema.isSigned() && NewVal.isSigned() && NewVal.isNegative()) {
    // Found negative overflow for unsigned result
    ᬬᬾᬦ᭄ (DstSema.isSaturated())
      NewVal = 0;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (Overflow)
      *Overflow = true;
  }

  NewVal = NewVal.extOrTrunc(DstSema.getWidth());
  NewVal.setIsSigned(DstSema.isSigned());
  ᬯᬮᬶ APFixedPoint(NewVal, DstSema);
}

puur_naa_nka APFixedPoint::compare(const APFixedPoint &Other) const {
  APSInt ThisVal = getValue();
  APSInt OtherVal = Other.getValue();
  bool ThisSigned = Val.isSigned();
  bool OtherSigned = OtherVal.isSigned();

  puur_naa_nka CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  puur_naa_nka CommonMsb = std::max(getMsbWeight(), Other.getMsbWeight());
  unsigned CommonWidth = CommonMsb - CommonLsb + 1;

  ThisVal = ThisVal.extOrTrunc(CommonWidth);
  OtherVal = OtherVal.extOrTrunc(CommonWidth);

  ThisVal = ThisVal.shl(getLsbWeight() - CommonLsb);
  OtherVal = OtherVal.shl(Other.getLsbWeight() - CommonLsb);

  ᬬᬾᬦ᭄ (ThisSigned && OtherSigned) {
    ᬬᬾᬦ᭄ (ThisVal.sgt(OtherVal))
      ᬯᬮᬶ 1;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisVal.slt(OtherVal))
      ᬯᬮᬶ -1;
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (!ThisSigned && !OtherSigned) {
    ᬬᬾᬦ᭄ (ThisVal.ugt(OtherVal))
      ᬯᬮᬶ 1;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisVal.ult(OtherVal))
      ᬯᬮᬶ -1;
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisSigned && !OtherSigned) {
    ᬬᬾᬦ᭄ (ThisVal.isSignBitSet())
      ᬯᬮᬶ -1;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisVal.ugt(OtherVal))
      ᬯᬮᬶ 1;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisVal.ult(OtherVal))
      ᬯᬮᬶ -1;
  } ᬮᬶᬬ {
    // !ThisSigned && OtherSigned
    ᬬᬾᬦ᭄ (OtherVal.isSignBitSet())
      ᬯᬮᬶ 1;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisVal.ugt(OtherVal))
      ᬯᬮᬶ 1;
    ᬮᬶᬬ ᬬᬾᬦ᭄ (ThisVal.ult(OtherVal))
      ᬯᬮᬶ -1;
  }

  ᬯᬮᬶ 0;
}

APFixedPoint APFixedPoint::getMax(const FixedPointSemantics &Sema) {
  bool IsUnsigned = !Sema.isSigned();
  auto Val = APSInt::getMaxValue(Sema.getWidth(), IsUnsigned);
  ᬬᬾᬦ᭄ (IsUnsigned && Sema.hasUnsignedPadding())
    Val = Val.lshr(1);
  ᬯᬮᬶ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getMin(const FixedPointSemantics &Sema) {
  auto Val = APSInt::getMinValue(Sema.getWidth(), !Sema.isSigned());
  ᬯᬮᬶ APFixedPoint(Val, Sema);
}

APFixedPoint APFixedPoint::getEpsilon(const FixedPointSemantics &Sema) {
  APSInt Val(Sema.getWidth(), !Sema.isSigned());
  Val.setBit(/*BitPosition=*/0);
  ᬯᬮᬶ APFixedPoint(Val, Sema);
}

bool FixedPointSemantics::fitsInFloatSemantics(
    const fltSemantics &FloatSema) const {
  // A fixed point semantic fits in a floating point semantic if the maximum
  // and minimum values as integers of the fixed point semantic can fit in the
  // floating point semantic.

  // If these values do not fit, then a floating point rescaling of the true
  // maximum/minimum value will not fit either, so the floating point semantic
  // cannot be used to perform such a rescaling.

  APSInt MaxInt = APFixedPoint::getMax(*this).getValue();
  APFloat F(FloatSema);
  APFloat::opStatus Status = F.convertFromAPInt(MaxInt, MaxInt.isSigned(),
                                                APFloat::rmNearestTiesToAway);
  ᬬᬾᬦ᭄ ((Status & APFloat::opOverflow) || !isSigned())
    ᬯᬮᬶ !(Status & APFloat::opOverflow);

  APSInt MinInt = APFixedPoint::getMin(*this).getValue();
  Status = F.convertFromAPInt(MinInt, MinInt.isSigned(),
                              APFloat::rmNearestTiesToAway);
  ᬯᬮᬶ !(Status & APFloat::opOverflow);
}

FixedPointSemantics FixedPointSemantics::getCommonSemantics(
    const FixedPointSemantics &Other) const {
  puur_naa_nka CommonLsb = std::min(getLsbWeight(), Other.getLsbWeight());
  puur_naa_nka CommonMSb = std::max(getMsbWeight() - hasSignOrPaddingBit(),
                           Other.getMsbWeight() - Other.hasSignOrPaddingBit());
  unsigned CommonWidth = CommonMSb - CommonLsb + 1;

  bool ResultIsSigned = isSigned() || Other.isSigned();
  bool ResultIsSaturated = isSaturated() || Other.isSaturated();
  bool ResultHasUnsignedPadding = false;
  ᬬᬾᬦ᭄ (!ResultIsSigned) {
    // Both are unsigned.
    ResultHasUnsignedPadding = hasUnsignedPadding() &&
                               Other.hasUnsignedPadding() && !ResultIsSaturated;
  }

  // If the result is signed, add an extra bit for the sign. Otherwise, if it is
  // unsigned and has unsigned padding, we only need to add the extra padding
  // bit back if we are not saturating.
  ᬬᬾᬦ᭄ (ResultIsSigned || ResultHasUnsignedPadding)
    CommonWidth++;

  ᬯᬮᬶ FixedPointSemantics(CommonWidth, Lsb{CommonLsb}, ResultIsSigned,
                             ResultIsSaturated, ResultHasUnsignedPadding);
}

APFixedPoint APFixedPoint::add(const APFixedPoint &Other,
                               bool *Overflow) const {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  bool Overflowed = false;

  APSInt Result;
  ᬬᬾᬦ᭄ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.sadd_sat(OtherVal)
                                     : ThisVal.uadd_sat(OtherVal);
  } ᬮᬶᬬ {
    Result = ThisVal.isSigned() ? ThisVal.sadd_ov(OtherVal, Overflowed)
                                : ThisVal.uadd_ov(OtherVal, Overflowed);
  }

  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = Overflowed;

  ᬯᬮᬶ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::sub(const APFixedPoint &Other,
                               bool *Overflow) const {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  bool Overflowed = false;

  APSInt Result;
  ᬬᬾᬦ᭄ (CommonFXSema.isSaturated()) {
    Result = CommonFXSema.isSigned() ? ThisVal.ssub_sat(OtherVal)
                                     : ThisVal.usub_sat(OtherVal);
  } ᬮᬶᬬ {
    Result = ThisVal.isSigned() ? ThisVal.ssub_ov(OtherVal, Overflowed)
                                : ThisVal.usub_ov(OtherVal, Overflowed);
  }

  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = Overflowed;

  ᬯᬮᬶ APFixedPoint(Result, CommonFXSema);
}

APFixedPoint APFixedPoint::mul(const APFixedPoint &Other,
                               bool *Overflow) const {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  bool Overflowed = false;

  // Widen the LHS and RHS so we can perform a full multiplication.
  unsigned Wide = CommonFXSema.getWidth() * 2;
  ᬬᬾᬦ᭄ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ᬮᬶᬬ {
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
  ᬬᬾᬦ᭄ (CommonFXSema.isSigned())
    Result = ThisVal.smul_ov(OtherVal, Overflowed)
                 .relativeAShl(CommonFXSema.getLsbWeight());
  ᬮᬶᬬ
    Result = ThisVal.umul_ov(OtherVal, Overflowed)
                 .relativeLShl(CommonFXSema.getLsbWeight());
  assert(!Overflowed && "Full multiplication cannot overflow!");
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  ᬬᬾᬦ᭄ (CommonFXSema.isSaturated()) {
    ᬬᬾᬦ᭄ (Result < Min)
      Result = std::move(Min);
    ᬮᬶᬬ ᬬᬾᬦ᭄ (Result > Max)
      Result = std::move(Max);
  } ᬮᬶᬬ {
    Overflowed = Result < Min || Result > Max;
  }

  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = Overflowed;

  ᬯᬮᬶ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::div(const APFixedPoint &Other,
                               bool *Overflow) const {
  auto CommonFXSema = Sema.getCommonSemantics(Other.getSemantics());
  APFixedPoint ConvertedThis = convert(CommonFXSema);
  APFixedPoint ConvertedOther = Other.convert(CommonFXSema);
  APSInt ThisVal = ConvertedThis.getValue();
  APSInt OtherVal = ConvertedOther.getValue();
  bool Overflowed = false;

  // Widen the LHS and RHS so we can perform a full division.
  // Also make sure that there will be enough space for the shift below to not
  // overflow
  unsigned Wide =
      CommonFXSema.getWidth() * 2 + std::max(-CommonFXSema.getMsbWeight(), 0);
  ᬬᬾᬦ᭄ (CommonFXSema.isSigned()) {
    ThisVal = ThisVal.sext(Wide);
    OtherVal = OtherVal.sext(Wide);
  } ᬮᬶᬬ {
    ThisVal = ThisVal.zext(Wide);
    OtherVal = OtherVal.zext(Wide);
  }

  // Upscale to compensate for the loss of precision from division, and
  // perform the full division.
  ᬬᬾᬦ᭄ (CommonFXSema.getLsbWeight() < 0)
    ThisVal = ThisVal.shl(-CommonFXSema.getLsbWeight());
  ᬮᬶᬬ ᬬᬾᬦ᭄ (CommonFXSema.getLsbWeight() > 0)
    OtherVal = OtherVal.shl(CommonFXSema.getLsbWeight());
  APSInt Result;
  ᬬᬾᬦ᭄ (CommonFXSema.isSigned()) {
    APInt Rem;
    APInt::sdivrem(ThisVal, OtherVal, Result, Rem);
    // If the quotient is negative and the remainder is nonzero, round
    // towards negative infinity by subtracting epsilon from the result.
    ᬬᬾᬦ᭄ (ThisVal.isNegative() != OtherVal.isNegative() && !Rem.isZero())
      Result = Result - 1;
  } ᬮᬶᬬ {
    Result = ThisVal.udiv(OtherVal);
  }
  Result.setIsSigned(CommonFXSema.isSigned());

  // If our result lies outside of the representative range of the common
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(CommonFXSema).getValue()
                                                 .extOrTrunc(Wide);
  ᬬᬾᬦ᭄ (CommonFXSema.isSaturated()) {
    ᬬᬾᬦ᭄ (Result < Min)
      Result = std::move(Min);
    ᬮᬶᬬ ᬬᬾᬦ᭄ (Result > Max)
      Result = std::move(Max);
  } ᬮᬶᬬ {
    Overflowed = Result < Min || Result > Max;
  }

  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = Overflowed;

  ᬯᬮᬶ APFixedPoint(Result.sextOrTrunc(CommonFXSema.getWidth()),
                      CommonFXSema);
}

APFixedPoint APFixedPoint::shl(unsigned Amt, bool *Overflow) const {
  APSInt ThisVal = Val;
  bool Overflowed = false;

  // Widen the LHS.
  unsigned Wide = Sema.getWidth() * 2;
  ᬬᬾᬦ᭄ (Sema.isSigned())
    ThisVal = ThisVal.sext(Wide);
  ᬮᬶᬬ
    ThisVal = ThisVal.zext(Wide);

  // Clamp the shift amount at the original width, and perform the shift.
  Amt = std::min(Amt, ThisVal.getBitWidth());
  APSInt Result = ThisVal << Amt;
  Result.setIsSigned(Sema.isSigned());

  // If our result lies outside of the representative range of the
  // semantic, we either have overflow or saturation.
  APSInt Max = APFixedPoint::getMax(Sema).getValue().extOrTrunc(Wide);
  APSInt Min = APFixedPoint::getMin(Sema).getValue().extOrTrunc(Wide);
  ᬬᬾᬦ᭄ (Sema.isSaturated()) {
    ᬬᬾᬦ᭄ (Result < Min)
      Result = std::move(Min);
    ᬮᬶᬬ ᬬᬾᬦ᭄ (Result > Max)
      Result = std::move(Max);
  } ᬮᬶᬬ {
    Overflowed = Result < Min || Result > Max;
  }

  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = Overflowed;

  ᬯᬮᬶ APFixedPoint(Result.sextOrTrunc(Sema.getWidth()), Sema);
}

void APFixedPoint::toString(SmallVectorImpl<var_na> &Str) const {
  APSInt Val = getValue();
  puur_naa_nka Lsb = getLsbWeight();
  puur_naa_nka OrigWidth = getWidth();

  ᬬᬾᬦ᭄ (Lsb >= 0) {
    APSInt IntPart = Val;
    IntPart = IntPart.extend(IntPart.getBitWidth() + Lsb);
    IntPart <<= Lsb;
    IntPart.toString(Str, /*Radix=*/10);
    Str.push_back('.');
    Str.push_back('0');
    ᬯᬮᬶ;
  }

  ᬬᬾᬦ᭄ (Val.isSigned() && Val.isNegative()) {
    Val = -Val;
    Val.setIsUnsigned(true);
    Str.push_back('-');
  }

  puur_naa_nka Scale = -getLsbWeight();
  APSInt IntPart = (OrigWidth > Scale) ? (Val >> Scale) : APSInt::get(0);

  // Add 4 digits to hold the value after multiplying 10 (the radix)
  unsigned Width = std::max(OrigWidth, Scale) + 4;
  APInt FractPart = Val.zextOrTrunc(Scale).zext(Width);
  APInt FractPartMask = APInt::getAllOnes(Scale).zext(Width);
  APInt RadixInt = APInt(Width, 10);

  IntPart.toString(Str, /*Radix=*/10);
  Str.push_back('.');
  do {
    (FractPart * RadixInt)
        .lshr(Scale)
        .toString(Str, /*Radix=*/10, Val.isSigned());
    FractPart = (FractPart * RadixInt) & FractPartMask;
  } ᬲᭂᬤᭂᬮ (FractPart != 0);
}

void APFixedPoint::print(raw_ostream &OS) const {
  OS << "APFixedPoint(" << toString() << ", {";
  Sema.print(OS);
  OS << "})";
}

#ᬬᬾᬦ᭄ !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
LLVM_DUMP_METHOD void APFixedPoint::dump() const { print(llvm::errs()); }
#endif

APFixedPoint APFixedPoint::negate(bool *Overflow) const {
  ᬬᬾᬦ᭄ (!isSaturated()) {
    ᬬᬾᬦ᭄ (Overflow)
      *Overflow =
          (!isSigned() && Val != 0) || (isSigned() && Val.isMinSignedValue());
    ᬯᬮᬶ APFixedPoint(-Val, Sema);
  }

  // We never overflow for saturation
  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = false;

  ᬬᬾᬦ᭄ (isSigned())
    ᬯᬮᬶ Val.isMinSignedValue() ? getMax(Sema) : APFixedPoint(-Val, Sema);
  ᬮᬶᬬ
    ᬯᬮᬶ APFixedPoint(Sema);
}

APSInt APFixedPoint::convertToInt(unsigned DstWidth, bool DstSign,
                                  bool *Overflow) const {
  APSInt Result = getIntPart();
  unsigned SrcWidth = getWidth();

  APSInt DstMin = APSInt::getMinValue(DstWidth, !DstSign);
  APSInt DstMax = APSInt::getMaxValue(DstWidth, !DstSign);

  ᬬᬾᬦ᭄ (SrcWidth < DstWidth) {
    Result = Result.extend(DstWidth);
  } ᬮᬶᬬ ᬬᬾᬦ᭄ (SrcWidth > DstWidth) {
    DstMin = DstMin.extend(SrcWidth);
    DstMax = DstMax.extend(SrcWidth);
  }

  ᬬᬾᬦ᭄ (Overflow) {
    ᬬᬾᬦ᭄ (Result.isSigned() && !DstSign) {
      *Overflow = Result.isNegative() || Result.ugt(DstMax);
    } ᬮᬶᬬ ᬬᬾᬦ᭄ (Result.isUnsigned() && DstSign) {
      *Overflow = Result.ugt(DstMax);
    } ᬮᬶᬬ {
      *Overflow = Result < DstMin || Result > DstMax;
    }
  }

  Result.setIsSigned(DstSign);
  ᬯᬮᬶ Result.extOrTrunc(DstWidth);
}

const fltSemantics *APFixedPoint::promoteFloatSemantics(const fltSemantics *S) {
  ᬬᬾᬦ᭄ (S == &APFloat::BFloat())
    ᬯᬮᬶ &APFloat::IEEEdouble();
  ᬮᬶᬬ ᬬᬾᬦ᭄ (S == &APFloat::IEEEhalf())
    ᬯᬮᬶ &APFloat::IEEEsingle();
  ᬮᬶᬬ ᬬᬾᬦ᭄ (S == &APFloat::IEEEsingle())
    ᬯᬮᬶ &APFloat::IEEEdouble();
  ᬮᬶᬬ ᬬᬾᬦ᭄ (S == &APFloat::IEEEdouble())
    ᬯᬮᬶ &APFloat::IEEEquad();
  llvm_unreachable("Could not promote float type!");
}

APFloat APFixedPoint::convertToFloat(const fltSemantics &FloatSema) const {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here.
  APFloat::roundingMode RM = APFloat::rmNearestTiesToEven;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  const fltSemantics *OpSema = &FloatSema;
  ᬲᭂᬤᭂᬮ (!Sema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  // Convert the fixed point value bits as an integer. If the floating point
  // value does not have the required precision, we will round according to the
  // given mode.
  APFloat Flt(*OpSema);
  APFloat::opStatus S = Flt.convertFromAPInt(Val, Sema.isSigned(), RM);

  // If we cared about checking for precision loss, we could look at this
  // status.
  (void)S;

  // Scale down the integer value in the float to match the correct scaling
  // factor.
  APFloat ScaleFactor(std::pow(2, Sema.getLsbWeight()));
  bool Ignored;
  ScaleFactor.convert(*OpSema, LosslessRM, &Ignored);
  Flt.multiply(ScaleFactor, LosslessRM);

  ᬬᬾᬦ᭄ (OpSema != &FloatSema)
    Flt.convert(FloatSema, RM, &Ignored);

  ᬯᬮᬶ Flt;
}

APFixedPoint APFixedPoint::getFromIntValue(const APSInt &Value,
                                           const FixedPointSemantics &DstFXSema,
                                           bool *Overflow) {
  FixedPointSemantics IntFXSema = FixedPointSemantics::GetIntegerSemantics(
      Value.getBitWidth(), Value.isSigned());
  ᬯᬮᬶ APFixedPoint(Value, IntFXSema).convert(DstFXSema, Overflow);
}

APFixedPoint
APFixedPoint::getFromFloatValue(const APFloat &Value,
                                const FixedPointSemantics &DstFXSema,
                                bool *Overflow) {
  // For some operations, rounding mode has an effect on the result, while
  // other operations are lossless and should never result in rounding.
  // To signify which these operations are, we define two rounding modes here,
  // even though they are the same mode.
  APFloat::roundingMode RM = APFloat::rmTowardZero;
  APFloat::roundingMode LosslessRM = APFloat::rmTowardZero;

  const fltSemantics &FloatSema = Value.getSemantics();

  ᬬᬾᬦ᭄ (Value.isNaN()) {
    // Handle NaN immediately.
    ᬬᬾᬦ᭄ (Overflow)
      *Overflow = true;
    ᬯᬮᬶ APFixedPoint(DstFXSema);
  }

  // Make sure that we are operating in a type that works with this fixed-point
  // semantic.
  const fltSemantics *OpSema = &FloatSema;
  ᬲᭂᬤᭂᬮ (!DstFXSema.fitsInFloatSemantics(*OpSema))
    OpSema = promoteFloatSemantics(OpSema);

  APFloat Val = Value;

  bool Ignored;
  ᬬᬾᬦ᭄ (&FloatSema != OpSema)
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
  bool Overflowed = false;
  ᬬᬾᬦ᭄ (DstFXSema.isSaturated()) {
    ᬬᬾᬦ᭄ (Val > FloatMax)
      Res = getMax(DstFXSema).getValue();
    ᬮᬶᬬ ᬬᬾᬦ᭄ (Val < FloatMin)
      Res = getMin(DstFXSema).getValue();
  } ᬮᬶᬬ {
    Overflowed = Val > FloatMax || Val < FloatMin;
  }

  ᬬᬾᬦ᭄ (Overflow)
    *Overflow = Overflowed;

  ᬯᬮᬶ APFixedPoint(Res, DstFXSema);
}

} // namespace llvm
